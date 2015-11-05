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

#include "functionmanager.h"
#include "sceneeditor.h"
#include "collection.h"
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
    , m_previewEnabled(false)
{
    m_filter = 0;
    m_sceneCount = m_chaserCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    m_sceneEditor = new SceneEditor(m_view, m_doc, this);

    qmlRegisterType<Collection>("com.qlcplus.classes", 1, 0, "Collection");

    m_view->rootContext()->setContextProperty("sceneEditor", m_sceneEditor);

    m_functionTree = new TreeModel(this);
    QQmlEngine::setObjectOwnership(m_functionTree, QQmlEngine::CppOwnership);
    QStringList treeColumns;
    treeColumns << "classRef";
    m_functionTree->setColumnNames(treeColumns);
    m_functionTree->enableSorting(true);
/*
    for (int i = 0; i < 10; i++)
    {
        QStringList vars;
        vars << QString::number(i) << 0;
        m_functionTree->addItem(QString("Entry %1").arg(i), vars);
    }
*/

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotDocLoaded()));
}

QVariant FunctionManager::functionsList()
{
    slotDocLoaded();
    return QVariant::fromValue(m_functionTree);
}

void FunctionManager::setFunctionFilter(quint32 filter, bool enable)
{
    if (enable)
        m_filter |= filter;
    else
        m_filter &= ~filter;
    slotDocLoaded();
}

void FunctionManager::selectFunction(quint32 id, QQuickItem *item, bool multiSelection)
{
    if (multiSelection == false)
    {
        foreach(selectedFunction sf, m_selectedFunctions)
        {
            if (m_previewEnabled && sf.m_fID == m_sceneEditor->sceneID())
                m_sceneEditor->setPreview(false);
            sf.m_item->setProperty("isSelected", false);
            Function *f = m_doc->function(sf.m_fID);
            if (f != NULL && f->isRunning())
                f->stop();
        }

        m_selectedFunctions.clear();
    }

    selectedFunction sf;
    sf.m_fID = id;
    sf.m_item = item;
    item->setProperty("isSelected", true);
    m_selectedFunctions.append(sf);

    if (m_previewEnabled == true)
    {
        Function *f = m_doc->function(sf.m_fID);
        if (f != NULL)
            f->start(m_doc->masterTimer());
    }

    QQuickItem *previewBtn = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("previewButton"));
    if (previewBtn != NULL)
    {
        if ((int)id != -1)
            previewBtn->setProperty("visible", true);
        else
            previewBtn->setProperty("visible", false);
    }
}

quint32 FunctionManager::createFunction(int type)
{
    Function* f = NULL;
    QString name;

    switch(type)
    {
        case Function::Scene:
        {
            f = new Scene(m_doc);
            name = tr("New Scene");
        }
        break;
        case Function::Chaser:
        {
            f = new Chaser(m_doc);
            name = tr("New Chaser");
        }
        break;
        case Function::EFX:
        {
            f = new EFX(m_doc);
            name = tr("New EFX");
        }
        break;
        case Function::Collection:
        {
            f = new Collection(m_doc);
            name = tr("New Collection");
        }
        break;
        case Function::RGBMatrix:
        {
            f = new RGBMatrix(m_doc);
            name = tr("New RGB Matrix");
        }
        break;
        case Function::Script:
        {
            f = new Script(m_doc);
            name = tr("New Script");
        }
        break;
        case Function::Show:
        {
            f = new Show(m_doc);
            name = tr("New Show");
        }
        break;
        case Function::Audio:
        {
            f = new Audio(m_doc);
            name = tr("New Audio");
        }
        break;
        case Function::Video:
        {
            f = new Video(m_doc);
            name = tr("New Video");
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

void FunctionManager::clearTree()
{
    m_selectedFunctions.clear();
    m_functionTree->clear();
}

void FunctionManager::setPreview(bool enable)
{
    foreach(selectedFunction sf, m_selectedFunctions)
    {
        Function *f = m_doc->function(sf.m_fID);
        if (f != NULL)
        {
            if (enable == false)
                f->stop();
            else
            {
                f->start(m_doc->masterTimer());
            }
        }
    }

    if (m_selectedFunctions.isEmpty())
    {
        if (m_sceneEditor->sceneID() != Function::invalidId())
            m_sceneEditor->setPreview(enable);
    }

    m_previewEnabled = enable;
}

void FunctionManager::setEditorFunction(quint32 fID)
{
    if ((int)fID == -1)
    {
        // reset all the editor functions
        m_sceneEditor->setSceneID(Function::invalidId());
        emit functionEditingChanged(false);
        return;
    }

    Function *f = m_doc->function(fID);
    if (f == NULL)
        return;

    switch(f->type())
    {
        case Function::Scene:
        {
            m_sceneEditor->setSceneID(fID);
            m_sceneEditor->setPreview(m_previewEnabled);
        }
        break;
        default:
        {
            if (m_previewEnabled)
                f->start(m_doc->masterTimer());
        }
        break;
    }

    emit functionEditingChanged(true);
}

/*********************************************************************
 * DMX values (dumping and Scene editor)
 *********************************************************************/

void FunctionManager::setDumpValue(quint32 fxID, quint32 channel, uchar value)
{
    m_dumpValues[QPair<quint32,quint32>(fxID, channel)] = value;
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
    if (m_sceneEditor->sceneID() != Function::invalidId())
        m_sceneEditor->setChannelValue(fxID, channel, value);
}

void FunctionManager::slotDocLoaded()
{
    m_sceneCount = m_chaserCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    m_selectedFunctions.clear();
    m_functionTree->clear();
    foreach(Function *func, m_doc->functions())
    {
        QQmlEngine::setObjectOwnership(func, QQmlEngine::CppOwnership);
        if (m_filter == 0 || m_filter & func->type())
        {
            QVariantList params;
            params.append(QVariant::fromValue(func));
            m_functionTree->addItem(func->name(), params, func->path(true));
        }
        switch (func->type())
        {
            case Function::Scene: m_sceneCount++; break;
            case Function::Chaser: m_chaserCount++; break;
            case Function::EFX: m_efxCount++; break;
            case Function::Collection: m_collectionCount++; break;
            case Function::RGBMatrix: m_rgbMatrixCount++; break;
            case Function::Script: m_scriptCount++; break;
            case Function::Show: m_showCount++; break;
            case Function::Audio: m_audioCount++; break;
            case Function::Video: m_videoCount++; break;
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


