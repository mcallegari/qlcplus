/*
  Q Light Controller Plus
  fixtureremap.h

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

#ifndef FIXTUREREMAP_H
#define FIXTUREREMAP_H

#include <QList>
#include <QMap>

#include "scenevalue.h"

class Doc;

/**
 * Helper class to perform fixture remapping (changing fixture profiles
 * while attempting to preserve programming).
 */
class FixtureRemap
{
public:
    FixtureRemap(Doc *doc);

    /**
     * Replaces the profile of the fixtures in $fixtureIDs with the new $manufacturer, $model and $mode.
     * Returns a map of the remapped channels (Source -> Target).
     */
    QMap<SceneValue, SceneValue> replaceProfiles(QList<quint32> fixtureIDs, const QString& manufacturer,
                                                  const QString& model, const QString& mode);

private:
    /** Remap all Scene functions */
    void remapScenes(const QMap<SceneValue, SceneValue>& remapMap);

    /** Remap 2D monitor properties */
    void remapMonitorProperties(const QMap<quint32, quint32>& fixtureRemapMap);

    Doc *m_doc;
};

#endif // FIXTUREREMAP_H
