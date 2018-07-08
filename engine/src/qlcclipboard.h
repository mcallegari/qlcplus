/*
  Q Light Controller Plus
  qlcclipboard.h

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

#ifndef QLCCLIPBOARD_H
#define QLCCLIPBOARD_H

#include <QList>

#include "chaserstep.h"
#include "scenevalue.h"

class Chaser;
class Scene;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

class QLCClipboard: public QObject
{
    Q_OBJECT

public:
    QLCClipboard(Doc *doc);

public:
    void resetContents();

private:
    Doc* m_doc;

    /********************************************************************
     * Copy Action
     ********************************************************************/

public:
    void copyContent(quint32 sourceID, QList <ChaserStep> steps);
    void copyContent(quint32 sourceID, QList <SceneValue> values);
    void copyContent(quint32 sourceID, Function *function);

    bool hasChaserSteps();
    bool hasSceneValues();
    bool hasFunction();

    QList <ChaserStep> getChaserSteps();
    QList <SceneValue> getSceneValues();
    Function *getFunction();

private:
    QList <ChaserStep> m_copySteps;
    QList <SceneValue> m_copySceneValues;
    Function *m_copyFunction;
};

/** @} */

#endif // QLCCLIPBOARD_H
