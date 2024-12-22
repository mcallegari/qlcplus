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
#include "collectioneditor.h"
#include "functionmanager.h"
#include "rgbmatrixeditor.h"
#include "treemodelitem.h"
#include "chasereditor.h"
#include "scripteditor.h"
#include "sceneeditor.h"
#include "audioeditor.h"
#include "videoeditor.h"
#include "collection.h"
#include "efxeditor.h"
#include "treemodel.h"
#include "rgbmatrix.h"
#include "function.h"
#include "sequence.h"
#include "script.h"
#include "chaser.h"
#include "scene.h"
#include "audio.h"
#include "video.h"
#include "show.h"
#include "efx.h"
#include "app.h"

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

    m_currentEditor = nullptr;
    m_sceneEditor = nullptr;

    m_view->rootContext()->setContextProperty("functionManager", this);
    qmlRegisterUncreatableType<Collection>("org.qlcplus.classes", 1, 0, "Collection", "Can't create a Collection");
    qmlRegisterUncreatableType<Chaser>("org.qlcplus.classes", 1, 0, "Chaser", "Can't create a Chaser");
    qmlRegisterUncreatableType<RGBMatrix>("org.qlcplus.classes", 1, 0, "RGBMatrix", "Can't create a RGBMatrix");
    qmlRegisterUncreatableType<EFX>("org.qlcplus.classes", 1, 0, "EFX", "Can't create an EFX");

    // register SceneValue to perform QVariant comparisons
    qRegisterMetaType<SceneValue>();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMetaType::registerComparators<SceneValue>();
#endif
    m_functionTree = new TreeModel(this);
    QQmlEngine::setObjectOwnership(m_functionTree, QQmlEngine::CppOwnership);
    QStringList treeColumns;
    treeColumns << "classRef" << "type";
    m_functionTree->setColumnNames(treeColumns);
    m_functionTree->enableSorting(true);

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotFunctionAdded(quint32)));
}


FunctionManager::~FunctionManager()
{
    m_view->rootContext()->setContextProperty("functionManager", nullptr);
}

quint32 FunctionManager::startupFunctionID() const
{
    return m_doc->startupFunction();
}

void FunctionManager::setStartupFunctionID(quint32 fid)
{
    if (fid == m_doc->startupFunction())
        m_doc->setStartupFunction(Function::invalidId());
    else
        m_doc->setStartupFunction(fid);

    m_doc->setModified();

    emit startupFunctionIDChanged();
}

QVariant FunctionManager::functionsList()
{
    return QVariant::fromValue(m_functionTree);
}

quint32 FunctionManager::nextFunctionId() const
{
    return m_doc->nextFunctionID();
}

QVariantList FunctionManager::usageList(quint32 fid)
{
    QVariantList list;
    QList<quint32> funcUsageList = m_doc->getUsage(fid);

    for (int i = 0; i < funcUsageList.count(); i+=2)
    {
        Function *f = m_doc->function(funcUsageList.at(i));
        if (f == nullptr)
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

QStringList FunctionManager::selectedItemNames()
{
    QStringList names;

    for (QVariant &fID : m_selectedIDList)
    {
        Function *f = m_doc->function(fID.toInt());
        if (f == nullptr)
            continue;
        names.append(f->name());
    }
    for (QString &path : m_selectedFolderList)
    {
        QStringList tokens = path.split(TreeModel::separator());
        names.append(tokens.last());
    }

    return names;
}

void FunctionManager::setFunctionFilter(quint32 filter, bool enable)
{
    if (enable)
        m_filter |= filter;
    else
        m_filter &= ~filter;

    m_selectedFolderList.clear();
    m_selectedIDList.clear();

    updateFunctionsTree();
    emit selectedFunctionCountChanged(m_selectedIDList.count());
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

quint32 FunctionManager::addFunctiontoDoc(Function *func, QString name, bool select)
{
    if (func == nullptr)
        return Function::invalidId();

    func->setName(QString("%1 %2").arg(name).arg(m_doc->nextFunctionID()));

    if (m_doc->addFunction(func) == true)
    {
        if (select)
            m_functionTree->setItemRoleData(func->name(), 1, TreeModel::IsSelectedRole);

        QQmlEngine::setObjectOwnership(func, QQmlEngine::CppOwnership);

        if (select)
        {
            m_selectedIDList.append(QVariant(func->id()));
            emit selectedFunctionCountChanged(m_selectedIDList.count());
        }

        Tardis::instance()->enqueueAction(Tardis::FunctionCreate, func->id(), QVariant(),
                                          Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, func->id()));

        return func->id();
    }
    else
        delete func;

    return Function::invalidId();
}

quint32 FunctionManager::createFunction(int type, QVariantList fixturesList)
{
    Function* f = nullptr;
    QString name;

    switch(type)
    {
        case Function::SceneType:
        {
            f = new Scene(m_doc);
            name = tr("New Scene");
            if (fixturesList.count())
            {
                Scene *scene = qobject_cast<Scene *>(f);
                for (QVariant &fixtureID : fixturesList)
                    scene->addFixture(fixtureID.toUInt());
            }
            m_sceneCount++;
            emit sceneCountChanged();
        }
        break;
        case Function::ChaserType:
        {
            f = new Chaser(m_doc);
            name = tr("New Chaser");
            if (f != nullptr)
            {
                /* give the Chaser a meaningful common duration, to avoid
                 * that awful effect of playing steps with 0 duration */
                Chaser *chaser = qobject_cast<Chaser*>(f);
                chaser->setDuration(1000);

                for (QVariant &fId : m_selectedIDList)
                {
                    ChaserStep chs;
                    chs.fid = fId.toUInt();
                    chaser->addStep(chs);
                }
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
            if (fixturesList.count())
            {
                EFX *efx = qobject_cast<EFX *>(f);
                for (QVariant &fixtureID : fixturesList)
                {
                    Fixture *fixture = m_doc->fixture(fixtureID.toUInt());
                    if (fixture == nullptr)
                        continue;

                    for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
                        efx->addFixture(fixture->id(), headIdx);
                }
            }
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
                for (QVariant &fID : m_selectedIDList)
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
        default:
        break;
    }

    return addFunctiontoDoc(f, name, true);
}

quint32 FunctionManager::createAudioVideoFunction(int type, QStringList fileList)
{
    Function* f = nullptr;
    QString name;

    switch(type)
    {
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

QString FunctionManager::functionPath(quint32 id)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return "";

    return f->path(true);
}

void FunctionManager::clearTree()
{
    setPreviewEnabled(false);
    m_selectedIDList.clear();
    m_selectedFolderList.clear();
    m_functionTree->clear();
}

bool FunctionManager::previewEnabled() const
{
    return m_previewEnabled;
}

void FunctionManager::setPreviewEnabled(bool enable)
{
    if (m_currentEditor != nullptr)
    {
        m_currentEditor->setPreviewEnabled(enable);
    }
    else
    {
        for (QVariant &fID : m_selectedIDList)
        {
            Function *f = m_doc->function(fID.toUInt());
            if (f != nullptr)
            {
                if (enable == false)
                    f->stop(FunctionParent::master());
                else
                    f->start(m_doc->masterTimer(), FunctionParent::master());
            }
        }
    }

    m_previewEnabled = enable;
    emit previewEnabledChanged();
}

void FunctionManager::selectFunctionID(quint32 fID, bool multiSelection)
{
    if (fID == Function::invalidId())
        m_functionTree->setSingleSelection(nullptr);

    qDebug() << "Selected function:" << fID << multiSelection;

    if (multiSelection == false)
    {
        // stop selected Function(s) that are running
        for (QVariant &funcID : m_selectedIDList)
        {
            if (m_previewEnabled == true)
            {
                Function *f = m_doc->function(funcID.toUInt());
                if (f != nullptr)
                    f->stop(FunctionParent::master());
            }
        }
        m_selectedIDList.clear();
        m_selectedFolderList.clear();
        emit selectedFolderCountChanged(0);
    }

    // if preview is requested, start this Function here
    if (m_previewEnabled == true)
    {
        Function *f = m_doc->function(fID);
        if (f != nullptr)
            f->start(m_doc->masterTimer(), FunctionParent::master());
    }
    if (fID != Function::invalidId())
        m_selectedIDList.append(QVariant(fID));

    emit selectedFunctionCountChanged(m_selectedIDList.count());
}

QString FunctionManager::getEditorResource(int funcID)
{
    Function *f = m_doc->function((quint32)funcID);
    if (f == nullptr)
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
    if (m_currentEditor != nullptr)
    {
        if (m_currentEditor->functionID() == fID && !back)
            return;

        if (!back)
            previousID = m_currentEditor->functionID();

        delete m_currentEditor;
        m_currentEditor = nullptr;
    }
    if (m_sceneEditor != nullptr)
    {
        if (m_sceneEditor->functionID() == fID && !back)
            return;

        delete m_sceneEditor;
        m_sceneEditor = nullptr;
    }

    if ((int)fID == -1)
    {
        emit isEditingChanged(false);

        if (requestUI == true)
        {
            QQuickItem *rightPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("funcRightPanel"));
            if (rightPanel != nullptr)
                QMetaObject::invokeMethod(rightPanel, "requestEditor", Q_ARG(QVariant, -1), Q_ARG(QVariant, 0));
        }
        return;
    }

    Function *f = m_doc->function(fID);
    if (f == nullptr)
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
        case Function::ScriptType:
        {
             m_currentEditor = new ScriptEditor(m_view, m_doc, this);
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

    if (m_currentEditor != nullptr)
    {
        m_currentEditor->setFunctionID(fID);
        m_currentEditor->setPreviousID(previousID);
        m_currentEditor->setPreviewEnabled(m_previewEnabled);
    }

    if (requestUI == true)
    {
        QQuickItem *rightPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("funcRightPanel"));
        if (rightPanel != nullptr)
        {
            QMetaObject::invokeMethod(rightPanel, "requestEditor",
                Q_ARG(QVariant, f->id()), Q_ARG(QVariant, f->type()));
        }
    }

    emit isEditingChanged(true);
}

FunctionEditor *FunctionManager::currentEditor() const
{
    return m_currentEditor == nullptr ? m_sceneEditor : m_currentEditor;
}

bool FunctionManager::isEditing() const
{
    if (m_currentEditor != nullptr)
        return true;

    return false;
}

void FunctionManager::deleteFunction(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f == nullptr)
        return;

    if (f->isRunning())
        f->stopAndWait();

    Tardis::instance()->enqueueAction(Tardis::FunctionDelete, f->id(),
                                      Tardis::instance()->actionToByteArray(Tardis::FunctionDelete, f->id()),
                                      QVariant());

    QString fullPath = f->name();
    QString funcPath = f->path(true);
    if (funcPath.isEmpty() == false)
    {
        funcPath.replace("/", TreeModel::separator());
        fullPath = QString("%1%2%3").arg(funcPath).arg(TreeModel::separator()).arg(f->name());
    }
    m_doc->deleteFunction(f->id());
    m_functionTree->removeItem(fullPath);
}

void FunctionManager::deleteFunctions(QVariantList IDList)
{
    for (QVariant &fID : IDList)
    {
        Function *f = m_doc->function(fID.toInt());
        if (f == nullptr)
            continue;

        if (f->isRunning())
            f->stop(FunctionParent::master());

        if (m_selectedIDList.contains(fID))
            m_selectedIDList.removeAll(fID);

        deleteFunction(f->id());
    }

    emit selectedFunctionCountChanged(m_selectedIDList.count());
}

void FunctionManager::moveFunction(quint32 fID, QString newPath)
{
    Function *f = m_doc->function(fID);
    if (f == nullptr)
        return;

    QString newPathSlashed = newPath;
    newPathSlashed.replace(TreeModel::separator(), "/");

    QString fPath = f->path(true);
    if (fPath.isEmpty())
    {
        m_functionTree->removeItem(f->name());
    }
    else
    {
        QString ftPath = fPath;
        QString itemPath = QString("%1%2%3").arg(ftPath.replace("/", TreeModel::separator()))
                                            .arg(TreeModel::separator()).arg(f->name());
        m_functionTree->removeItem(itemPath);
    }

    Tardis::instance()->enqueueAction(Tardis::FunctionSetPath, f->id(), fPath, newPathSlashed);
    f->setPath(newPathSlashed);

    QVariantList params;
    params.append(QVariant::fromValue(f)); // classRef
    params.append(App::FunctionDragItem); // type
    m_functionTree->addItem(f->name(), params, newPath);
}

void FunctionManager::moveFunctions(QString newPath)
{
    bool wasEmptyNode = false;

    qDebug() << "Moving" << m_selectedIDList.count() << "functions to" << newPath;

    if (m_emptyFolderList.contains(newPath))
    {
        m_functionTree->removeItem(newPath);
        m_emptyFolderList.removeAll(newPath);
        wasEmptyNode = true;
    }

    for (QVariant &fID : m_selectedIDList)
        moveFunction(fID.toUInt(), newPath);

    if (wasEmptyNode)
    {
        QVariantList folderParams;
        folderParams.append(QVariant()); // classRef
        folderParams.append(App::FolderDragItem); // type
        m_functionTree->setPathData(newPath, folderParams);
    }

    if (m_selectedFolderList.count())
    {
        for (QString &path : m_selectedFolderList)
        {
            QStringList tokens = path.split(TreeModel::separator());
            QString newAbsPath;
            if (newPath.isEmpty())
                newAbsPath = tokens.last();
            else
                newAbsPath = newPath + TreeModel::separator() + tokens.last();
            setFolderPath(path, newAbsPath, false);
        }
    }

    //updateFunctionsTree();
}

void FunctionManager::cloneFunctions()
{
    for (QVariant &fidVar : m_selectedIDList)
    {
        Function *func = m_doc->function(fidVar.toUInt());
        if (func == nullptr)
            continue;

        Function* copy = func->createCopy(m_doc);
        if (copy != nullptr)
        {
            copy->setName(copy->name() + tr(" (Copy)"));

            /* If the cloned Function is a Sequence,
             * clone the bound Scene too */
            if (func->type() == Function::SequenceType)
            {
                Sequence *sequence = qobject_cast<Sequence *>(copy);
                quint32 sceneID = sequence->boundSceneID();
                Function *scene = m_doc->function(sceneID);
                if (scene != nullptr)
                {
                    Function *sceneCopy = scene->createCopy(m_doc);
                    if (sceneCopy != nullptr)
                        sequence->setBoundSceneID(sceneCopy->id());
                }
            }
        }
    }
}

void FunctionManager::deleteEditorItems(QVariantList list)
{
    if (m_currentEditor != nullptr)
        m_currentEditor->deleteItems(list);
}

void FunctionManager::deleteSequenceFixtures(QVariantList list)
{
    if (m_sceneEditor == nullptr)
        return;

    // First remove fixtures from the Sequence steps
    ChaserEditor *chaserEditor = qobject_cast<ChaserEditor*>(m_currentEditor);
    chaserEditor->removeFixtures(list);

    // Then delete the fixtures from the Scene
    m_sceneEditor->deleteItems(list);
}

bool FunctionManager::renameSelectedItems(QString newName, bool numbering, int startNumber, int digits)
{
    if (m_selectedIDList.isEmpty() && m_selectedFolderList.isEmpty())
        return false;

    int currNumber = startNumber;

    // rename folders first
    for (QString &path : m_selectedFolderList)
        setFolderPath(path, newName, true);

    for (QVariant &id : m_selectedIDList) // C++11
    {
        Function *f = m_doc->function(id.toUInt());
        if (f == nullptr)
            continue;

        QString fName = newName.simplified();

        if (numbering)
        {
            fName = QString("%1 %2").arg(fName).arg(currNumber, digits, 10, QChar('0'));
            currNumber++;
        }

        if (m_doc->functionByName(fName) != nullptr)
            return false;

        Tardis::instance()->enqueueAction(Tardis::FunctionSetName, f->id(), f->name(), fName);
        f->setName(fName);
    }

    return true;
}

int FunctionManager::selectedFunctionCount() const
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
 * Folders
 *********************************************************************/

void FunctionManager::selectFolder(QString path, bool multiSelection)
{
    qDebug() << "Selected folder:" << path << multiSelection;

    if (multiSelection == false && !path.isEmpty())
    {
        m_selectedFolderList.clear();
        m_selectedIDList.clear();
        emit selectedFunctionCountChanged(0);
    }

    if (path.isEmpty())
        return;

    m_selectedFolderList.append(path);
    emit selectedFolderCountChanged(m_selectedFolderList.count());
}

int FunctionManager::selectedFolderCount() const
{
    return m_selectedFolderList.count();
}

void FunctionManager::setFolderPath(QString oldAbsPath, QString newPath, bool isRelative)
{
    QStringList tokens = oldAbsPath.split(TreeModel::separator());
    QString newAbsPath;

    if (isRelative)
    {
        tokens.removeLast();
        tokens.append(newPath);
        newAbsPath = tokens.join(TreeModel::separator());

        // change the item label first
        m_functionTree->setItemRoleData(oldAbsPath, tokens.last(), TreeModel::LabelRole);
        // once label has changed, the item can now be accessed with the new path
        m_functionTree->setItemRoleData(newAbsPath, tokens.last(), TreeModel::PathRole);
    }
    else
    {
        newAbsPath = newPath;
    }

    tokens = newAbsPath.split(TreeModel::separator());

    qDebug() << "Folder path changed from" << oldAbsPath << "to" << newAbsPath;

    if (m_emptyFolderList.contains(oldAbsPath))
    {
        m_emptyFolderList.removeOne(oldAbsPath);
        m_emptyFolderList.append(newAbsPath);
    }
    else
    {
        QString oldAbsPathSlashed = oldAbsPath;
        QString newAbsPathSlashed = newAbsPath;
        oldAbsPathSlashed.replace(TreeModel::separator(), "/");
        newAbsPathSlashed.replace(TreeModel::separator(), "/");

        for (Function *f : m_doc->functions())
        {
            QString funcPath = f->path(true);
            if (funcPath.startsWith(oldAbsPathSlashed))
            {
                if (isRelative)
                {
                    Tardis::instance()->enqueueAction(Tardis::FunctionSetPath, f->id(), funcPath, newAbsPathSlashed);
                    f->setPath(newAbsPathSlashed);
                }
                else
                {
                    QString repPath = funcPath.replace(oldAbsPathSlashed, newAbsPathSlashed);
                    repPath.replace("/", TreeModel::separator());
                    moveFunction(f->id(), repPath);
                }
            }
        }
    }

    if (isRelative == false)
        m_functionTree->removeItem(oldAbsPath);

    //m_functionTree->printTree();
}

void FunctionManager::createFolder()
{
    QString fName;
    QString basePath;
    QString compPath;
    int index = 1;

    do
    {
        fName = QString("%1 %2").arg(tr("New folder")).arg(index);
        if (m_emptyFolderList.contains(fName) == false)
            break;
        index++;
    } while (1);

    // check if there is some selected folder
    if (m_selectedFolderList.count())
    {
        basePath = m_selectedFolderList.first();
    }
    else if (m_selectedIDList.count())
    {
        quint32 firstID = m_selectedIDList.first().toUInt();
        Function *firstFunc = m_doc->function(firstID);
        if (firstFunc)
            basePath = firstFunc->path(true).replace("/", TreeModel::separator());
    }

    if (basePath.isEmpty())
    {
        m_emptyFolderList.append(fName);
    }
    else
    {
        compPath = QString("%1%2%3").arg(basePath).arg(TreeModel::separator()).arg(fName);
        m_emptyFolderList.append(compPath);
    }

    QVariantList params;
    params.append(QVariant()); // classRef
    params.append(App::FolderDragItem); // type

    m_functionTree->addItem(fName, params, basePath, TreeModel::EmptyNode | TreeModel::Expanded);

    m_functionTree->printTree();
}

void FunctionManager::deleteSelectedFolders()
{
    for (QString &path : m_selectedFolderList)
    {
        if (m_emptyFolderList.contains(path))
        {
            m_emptyFolderList.removeAll(path);
        }
        else
        {
            for (Function *func : m_doc->functions())
            {
                if (func == nullptr)
                    continue;

                if (func->path(true).startsWith(path))
                    deleteFunction(func->id());
            }
        }

        m_functionTree->removeItem(path);
    }

    m_selectedFolderList.clear();
    emit selectedFolderCountChanged(0);
}

/*********************************************************************
 * DMX values (dumping and Scene editor)
 *********************************************************************/

void FunctionManager::dumpDmxValues(QList<SceneValue> dumpValues, QList<quint32> selectedFixtures,
                                    quint32 channelMask, QString sceneName, quint32 sceneID, bool nonZeroOnly)
{
    qDebug() << "[DUMP] # of values:" << dumpValues.count();
    qDebug() << "[DUMP] Selected fixture IDs:" << selectedFixtures;
    qDebug() << "[DUMP] Channel mask:" << channelMask;
    qDebug() << "[DUMP] Scene name/ID:" << sceneName << sceneID;
    qDebug() << "[DUMP] Only non-zero?" << nonZeroOnly;

    QList<Universe*> ua = m_doc->inputOutputMap()->claimUniverses();

    // 1- load current pre-GM values from all the universes
    QByteArray preGMValues(ua.size() * UNIVERSE_SIZE, 0);

    for (int i = 0; i < ua.count(); ++i)
    {
        const int offset = i * UNIVERSE_SIZE;
        preGMValues.replace(offset, UNIVERSE_SIZE, ua.at(i)->preGMValues());
        if (ua.at(i)->passthrough())
        {
            for (int j = 0; j < UNIVERSE_SIZE; ++j)
            {
                const int ofs = offset + j;
                preGMValues[ofs] =
                    static_cast<char>(ua.at(i)->applyPassthrough(j, static_cast<uchar>(preGMValues[ofs])));
            }
        }
    }

    m_doc->inputOutputMap()->releaseUniverses(false);

    // 2- determine if we're dumping on a new or existing Scene
    Scene *targetScene = nullptr;
    if (sceneID != Function::invalidId())
    {
        targetScene = qobject_cast<Scene*>(m_doc->function(sceneID));
    }
    else
    {
        targetScene = new Scene(m_doc);
        targetScene->setName(sceneName);
    }

    // 3- prepare the fixture list. If 'all channels' is required,
    // selectedFixtures list will be empty

    QList<Fixture *> fixtureList;
    bool allChannels = false;
    for (quint32 fixtureID : selectedFixtures)
    {
        Fixture *fixture = m_doc->fixture(fixtureID);
        if (fixture != nullptr)
            fixtureList.append(fixture);
    }

    if (fixtureList.isEmpty())
    {
        fixtureList.append(m_doc->fixtures());
        allChannels = true;
    }

    // 4- iterate over all channels of all gathered fixtures
    // and store values in the target Scene
    for (Fixture *fixture : fixtureList)
    {
        quint32 baseAddress = fixture->universeAddress();

        for (quint32 chIndex = 0; chIndex < fixture->channels(); chIndex++)
        {
            if (allChannels)
            {
                uchar value = preGMValues.at(baseAddress + chIndex);
                if (!nonZeroOnly || (nonZeroOnly && value > 0))
                {
                    SceneValue scv = SceneValue(fixture->id(), chIndex, value);
                    targetScene->setValue(scv);
                }
            }
            else
            {
                const QLCChannel *channel = fixture->channel(chIndex);
                quint32 chTypeBit = 0;

                if (channel->group() == QLCChannel::Intensity)
                {
                    if (channel->colour() == QLCChannel::NoColour)
                        chTypeBit |= App::DimmerType;
                    else
                        chTypeBit |= App::ColorType;
                }
                else
                {
                    chTypeBit |= (1 << channel->group());
                }

                if (channelMask & chTypeBit)
                {
                    uchar value = preGMValues.at(baseAddress + chIndex);
                    SceneValue scv = SceneValue(fixture->id(), chIndex, value);
                    int matchVal = dumpValues.indexOf(scv);
                    if (matchVal != -1)
                        scv.value = dumpValues.at(matchVal).value;

                    targetScene->setValue(scv);
                }
            }
        }
    }

    // 5- add Scene to the project, if needed
    if (sceneID == Function::invalidId())
    {
        if (sceneName.isEmpty())
            targetScene->setName(QString("%1 %2").arg(targetScene->name()).arg(m_doc->nextFunctionID() + 1));
        else
            targetScene->setName(sceneName);

        if (m_doc->addFunction(targetScene) == true)
        {
            setPreviewEnabled(false);
            Tardis::instance()->enqueueAction(Tardis::FunctionCreate, targetScene->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, targetScene->id()));
        }
        else
            delete targetScene;
    }
}

quint32 FunctionManager::getChannelTypeMask(quint32 fxID, quint32 channel)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return 0;

    const QLCChannel *ch = fixture->channel(channel);
    if (ch == nullptr)
        return 0;

    quint32 chTypeBit = 0;

    if (ch->group() == QLCChannel::Intensity)
    {
        if (ch->colour() == QLCChannel::NoColour)
            chTypeBit |= App::DimmerType;
        else
            chTypeBit |= App::ColorType;
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
    if (dumpValues.isEmpty() || channelMask == 0)
        return;

    Scene *newScene = new Scene(m_doc);

    for (SceneValue &sv : dumpValues)
    {
        if (selectedFixtures.count() && selectedFixtures.contains(sv.fxi) == false)
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
        setPreviewEnabled(false);
        Tardis::instance()->enqueueAction(Tardis::FunctionCreate, newScene->id(), QVariant(),
                                          Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, newScene->id()));
    }
    else
        delete newScene;
}

void FunctionManager::dumpOnScene(QList<SceneValue> dumpValues, QList<quint32> selectedFixtures,
                                  quint32 channelMask, quint32 sceneID)
{
    if (dumpValues.isEmpty() || channelMask == 0)
        return;

    Scene *scene = qobject_cast<Scene *>(m_doc->function(sceneID));

    if (scene == nullptr)
        return;

    for (SceneValue &sv : dumpValues)
    {
        if (selectedFixtures.count() && selectedFixtures.contains(sv.fxi) == false)
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
    SceneValue scv(fxID, channel, value);
    QVariant currentVal, newVal;

    if (editor == nullptr)
        return;

    if (editor->functionType() == Function::SequenceType)
    {
        ChaserEditor *cEditor = qobject_cast<ChaserEditor *>(editor);
        cEditor->setSequenceStepValue(scv);
        editor = m_sceneEditor;
    }

    if (editor->functionType() == Function::SceneType)
    {
        Scene *scene = qobject_cast<Scene *>(m_doc->function(editor->functionID()));
        if (scene == nullptr)
            return;

        newVal.setValue(scv);

        if (scene->checkValue(scv) == false)
        {
            Tardis::instance()->enqueueAction(Tardis::SceneSetChannelValue, scene->id(), QVariant(), newVal);
            scene->setValue(fxID, channel, value);
        }
        else
        {
            uchar currDmxValue = scene->value(fxID, channel);
            currentVal.setValue(SceneValue(fxID, channel, currDmxValue));

            if (currentVal != newVal || value != currDmxValue)
            {
                Tardis::instance()->enqueueAction(Tardis::SceneSetChannelValue, scene->id(), currentVal, newVal);
                if (scene->isRunning())
                    scene->setValue(scv, false, false);
                else
                    scene->setValue(fxID, channel, value);
            }
        }
    }
}

void FunctionManager::addFunctionTreeItem(Function *func)
{
    if (func == nullptr || func->isVisible() == false)
        return;

    bool expandAll = m_searchFilter.length() >= SEARCH_MIN_CHARS;

    QQmlEngine::setObjectOwnership(func, QQmlEngine::CppOwnership);

    if ((m_filter == 0 || m_filter & func->type()) &&
        (m_searchFilter.length() < SEARCH_MIN_CHARS || func->name().toLower().contains(m_searchFilter)))
    {
        QVariantList params;
        params.append(QVariant::fromValue(func)); // classRef
        params.append(App::FunctionDragItem); // type
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
    QStringList pathsList;

    m_sceneCount = m_chaserCount = m_sequenceCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    m_functionTree->clear();

    for (Function *func : m_doc->functions()) // C++11
    {
        QString fPath = func->path(true);
        if (pathsList.contains(fPath) == false)
            pathsList.append(fPath);
        addFunctionTreeItem(func);
    }

    QVariantList folderParams;
    folderParams.append(QVariant()); // classRef
    folderParams.append(App::FolderDragItem); // type

    for (QString &path : pathsList)
    {
        QString treePath = path.replace("/", TreeModel::separator());
        m_functionTree->setPathData(treePath, folderParams);
    }

    for (QString &folderPath : m_emptyFolderList)
    {
        QStringList tokens = folderPath.split(TreeModel::separator());
        QString fName = tokens.last();
        QString basePath;
        if (tokens.count() > 1)
        {
            tokens.takeLast();
            basePath = tokens.join(TreeModel::separator());
        }
        m_functionTree->addItem(fName, folderParams, basePath, TreeModel::EmptyNode | TreeModel::Expanded);
    }

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
    setPreviewEnabled(false);
    updateFunctionsTree();
}

void FunctionManager::slotFunctionAdded(quint32 fid)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    Function *func = m_doc->function(fid);
    addFunctionTreeItem(func);
}


