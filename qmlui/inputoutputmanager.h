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
class QLCInputProfile;
class InputProfileEditor;

class InputOutputManager : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(QVariant universes READ universes NOTIFY universesChanged)
    Q_PROPERTY(QStringList universeNames READ universeNames NOTIFY universeNamesChanged)
    Q_PROPERTY(QVariant universesListModel READ universesListModel NOTIFY universesListModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

    Q_PROPERTY(QVariant audioInputSources READ audioInputSources NOTIFY audioInputSourcesChanged)
    Q_PROPERTY(QVariant audioOutputSources READ audioOutputSources NOTIFY audioOutputSourcesChanged)
    Q_PROPERTY(QVariant audioInputDevice READ audioInputDevice NOTIFY audioInputDeviceChanged)
    Q_PROPERTY(QVariant audioOutputDevice READ audioOutputDevice NOTIFY audioOutputDeviceChanged)
    Q_PROPERTY(bool blackout READ blackout WRITE setBlackout NOTIFY blackoutChanged)

    Q_PROPERTY(bool inputCanConfigure READ inputCanConfigure NOTIFY inputCanConfigureChanged)
    Q_PROPERTY(bool outputCanConfigure READ outputCanConfigure NOTIFY outputCanConfigureChanged)

    Q_PROPERTY(QString beatType READ beatType WRITE setBeatType NOTIFY beatTypeChanged)
    Q_PROPERTY(int bpmNumber READ bpmNumber WRITE setBpmNumber NOTIFY bpmNumberChanged)

    Q_PROPERTY(QString profileUserFolder READ profileUserFolder CONSTANT)

public:
    InputOutputManager(QQuickView *view, Doc *doc, QObject *parent = 0);

protected slots:
    void slotDocLoaded();

private:
    InputOutputMap *m_ioMap;

    /*********************************************************************
     * Universes
     *********************************************************************/
public:
    QVariant universes();
    QStringList universeNames() const;
    Q_INVOKABLE QString universeName(quint32 universeId);
    QVariant universesListModel() const;

    /** Get/Set the currently selected universe index */
    int selectedIndex() const;
    void setSelectedIndex(int index);

    Q_INVOKABLE void addUniverse();
    Q_INVOKABLE void removeLastUniverse();
    Q_INVOKABLE int  universesCount();

    /** Get/Set the global output blackout state */
    bool blackout() const;
    void setBlackout(bool blackout);

signals:
    void universesChanged();
    void universeNamesChanged();
    void universesListModelChanged();
    void selectedIndexChanged();
    void blackoutChanged(bool blackout);

private:
    /** List of references to the current Universes in Doc */
    QList<Universe *> m_universeList;

    int m_selectedUniverseIndex;
    bool m_blackout;

    /*********************************************************************
     * Audio IO
     *********************************************************************/
public:
    QVariant audioInputDevice();
    QVariant audioOutputDevice();

    QVariant audioInputSources() const;
    QVariant audioOutputSources() const;

    Q_INVOKABLE void setAudioInput(QString privateName);
    Q_INVOKABLE void setAudioOutput(QString privateName);

signals:
    void audioInputDeviceChanged();
    void audioOutputDeviceChanged();

    void audioInputSourcesChanged();
    void audioOutputSourcesChanged();

    /*********************************************************************
     * IO Patches
     *********************************************************************/
public:
    Q_INVOKABLE QVariant universeInputSources(int universe);
    Q_INVOKABLE QVariant universeOutputSources(int universe);

    Q_INVOKABLE int outputPatchesCount(int universe) const;
    Q_INVOKABLE void setOutputPatch(int universe, QString plugin, QString line, int index);
    Q_INVOKABLE void removeOutputPatch(int universe, int index);
    Q_INVOKABLE void addInputPatch(int universe, QString plugin, QString line);
    Q_INVOKABLE void setFeedbackPatch(int universe, bool enable);
    Q_INVOKABLE void removeInputPatch(int universe);
    Q_INVOKABLE void setInputProfile(int universe, QString profileName);

    Q_INVOKABLE void configurePlugin(bool input);

    bool inputCanConfigure() const;
    bool outputCanConfigure() const;

signals:
    void inputCanConfigureChanged();
    void outputCanConfigureChanged();

private:
    void clearInputList();
    void clearOutputList();

    /*********************************************************************
     * Input Profiles
     *********************************************************************/
public:
    QString profileUserFolder();

    Q_INVOKABLE void createInputProfile();
    Q_INVOKABLE bool editInputProfile(QString name);
    Q_INVOKABLE bool saveInputProfile();
    Q_INVOKABLE void finishInputProfile();
    Q_INVOKABLE bool removeInputProfile(QString name);
    Q_INVOKABLE QVariant universeInputProfiles(int universe);

private:
    InputProfileEditor *m_profileEditor;
    QLCInputProfile *m_editProfile;

    /*********************************************************************
     * Beats
     *********************************************************************/
public:
    Q_INVOKABLE QVariant beatGeneratorsList();

    /** Get/Set the beat generator type */
    QString beatType() const;
    void setBeatType(QString beatType);

    /** Get/Set the number of beats per minute to emit
     *  if beat generator is internal */
    int bpmNumber() const;
    void setBpmNumber(int bpmNumber);

signals:
    void beatTypeChanged(QString beatType);
    void beat();
    void bpmNumberChanged(int bpmNumber);

protected slots:
    void slotBeatTypeChanged();

private:
    QString m_beatType;
};

#endif // INPUTOUTPUTMANAGER_H
