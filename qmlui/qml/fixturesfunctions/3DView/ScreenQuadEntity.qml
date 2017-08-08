/*
  Q Light Controller Plus
  ScreenQuadEntity.qml

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

import QtQuick 2.7

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0

Entity
{
    id: root

    readonly property Layer layer: screenQuadLayer
    readonly property Effect lightPassEffect: LightPassEffect { }

    // dummy component to create a uniform
    Component
    {
        id: uniformComp
        Parameter {}
    }

    // We need to have the actual screen quad entity separate from the lights.
    // If the lights were sub-entities of this screen quad entity, they would
    // be affected by the rotation matrix, and their world positions would thus
    // be changed.
    Entity
    {
        components : [
            Layer { id: screenQuadLayer },

            PlaneMesh {
                width: 2.0
                height: 2.0
                meshResolution: Qt.size(2, 2)
            },

            Transform { // We rotate the plane so that it faces us
                rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90)
            },

            Material {
                id: lightPassMaterial

                property int lightsNumber: 1

                function addLight(fxItem, index, t)
                {
                    console.log("Adding light with index " + index)

                    fxItem.direction = Qt.binding(function() {
                        var lightMatrix = t.matrix.times(fxItem.panTransform.matrix)
                        lightMatrix = lightMatrix.times(fxItem.tiltTransform.matrix)
                        lightMatrix = lightMatrix.times(Qt.vector4d(0.0, -1.0, 0.0, 0.0))
                        return lightMatrix.toVector3d()
                    })

                    var parameters = [].slice.apply(lightPassMaterial.parameters)

                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].type".arg(index), value: 2 }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].position".arg(index), value: Qt.binding(function() { return fxItem.position }) }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].color".arg(index), value: Qt.binding(function() { return fxItem.lightColor }) }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].intensity".arg(index), value: Qt.binding(function() { return fxItem.intensity }) }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].constantAttenuation".arg(index), value: 1.0 }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].linearAttenuation".arg(index), value: 0.0 }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].quadraticAttenuation".arg(index), value: 0.0 }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].direction".arg(index), value: Qt.binding(function() { return fxItem.direction }) }))
                    parameters.push(uniformComp.createObject(lightPassMaterial,
                                    { name: "lightsArray[%1].cutOffAngle".arg(index), value: Qt.binding(function() { return fxItem.cutOff }) }))

                    // dump the uniform list
                    parameters.forEach(function (p) { console.log(p.name, '=', p.value); })

                    lightPassMaterial.parameters = parameters
                    lightPassMaterial.lightsNumber++
                }

                effect: lightPassEffect
                parameters: [
                    Parameter { name: "lightsArray[0].type"; value : 0 }, // Point Light
                    Parameter { name: "lightsArray[0].position"; value : Qt.vector3d(0, 10, 0) },
                    Parameter { name: "lightsArray[0].color"; value : Qt.rgba(1, 1, 1, 1) },
                    Parameter { name: "lightsArray[0].intensity"; value : 0.8 },
                    Parameter { name: "lightsArray[0].constantAttenuation"; value: 1.0 },
                    Parameter { name: "lightsArray[0].linearAttenuation"; value: 0.0 },
                    Parameter { name: "lightsArray[0].quadraticAttenuation"; value: 0.0 },
                    Parameter { name: "lightsNumber"; value : lightPassMaterial.lightsNumber }
                ]

                Component.onCompleted: View3D.quadReady(lightPassMaterial)
            }
        ]
    }
}
