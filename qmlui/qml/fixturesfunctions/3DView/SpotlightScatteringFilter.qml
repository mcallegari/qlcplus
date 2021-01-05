/*
  Q Light Controller Plus
  SpotlightScatteringFilter.qml

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

import Qt3D.Core 2.0
import Qt3D.Render 2.0

import QtQuick 2.0

// Lights pass technique
TechniqueFilter
{
    property alias spotlightScatteringLayer: slLayerFilter.layers
    property alias frameTarget: slRenderTarget.target
    property DepthTarget frontDepth
    property GBuffer gBuffer
    property Texture2D shadowTex: null
    property bool useShadows: true

    property Entity fixtureItem

    parameters: [
        Parameter { name: "frontDepthTex"; value: frontDepth ? frontDepth.positionTex : null },
        Parameter { name: "depthTex"; value: gBuffer ? gBuffer.depth : null },
        Parameter { name: "lightColor"; value:  fixtureItem ? fixtureItem.lightColor : Qt.rgba(0, 0, 0, 1) },
        Parameter { name: "lightIntensity"; value: fixtureItem ? fixtureItem.lightIntensity : 0.0 },
        Parameter { name: "shadowTex"; value: shadowTex },
        Parameter { name: "useShadows"; value: (useShadows ? 1 : 0) }
    ]

    RenderStateSet
    {
        // Render cone
        renderStates: [
            BlendEquation { blendFunction: BlendEquation.Add },
            BlendEquationArguments
            {
                sourceRgb: BlendEquationArguments.One
                destinationRgb: BlendEquationArguments.One

                sourceAlpha: BlendEquationArguments.One
                destinationAlpha: BlendEquationArguments.One

            },
            CullFace { mode: CullFace.Front }
        ]
        LayerFilter
        {
            id: slLayerFilter

            RenderTargetSelector
            {
                id: slRenderTarget

                RenderPassFilter
                {
                    matchAny: FilterKey { name: "pass"; value: "spotlight_scattering" }
                }
            }
        }
    }
} // TechniqueFilter
