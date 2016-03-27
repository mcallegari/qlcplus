/*
  Q Light Controller Plus
  inputoutputmanager.h

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

#ifndef INPUTOUTPUTMANAGER_H
#define INPUTOUTPUTMANAGER_H

#include <QStringList>
#include <QQuickItem>
#include <QVariant>
#include <QObject>

class Doc;
class Universe;
class InputOutputMap;

class InputOutputManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<Universe> universes READ universes CONSTANT)
    Q_PROPERTY(QStringList universeNames READ universeNames CONSTANT)
    Q_PROPERTY(QVariant universesListModel READ universesListModel NOTIFY universesListModelChanged)
    Q_PROPERTY(QVariant audioInputDevice READ audioInputDevice NOTIFY audioInputDeviceChanged)
    Q_PROPERTY(QVariant audioOutputDevice READ audioOutputDevice NOTIFY audioOutputDeviceChanged)

public:
    InputOutputManager(Doc *doc, QObject *parent = 0);

    QQmlListProperty<Universe> universes();
    QStringList universeNames() const;
    QVariant universesListModel() const;

    QVariant audioInputDevice();
    QVariant audioOutputDevice();

    Q_INVOKABLE QVariant audioInputSources();
    Q_INVOKABLE QVariant audioOutputSources();

    Q_INVOKABLE QVariant universeInputSources(int universe);
    Q_INVOKABLE QVariant universeOutputSources(int universe);
    Q_INVOKABLE QVariant universeInputProfiles(int universe);

    Q_INVOKABLE void addOutputPatch(int universe, QString plugin, QString line);
    Q_INVOKABLE void removeOutputPatch(int universe);
    Q_INVOKABLE void addInputPatch(int universe, QString plugin, QString line);
    Q_INVOKABLE void removeInputPatch(int universe);
    Q_INVOKABLE void setInputProfile(int universe, QString profileName);

    Q_INVOKABLE void setSelectedItem(QQuickItem *item, int index);

private:
    void clearInputList();
    void clearOutputList();

signals:
    void universesChanged();
    void audioInputDeviceChanged();
    void audioOutputDeviceChanged();
    void universesListModelChanged();

protected slots:
    void slotDocLoaded();

private:
    Doc *m_doc;
    InputOutputMap* m_ioMap;
    /** List of references to the current Universes in Doc */
    QList<Universe *> m_universeList;

    QQuickItem *m_selectedItem;
    int m_selectedUniverseIndex;
};

#endif // INPUTOUTPUTMANAGER_H
