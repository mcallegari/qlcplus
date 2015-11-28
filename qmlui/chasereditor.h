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

class Chaser;

class ChaserEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QString chaserName READ chaserName WRITE setChaserName NOTIFY chaserNameChanged)
    Q_PROPERTY(QVariant stepsList READ stepsList NOTIFY stepsListChanged)
    Q_PROPERTY(int runOrder READ runOrder WRITE setRunOrder NOTIFY runOrderChanged)
    Q_PROPERTY(int direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(int stepsFadeIn READ stepsFadeIn WRITE setStepsFadeIn NOTIFY stepsFadeInChanged)
    Q_PROPERTY(int stepsFadeOut READ stepsFadeOut WRITE setStepsFadeOut NOTIFY stepsFadeOutChanged)
    Q_PROPERTY(int stepsDuration READ stepsDuration WRITE setStepsDuration NOTIFY stepsDurationChanged)

public:
    ChaserEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Chaser being edited */
    void setFunctionID(quint32 ID);

    QVariant stepsList() const;

    /** Return the name of the Chaser being edited */
    QString chaserName() const;

    /** Set the name of the Chaser being edited */
    void setChaserName(QString chaserName);

    /**
     * Add a function to the Chaser being edited.
     *
     * @param fid The function ID to add
     * @return true if successful, otherwise false
     */
    Q_INVOKABLE bool addFunction(quint32 fid, int insertIndex = -1);

    /*********************************************************************
     * Chaser playback modes
     *********************************************************************/

    /** Return the run order of the Chaser being edited */
    int runOrder() const;

    /** Set the run order of the Chaser being edited */
    void setRunOrder(int runOrder);

    /** Return the playback direction of the Chaser being edited */
    int direction() const;

    /** Set the run order of the Chaser being edited */
    void setDirection(int direction);

    /*********************************************************************
     * Steps speed mode
     *********************************************************************/

    /** Return the steps fade in mode of the Chaser being edited */
    int stepsFadeIn() const;

    /** Set the steps fade in mode of the Chaser being edited */
    void setStepsFadeIn(int stepsFadeIn);

    /** Return the steps fade out mode of the Chaser being edited */
    int stepsFadeOut() const;

    /** Set the steps fade out mode of the Chaser being edited */
    void setStepsFadeOut(int stepsFadeOut);

    /** Return the steps duration mode of the Chaser being edited */
    int stepsDuration() const;

    /** Set the steps duration mode of the Chaser being edited */
    void setStepsDuration(int stepsDuration);

signals:
    void chaserNameChanged(QString chaserName);
    void stepsListChanged();
    void runOrderChanged(int runOrder);
    void directionChanged(int direction);
    void stepsFadeInChanged(int stepsFadeIn);
    void stepsFadeOutChanged(int stepsFadeOut);
    void stepsDurationChanged(int stepsDuration);

private:
    /** Reference of the Chaser currently being edited */
    Chaser *m_chaser;
};

#endif // CHASEREDITOR_H

