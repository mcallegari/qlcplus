/*
  Q Light Controller Plus
  Generic3DItem.qml

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

import QtQuick

import Qt3D.Core
import Qt3D.Render
import Qt3D.Extras

import org.qlcplus.classes 1.0
import "."

Entity
{
    id: genericEntity

    property int itemID: -1
    property alias itemSource: eSceneLoader.source
    property bool isSelected: false

    /** Width in metres of one repeating section of a tileable mesh, or 0 for
     *  the ordinary meshes that simply stretch when scaled.
     *  Set by MainView3D::initializeItem */
    property real tileWidthX: 0

    /** Number of sections a tileable mesh is repeated into along the X axis.
     *  Set by MainView3D::updateGenericItemScale, which rounds the item X
     *  scale to a whole number of sections */
    property int tileCountX: 1

    readonly property int sectionCount: tileWidthX > 0 ? Math.max(1, tileCountX) : 1

    property Layer sceneLayer
    property Effect sceneEffect

    property Transform transform: Transform { }

    components: [ transform ]

    onSectionCountChanged: rebuildSections()

    /** Create the sections that follow the first one. Section 0 is declared
     *  below, so that the item always has a mesh even before any scaling */
    function rebuildSections()
    {
        for (var i = 0; i < extraSections.length; i++)
            extraSections[i].destroy()

        extraSections = []

        for (var s = 1; s < sectionCount; s++)
            extraSections.push(sectionComponent.createObject(sectionsRoot,
                                                            { "xOffset": s * tileWidthX,
                                                              "source": eSceneLoader.source }))
    }

    property var extraSections: []

    Component
    {
        id: sectionComponent

        Entity
        {
            property real xOffset
            property alias source: sectionLoader.source

            SceneLoader
            {
                id: sectionLoader

                onStatusChanged: (status) =>
                {
                    if (status === SceneLoader.Ready)
                        View3D.initializeItemTile(genericEntity.itemID, sectionLoader)
                }
            }

            Transform
            {
                id: sectionTransform
                translation: Qt.vector3d(xOffset, 0, 0)
            }

            components: [ sectionLoader, sectionTransform ]
        }
    }

    // `transform` scales X by the number of sections, so that the selection box
    // and the picking volume cover the whole item. Undo it here: the item has to
    // grow along X by repeating the mesh, not by stretching it
    Entity
    {
        Transform
        {
            id: unscaleTransform
            scale3D: Qt.vector3d(1 / genericEntity.sectionCount, 1, 1)
        }

        components: [ unscaleTransform ]

        // Sections are laid out from the origin towards +X, so shift the run to
        // keep it centred on the item, where the selection box also is
        Entity
        {
            id: sectionsRoot

            Transform
            {
                id: sectionsTransform
                translation: Qt.vector3d(-(genericEntity.sectionCount - 1) * genericEntity.tileWidthX / 2, 0, 0)
            }

            components: [ sectionsTransform ]

            // Section 0. It carries the initialization of the item as a whole,
            // and is deliberately left untransformed: initializeItem sums up the
            // transforms it finds from here down to work out the bounding volume
            Entity
            {
                SceneLoader
                {
                    id: eSceneLoader

                    onStatusChanged: (status) =>
                    {
                        if (status === SceneLoader.Ready)
                            View3D.initializeItem(genericEntity.itemID, genericEntity, eSceneLoader)
                    }
                }

                components: [ eSceneLoader ]
            }
        }
    }
}
