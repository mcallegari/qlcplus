/*
  Q Light Controller Plus
  chasereditor.h

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

#ifndef CHASEREDITOR_H
#define CHASEREDITOR_H

#include "functioneditor.h"
#include "scenevalue.h"

class Chaser;
class ListModel;
class ChaserStep;

class ChaserEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(bool isSequence READ isSequence CONSTANT)
    Q_PROPERTY(QVariant stepsList READ stepsList NOTIFY stepsListChanged)
    Q_PROPERTY(int tempoType READ tempoType WRITE setTempoType NOTIFY tempoTypeChanged)
    Q_PROPERTY(int stepsFadeIn READ stepsFadeIn WRITE setStepsFadeIn NOTIFY stepsFadeInChanged)
    Q_PROPERTY(int stepsFadeOut READ stepsFadeOut WRITE setStepsFadeOut NOTIFY stepsFadeOutChanged)
    Q_PROPERTY(int stepsDuration READ stepsDuration WRITE setStepsDuration NOTIFY stepsDurationChanged)
    Q_PROPERTY(int playbackIndex READ playbackIndex WRITE setPlaybackIndex NOTIFY playbackIndexChanged)

public:
    ChaserEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Chaser being edited */
    void setFunctionID(quint32 ID);

    /** Returns if the Chaser being edited is a Sequence */
    bool isSequence() const;

    /** Static method to update the whole steps list */
    static void updateStepsList(Doc *doc, Chaser *chaser, ListModel *stepsList);

    /** Static method to update the data of a specific step */
    static void updateStepInListModel(Doc *doc, Chaser *chaser, ListModel *stepsList, ChaserStep *step, int index);

    /** Return the Chaser step list formatted as explained in m_stepsList */
    QVariant stepsList() const;

    /**
     * Add a function to the Chaser being edited.
     *
     * @param idsList A list of function IDs to add
     * @param insertIndex a specific insertion index (-1 means append)
     *
     * @return true if successful, otherwise false
     */
    Q_INVOKABLE bool addFunctions(QVariantList idsList, int insertIndex = -1);

    /**
     * Add a new step to the Sequence being edited.
     *
     * @param insertIndex a specific insertion index (-1 means append)
     *
     * @return true if successful, otherwise false
     */
    Q_INVOKABLE bool addStep(int insertIndex = -1);

    /**
     * Move the selected steps at @insertIndex position
     *
     * @param indicesList A list of step indices to move
     * @param insertIndex a specific insertion index (-1 means append)
     *
     * @return true if successful, otherwise false
     */
    Q_INVOKABLE bool moveSteps(QVariantList indicesList, int insertIndex = -1);

    /** Duplicate the selected steps at the end of the existing steps
     *
     *  @param indicesList A list of step indices to move
     *
     *  @return true if successful, otherwise false
     */
    Q_INVOKABLE bool duplicateSteps(QVariantList indicesList);

    /** @reimp */
    void deleteItems(QVariantList list);

    void setSequenceStepValue(SceneValue& scv);
    void removeFixtures(QVariantList list);

    /** Get/Set the Chaser playback start index */
    int playbackIndex() const;
    void setPlaybackIndex(int playbackIndex);

    /** @reimp */
    void setPreviewEnabled(bool enable);

    Q_INVOKABLE void gotoPreviousStep();
    Q_INVOKABLE void gotoNextStep();

protected:
    /** Set the steps $param to $value.
     *  If $selectedOnly is true, $value is applied only to the selected steps,
     *  otherwise it will be applied to all the steps */
    void setSelectedValue(Function::PropType type, QString param, uint value, bool selectedOnly = true);

    static QVariantMap stepDataMap(Doc *doc, Chaser *chaser, ChaserStep *step);

    static void addStepToListModel(Doc *doc, Chaser *chaser, ListModel *stepsList, ChaserStep *step);

protected slots:
    /** Slot invoked during Chaser playback when the step index changes */
    void slotStepIndexChanged(int index);

signals:
    void stepsListChanged();
    void playbackIndexChanged(int playbackIndex);

private:
    /** Reference of the Chaser currently being edited */
    Chaser *m_chaser;

    /** Reference to a ListModel representing the steps list for the QML UI,
     *  organized as follows:
     *  funcID | isSelected | fadeIn | fadeOut | hold | duration | note
     */
    ListModel *m_stepsList;

    /** Index of the current step being played. -1 when stopped */
    int m_playbackIndex;

    /*********************************************************************
     * Steps speed mode
     *********************************************************************/
public:
    /** Get/Set the steps tempo type of the Chaser being edited */
    int tempoType() const;
    void setTempoType(int tempoType);

    /** Get/Set the steps fade in mode of the Chaser being edited */
    int stepsFadeIn() const;
    void setStepsFadeIn(int stepsFadeIn);

    /** Get/Set the steps fade out mode of the Chaser being edited */
    int stepsFadeOut() const;
    void setStepsFadeOut(int stepsFadeOut);

    /** Get/Set the steps duration mode of the Chaser being edited */
    int stepsDuration() const;
    void setStepsDuration(int stepsDuration);

    /** Set the speed value with $type of the step at $index */
    Q_INVOKABLE void setStepSpeed(int index, int value, int type);

    /** Ste the notes text of the step at $index */
    Q_INVOKABLE void setStepNote(int index, QString text);

signals:
    void tempoTypeChanged(int tempoType);
    void stepsFadeInChanged(int stepsFadeIn);
    void stepsFadeOutChanged(int stepsFadeOut);
    void stepsDurationChanged(int stepsDuration);
};

#endif // CHASEREDITOR_H

