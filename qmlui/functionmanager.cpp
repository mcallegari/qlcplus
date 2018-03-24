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

#include "audioplugincache.h"
#include "genericdmxsource.h"
#include "collectioneditor.h"
#include "functionmanager.h"
#include "rgbmatrixeditor.h"
#include "contextmanager.h"
#include "treemodelitem.h"
#include "chasereditor.h"
#include "sceneeditor.h"
#include "audioeditor.h"
#include "videoeditor.h"
#include "collection.h"
#include "efxeditor.h"
#include "treemodel.h"
#include "rgbmatrix.h"
#include "function.h"
#include "sequence.h"
#include "chaser.h"
#include "script.h"
#include "scene.h"
#include "audio.h"
#include "video.h"
#include "show.h"
#include "efx.h"

#include "tardis.h"
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
    m_sceneCount = m_chaserCount = m_sequenceCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    m_currentEditor = NULL;
    m_sceneEditor = NULL;

    m_view->rootContext()->setContextProperty("functionManager", this);
    qmlRegisterUncreatableType<Collection>("org.qlcplus.classes", 1, 0, "Collection", "Can't create a Collection");
    qmlRegisterUncreatableType<Chaser>("org.qlcplus.classes", 1, 0, "Chaser", "Can't create a Chaser");
    qmlRegisterUncreatableType<RGBMatrix>("org.qlcplus.classes", 1, 0, "RGBMatrix", "Can't create a RGBMatrix");
    qmlRegisterUncreatableType<EFX>("org.qlcplus.classes", 1, 0, "EFX", "Can't create an EFX");

    // register SceneValue to perform QVariant comparisons
    qRegisterMetaType<SceneValue>();
    QMetaType::registerComparators<SceneValue>();

    m_functionTree = new TreeModel(this);
    QQmlEngine::setObjectOwnership(m_functionTree, QQmlEngine::CppOwnership);
    QStringList treeColumns;
    treeColumns << "classRef";
    m_functionTree->setColumnNames(treeColumns);
    m_functionTree->enableSorting(true);

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotFunctionAdded(quint32)));
}


FunctionManager::~FunctionManager()
{
    m_view->rootContext()->setContextProperty("functionManager", NULL);
}

QVariant FunctionManager::functionsList()
{
    return QVariant::fromValue(m_functionTree);
}

QVariantList FunctionManager::usageList(quint32 fid)
{
    QVariantList list;
    QList<quint32> funcUsageList = m_doc->getUsage(fid);

    for (int i = 0; i < funcUsageList.count(); i+=2)
    {
        Function *f = m_doc->function(funcUsageList.at(i));
        if (f == NULL)
            continue;

        QVariantMap funcMap;
        funcMap.insert("classRef", QVariant::fromValue(f));
        funcMap.insert("label", QString("%1 (@ %2)").arg(f->name()).arg(funcUsageList.at(i + 1)));
        list.append(funcMap);
    }

    if (list.isEmpty())
    {
        QVariantMap noneMap;
        noneMap.insert("label", tr("<None>"));
        list.append(noneMap);
    }

    return list;
}

QVariantList FunctionManager::selectedFunctionsID()
{
    return m_selectedIDList;
}

QStringList FunctionManager::selectedFunctionsName()
{
    QStringList names;

    for (QVariant fID : m_selectedIDList)
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

    int currLen = m_searchFilter.length();

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
            updateFunctionsTree();

    emit searchFilterChanged();
}

void FunctionManager::setFolderPath(QString oldAbsPath, QString newRelPath)
{
    int slashPos = oldAbsPath.lastIndexOf('/');
    QString newAbsPath;

    if (slashPos != -1)
        newAbsPath = oldAbsPath.mid(0, slashPos + 1) + newRelPath;
    else
        newAbsPath = newRelPath;

    qDebug() << "Folder path changed from" << oldAbsPath << "to" << newAbsPath;

    for (Function *f : m_doc->functions())
    {
        qDebug() << "Function path:" << f->path();
        if (f->path(true).startsWith(oldAbsPath))
        {
            Tardis::instance()->enqueueAction(Tardis::FunctionSetPath, f->id(), f->path(true), newAbsPath);
            f->setPath(newAbsPath);
        }
    }
    updateFunctionsTree();
}

quint32 FunctionManager::addFunctiontoDoc(Function *func, QString name, bool select)
{
    if (func == NULL)
        return Function::invalidId();

    if (m_doc->addFunction(func) == true)
    {
        if (select)
        {
            m_functionTree->setItemRoleData(QString("%1%2%3")
                                            .arg(func->path(true))
                                            .arg(TreeModel::separator())
                                            .arg(func->name()), 1, TreeModel::IsSelectedRole);
        }

        func->setName(QString("%1 %2").arg(name).arg(func->id()));
        QQmlEngine::setObjectOwnership(func, QQmlEngine::CppOwnership);

        if (select)
        {
            m_selectedIDList.append(QVariant(func->id()));
            emit selectionCountChanged(m_selectedIDList.count());
        }

        Tardis::instance()->enqueueAction(Tardis::FunctionCreate, func->id(), QVariant(),
                                          Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, func->id()));

        return func->id();
    }
    else
        delete func;

    return Function::invalidId();
}

quint32 FunctionManager::createFunction(int type, QStringList fileList)
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
        case Function::SequenceType:
        {
            /* a Sequence depends on a Scene, so let's create
             * a new hidden Scene first */
            Function *scene = new Scene(m_doc);
            scene->setVisible(false);

            if (m_doc->addFunction(scene) == true)
            {
                f = new Sequence(m_doc);
                name = tr("New Sequence");
                Sequence *sequence = qobject_cast<Sequence *>(f);
                sequence->setBoundSceneID(scene->id());
                m_sequenceCount++;
                emit sequenceCountChanged();
            }
            else
                delete scene;
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
            if (m_selectedIDList.count())
            {
                Collection *collection = qobject_cast<Collection *>(f);
                for (QVariant fID : m_selectedIDList)
                    collection->addFunction(fID.toUInt());
            }

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
            name = tr("New Audio");

            if (fileList.isEmpty())
            {
                f = new Audio(m_doc);
                m_audioCount++;
                emit audioCountChanged();
            }
            else
            {
                quint32 lastFuncID = Function::invalidId();
                for (QString filePath : fileList)
                {
                    if (filePath.startsWith("file:"))
                        filePath = QUrl(filePath).toLocalFile();

                    f = new Audio(m_doc);
                    lastFuncID = addFunctiontoDoc(f, name, fileList.count() == 1 ? true : false);
                    if (lastFuncID != Function::invalidId())
                    {
                        Audio *audio = qobject_cast<Audio *>(f);
                        audio->setSourceFileName(filePath);
                    }
                    m_audioCount++;
                }
                emit audioCountChanged();
                return lastFuncID;
            }
        }
        break;
        case Function::VideoType:
        {
            name = tr("New Video");

            if (fileList.isEmpty())
            {
                f = new Video(m_doc);
                m_videoCount++;
                emit videoCountChanged();
            }
            else
            {
                quint32 lastFuncID = Function::invalidId();
                for (QString filePath : fileList)
                {
                    if (filePath.startsWith("file:"))
                        filePath = QUrl(filePath).toLocalFile();

                    f = new Video(m_doc);
                    lastFuncID = addFunctiontoDoc(f, name, fileList.count() == 1 ? true : false);
                    if (lastFuncID != Function::invalidId())
                    {
                        Video *video = qobject_cast<Video *>(f);
                        video->setSourceUrl(filePath);
                    }
                    m_videoCount++;
                }
                emit videoCountChanged();
                return lastFuncID;
            }
        }
        break;
        default:
        break;
    }

    return addFunctiontoDoc(f, name, true);
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
        case Function::SequenceType: return "qrc:/sequence.svg";
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
        for (QVariant fID : m_selectedIDList)
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
        for (QVariant fID : m_selectedIDList)
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

QString FunctionManager::getEditorResource(int funcID)
{
    Function *f = m_doc->function((quint32)funcID);
    if (f == NULL)
        return "qrc:/FunctionManager.qml";

    switch(f->type())
    {
        case Function::SceneType: return "qrc:/SceneEditor.qml";
        case Function::ChaserType: return "qrc:/ChaserEditor.qml";
        case Function::SequenceType: return "qrc:/SequenceEditor.qml";
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

void FunctionManager::setEditorFunction(quint32 fID, bool requestUI, bool back)
{
    int previousID = -1;

    // reset all the editor functions
    if (m_currentEditor != NULL)
    {
        if (m_currentEditor->functionID() == fID)
            return;

        if (!back)
            previousID = m_currentEditor->functionID();

        delete m_currentEditor;
        m_currentEditor = NULL;
    }
    if (m_sceneEditor != NULL)
    {
        if (m_sceneEditor->functionID() == fID)
            return;

        delete m_sceneEditor;
        m_sceneEditor = NULL;
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
        case Function::SequenceType:
        {
            Sequence *sequence = qobject_cast<Sequence *>(f);
            m_sceneEditor = new SceneEditor(m_view, m_doc, this);
            m_sceneEditor->setFunctionID(sequence->boundSceneID());
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
        case Function::VideoType:
        {
            m_currentEditor = new VideoEditor(m_view, m_doc, this);
        }
        break;
        case Function::ShowType:
            // a Show is edited by the Show Manager
        break;
        default:
        {
            qDebug() << "Requested function type" << f->type() << "doesn't have a dedicated Function editor";
        }
        break;
    }

    if (m_currentEditor != NULL)
    {
        m_currentEditor->setFunctionID(fID);
        m_currentEditor->setPreviousID(previousID);
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

FunctionEditor *FunctionManager::currentEditor() const
{
    return m_currentEditor == NULL ? m_sceneEditor : m_currentEditor;
}

bool FunctionManager::isEditing() const
{
    if (m_currentEditor != NULL)
        return true;

    return false;
}

void FunctionManager::deleteFunctions(QVariantList IDList)
{
    for (QVariant fID : IDList)
    {
        Function *f = m_doc->function(fID.toInt());
        if (f == NULL)
            continue;

        if (f->isRunning())
            f->stop(FunctionParent::master());

        if (m_selectedIDList.contains(fID))
            m_selectedIDList.removeAll(fID);

        Tardis::instance()->enqueueAction(Tardis::FunctionDelete, f->id(),
                                          Tardis::instance()->actionToByteArray(Tardis::FunctionDelete, f->id()),
                                          QVariant());
        m_doc->deleteFunction(f->id());
    }

    m_selectedIDList.clear();
    emit selectionCountChanged(0);
    updateFunctionsTree();
}

void FunctionManager::moveFunctions(QString newPath)
{
    for (QVariant fID : m_selectedIDList)
    {
        Function *f = m_doc->function(fID.toInt());
        if (f == NULL)
            continue;

        Tardis::instance()->enqueueAction(Tardis::FunctionSetPath, f->id(), f->path(true), newPath);
        f->setPath(newPath);
    }

    updateFunctionsTree();
}

void FunctionManager::deleteEditorItems(QVariantList list)
{
    if (m_currentEditor != NULL)
        m_currentEditor->deleteItems(list);
}

void FunctionManager::renameFunctions(QVariantList IDList, QString newName, bool numbering, int startNumber, int digits)
{
    if (IDList.isEmpty())
        return;

    if (IDList.count() == 1)
    {
        // single Function rename
        Function *f = m_doc->function(IDList.first().toUInt());
        if (f != NULL)
        {
            Tardis::instance()->enqueueAction(Tardis::FunctionSetName, f->id(), f->name(), newName.simplified());
            f->setName(newName.simplified());
        }
    }
    else
    {
        int currNumber = startNumber;

        for(QVariant id : IDList) // C++11
        {
            Function *f = m_doc->function(id.toUInt());
            if (f == NULL)
                continue;

            if (numbering)
            {
                QString fName = QString("%1 %2").arg(newName.simplified()).arg(currNumber, digits, 10, QChar('0'));
                Tardis::instance()->enqueueAction(Tardis::FunctionSetName, f->id(), f->name(), fName);
                f->setName(fName);
                currNumber++;
            }
            else
            {
                Tardis::instance()->enqueueAction(Tardis::FunctionSetName, f->id(), f->name(), newName.simplified());
                f->setName(newName.simplified());
            }
        }
    }

    updateFunctionsTree();
}

int FunctionManager::selectionCount() const
{
    return m_selectedIDList.count();
}

QStringList FunctionManager::audioExtensions() const
{
    return m_doc->audioPluginCache()->getSupportedFormats();
}

QStringList FunctionManager::pictureExtensions() const
{
    return Video::getPictureCapabilities();
}

QStringList FunctionManager::videoExtensions() const
{
    return Video::getVideoCapabilities();
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

quint32 FunctionManager::getChannelTypeMask(quint32 fxID, quint32 channel)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return 0;

    const QLCChannel *ch = fixture->channel(channel);
    if (ch == NULL)
        return 0;

    quint32 chTypeBit = 0;

    if (ch->group() == QLCChannel::Intensity)
    {
        if (ch->colour() == QLCChannel::NoColour)
            chTypeBit |= ContextManager::DimmerType;
        else
            chTypeBit |= ContextManager::ColorType;
    }
    else
    {
        chTypeBit |= (1 << ch->group());
    }

    return chTypeBit;
}

void FunctionManager::dumpOnNewScene(QList<SceneValue> dumpValues, QList<quint32> selectedFixtures,
                                     quint32 channelMask, QString name)
{
    if (selectedFixtures.isEmpty() || dumpValues.isEmpty() || channelMask == 0)
        return;

    Scene *newScene = new Scene(m_doc);

    for (SceneValue sv : dumpValues)
    {
        if (selectedFixtures.contains(sv.fxi) == false)
            continue;

        quint32 chTypeBit = getChannelTypeMask(sv.fxi, sv.channel);

        if (channelMask & chTypeBit)
            newScene->setValue(sv);
    }

    if (name.isEmpty())
        newScene->setName(QString("%1 %2").arg(newScene->name()).arg(m_doc->nextFunctionID() + 1));
    else
        newScene->setName(name);

    if (m_doc->addFunction(newScene) == true)
    {
        setPreview(false);
        updateFunctionsTree();
        Tardis::instance()->enqueueAction(Tardis::FunctionCreate, newScene->id(), QVariant(),
                                          Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, newScene->id()));
    }
    else
        delete newScene;
}

void FunctionManager::dumpOnScene(QList<SceneValue> dumpValues, QList<quint32> selectedFixtures,
                                  quint32 channelMask, quint32 sceneID)
{
    if (selectedFixtures.isEmpty() || dumpValues.isEmpty() || channelMask == 0)
        return;

    Scene *scene = qobject_cast<Scene *>(m_doc->function(sceneID));

    if (scene == NULL)
        return;

    for (SceneValue sv : dumpValues)
    {
        if (selectedFixtures.contains(sv.fxi) == false)
            continue;

        quint32 chTypeBit = getChannelTypeMask(sv.fxi, sv.channel);

        if (channelMask & chTypeBit)
        {
            QVariant currentVal, newVal;
            uchar currDmxValue = scene->value(sv.fxi, sv.channel);
            currentVal.setValue(SceneValue(sv.fxi, sv.channel, currDmxValue));
            newVal.setValue(sv);
            if (currentVal != newVal || sv.value != currDmxValue)
            {
                Tardis::instance()->enqueueAction(Tardis::SceneSetChannelValue, scene->id(), currentVal, newVal);
                scene->setValue(sv);
            }
        }
    }
}

void FunctionManager::setChannelValue(quint32 fxID, quint32 channel, uchar value)
{
    FunctionEditor *editor = m_currentEditor;

    if (editor != NULL && editor->functionType() == Function::SequenceType)
        editor = m_sceneEditor;

    if (editor != NULL && editor->functionType() == Function::SceneType)
    {
        Scene *scene = qobject_cast<Scene *>(m_doc->function(editor->functionID()));
        if (scene == NULL)
            return;

        QVariant currentVal, newVal;
        uchar currDmxValue = scene->value(fxID, channel);
        currentVal.setValue(SceneValue(fxID, channel, currDmxValue));
        newVal.setValue(SceneValue(fxID, channel, value));
        if (currentVal != newVal || value != currDmxValue)
        {
            Tardis::instance()->enqueueAction(Tardis::SceneSetChannelValue, scene->id(), currentVal, newVal);
            scene->setValue(fxID, channel, value);
        }
    }
}

void FunctionManager::addFunctionTreeItem(Function *func)
{
    if (func == NULL || func->isVisible() == false)
        return;

    bool expandAll = m_searchFilter.length() >= SEARCH_MIN_CHARS;

    QQmlEngine::setObjectOwnership(func, QQmlEngine::CppOwnership);

    if ((m_filter == 0 || m_filter & func->type()) &&
        (m_searchFilter.length() < SEARCH_MIN_CHARS || func->name().toLower().contains(m_searchFilter)))
    {
        QVariantList params;
        params.append(QVariant::fromValue(func));
        QString fPath = func->path(true).replace("/", TreeModel::separator());
        TreeModelItem *item = m_functionTree->addItem(func->name(), params, fPath, expandAll ? TreeModel::Expanded : 0);
        if (m_selectedIDList.contains(QVariant(func->id())))
            item->setFlag(TreeModel::Selected, true);
    }

    switch (func->type())
    {
        case Function::SceneType: m_sceneCount++; break;
        case Function::ChaserType: m_chaserCount++; break;
        case Function::SequenceType: m_sequenceCount++; break;
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

void FunctionManager::updateFunctionsTree()
{
    m_sceneCount = m_chaserCount = m_sequenceCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    //m_selectedIDList.clear();
    m_functionTree->clear();

    for(Function *func : m_doc->functions()) // C++11
        addFunctionTreeItem(func);

    //m_functionTree->printTree(); // enable for debug purposes

    emit sceneCountChanged();
    emit chaserCountChanged();
    emit sequenceCountChanged();
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

void FunctionManager::slotFunctionAdded(quint32 fid)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    Function *func = m_doc->function(fid);
    addFunctionTreeItem(func);
}


