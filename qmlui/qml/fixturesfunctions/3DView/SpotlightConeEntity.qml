/*
  Q Light Controller Plus
  SpotlightConeEntity.qml

  Copyright (c) Eric Arnebäck

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

import QtQuick 2.0

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity
{
    id: spotlightConeEntity
    property Effect coneEffect
    property Layer coneLayer: null
    property alias coneMaterial: spotlightConeMaterial

    Component
    {
        id: uniformComp
        Parameter {}
    }

    property ConeMesh spotlightConeMesh: null

    Material
    {
        id: spotlightConeMaterial

        effect: coneEffect
        parameters: [
            Parameter { name: "meshColor"; value: "blue" }
        ]

        function bindFixture(fxItem)
        {
            var parameters = [].slice.apply(spotlightConeMaterial.parameters)

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "raymarchSteps", value: Qt.binding(function() { return fxItem.raymarchSteps }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "customModelMatrix", value: Qt.binding(
                                function() { 
                                    var m = Qt.matrix4x4();
                                    m.translate(fxItem.lightPos.times(+1.0));
                                    m = m.times(fxItem.lightMatrix)
                                    m.rotate(fxItem.panRotation, Qt.vector3d(0, 1, 0));
                                    m.rotate(fxItem.tiltRotation, Qt.vector3d(1, 0, 0));
                                    m.translate( Qt.vector3d(0, -0.5 * fxItem.distCutoff - 0.5 * fxItem.headLength, 0));
                                    return m;
                             })}))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "coneTopRadius", value: Qt.binding(function() { return fxItem.coneTopRadius }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "headLength", value: Qt.binding(function() { return fxItem.headLength }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "coneBottomRadius", value: Qt.binding(function() { return fxItem.coneBottomRadius }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "coneDistCutoff", value: Qt.binding(function() { return fxItem.distCutoff }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightDir", value: Qt.binding(function() { return fxItem.lightDir }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightPos", value: Qt.binding(function() { return fxItem.lightPos }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightViewMatrix", value: Qt.binding(function() { return fxItem.lightViewMatrix }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightProjectionMatrix", value: Qt.binding(function() { return fxItem.lightProjectionMatrix }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightViewProjectionMatrix",
                              value: Qt.binding(function() { return fxItem.lightViewProjectionMatrix }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightViewProjectionScaleAndOffsetMatrix",
                              value: Qt.binding(function() { return fxItem.lightViewProjectionScaleAndOffsetMatrix }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightIntensity", value: Qt.binding(function() { return fxItem.lightIntensity }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "lightColor", value: Qt.binding(function() { return fxItem.lightColor }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "goboTex", value: Qt.binding(function() { return fxItem.goboTexture }) }))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "goboRotation", value: Qt.binding(
                                function() {
                                    var theta = fxItem.goboRotation
                                    return  Qt.vector4d(Math.cos(theta), -Math.sin(theta), Math.sin(theta), Math.cos(theta));
                                })}))

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "smokeAmount", value: Qt.binding(function() { return View3D.smokeAmount }) }))  

            // dump the uniform list (uncomment for debug purposes)
            // parameters.forEach(function (p) { console.log(p.name, '=', p.value); })

            spotlightConeMaterial.parameters = parameters
        }
    }

    components: [
        coneLayer,
        spotlightConeMaterial,
        spotlightConeMesh,
    ]
}
