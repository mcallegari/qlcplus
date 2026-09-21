/*
  Q Light Controller Plus
  uimanager.h

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

#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QQuickView>
#include <QJsonObject>
#include <QObject>
#include <QTimer>

typedef struct
{
    QVariant m_default;
    QVariant m_modified;
    QString m_category;
} UiProperty;

class Doc;

class UiManager final : public QObject
{
    Q_OBJECT

public:
    UiManager(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~UiManager();

    void initialize();
    void setDefaultParameter(QString category, QString name, QVariant value);

    /** Write out any pending UI settings change right away. To be called
     *  when the application is closing, so a change made in the last
     *  second before quitting is not lost */
    void flushSettings();

    Q_INVOKABLE QVariant getDefault(QString name) const;

    Q_INVOKABLE QVariant getModified(QString name) const;
    Q_INVOKABLE void setModified(QString name, QVariant value);

    /** Return the folder holding the user UI settings, which is also
     *  where UI profiles are proposed to be saved */
    Q_INVOKABLE QString userConfFolder() const;
    QString userConfFilepath() const;
    bool saveSettings() const;

    /** Save the current UI settings as a profile to $filePath.
     *  A ".json" extension is appended when missing */
    Q_INVOKABLE bool saveProfile(QString filePath) const;

    /** Replace the current UI settings with the profile stored
     *  in $filePath. Parameters not present in the profile are
     *  reverted to their default value */
    Q_INVOKABLE bool loadProfile(QString filePath);

private:
    /** Write the parameters that differ from their default to $filePath */
    bool saveToFile(QString filePath) const;

    /** Parse the JSON file at $filePath into $root.
     *  Return false if the file cannot be read or parsed */
    bool readFromFile(QString filePath, QJsonObject &root) const;

    /** Apply the parameters found in $root. Unknown ones are ignored */
    void applySettings(const QJsonObject &root);

    /** Schedule a deferred save of the UI settings. Changes come in bursts
     *  (dragging the scaling factor slider emits one per pixel), so the
     *  actual write is coalesced into a single one */
    void scheduleSave();

    /** Reference to the QML view root */
    QQuickView *m_view;

    /** Reference to the project workspace */
    Doc *m_doc;

    /** Reference to the UI QML settings */
    QObject *m_uiStyle;

    /** A map ok key,value representing every UI parameter
     *  that can be changed at runtime */
    QMap<QString, UiProperty> m_parameterMap;

    /** Timer used to coalesce the automatic saving of the UI settings */
    QTimer m_saveTimer;

    /** Flag raised while loading the user configuration, to tell the
     *  parameter changes coming from file apart from the ones made by
     *  the user. The former must not trigger a save */
    bool m_loading;
};

#endif // UIMANAGER_H
