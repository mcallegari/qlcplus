/*
  Q Light Controller Plus
  collectioneditor.cpp

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

#include "collectioneditor.h"
#include "collection.h"
#include "listmodel.h"
#include "tardis.h"
#include "doc.h"

CollectionEditor::CollectionEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_collection(nullptr)
{
    m_view->rootContext()->setContextProperty("collectionEditor", this);

    m_functionsList = new ListModel(this);
    QStringList listRoles;
    listRoles << "funcID" << "isSelected";
    m_functionsList->setRoleNames(listRoles);
}

void CollectionEditor::setFunctionID(quint32 ID)
{
    m_collection = qobject_cast<Collection *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
    updateFunctionsList();
}

QVariant CollectionEditor::functionsList() const
{
    return QVariant::fromValue(m_functionsList);
}

bool CollectionEditor::addFunction(quint32 fid, int insertIndex)
{
    if (m_collection != nullptr)
    {
        if (m_collection->addFunction(fid, insertIndex) == true)
        {
            Tardis::instance()->enqueueAction(Tardis::CollectionAddFunction, m_collection->id(), QVariant(),
                                              QVariant::fromValue(UIntPair(fid, insertIndex)));
            updateFunctionsList();
            return true;
        }
    }

    return false;
}

bool CollectionEditor::moveFunction(quint32 fid, int newIndex)
{
    if (m_collection == nullptr)
        return false;

    QList<quint32> funcList = m_collection->functions();
    Tardis::instance()->enqueueAction(Tardis::CollectionRemoveFunction, m_collection->id(),
                                      QVariant::fromValue(UIntPair(fid, funcList.indexOf(fid))), QVariant());
    m_collection->removeFunction(fid);
    m_collection->addFunction(fid, newIndex);
    Tardis::instance()->enqueueAction(Tardis::CollectionAddFunction, m_collection->id(), QVariant(),
                                      QVariant::fromValue(UIntPair(fid, newIndex)));
    updateFunctionsList();
    return true;
}

void CollectionEditor::deleteItems(QVariantList list)
{
    if (m_collection == nullptr)
        return;

    /** Retrieve the list of the current Functions */
    QList<quint32> funcList = m_collection->functions();

    for (QVariant index : list) // C++11
    {
        int idx = index.toInt();
        if (idx < 0 || idx >= funcList.count())
            continue;

        Tardis::instance()->enqueueAction(Tardis::CollectionRemoveFunction, m_collection->id(),
                                          QVariant::fromValue(UIntPair(funcList.at(idx), idx)), QVariant());
        m_collection->removeFunction(funcList.at(idx));
    }
    updateFunctionsList();
}

void CollectionEditor::updateFunctionsList()
{
    if (m_collection != nullptr)
    {
        m_functionsList->clear();

        for (quint32 fId : m_collection->functions())
        {
            qDebug() << "Adding" << fId << "to collection list";
            QVariantMap funcMap;
            funcMap.insert("funcID", fId);
            funcMap.insert("isSelected", false);
            m_functionsList->addDataMap(funcMap);
        }
    }
    emit functionsListChanged();
}

