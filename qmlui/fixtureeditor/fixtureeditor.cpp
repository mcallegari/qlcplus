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
#include <QSettings>

#include "qlcfixturedef.h"
#include "doc.h"

#include "avolitesd4parser.h"
#include "fixtureeditor.h"
#include "physicaledit.h"
#include "channeledit.h"
#include "editorview.h"
#include "modeedit.h"
#include "qlcfile.h"

#define SETTINGS_DEF_WORKINGPATH "defeditor/workingpath"

FixtureEditor::FixtureEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_lastId(0)
{
    m_view->rootContext()->setContextProperty("fixtureEditor", this);
    qmlRegisterUncreatableType<EditorView>("org.qlcplus.classes", 1, 0, "EditorRef", "Can't create EditorView!");
    qmlRegisterUncreatableType<PhysicalEdit>("org.qlcplus.classes", 1, 0, "PhysicalEdit", "Can't create PhysicalEdit!");
    qmlRegisterUncreatableType<ChannelEdit>("org.qlcplus.classes", 1, 0, "ChannelEdit", "Can't create ChannelEdit!");
    qmlRegisterUncreatableType<ModeEdit>("org.qlcplus.classes", 1, 0, "ModeEdit", "Can't create ModeEdit!");

    QSettings settings;
    QVariant dir = settings.value(SETTINGS_DEF_WORKINGPATH);
    if (dir.isValid() == true)
        m_workingPath = dir.toString();
    else
        m_workingPath = "file://" + userFolder();

    qDebug() << "working path:" << m_workingPath;
}

FixtureEditor::~FixtureEditor()
{

}

QString FixtureEditor::userFolder() const
{
    return m_doc->fixtureDefCache()->userDefinitionDirectory().absolutePath();
}

QString FixtureEditor::workingPath() const
{
    return m_workingPath;
}

void FixtureEditor::setWorkingPath(QString workingPath)
{
    qDebug() << "Setting new path:" << workingPath;
    if (m_workingPath == workingPath)
        return;

    m_workingPath = workingPath;

    QSettings settings;
    settings.setValue(SETTINGS_DEF_WORKINGPATH, m_workingPath);

    emit workingPathChanged(workingPath);
}

void FixtureEditor::createDefinition()
{
    QLCFixtureDef *def = new QLCFixtureDef();
    def->setIsUser(true);
    m_editors[m_lastId] = new EditorView(m_view, m_lastId, def);
    m_lastId++;
    emit editorsListChanged();
}

bool FixtureEditor::loadDefinition(QString fileName)
{
    QLCFixtureDef *def = new QLCFixtureDef();
    QString localFilename = fileName;
    bool result = false;

    if (localFilename.startsWith("file:"))
        localFilename = QUrl(fileName).toLocalFile();

    qDebug() << "Loading definition:" << localFilename;

    if (localFilename.endsWith("qxf", Qt::CaseInsensitive))
    {
        QFile::FileError error = def->loadXML(localFilename);
        if (error == QFile::NoError)
            result = true;
    }
    else if (localFilename.endsWith(KExtAvolitesFixture, Qt::CaseInsensitive))
    {
        AvolitesD4Parser parser;
        result = parser.loadXML(localFilename, def);
    }
    if (result == false)
    {

        delete def;
        return false;
    }

    def->setDefinitionSourceFile(localFilename);
    def->setIsUser(true);
    m_editors[m_lastId] = new EditorView(m_view, m_lastId, def);
    m_lastId++;
    emit editorsListChanged();
    return true;
}

bool FixtureEditor::editDefinition(QString manufacturer, QString model)
{
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);

    if (def == nullptr)
        return false;

    m_editors[m_lastId] = new EditorView(m_view, m_lastId, def);
    m_lastId++;
    emit editorsListChanged();
    return true;
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

void FixtureEditor::deleteEditor(int id)
{
    if (m_editors.contains(id) == false)
    {
        qWarning() << "No definition editor found with ID" << id;
        return;
    }

    EditorView *editor = m_editors.take(id);

    // reload fixture definition from disk
    QLCFixtureDef *def = editor->fixtureDefinition();
    if (def != nullptr)
        m_doc->fixtureDefCache()->reloadFixtureDef(def);

    delete editor;
    emit editorsListChanged();
}
