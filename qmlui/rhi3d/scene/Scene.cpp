/*
  Q Light Controller Plus
  Scene.cpp

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

#include "scene/Scene.h"

namespace {

bool lightEquals(const Light &a, const Light &b)
{
    return a.type == b.type
            && a.color == b.color
            && a.intensity == b.intensity
            && a.position == b.position
            && a.direction == b.direction
            && a.range == b.range
            && a.innerCone == b.innerCone
            && a.outerCone == b.outerCone
            && a.areaSize == b.areaSize
            && a.castShadows == b.castShadows
            && a.qualitySteps == b.qualitySteps
            && a.goboPath == b.goboPath
            && a.beamRadius == b.beamRadius
            && a.beamShape == b.beamShape;
}

} // namespace

bool RhiScene::setLights(const QVector<Light> &lights)
{
    if (m_lights.size() == lights.size())
    {
        bool same = true;
        for (int i = 0; i < lights.size(); ++i)
        {
            if (!lightEquals(m_lights[i], lights[i]))
            {
                same = false;
                break;
            }
        }
        if (same)
            return false;
    }
    m_lights = lights;
    m_lightsDirty = true;
    return true;
}
