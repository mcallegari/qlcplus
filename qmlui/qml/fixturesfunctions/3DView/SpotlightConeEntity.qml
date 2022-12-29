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
    enabled: mtl.fxItem && mtl.fxItem.enabled && mtl.fxItem.lightIntensity ? true : false
    property Effect coneEffect
    property Layer coneLayer: null
    property alias coneMaterial: mtl
    property ConeMesh spotlightConeMesh: null
    property alias fxEntity: mtl.fxItem

    Material
    {
        id: mtl
        effect: coneEffect

        property Entity fxItem: null

        parameters: [
            Parameter { name: "diffuse"; value: "blue" },
            Parameter { name: "specular"; value: "black" },
            Parameter { name: "shininess"; value: 1.0 },
            Parameter { name: "bloom"; value: 0 },

            Parameter { name: "raymarchSteps"; value: mtl.fxItem ? mtl.fxItem.raymarchSteps : 0 },
            Parameter { name: "customModelMatrix";
                        value: {
                            var m = Qt.matrix4x4()

                            if (mtl.fxItem === null)
                                return m

                            var panRot = mtl.fxItem.invertedPan ? mtl.fxItem.panMaxDegrees - mtl.fxItem.panRotation : mtl.fxItem.panRotation
                            var tiltRot = mtl.fxItem.invertedTilt ? mtl.fxItem.tiltMaxDegrees - mtl.fxItem.tiltRotation : mtl.fxItem.tiltRotation

                            m.translate(mtl.fxItem.lightPos.times(+1.0))
                            m = m.times(mtl.fxItem.lightMatrix)
                            m.rotate(panRot, Qt.vector3d(0, 1, 0))
                            m.rotate(tiltRot, Qt.vector3d(1, 0, 0))
                            m.translate(Qt.vector3d(0, -0.5 * mtl.fxItem.distCutoff - 0.5 * mtl.fxItem.headLength, 0))
                            return m
                        }},
            Parameter { name: "coneTopRadius"; value: mtl.fxItem ? mtl.fxItem.coneTopRadius : 0 },
            Parameter { name: "coneBottomRadius"; value: mtl.fxItem ? mtl.fxItem.coneBottomRadius : 0 },
            Parameter { name: "coneDistCutoff"; value: mtl.fxItem ? mtl.fxItem.distCutoff : 0 },
            Parameter { name: "headLength"; value: mtl.fxItem ? mtl.fxItem.headLength : 0 },
            Parameter { name: "lightIntensity"; value: mtl.fxItem ? mtl.fxItem.lightIntensity : 0 },
            Parameter { name: "lightColor"; value: mtl.fxItem ? mtl.fxItem.lightColor : Qt.rgba(0,0,0,0) },
            Parameter { name: "lightDir"; value: mtl.fxItem ? mtl.fxItem.lightDir : Qt.vector3d(0,0,0) },
            Parameter { name: "lightPos"; value: mtl.fxItem ? mtl.fxItem.lightPos : Qt.vector3d(0,0,0) },
            Parameter { name: "lightViewMatrix"; value: mtl.fxItem ? mtl.fxItem.lightViewMatrix : Qt.matrix4x4() },
            Parameter { name: "lightProjectionMatrix";
                        value: mtl.fxItem ? mtl.fxItem.lightProjectionMatrix : Qt.matrix4x4() },
            Parameter { name: "lightViewProjectionMatrix";
                        value: mtl.fxItem ? mtl.fxItem.lightViewProjectionMatrix : Qt.matrix4x4() },
            Parameter { name: "lightViewProjectionScaleAndOffsetMatrix";
                        value: mtl.fxItem ? mtl.fxItem.lightViewProjectionScaleAndOffsetMatrix : Qt.matrix4x4() },

            Parameter { name: "smokeAmount"; value: View3D.smokeAmount },
            Parameter { name: "goboTex"; value: mtl.fxItem ? mtl.fxItem.goboTexture : null },
            Parameter { name: "goboRotation";
                        value: {
                             var theta = mtl.fxItem ? mtl.fxItem.goboRotation : 0
                             return  Qt.vector4d(Math.cos(theta), -Math.sin(theta), Math.sin(theta), Math.cos(theta));
                         } }
        ]
    }

    components: [
        coneLayer,
        coneMaterial,
        spotlightConeMesh,
    ]
}
