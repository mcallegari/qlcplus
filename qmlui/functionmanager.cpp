/*
  Q Light Controller Plus
  functionmanager.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QQmlContext>
#include <QQmlEngine>
#include <QDebug>

#include "collectioneditor.h"
#include "functionmanager.h"
#include "rgbmatrixeditor.h"
#include "treemodelitem.h"
#include "chasereditor.h"
#include "sceneeditor.h"
#include "audioeditor.h"
#include "collection.h"
#include "efxeditor.h"
#include "treemodel.h"
#include "rgbmatrix.h"
#include "function.h"
#include "chaser.h"
#include "script.h"
#include "scene.h"
#include "audio.h"
#include "video.h"
#include "show.h"
#include "efx.h"

#include "doc.h"

FunctionManager::FunctionManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_viewPosition(0)
    , m_previewEnabled(false)
    , m_filter(0)
    , m_searchFilter(QString())
{
    m_sceneCount = m_chaserCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    m_currentEditor = NULL;

    qmlRegisterUncreatableType<Collection>("com.qlcplus.classes", 1, 0, "Collection", "Can't create a Collection");
    qmlRegisterUncreatableType<Chaser>("com.qlcplus.classes", 1, 0, "Chaser", "Can't create a Chaser");
    qmlRegisterUncreatableType<RGBMatrix>("com.qlcplus.classes", 1, 0, "RGBMatrix", "Can't create a RGBMatrix");
    qmlRegisterUncreatableType<RGBMatrix>("com.qlcplus.classes", 1, 0, "EFX", "Can't create an EFX");

    m_functionTree = new TreeModel(this);
    QQmlEngine::setObjectOwnership(m_functionTree, QQmlEngine::CppOwnership);
    QStringList treeColumns;
    treeColumns << "classRef";
    m_functionTree->setColumnNames(treeColumns);
    m_functionTree->enableSorting(true);

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotDocLoaded()));
    connect(m_doc, SIGNAL(functionAdded(quint32)),
            this, SLOT(slotFunctionAdded()));
}

QVariant FunctionManager::functionsList()
{
    return QVariant::fromValue(m_functionTree);
}

QVariantList FunctionManager::selectedFunctionsID()
{
    return m_selectedIDList;
}

QStringList FunctionManager::selectedFunctionsName()
{
    QStringList names;

    foreach(QVariant fID, m_selectedIDList)
    {
        Function *f = m_doc->function(fID.toInt());
        if (f == NULL)
            continue;
        names.append(f->name());
    }

    return names;
}

void FunctionManager::setFunctionFilter(quint32 filter, bool enable)
{
    if (enable)
        m_filter |= filter;
    else
        m_filter &= ~filter;

    updateFunctionsTree();
    emit selectionCountChanged(m_selectedIDList.count());
}

int FunctionManager::functionsFilter() const
{
    return (int)m_filter;
}

QString FunctionManager::searchFilter() const
{
    return m_searchFilter;
}

void FunctionManager::setSearchFilter(QString searchFilter)
{
    if (m_searchFilter == searchFilter)
        return;

    int curreLen = m_searchFilter.length();

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS)
        updateFunctionsTree();
    else if(curreLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS)
        updateFunctionsTree();

    emit searchFilterChanged();
}

quint32 FunctionManager::createFunction(int type)
{
    Function* f = NULL;
    QString name;

    switch(type)
    {
    case Function::SceneType:
    {
        f = new Scene(m_doc);
        name = tr("New Scene");
            m_sceneCount++;
            emit sceneCountChanged();
        }
        break;
        case Function::ChaserType:
        {
            f = new Chaser(m_doc);
            name = tr("New Chaser");
            if (f != NULL)
            {
                /* give the Chaser a meaningful common duration, to avoid
                 * that awful effect of playing steps with 0 duration */
                Chaser *chaser = qobject_cast<Chaser*>(f);
                chaser->setDuration(1000);
            }
            m_chaserCount++;
            emit chaserCountChanged();
        }
        break;
        case Function::EFXType:
        {
            f = new EFX(m_doc);
            name = tr("New EFX");
            m_efxCount++;
            emit efxCountChanged();
        }
        break;
        case Function::CollectionType:
        {
            f = new Collection(m_doc);
            name = tr("New Collection");
            m_collectionCount++;
            emit collectionCountChanged();
        }
        break;
        case Function::RGBMatrixType:
        {
            f = new RGBMatrix(m_doc);
            name = tr("New RGB Matrix");
            m_rgbMatrixCount++;
            emit rgbMatrixCountChanged();
        }
        break;
        case Function::ScriptType:
        {
            f = new Script(m_doc);
            name = tr("New Script");
            m_scriptCount++;
            emit scriptCountChanged();
        }
        break;
        case Function::ShowType:
        {
            f = new Show(m_doc);
            name = tr("New Show");
            m_showCount++;
            emit showCountChanged();
        }
        break;
        case Function::AudioType:
        {
            f = new Audio(m_doc);
            name = tr("New Audio");
            m_audioCount++;
            emit audioCountChanged();
        }
        break;
        case Function::VideoType:
        {
            f = new Video(m_doc);
            name = tr("New Video");
            m_videoCount++;
            emit videoCountChanged();
        }
        break;
        default:
        break;
    }
    if (f == NULL)
        return Function::invalidId();

    if (m_doc->addFunction(f) == true)
    {
        f->setName(QString("%1 %2").arg(name).arg(f->id()));
        QQmlEngine::setObjectOwnership(f, QQmlEngine::CppOwnership);

        QVariantList params;
        params.append(QVariant::fromValue(f));
        TreeModelItem *item = m_functionTree->addItem(f->name(), params, f->path(true));
        if (item != NULL)
            item->setFlag(TreeModel::Selected, true);
        m_selectedIDList.append(QVariant(f->id()));
        emit selectionCountChanged(m_selectedIDList.count());
        emit functionsListChanged();

        return f->id();
    }
    else
        delete f;

    return Function::invalidId();
}

Function *FunctionManager::getFunction(quint32 id)
{
    return m_doc->function(id);
}

QString FunctionManager::functionIcon(int type)
{
    switch (type)
    {
        case Function::SceneType: return "qrc:/scene.svg";
        case Function::ChaserType: return "qrc:/chaser.svg";
        case Function::EFXType: return "qrc:/efx.svg";
        case Function::CollectionType: return "qrc:/collection.svg";
        case Function::ScriptType: return "qrc:/script.svg";
        case Function::RGBMatrixType: return "qrc:/rgbmatrix.svg";
        case Function::ShowType: return "qrc:/showmanager.svg";
        case Function::AudioType: return "qrc:/audio.svg";
        case Function::VideoType: return "qrc:/video.svg";
    }

    return "";
}

void FunctionManager::clearTree()
{
    setPreview(false);
    m_selectedIDList.clear();
    m_functionTree->clear();
}

void FunctionManager::setPreview(bool enable)
{
    if (m_currentEditor != NULL)
    {
        m_currentEditor->setPreviewEnabled(enable);
    }
    else
    {
        foreach(QVariant fID, m_selectedIDList)
        {
            Function *f = m_doc->function(fID.toUInt());
            if (f != NULL)
            {
                if (enable == false)
                    f->stop(FunctionParent::master());
                else
                {
                    f->start(m_doc->masterTimer(), FunctionParent::master());
                }
            }
        }
    }

    m_previewEnabled = enable;
}

void FunctionManager::selectFunctionID(quint32 fID, bool multiSelection)
{
    if (multiSelection == false)
    {
        foreach(QVariant fID, m_selectedIDList)
        {
            if (m_previewEnabled == true)
            {
                Function *f = m_doc->function(fID.toUInt());
                if (f != NULL)
                    f->stop(FunctionParent::master());
            }
        }
        m_selectedIDList.clear();
    }

    if (m_previewEnabled == true)
    {
        Function *f = m_doc->function(fID);
        if (f != NULL)
            f->start(m_doc->masterTimer(), FunctionParent::master());
    }
    if (fID != Function::invalidId())
        m_selectedIDList.append(QVariant(fID));

    emit selectionCountChanged(m_selectedIDList.count());
}

QString FunctionManager::getEditorResource(int type)
{
    switch(type)
    {
        case Function::SceneType: return "qrc:/SceneEditor.qml";
        case Function::ChaserType: return "qrc:/ChaserEditor.qml";
        case Function::EFXType: return "qrc:/EFXEditor.qml";
        case Function::CollectionType: return "qrc:/CollectionEditor.qml";
        case Function::RGBMatrixType: return "qrc:/RGBMatrixEditor.qml";
        case Function::ShowType: return "qrc:/ShowManager.qml";
        case Function::ScriptType: return "qrc:/ScriptEditor.qml";
        case Function::AudioType: return "qrc:/AudioEditor.qml";
        case Function::VideoType: return "qrc:/VideoEditor.qml";
        default: return ""; break;
    }
}

void FunctionManager::setEditorFunction(quint32 fID, bool requestUI)
{
    // reset all the editor functions
    if (m_currentEditor != NULL)
    {
        delete m_currentEditor;
        m_currentEditor = NULL;
    }

    if ((int)fID == -1)
    {
        emit isEditingChanged(false);
        return;
    }

    Function *f = m_doc->function(fID);
    if (f == NULL)
        return;

    switch(f->type())
    {
        case Function::SceneType:
        {
            m_currentEditor = new SceneEditor(m_view, m_doc, this);
        }
        break;
        case Function::ChaserType:
        {
            m_currentEditor = new ChaserEditor(m_view, m_doc, this);
        }
        break;
        case Function::EFXType:
        {
            m_currentEditor = new EFXEditor(m_view, m_doc, this);
        }
        break;
        case Function::CollectionType:
        {
            m_currentEditor = new CollectionEditor(m_view, m_doc, this);
        }
        break;
        case Function::RGBMatrixType:
        {
            m_currentEditor = new RGBMatrixEditor(m_view, m_doc, this);
        }
        break;
        case Function::AudioType:
        {
            m_currentEditor = new AudioEditor(m_view, m_doc, this);
        }
        break;
        case Function::ShowType: break; // a Show is edited by the Show Manager
        default:
        {
            qDebug() << "Requested function type" << f->type() << "doesn't have a dedicated Function editor";
        }
        break;
    }

    if (m_currentEditor != NULL)
    {
        m_currentEditor->setFunctionID(fID);
        m_currentEditor->setPreviewEnabled(m_previewEnabled);
    }

    if (requestUI == true)
    {
        QQuickItem *rightPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("funcRightPanel"));
        if (rightPanel != NULL)
        {
            QMetaObject::invokeMethod(rightPanel, "requestEditor",
                Q_ARG(QVariant, f->id()), Q_ARG(QVariant, f->type()));
        }
    }

    emit isEditingChanged(true);
}

bool FunctionManager::isEditing() const
{
    if (m_currentEditor != NULL)
        return true;

    return false;
}

void FunctionManager::deleteFunctions(QVariantList IDList)
{
    foreach(QVariant fID, IDList)
    {
        Function *f = m_doc->function(fID.toInt());
        if (f == NULL)
            continue;

        if (f->isRunning())
            f->stop(FunctionParent::master());

        if (m_selectedIDList.contains(fID))
            m_selectedIDList.removeAll(fID);

        m_doc->deleteFunction(f->id());
    }

    m_selectedIDList.clear();
    emit selectionCountChanged(0);
    updateFunctionsTree();
}

void FunctionManager::deleteEditorItems(QVariantList list)
{
    if (m_currentEditor != NULL)
        m_currentEditor->deleteItems(list);
}

void FunctionManager::renameFunctions(QVariantList IDList, QString newName, int startNumber, int digits)
{
    if (IDList.isEmpty())
        return;

    if (IDList.count() == 1)
    {
        // single Function rename
        Function *f = m_doc->function(IDList.first().toUInt());
        if (f != NULL)
            f->setName(newName.simplified());
    }
    else
    {
        int currNumber = startNumber;

        for(QVariant id : IDList) // C++11
        {
            Function *f = m_doc->function(id.toUInt());
            if (f == NULL)
                continue;

            QString fName = QString("%1 %2").arg(newName.simplified()).arg(currNumber, digits, 10, QChar('0'));
            f->setName(fName);
            currNumber++;
        }
    }

    updateFunctionsTree();
}

int FunctionManager::selectionCount() const
{
    return m_selectedIDList.count();
}

void FunctionManager::setViewPosition(int viewPosition)
{
    if (m_viewPosition == viewPosition)
        return;

    m_viewPosition = viewPosition;
    emit viewPositionChanged(viewPosition);
}

int FunctionManager::viewPosition() const
{
    return m_viewPosition;
}

/*********************************************************************
 * DMX values (dumping and Scene editor)
 *********************************************************************/

void FunctionManager::setDumpValue(quint32 fxID, quint32 channel, uchar value)
{
    m_dumpValues[QPair<quint32,quint32>(fxID, channel)] = value;
    emit dumpValuesCountChanged();
}

QMap<QPair<quint32, quint32>, uchar> FunctionManager::dumpValues() const
{
    return m_dumpValues;
}

int FunctionManager::dumpValuesCount() const
{
    return m_dumpValues.count();
}

void FunctionManager::resetDumpValues()
{    
    m_dumpValues.clear();
    emit dumpValuesCountChanged();
}

void FunctionManager::dumpOnNewScene(QList<quint32> selectedFixtures)
{
    if (selectedFixtures.isEmpty() || m_dumpValues.isEmpty())
        return;

    Scene *newScene = new Scene(m_doc);

    QMutableMapIterator <QPair<quint32,quint32>,uchar> it(m_dumpValues);
    while (it.hasNext() == true)
    {
        it.next();
        SceneValue sv;
        sv.fxi = it.key().first;
        sv.channel = it.key().second;
        sv.value = it.value();
        if (selectedFixtures.contains(sv.fxi))
            newScene->setValue(sv);
    }

    newScene->setName(QString("%1 %2").arg(newScene->name()).arg(m_doc->nextFunctionID() + 1));

    if (m_doc->addFunction(newScene) == true)
    {
        slotDocLoaded();
    }
    else
        delete newScene;
}

void FunctionManager::setChannelValue(quint32 fxID, quint32 channel, uchar value)
{
    if (m_currentEditor != NULL && m_currentEditor->functionType() == Function::SceneType)
    {
        SceneEditor *se = qobject_cast<SceneEditor *>(m_currentEditor);
        se->setChannelValue(fxID, channel, value);
    }
}

void FunctionManager::updateFunctionsTree()
{
    bool expandAll = m_searchFilter.length() >= SEARCH_MIN_CHARS;

    m_sceneCount = m_chaserCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    //m_selectedIDList.clear();
    m_functionTree->clear();

    for(Function *func : m_doc->functions()) // C++11
    {
        QQmlEngine::setObjectOwnership(func, QQmlEngine::CppOwnership);

        if ((m_filter == 0 || m_filter & func->type()) &&
            (m_searchFilter.length() < SEARCH_MIN_CHARS || func->name().toLower().contains(m_searchFilter)))
        {
            QVariantList params;
            params.append(QVariant::fromValue(func));
            TreeModelItem *item = m_functionTree->addItem(func->name(), params, func->path(true), expandAll ? TreeModel::Expanded : 0);
            if (m_selectedIDList.contains(QVariant(func->id())))
                item->setFlag(TreeModel::Selected, true);
        }

        switch (func->type())
        {
            case Function::SceneType: m_sceneCount++; break;
            case Function::ChaserType: m_chaserCount++; break;
            case Function::EFXType: m_efxCount++; break;
            case Function::CollectionType: m_collectionCount++; break;
            case Function::RGBMatrixType: m_rgbMatrixCount++; break;
            case Function::ScriptType: m_scriptCount++; break;
            case Function::ShowType: m_showCount++; break;
            case Function::AudioType: m_audioCount++; break;
            case Function::VideoType: m_videoCount++; break;
            default:
            break;
        }
    }
    //m_functionTree->printTree(); // enable for debug purposes

    emit sceneCountChanged();
    emit chaserCountChanged();
    emit efxCountChanged();
    emit collectionCountChanged();
    emit rgbMatrixCountChanged();
    emit scriptCountChanged();
    emit showCountChanged();
    emit audioCountChanged();
    emit videoCountChanged();

    emit functionsListChanged();
}

void FunctionManager::slotDocLoaded()
{
    setPreview(false);
    updateFunctionsTree();
}

void FunctionManager::slotFunctionAdded()
{
    if (m_doc->loadStatus() != Doc::Loaded)
        return;
    updateFunctionsTree();
}


