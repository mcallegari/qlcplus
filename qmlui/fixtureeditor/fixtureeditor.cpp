/*
  Q Light Controller Plus
  fixtureeditor.cpp

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

#include "qlcfixturedef.h"
#include "doc.h"

#include "fixtureeditor.h"
#include "physicaledit.h"
#include "editorview.h"

FixtureEditor::FixtureEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_lastId(0)
{
    m_view->rootContext()->setContextProperty("fixtureEditor", this);
    qmlRegisterUncreatableType<EditorView>("org.qlcplus.classes", 1, 0, "EditorView", "Can't create EditorView!");
    qmlRegisterUncreatableType<PhysicalEdit>("org.qlcplus.classes", 1, 0, "PhysicalEdit", "Can't create PhysicalEdit!");
}

FixtureEditor::~FixtureEditor()
{

}

void FixtureEditor::createDefinition()
{
    m_editors[m_lastId] = new EditorView(m_view, new QLCFixtureDef());
    m_lastId++;
    emit editorsListChanged();
}

void FixtureEditor::editDefinition(QString manufacturer, QString model)
{
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);

    m_editors[m_lastId] = new EditorView(m_view, def);
    m_lastId++;
    emit editorsListChanged();
}

QVariantList FixtureEditor::editorsList() const
{
    QVariantList list;

    QMap<int, EditorView*>::const_iterator i = m_editors.constBegin();
    while (i != m_editors.constEnd())
    {
        QVariantMap eMap;
        eMap.insert("id", i.key());
        eMap.insert("cRef", QVariant::fromValue(i.value()));
        list.append(eMap);
        ++i;
    }

    return list;
}
