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

#include "previewcontext.h"

class Doc;
class Universe;
class InputOutputMap;

class InputOutputManager : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<Universe> universes READ universes CONSTANT)
    Q_PROPERTY(QStringList universeNames READ universeNames CONSTANT)
    Q_PROPERTY(QVariant universesListModel READ universesListModel NOTIFY universesListModelChanged)
    Q_PROPERTY(QVariant audioInputDevice READ audioInputDevice NOTIFY audioInputDeviceChanged)
    Q_PROPERTY(QVariant audioOutputDevice READ audioOutputDevice NOTIFY audioOutputDeviceChanged)

    Q_PROPERTY(QString beatType READ beatType WRITE setBeatType NOTIFY beatTypeChanged)
    Q_PROPERTY(int bpmNumber READ bpmNumber WRITE setBpmNumber NOTIFY bpmNumberChanged)

public:
    InputOutputManager(QQuickView *view, Doc *doc, QObject *parent = 0);

protected slots:
    void slotDocLoaded();

private:
    InputOutputMap* m_ioMap;

    /*********************************************************************
     * Universes
     *********************************************************************/
public:
    QQmlListProperty<Universe> universes();
    QStringList universeNames() const;
    QVariant universesListModel() const;

    Q_INVOKABLE void setSelectedItem(QQuickItem *item, int index);

signals:
    void universesChanged();
    void universesListModelChanged();

private:
    /** List of references to the current Universes in Doc */
    QList<Universe *> m_universeList;

    QQuickItem *m_selectedItem;
    int m_selectedUniverseIndex;

    /*********************************************************************
     * Audio IO
     *********************************************************************/
public:
    QVariant audioInputDevice();
    QVariant audioOutputDevice();

    Q_INVOKABLE QVariant audioInputSources();
    Q_INVOKABLE QVariant audioOutputSources();

signals:
    void audioInputDeviceChanged();
    void audioOutputDeviceChanged();

    /*********************************************************************
     * IO Patches
     *********************************************************************/
public:
    Q_INVOKABLE QVariant universeInputSources(int universe);
    Q_INVOKABLE QVariant universeOutputSources(int universe);
    Q_INVOKABLE QVariant universeInputProfiles(int universe);

    Q_INVOKABLE int outputPatchesCount(int universe) const;
    Q_INVOKABLE void setOutputPatch(int universe, QString plugin, QString line, int index);
    Q_INVOKABLE void removeOutputPatch(int universe, int index);
    Q_INVOKABLE void addInputPatch(int universe, QString plugin, QString line);
    Q_INVOKABLE void removeInputPatch(int universe);
    Q_INVOKABLE void setInputProfile(int universe, QString profileName);

private:
    void clearInputList();
    void clearOutputList();

    /*********************************************************************
     * Beats
     *********************************************************************/
public:
    Q_INVOKABLE QVariant beatGeneratorsList();

    QString beatType() const;

    void setBeatType(QString beatType);

    int bpmNumber() const;

    void setBpmNumber(int bpmNumber);

signals:
    void beatTypeChanged(QString beatType);
    void beat();
    void bpmNumberChanged(int bpmNumber);

protected slots:
    void slotBeatTypeChanged();
    void slotBpmNumberChanged(int bpmNumber);

private:
    QString m_beatType;
    int m_bpmNumber;
};

#endif // INPUTOUTPUTMANAGER_H
