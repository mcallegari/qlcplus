/*
  Q Light Controller Plus
  uimanager.cpp

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

#include <QQmlComponent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QUrl>
#include <QDir>

#include "qlcfile.h"
#include "qlcconfig.h"
#include "uimanager.h"

#define UISTYLEFILE "qlcplusUiStyle.json"

/** Time to wait before writing the UI settings out, in milliseconds */
#define UISTYLE_SAVE_DELAY 1000

UiManager::UiManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_loading(false)
{
    /** Every change made in the UI settings page is stored automatically,
     *  so the look of the application is preserved across restarts */
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(UISTYLE_SAVE_DELAY);
    connect(&m_saveTimer, &QTimer::timeout, this, [this]() { saveSettings(); });
}

UiManager::~UiManager()
{
}

void UiManager::initialize()
{
    /** Force the creation of the UISettings singleton and
     *  store a reference to it. In this way it is possible
     *  to change the UI settings at runtime */
    QQmlComponent component(m_view->engine());
    const char *source =
            "import QtQuick\n"
            "import \".\"\n"
            "QtObject {\n"
            "    property var style: UISettings\n"
            "}";
    component.setData(source, QUrl("qrc:/"));
    QObject *item = component.create();
    m_uiStyle = qvariant_cast<QObject*>(item->property("style"));

    /** Store default values first */
    setDefaultParameter("sizes", "scalingFactor", 1.0);

    setDefaultParameter("colors", "bgStronger", m_uiStyle->property("bgStronger"));
    setDefaultParameter("colors", "bgStrong", m_uiStyle->property("bgStrong"));
    setDefaultParameter("colors", "bgMedium", m_uiStyle->property("bgMedium"));
    setDefaultParameter("colors", "bgControl", m_uiStyle->property("bgControl"));
    setDefaultParameter("colors", "bgLight", m_uiStyle->property("bgLight"));
    setDefaultParameter("colors", "bgLighter", m_uiStyle->property("bgLighter"));
    setDefaultParameter("colors", "fgMain", m_uiStyle->property("fgMain"));
    setDefaultParameter("colors", "fgMedium", m_uiStyle->property("fgMedium"));
    setDefaultParameter("colors", "fgLight", m_uiStyle->property("fgLight"));

    setDefaultParameter("colors", "sectionHeader", m_uiStyle->property("sectionHeader"));
    setDefaultParameter("colors", "sectionHeaderDiv", m_uiStyle->property("sectionHeaderDiv"));
    setDefaultParameter("colors", "highlight", m_uiStyle->property("highlight"));
    setDefaultParameter("colors", "highlightPressed", m_uiStyle->property("highlightPressed"));
    setDefaultParameter("colors", "hover", m_uiStyle->property("hover"));
    setDefaultParameter("colors", "selection", m_uiStyle->property("selection"));
    setDefaultParameter("colors", "activeDropArea", m_uiStyle->property("activeDropArea"));
    setDefaultParameter("colors", "borderColorDark", m_uiStyle->property("borderColorDark"));

    setDefaultParameter("colors", "toolbarStartMain", m_uiStyle->property("toolbarStartMain"));
    setDefaultParameter("colors", "toolbarStartSub", m_uiStyle->property("toolbarStartSub"));
    setDefaultParameter("colors", "toolbarEnd", m_uiStyle->property("toolbarEnd"));
    setDefaultParameter("colors", "toolbarHoverStart", m_uiStyle->property("toolbarHoverStart"));
    setDefaultParameter("colors", "toolbarHoverEnd", m_uiStyle->property("toolbarHoverEnd"));

    setDefaultParameter("colors", "toolbarSelectionMain", m_uiStyle->property("toolbarSelectionMain"));
    setDefaultParameter("colors", "toolbarSelectionSub", m_uiStyle->property("toolbarSelectionSub"));

    /** Then load (if available) the user configuration. Changes applied
     *  from file must not schedule a save of what has just been read */
    QJsonObject root;
    if (QFile::exists(userConfFilepath()) && readFromFile(userConfFilepath(), root))
    {
        m_loading = true;
        applySettings(root);
        m_loading = false;
    }
}

bool UiManager::readFromFile(QString filePath, QJsonObject &root) const
{
    QFile jsonFile(filePath);
    if (jsonFile.open(QIODevice::ReadOnly) != true)
        return false;

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &parseError);
    jsonFile.close();

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "UI Style parse error at" << parseError.offset << ":" << parseError.errorString();
        return false;
    }

    if (jsonDoc.isObject() == false)
        return false;

    root = jsonDoc.object();
    return true;
}

void UiManager::applySettings(const QJsonObject &root)
{
    for (const QString &category : root.keys())
    {
        QJsonObject categoryObj = root.value(category).toObject();
        for (const QString &paramName : categoryObj.keys())
        {
            if (m_parameterMap.contains(paramName) == false)
            {
                qWarning() << "Unknown UI parameter" << paramName;
                continue;
            }
            setModified(paramName, categoryObj.value(paramName).toVariant());
        }
    }
}

void UiManager::setDefaultParameter(QString category, QString name, QVariant value)
{
    UiProperty prop;
    prop.m_category = category;
    prop.m_default = value;
    prop.m_modified = value;
    m_parameterMap.insert(name, prop);
}

QVariant UiManager::getDefault(QString name) const
{
    UiProperty prop = m_parameterMap.value(name);
    return prop.m_default;
}

QVariant UiManager::getModified(QString name) const
{
    UiProperty prop = m_parameterMap.value(name);
    return prop.m_modified;
}

void UiManager::setModified(QString name, QVariant value)
{
    UiProperty prop = m_parameterMap.value(name);
    prop.m_modified = value;
    m_parameterMap.insert(name, prop);
    std::string str = name.toStdString();
    m_uiStyle->setProperty(str.c_str(), value);

    scheduleSave();
}

void UiManager::scheduleSave()
{
    if (m_loading == false)
        m_saveTimer.start();
}

void UiManager::flushSettings()
{
    if (m_saveTimer.isActive() == false)
        return;

    m_saveTimer.stop();
    saveSettings();
}

QString UiManager::userConfFolder() const
{
    QDir userConfDir = QLCFile::userDirectory(QString(USERQLCPLUSDIR), QString(USERQLCPLUSDIR), QStringList());
    return userConfDir.absolutePath();
}

QString UiManager::userConfFilepath() const
{
    return userConfFolder() + QDir::separator() + UISTYLEFILE;
}

bool UiManager::saveSettings() const
{
    return saveToFile(userConfFilepath());
}

bool UiManager::saveProfile(QString filePath) const
{
    QString localPath = filePath;
    if (localPath.startsWith("file:"))
        localPath = QUrl(filePath).toLocalFile();

    if (localPath.endsWith(".json", Qt::CaseInsensitive) == false)
        localPath.append(".json");

    return saveToFile(localPath);
}

bool UiManager::loadProfile(QString filePath)
{
    QString localPath = filePath;
    if (localPath.startsWith("file:"))
        localPath = QUrl(filePath).toLocalFile();

    QJsonObject root;
    if (readFromFile(localPath, root) == false)
        return false;

    /** A profile only carries the parameters that differ from the
     *  default, so everything else reverts to its default. Only the
     *  parameters actually changing are touched, to avoid needless
     *  relayouts of the whole UI */
    QMap<QString, QVariant> values;
    for (auto it = m_parameterMap.cbegin(); it != m_parameterMap.cend(); ++it)
        values.insert(it.key(), it.value().m_default);

    int found = 0;
    for (const QString &category : root.keys())
    {
        QJsonObject categoryObj = root.value(category).toObject();
        for (const QString &paramName : categoryObj.keys())
        {
            if (values.contains(paramName) == false)
                continue;

            values.insert(paramName, categoryObj.value(paramName).toVariant());
            found++;
        }
    }

    /** Refuse a JSON file that is not a UI profile, rather than
     *  silently reverting everything to the defaults */
    if (root.isEmpty() == false && found == 0)
        return false;

    for (auto it = values.cbegin(); it != values.cend(); ++it)
    {
        if (QJsonValue::fromVariant(it.value()) !=
            QJsonValue::fromVariant(getModified(it.key())))
            setModified(it.key(), it.value());
    }

    return true;
}

bool UiManager::saveToFile(QString filePath) const
{
    bool ret = true;
    QFile jsonFile(filePath);
    QMap<QString, QJsonObject*> objMap;
    QJsonObject objRoot;

    /** Add parameters to JSON objects representing categories */
    QMapIterator<QString, UiProperty> it(m_parameterMap);
    while (it.hasNext())
    {
        it.next();
        QString paramName = it.key();
        UiProperty prop = it.value();

        /** Skip the parameters left untouched, so that a future change of
         *  the QLC+ default style still reaches the users who customized
         *  something else */
        if (QJsonValue::fromVariant(prop.m_modified) ==
            QJsonValue::fromVariant(prop.m_default))
            continue;

        if (objMap.contains(prop.m_category) == false)
            objMap.insert(prop.m_category, new QJsonObject());

        QJsonObject *categoryObj = objMap.value(prop.m_category);
        categoryObj->insert(paramName, QJsonValue::fromVariant(prop.m_modified));
    }

    /** Add each JSON object to the root object */
    QMapIterator<QString, QJsonObject*> cIt(objMap);
    while (cIt.hasNext())
    {
        cIt.next();
        objRoot[cIt.key()] = *cIt.value();
    }

    qDeleteAll(objMap);

    /** Finally, store on file */
    QByteArray ba = QJsonDocument(objRoot).toJson();
    //QTextStream ts(stdout);
    //ts << "rendered JSON" << endl;
    //ts << ba;

    if (jsonFile.open(QIODevice::WriteOnly) == true)
    {
        if (jsonFile.write(ba) <= 0)
            ret = false;

        jsonFile.close();
    }
    else
    {
        ret = false;
    }

    return ret;
}
