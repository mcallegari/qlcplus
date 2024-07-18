/*
  Q Light Controller Plus
  importmanager.cpp

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

#include <QXmlStreamReader>
#include <QQmlContext>

#include "importmanager.h"
#include "treemodelitem.h"
#include "fixturemanager.h"

#include "qlcfixturedefcache.h"
#include "monitorproperties.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixtureutils.h"
#include "qlcpalette.h"
#include "collection.h"
#include "rgbmatrix.h"
#include "sequence.h"
#include "qlcfile.h"
#include "chaser.h"
#include "script.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"
#include "app.h"

ImportManager::ImportManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fixtureTree(nullptr)
    , m_fixtureTreeUpdating(false)
    , m_functionTree(nullptr)
    , m_functionTreeUpdating(false)
{
    m_importDoc = new Doc(this);

    // delete the default cache to use the one in m_doc
    delete m_importDoc->fixtureDefCache();

    // share the original Doc fixture cache
    m_importDoc->setFixtureDefinitionCache(m_doc->fixtureDefCache());

    /* Load channel modifiers templates */
    //m_importDoc->modifiersCache()->load(QLCModifiersCache::systemTemplateDirectory(), true);
    //m_importDoc->modifiersCache()->load(QLCModifiersCache::userTemplateDirectory());

    /* Load RGB scripts */
    //m_importDoc->rgbScriptsCache()->load(RGBScriptsCache::systemScriptsDirectory());
    //m_importDoc->rgbScriptsCache()->load(RGBScriptsCache::userScriptsDirectory());

    m_view->rootContext()->setContextProperty("importManager", this);
}

ImportManager::~ImportManager()
{
    /* invalidate the fixture cache reference since
     * this is shared with the original Doc */
    m_importDoc->setFixtureDefinitionCache(nullptr);
    delete m_importDoc;

    m_functionTree->clear();
    m_fixtureTree->clear();

    delete m_functionTree;
    delete m_fixtureTree;
}

bool ImportManager::loadWorkspace(const QString &fileName)
{
    QString localFilename =  fileName;
    if (localFilename.startsWith("file:"))
        localFilename = QUrl(fileName).toLocalFile();

    bool retval = false;

    if (localFilename.isEmpty() == true)
        return false;

    QXmlStreamReader *doc = QLCFile::getXMLReader(localFilename);
    if (doc == nullptr || doc->device() == nullptr || doc->hasError())
    {
        qWarning() << Q_FUNC_INFO << "Unable to read from" << localFilename;
        return false;
    }

    while (!doc->atEnd())
    {
        if (doc->readNext() == QXmlStreamReader::DTD)
            break;
    }
    if (doc->hasError())
    {
        QLCFile::releaseXMLReader(doc);
        return false;
    }

    m_importDoc->clearContents();

    /* Set the workspace path before loading the new XML. In this way local files
       can be loaded even if the workspace file has been moved */
    m_importDoc->setWorkspacePath(QFileInfo(localFilename).absolutePath());

    if (doc->dtdName() == KXMLQLCWorkspace)
    {
        retval = loadXML(*doc);
    }
    else
    {
        qWarning() << Q_FUNC_INFO << localFilename << "is not a workspace file";
    }

    QLCFile::releaseXMLReader(doc);

    return retval;
}

void ImportManager::apply()
{
    // try to preserve the Fixture order
    std::sort(m_fixtureIDList.begin(), m_fixtureIDList.end());
    importFixtures();

    importPalettes();

    /* Functions need to be imported respecting their
     * dependency order. Otherwise ID remapping will be
     * messed up */
    while (!m_functionIDList.isEmpty())
    {
        importFunctionID(m_functionIDList.first());
    }
}

bool ImportManager::loadXML(QXmlStreamReader &doc)
{
    if (doc.readNextStartElement() == false)
        return false;

    if (doc.name() != KXMLQLCWorkspace)
    {
        qWarning() << Q_FUNC_INFO << "Workspace node not found";
        return false;
    }

    while (doc.readNextStartElement())
    {
        if (doc.name() == KXMLQLCEngine)
        {
            m_importDoc->loadXML(doc, false);
        }
/*
        else if (doc.name() == KXMLQLCVirtualConsole)
        {
            m_virtualConsole->loadXML(doc);
        }
*/
        else
        {
            qDebug() << "Skipping tag:" << doc.name().toString();
            doc.skipCurrentElement();
        }
    }

    return true;
}

void ImportManager::getAvailableFixtureAddress(int channels, int &universe, int &address)
{
    int freeCounter = 0;
    quint32 absAddress = (universe << 9) + address;

    while (1)
    {
        if (m_doc->fixtureForAddress(absAddress) == Fixture::invalidId())
            freeCounter++;
        else
            freeCounter = 0;

        if (freeCounter == channels)
        {
            universe = (absAddress >> 9);
            address = absAddress - (universe * 512) - (channels - 1);
            return;
        }

        absAddress++;
    }
}

void ImportManager::importFixtures()
{
    MonitorProperties *importMonProps = m_importDoc->monitorProperties();
    MonitorProperties *monProps = m_doc->monitorProperties();

    /* ************************ Import fixtures ************************ */
    for (quint32 importID : m_fixtureIDList)
    {
        bool matchFound = false;
        Fixture *importFixture = m_importDoc->fixture(importID);

        qDebug() << "Import fixture" << importFixture->name();

        /* Check if a Fixture with the same name already exists in m_doc.
         * If it does, check also if the ID needs to be remapped */
        for (Fixture *docFixture : m_doc->fixtures())
        {
            if (docFixture->name() == importFixture->name())
            {
                qDebug() << "Match found. Import ID:" << importID << docFixture->id();
                if (docFixture->id() != importID)
                {
                    qDebug() << "Fixture" << importFixture->name() << "with ID" << importID
                             << "will be remapped to ID" << docFixture->id();
                }

                m_fixtureIDRemap[importID] = docFixture->id();
                matchFound = true;
                break;
            }
        }

        /* if no match is found, it means a new Fixture needs to be created
         * in m_doc, which implies finding an available address and ID remapping */
        if (matchFound == false)
        {
            // Attempt to preserve original universe/address.
            // Will be checked later if available
            int uniIdx = importFixture->universe();
            int address = importFixture->address();

            QLCFixtureDef *importDef = importFixture->fixtureDef();
            QLCFixtureMode *importMode = importFixture->fixtureMode();
            QLCFixtureDef *fxiDef = m_doc->fixtureDefCache()->fixtureDef(importDef->manufacturer(), importDef->model());
            QLCFixtureMode *fxiMode = nullptr;

            if (fxiDef != nullptr && importMode != nullptr)
                fxiMode = fxiDef->mode(importMode->name());

            Fixture *fxi = new Fixture(m_doc);
            fxi->setName(importFixture->name());

            getAvailableFixtureAddress(importFixture->channels(), uniIdx, address);
            fxi->setUniverse(uniIdx);
            fxi->setAddress(address);

            if (fxiDef == nullptr && fxiMode == nullptr)
            {
                if (importDef->model() == "Generic")
                {
                    fxiDef = fxi->genericDimmerDef(importFixture->channels());
                    fxiMode = fxi->genericDimmerMode(fxiDef, importFixture->channels());
                }
                else
                {
                    qWarning() << "FIXME: Something really bad happened";
                    delete fxi;
                    continue;
                }
            }

            fxi->setFixtureDefinition(fxiDef, fxiMode);

            if (m_doc->addFixture(fxi) == true)
            {
                m_fixtureIDRemap[importID] = fxi->id();
            }
            else
            {
                qWarning() << "ERROR: Failed to add fixture" << importFixture->name()
                         << "to universe" << uniIdx << "@address" << address;
                delete fxi;
            }
        }
    }
    /* ******************** Import linked fixtures ********************* */
    for (quint32 itemID : m_itemIDList)
    {
        quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

        // if no original fixture was selected, skip linked
        if (m_fixtureIDList.contains(fixtureID) == false)
            continue;

        QString name = importMonProps->fixtureName(fixtureID, headIndex, linkedIndex);
        //quint32 remappedID = FixtureUtils::fixtureItemID(m_fixtureIDRemap[fixtureID], headIndex, linkedIndex);
        monProps->setFixtureName(m_fixtureIDRemap[fixtureID], headIndex, linkedIndex, name);
    }


    /* ********************* Import fixture groups ********************* */
    for (quint32 groupID : m_fixtureGroupIDList)
    {
        bool matchFound = false;
        FixtureGroup *importGroup = m_importDoc->fixtureGroup(groupID);

        qDebug() << "Import fixture group" << importGroup->name();

        for (FixtureGroup *docGroup : m_doc->fixtureGroups())
        {
            if (docGroup->name() == importGroup->name())
            {
                m_fixtureGroupIDRemap[groupID] = docGroup->id();
                matchFound = true;
                break;
            }
        }

        /* if no match is found, it means a new FixtureGroup needs to be created
         * in m_doc, which implies ID remapping */
        if (matchFound == false)
        {
            FixtureGroup *newGroup = new FixtureGroup(m_doc);
            newGroup->setName(importGroup->name());
            newGroup->setSize(importGroup->size());

            QMap<QLCPoint, GroupHead> headsMap = importGroup->headsMap();
            QMap<QLCPoint, GroupHead>::const_iterator i = headsMap.constBegin();
            while (i != headsMap.constEnd())
            {
                QLCPoint p = i.key();
                GroupHead head = i.value();

                if (m_fixtureIDList.contains(head.fxi))
                {
                    head.fxi = m_fixtureIDRemap[head.fxi];
                    newGroup->assignHead(p, head);
                }

                ++i;
            }

            if (m_doc->addFixtureGroup(newGroup) == true)
            {
                m_fixtureGroupIDRemap[groupID] = newGroup->id();
            }
            else
            {
                qWarning() << "ERROR: Failed to add fixture group" << newGroup->name();
                delete newGroup;
            }
        }
    }
}

void ImportManager::importPalettes()
{
    for (quint32 paletteID : m_paletteIDList)
    {
        QLCPalette *importPalette = m_importDoc->palette(paletteID);
        bool matchFound = false;

        qDebug() << "Import palette" << importPalette->name();

        /* Check if a Palette with the same name already exists in m_doc.
         * If it does, check also if the ID needs to be remapped */
        for (QLCPalette *docPalette : m_doc->palettes())
        {
            if (docPalette->name() == importPalette->name())
            {
                if (docPalette->id() != paletteID)
                {
                    qDebug() << "Match found. Palette" << importPalette->name()
                             << "with ID" << paletteID << "will be remapped to ID" << docPalette->id();
                }

                m_fixtureIDRemap[paletteID] = docPalette->id();
                matchFound = true;
                break;
            }
        }

        if (matchFound == false)
        {
            QLCPalette *palette = new QLCPalette(importPalette->type());
            palette->setName(importPalette->name());

            palette->setValues(importPalette->values());
            palette->setFanningType(importPalette->fanningType());
            palette->setFanningLayout(importPalette->fanningLayout());
            palette->setFanningAmount(importPalette->fanningAmount());
            palette->setFanningValue(importPalette->fanningValue());

            if (m_doc->addPalette(palette) == true)
            {
                m_paletteIDRemap[paletteID] = palette->id();
            }
            else
            {
                qWarning() << "ERROR: Failed to add Palette" << palette->name();
                delete palette;
            }
        }
    }
}

void ImportManager::importFunctionID(quint32 funcID)
{
    Function *importFunction = m_importDoc->function(funcID);
    QList<quint32> funcList;

    // 1. Get a list of Function ID upon importFunction depends on
    switch (importFunction->type())
    {
        // these will return only Function IDs
        case Function::ChaserType:
        case Function::SequenceType:
        case Function::CollectionType:
            funcList = importFunction->components();
        break;

        // Scripts are a mix: they can control Fixtures AND Functions
        case Function::ScriptType:
        {
            Script *script = qobject_cast<Script *>(importFunction);
            funcList = script->functionList();
        }
        break;
        default:
        break;
    }

    // 2. import the dependecies first, if any
    for (quint32 depID : funcList)
    {
        if (m_functionIDList.contains(depID))
            importFunctionID(depID);
    }

    // 3. Finally create a copy of the original Function. This will always create a new ID
    Function *docFunction = importFunction->createCopy(m_doc, true);
    m_functionIDRemap[funcID] = docFunction->id();

    qDebug() << "Importing function" << docFunction->name() << "with ID" << docFunction->id();

    // 4. Check Fixture/Function remapping depending on the Function type
    switch (docFunction->type())
    {
        case Function::SceneType:
        {
            Scene *scene = qobject_cast<Scene *>(docFunction);
            // create a copy of the existing components
            QList<SceneValue> sceneValues = scene->values();
            QList<quint32> fixtureGroupList = scene->fixtureGroups();
            QList<quint32> paletteList = scene->palettes();

            // point of no return. Delete everything
            scene->clear();

            // add referenced fixture groups
            for (quint32 groupID : fixtureGroupList)
            {
                if (m_fixtureGroupIDRemap.contains(groupID))
                    scene->addFixtureGroup(m_fixtureGroupIDRemap[groupID]);
            }

            // add referenced palettes
            for (quint32 paletteID : paletteList)
            {
                if (m_paletteIDRemap.contains(paletteID))
                    scene->addPalette(m_paletteIDRemap[paletteID]);
            }

            // remap values against existing/remapped fixtures
            for (SceneValue scv : sceneValues)
            {
                // add a value only if it is present in the remapping map,
                // otherwise it means the Fixture disappeared
                if (m_fixtureIDRemap.contains(scv.fxi))
                {
                    scv.fxi = m_fixtureIDRemap[scv.fxi];
                    scene->setValue(scv);
                }
            }
        }
        break;
        case Function::CollectionType:
        {
            Collection *collection = qobject_cast<Collection *>(docFunction);
            // create a copy of the existing function IDs
            QList<quint32> funcList = collection->functions();

            // point of no return. Empty the Collection
            for (quint32 id : funcList)
                collection->removeFunction(id);

            // Add only the functions that have been imported,
            // with their remapped IDs
            for (quint32 id : funcList)
            {
                if (m_functionIDRemap.contains(id))
                    collection->addFunction(m_functionIDRemap[id]);
            }
        }
        break;
        case Function::ChaserType:
        {
            Chaser *chaser = qobject_cast<Chaser *>(docFunction);
            QList<quint32> removeList;

            for (int i = 0; i < chaser->stepsCount(); i++)
            {
                ChaserStep *step = chaser->stepAt(i);
                if (m_functionIDRemap.contains(step->fid))
                {
                    step->fid = m_functionIDRemap[step->fid];
                }
                else
                {
                    /* this might mean:
                     * - same ID (nothing to do)
                     * - missing ID (add step index to remove list)
                     */
                    if (m_doc->function(step->fid) == nullptr)
                        removeList.append(i);
                }
            }


            if (removeList.isEmpty() == false)
            {
                std::sort(removeList.begin(), removeList.end());
                for (int i = removeList.count() - 1; i >= 0; i--)
                    chaser->removeStep(removeList.at(i));
            }
        }
        break;
        case Function::SequenceType:
        {
            Sequence *sequence = qobject_cast<Sequence *>(docFunction);
            quint32 boundSceneID = sequence->boundSceneID();

            if (boundSceneID != Function::invalidId() &&
                m_functionIDRemap.contains(boundSceneID))
                    sequence->setBoundSceneID(m_functionIDRemap[boundSceneID]);
        }
        break;
        case Function::EFXType:
        {
            EFX *efx = qobject_cast<EFX *>(docFunction);
            for (EFXFixture *efxFixture : efx->fixtures())
            {
                GroupHead head(efxFixture->head());

                if (m_functionIDRemap.contains(head.fxi))
                {
                    head.fxi = m_functionIDRemap[head.fxi];
                    efxFixture->setHead(head);
                }
            }
        }
        break;
        case Function::RGBMatrixType:
        {
            RGBMatrix *rgbm = qobject_cast<RGBMatrix *>(docFunction);
            if (rgbm->fixtureGroup() == FixtureGroup::invalidId())
                break;

            if (m_fixtureGroupIDRemap.contains(rgbm->fixtureGroup()))
                rgbm->setFixtureGroup(m_fixtureGroupIDRemap[rgbm->fixtureGroup()]);
        }
        break;
        default:
            qDebug() << "FIXME: Unhandled Function type" << docFunction->type();
        break;
    }

    m_functionIDList.removeOne(funcID);
}

void ImportManager::setChildrenChecked(TreeModel *tree, bool checked)
{
    if (tree == nullptr)
        return;

    for (TreeModelItem *item : tree->items())
    {
        tree->setItemRoleData(item, checked, TreeModel::IsCheckedRole);

        if (item->hasChildren())
            setChildrenChecked(item->children(), checked);
    }
}

/*********************************************************************
 * Fixture tree
 *********************************************************************/

QVariant ImportManager::groupsTreeModel()
{
    if (m_fixtureTree == nullptr)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);

        FixtureManager::updateGroupsTree(m_importDoc, m_fixtureTree, m_fixtureSearchFilter,
                                         FixtureManager::ShowCheckBoxes | FixtureManager::ShowHeads | FixtureManager::ShowLinked);

        connect(m_fixtureTree, SIGNAL(roleChanged(TreeModelItem*,int,const QVariant&)),
                this, SLOT(slotFixtureTreeDataChanged(TreeModelItem*,int,const QVariant&)));
    }

    return QVariant::fromValue(m_fixtureTree);
}

void ImportManager::slotFixtureTreeDataChanged(TreeModelItem *item, int role, const QVariant &value)
{
    if (role != TreeModel::IsCheckedRole)
        return;

    bool checked = value.toBool();

    if (m_fixtureTreeUpdating)
        return;

    m_fixtureTreeUpdating = true;

    qDebug() << "Fixture tree data changed" << value.toInt() << "data" << item->data() << "value" << value;

    if (item->hasChildren())
        setChildrenChecked(item->children(), checked);

    QVariantList itemData = item->data();
    // itemData must be "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
    if (itemData.count() != 6)
        return;

    int itemType = itemData.at(1).toInt();
    quint32 itemID = itemData.at(2).toUInt();

    if (itemType == App::FixtureDragItem)
    {
        quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

        if (checked)
        {
            if (m_fixtureIDList.contains(fixtureID) == false)
                m_fixtureIDList.append(fixtureID);

            if (linkedIndex > 0 && m_itemIDList.contains(itemID) == false)
            {
                m_itemIDList.append(itemID);
                checkFixtureTree(m_fixtureTree);
            }
        }
        else
        {
            if (linkedIndex > 0)
                m_itemIDList.removeOne(itemID);
            else
                m_fixtureIDList.removeOne(fixtureID);
        }
    }
    else if (itemType == App::FixtureGroupDragItem)
    {
        qDebug() << "Fixture group checked" << itemID;
        if (checked)
        {
            if (m_fixtureGroupIDList.contains(itemID) == false)
                m_fixtureGroupIDList.append(itemID);
        }
        else
        {
            m_fixtureGroupIDList.removeOne(itemID);
        }
    }

    m_fixtureTreeUpdating = false;

    qDebug() << "Selected fixtures:" << m_fixtureIDList.count();
}

QString ImportManager::fixtureSearchFilter() const
{
    return m_fixtureSearchFilter;
}

void ImportManager::setFixtureSearchFilter(QString searchFilter)
{
    if (m_fixtureSearchFilter == searchFilter)
        return;

    int currLen = m_fixtureSearchFilter.length();

    m_fixtureSearchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
    {
        FixtureManager::updateGroupsTree(m_importDoc, m_fixtureTree, m_fixtureSearchFilter,
                                         FixtureManager::ShowCheckBoxes | FixtureManager::ShowGroups);
        checkFixtureTree(m_fixtureTree);
        emit groupsTreeModelChanged();
    }

    emit fixtureSearchFilterChanged();
}

void ImportManager::checkFixtureTree(TreeModel *tree)
{
    if (tree == nullptr)
        return;

    for (TreeModelItem *item : tree->items())
    {
        QVariantList itemData = item->data();

        // itemData must be "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
        if (itemData.count() == 6 && itemData.at(1).toInt() == App::FixtureDragItem)
        {
            quint32 itemID = itemData.at(2).toUInt();
            quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
            quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

            if (m_fixtureIDList.contains(fixtureID) && linkedIndex == 0)
                tree->setItemRoleData(item, true, TreeModel::IsCheckedRole);
        }

        if (item->hasChildren())
            checkFixtureTree(item->children());
    }
}

/*********************************************************************
 * Function tree
 *********************************************************************/

void ImportManager::updateFunctionsTree()
{
    m_functionTree->clear();

    for (Function *func : m_importDoc->functions()) // C++11
    {
        if (func == nullptr || func->isVisible() == false)
            return;

        bool expandAll = m_functionSearchFilter.length() >= SEARCH_MIN_CHARS;

        QQmlEngine::setObjectOwnership(func, QQmlEngine::CppOwnership);

        if (m_functionSearchFilter.length() < SEARCH_MIN_CHARS || func->name().toLower().contains(m_functionSearchFilter))
        {
            QVariantList params;
            params.append(QVariant::fromValue(func));
            QString fPath = func->path(true).replace("/", TreeModel::separator());
            m_functionTree->addItem(func->name(), params, fPath, expandAll ? TreeModel::Expanded : 0);
        }
    }

    checkFunctionTree(m_functionTree);
}

QVariant ImportManager::functionsTreeModel()
{
    if (m_functionTree == nullptr)
    {
        m_functionTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_functionTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef";
        m_functionTree->setColumnNames(treeColumns);
        m_functionTree->enableSorting(false);
        m_functionTree->setCheckable(true);

        updateFunctionsTree();

        connect(m_functionTree, SIGNAL(roleChanged(TreeModelItem*,int,const QVariant&)),
                this, SLOT(slotFunctionTreeDataChanged(TreeModelItem*,int,const QVariant&)));
    }

    return QVariant::fromValue(m_functionTree);
}

void ImportManager::checkFunctionTree(TreeModel *tree)
{
    if (tree == nullptr)
        return;

    for (TreeModelItem *item : tree->items())
    {
        if (item->data().count() == 0)
            continue;

        QVariant cRef = item->data().first();
        if (cRef.canConvert<Function *>())
        {
            Function *func = cRef.value<Function *>();
            if (func != nullptr)
            {
                if (m_functionIDList.contains(func->id()))
                    tree->setItemRoleData(item, true, TreeModel::IsCheckedRole);
            }
        }
        if (item->hasChildren())
            checkFunctionTree(item->children());
    }
}

void ImportManager::slotFunctionTreeDataChanged(TreeModelItem *item, int role, const QVariant &value)
{
    if (role != TreeModel::IsCheckedRole)
        return;

    bool checked = value.toBool();

    if (m_functionTreeUpdating)
        return;

    m_functionTreeUpdating = true;

    qDebug() << "Function tree data changed" << value.toInt() << "data" << item->data();

    if (item->hasChildren())
        setChildrenChecked(item->children(), checked);

    QVariantList itemData = item->data();
    if (itemData.isEmpty())
        return;

    QVariant cRef = item->data().first();
    if (cRef.canConvert<Function *>())
    {
        Function *func = cRef.value<Function *>();
        if (checked)
        {
            if (m_functionIDList.contains(func->id()) == false)
            {
                m_functionIDList.append(func->id());
                checkFunctionDependency(func->id());
                qDebug() << "Fixtures count:" << m_fixtureIDList.count() << "functions count:" << m_functionIDList.count();
                checkFunctionTree(m_functionTree);
                checkFixtureTree(m_fixtureTree);
            }
        }
        else
        {
            m_functionIDList.removeOne(func->id());
        }
    }

    m_functionTreeUpdating = false;

    qDebug() << "Selected functions:" << m_functionIDList.count();
}

QString ImportManager::functionSearchFilter() const
{
    return m_functionSearchFilter;
}

void ImportManager::setFunctionSearchFilter(QString searchFilter)
{
    if (m_functionSearchFilter == searchFilter)
        return;

    int currLen = m_functionSearchFilter.length();

    m_functionSearchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
    {
        updateFunctionsTree();
        emit functionsTreeModelChanged();
    }

    emit functionSearchFilterChanged();
}

void ImportManager::checkFunctionDependency(quint32 fid)
{
    Function *func = m_importDoc->function(fid);
    if (func == nullptr)
        return;

    QList<quint32> funcList;
    QList<quint32> fxList;
    QList<quint32> fxGroupList;
    QList<quint32> paletteList;

    switch (func->type())
    {
        // a Scene can reference fixtures, fixture groups and palettes
        case Function::SceneType:
        {
            Scene *scene = qobject_cast<Scene *>(func);
            fxList = scene->components();
            fxGroupList = scene->fixtureGroups();
            paletteList = scene->palettes();
        }
        break;

        // EFX needs only fixtures
        case Function::EFXType:
            fxList = func->components();
        break;

        // RGB Matrix requires a fixture group
        case Function::RGBMatrixType:
        {
            RGBMatrix *rgbm = qobject_cast<RGBMatrix *>(func);
            fxList = rgbm->components();
            quint32 groupID = rgbm->fixtureGroup();
            fxGroupList.append(groupID);
        }
        break;

        // these will return only Function IDs
        case Function::ChaserType:
        case Function::SequenceType:
        case Function::CollectionType:
            funcList = func->components();
        break;

        // Script are a mix: they can control Fixtures AND Functions
        case Function::ScriptType:
        {
            Script *script = qobject_cast<Script *>(func);
            funcList = script->functionList();
            fxList = script->fixtureList();
        }
        break;

        // Audio/Video go here. No dependency
        default:
        break;
    }

    for (quint32 groupID : fxGroupList)
    {
        if (groupID != FixtureGroup::invalidId() &&
            m_fixtureGroupIDList.contains(groupID) == false)
        {
            FixtureGroup *group = m_importDoc->fixtureGroup(groupID);
            m_fixtureGroupIDList.append(groupID);

            for (quint32 id : group->fixtureList())
            {
                if (m_fixtureIDList.contains(id) == false)
                    m_fixtureIDList.append(id);
            }
        }
    }

    for (quint32 paletteID : paletteList)
    {
        if (paletteID != QLCPalette::invalidId() &&
            m_paletteIDList.contains(paletteID) == false)
        {
            m_paletteIDList.append(paletteID);
        }
    }

    for (quint32 fixtureID : fxList)
    {
        if (m_fixtureIDList.contains(fixtureID) == false)
            m_fixtureIDList.append(fixtureID);
    }

    for (quint32 functionID : funcList)
    {
        if (m_functionIDList.contains(functionID) == false)
        {
            m_functionIDList.append(functionID);
            checkFunctionDependency(functionID);
        }
    }
}
