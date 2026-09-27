/*
  Q Light Controller Plus
  LightEntity.qml

  Copyright (c) Massimo Callegari, Eric Arnebäck

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

import QtQuick

import Qt3D.Core
import Qt3D.Render
import Qt3D.Extras

import "Math3DView.js" as Math3D
import "."

/*
  A single light emitter of a multi-beam fixture (see MultiBeams3DItem).
  It plays the role Fixture3DItem plays for a one-head fixture: it owns the
  three spotlight cones and exposes the uniforms the spotlight shaders read.
  It has no geometry of its own - the bar mesh is drawn by the parent item.
*/
Entity
{
    id: beamEntity

    property int headIndex
    property real dimmerValue: 0
    property real shutterValue: 1.0
    property real lightIntensity: dimmerValue * shutterValue

    property color lightColor: Qt.rgba(0, 0, 0, 1)
    property vector3d lightPos: Qt.vector3d(0, 0, 0)
    property vector3d lightDir: Qt.vector3d(0, 0, 0)

    /* Orientation of the emitter. SpotlightConeEntity and the shading/scattering
       shaders need a pan/tilt angle to place and aim the cone; when they are
       missing the cone model matrix collapses and nothing is drawn. LED bars
       never pan, and tilt (motorized bars) is pushed down from the parent item. */
    property real panRotation: 0
    property real tiltRotation: 0

    /* This MUST stay an int. It ends up in the "raymarchSteps" uniform of
       spotlight_scattering.frag, which is declared as an int, and Qt3D does not
       convert: a QML real is stored as a float and its raw bytes are handed to
       glUniform1iv, so the shader would read the *bit pattern* of the float
       (40.0 becomes 1109393408) and ray march for over a billion iterations.
       That hangs the GPU, which is why the scattering cone below used to be
       commented out with a "this hangs your PC" note. Fixture3DItem declares
       the same property as an int. */
    property int raymarchSteps: 0

    property real cutoffAngle
    property real distCutoff
    property real headLength
    property real coneTopRadius
    /* Derived exactly like Fixture3DItem so that changing the beam aperture
       (zoom) keeps the cone geometry consistent. */
    property real coneBottomRadius: distCutoff * Math.tan(cutoffAngle) + coneTopRadius
    property Texture2D goboTexture
    property real goboRotation: 0

    /* Spotlight matrices - same math as Fixture3DItem.qml. These are read as
       uniforms by SpotlightConeEntity; the per-head lightPos and lightMatrix
       are provided by the parent through setHeadLightProps(). */
    property matrix4x4 lightMatrix
    property matrix4x4 lightViewMatrix:
        Math3D.getLightViewMatrix(lightMatrix, panRotation, tiltRotation, lightPos)
    // getLightProjectionMatrix() divides by (coneBottomRadius / coneTopRadius - 1),
    // so a not-yet-known aperture would produce a degenerate matrix
    property matrix4x4 lightProjectionMatrix:
        coneTopRadius > 0 && coneBottomRadius > coneTopRadius ?
            Math3D.getLightProjectionMatrix(distCutoff, coneBottomRadius, coneTopRadius, headLength, cutoffAngle) :
            Qt.matrix4x4()
    property matrix4x4 lightViewProjectionMatrix: lightProjectionMatrix.times(lightViewMatrix)
    property matrix4x4 lightViewProjectionScaleAndOffsetMatrix:
        Math3D.getLightViewProjectionScaleOffsetMatrix(lightViewProjectionMatrix)

    readonly property Layer spotlightShadingLayer: Layer { }
    readonly property Layer outputDepthLayer: Layer { }
    readonly property Layer spotlightScatteringLayer: Layer { }

    /* Shadow map of this emitter. It is what makes the beam stop at the first
       surface it hits: spotlight_shading.frag has no distance bound of its own,
       so without a shadow map the cone lights every G-buffer texel inside its
       projection - through walls and everything behind them.

       Same size as Fixture3DItem, deliberately. The frustum covers exactly the
       cone, so what matters is resolution per degree of aperture, and a bar is
       not the narrow emitter it might seem: real definitions of this type carry
       lenses of 40 and even 120 degrees, against the 30 of a typical PAR. A
       smaller map is therefore coarser here than for a single head fixture, not
       finer, and a coarse map is what makes a wide beam fail its own shadow
       test and emit nothing at all. */
    property int shadowMapSize: 1024

    property Texture2D depthTex:
        Texture2D
        {
            width: shadowMapSize
            height: shadowMapSize
            format: Texture.D32F
            generateMipMaps: false
            magnificationFilter: Texture.Nearest
            minificationFilter: Texture.Nearest
            wrapMode
            {
                x: WrapMode.ClampToEdge
                y: WrapMode.ClampToEdge
            }
        }

    property RenderTarget shadowMap:
        RenderTarget
        {
            attachments: [
                RenderTargetOutput
                {
                    attachmentPoint: RenderTargetOutput.Depth
                    texture: depthTex
                }
            ] // attachments
        }

    function setupScattering(sceneEntity)
    {
        shadingCone.coneEffect = sceneEntity.spotlightShadingEffect
        shadingCone.parent = sceneEntity
        shadingCone.spotlightConeMesh = sceneEntity.coneMesh

        // Volumetric beam in the air. As in Fixture3DItem, its cost is governed
        // by raymarchSteps, which is 0 on Low render quality.
        scatteringCone.coneEffect = sceneEntity.spotlightScatteringEffect
        scatteringCone.parent = sceneEntity
        scatteringCone.spotlightConeMesh = sceneEntity.coneMesh

        outDepthCone.coneEffect = sceneEntity.outputFrontDepthEffect
        outDepthCone.parent = sceneEntity
        outDepthCone.spotlightConeMesh = sceneEntity.coneMesh
    }

    /* Must be called before this entity is destroyed: setupScattering() moves the
       three cones under the scene root, so they do not go away with their
       declaring entity and would be left in the scene bound to a dead object. */
    function cleanupScattering()
    {
        if (shadingCone)
            shadingCone.destroy()
        if (scatteringCone)
            scatteringCone.destroy()
        if (outDepthCone)
            outDepthCone.destroy()
    }

    /* Cone meshes used for scattering. These get re-parented to
       the main Scene entity via setupScattering */
    SpotlightConeEntity
    {
        id: shadingCone
        coneLayer: spotlightShadingLayer
        fxEntity: beamEntity
    }
    SpotlightConeEntity
    {
        id: scatteringCone
        coneLayer: spotlightScatteringLayer
        fxEntity: beamEntity
    }
    SpotlightConeEntity
    {
        id: outDepthCone
        coneLayer: outputDepthLayer
        fxEntity: beamEntity
    }
} // Entity
