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
#include "functionmanager.h"

#include "qlcfixturedefcache.h"
#include "audioplugincache.h"
#include "rgbscriptscache.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "doc.h"
#include "app.h"

ImportManager::ImportManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fixtureTree(NULL)
    , m_functionTree(NULL)
{
    m_importDoc = new Doc(this);

    /* Load user fixtures first so that they override system fixtures */
    m_importDoc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_importDoc->fixtureDefCache()->loadMap(QLCFixtureDefCache::systemDefinitionDirectory());

    /* Load channel modifiers templates */
    m_importDoc->modifiersCache()->load(QLCModifiersCache::systemTemplateDirectory(), true);
    m_importDoc->modifiersCache()->load(QLCModifiersCache::userTemplateDirectory());

    /* Load RGB scripts */
    m_importDoc->rgbScriptsCache()->load(RGBScriptsCache::systemScriptsDirectory());
    m_importDoc->rgbScriptsCache()->load(RGBScriptsCache::userScriptsDirectory());

    m_view->rootContext()->setContextProperty("importManager", this);
}

ImportManager::~ImportManager()
{

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
    if (doc == NULL || doc->device() == NULL || doc->hasError())
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
        return QFile::ResourceError;
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
            m_importDoc->loadXML(doc);
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

void ImportManager::slotFunctionTreeDataChanged(TreeModelItem *item, int role, const QVariant &value)
{
    qDebug() << "Function tree data changed" << value.toInt();
    qDebug() << "Item data:" << item->data();

    if (role == TreeModel::IsCheckedRole)
    {

    }
}

QVariant ImportManager::groupsTreeModel()
{
    if (m_fixtureTree == NULL)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);
        m_fixtureTree->setCheckable(true);

        FixtureManager::updateGroupsTree(m_importDoc, m_fixtureTree, m_fixtureSearchFilter, false);
    }

    return QVariant::fromValue(m_fixtureTree);
}

void ImportManager::updateFunctionsTree()
{
    m_functionTree->clear();

    for(Function *func : m_importDoc->functions()) // C++11
    {
        if (func == NULL || func->isVisible() == false)
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
}

QVariant ImportManager::functionsTreeModel()
{
    if (m_functionTree == NULL)
    {
        m_functionTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
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
        FixtureManager::updateGroupsTree(m_importDoc, m_fixtureTree, m_fixtureSearchFilter, false);
        emit groupsTreeModelChanged();
    }

    emit fixtureSearchFilterChanged();
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
