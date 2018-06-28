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

    ConeMesh
    {
        id: spotlightConeMesh
        length: 1.0
        bottomRadius: 9.9
    }

    Material
    {
        id: spotlightConeMaterial

        effect: coneEffect
        parameters: [
            Parameter { name: "meshColor"; value: "blue" }
        ]

        function bindFixture(fxItem)
        {
            spotlightConeMesh.length =  Qt.binding(function() { return fxItem.distCutoff })
            spotlightConeTransform.translation = Qt.binding(function() { return Qt.vector3d(0, -0.5 * fxItem.distCutoff, 0) })
            spotlightConeMesh.bottomRadius = Qt.binding(function() { return fxItem.coneRadius })

            var parameters = [].slice.apply(spotlightConeMaterial.parameters)

            parameters.push(uniformComp.createObject(spotlightConeMaterial,
                            { name: "raymarchSteps", value: Qt.binding(function() { return fxItem.raymarchSteps }) }))

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
                         { name: "uLightTanCutoffAngle",
                           value: Qt.binding(function() { return fxItem.coneRadius /  fxItem.distCutoff }) }))

            // dump the uniform list (uncomment for debug purposes)
            // parameters.forEach(function (p) { console.log(p.name, '=', p.value); })

            spotlightConeMaterial.parameters = parameters
        }
    }

    Transform
    {
        id: spotlightConeTransform
        translation: Qt.vector3d(0, 0, 0)
    }

    components: [
        coneLayer,
        spotlightConeMaterial,
        spotlightConeMesh,
        spotlightConeTransform
    ]
}
