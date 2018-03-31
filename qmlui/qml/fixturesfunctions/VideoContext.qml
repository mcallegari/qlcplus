/*
  Q Light Controller Plus
  VideoContext.qml

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

import QtQuick 2.0
import QtMultimedia 5.8

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: ctxRoot
    anchors.fill: parent
    //width: 800
    //height: 600
    color: "black"

    // array of IDs of the contents currently playing
    property var mediaArray: []

    function addVideo(vContent)
    {
        videoComponent.createObject(ctxRoot,
                       {"video": vContent, "z": 1 });
        if (videoComponent.status !== Component.Ready)
            console.log("Video component is not ready !!")
        mediaArray.push(vContent.id)
    }

    function addPicture(pContent)
    {
        pictureComponent.createObject(ctxRoot,
                       {"picture": pContent, "z": 2 });
        if (pictureComponent.status !== Component.Ready)
            console.log("Picture component is not ready !!")
        mediaArray.push(pContent.id)
    }

    function removeContent(id)
    {
        var cIdx = mediaArray.indexOf(id)
        if (cIdx > -1)
            mediaArray.splice(cIdx, 1)

        console.log("Removing content with ID: " + id + ", count: " + mediaArray.length)

        if (mediaArray.length == 0)
            videoContent.destroyContext()
    }

    // Component representing a video content
    Component
    {
        id: videoComponent

        Rectangle
        {
            id: videoRect
            //anchors.fill: parent
            color: "black"
            opacity: video ? video.intensity : 1.0

            property VideoFunction video: null

            onVideoChanged:
            {
                if (video.customGeometry.width !== 0 && video.customGeometry.height !== 0)
                {
                    if (video.fullscreen)
                    {
                        x = video.customGeometry.x
                        y = video.customGeometry.y
                    }
                    width = video.customGeometry.width
                    height = video.customGeometry.height
                }
                else
                    anchors.fill = parent

                player.source = video.sourceUrl.indexOf("://") !== -1 ? video.sourceUrl : "file://" + video.sourceUrl
                console.log("QML video source: " + player.source)
            }

            transform: [
                Rotation
                {
                    origin.x: width / 2
                    origin.y: video.rotation.x > 0 ? height : 0
                    axis { x: 1; y: 0; z: 0 }
                    angle: video.rotation.x
                },
                Rotation
                {
                    origin.x: video.rotation.y > 0 ? 0 : width
                    origin.y: height / 2
                    axis { x: 0; y: 1; z: 0 }
                    angle: video.rotation.y
                },
                Rotation
                {
                    origin.x: width / 2
                    origin.y: height / 2
                    axis { x: 0; y: 0; z: 1 }
                    angle: video.rotation.z
                }
            ]

            MediaPlayer
            {
                id: player
                //source: "sourceURL"
                autoPlay: true

                onStopped:
                {
                    console.log("Video stopped")
                    ctxRoot.removeContent(video.id)
                }
            }

            VideoOutput
            {
                id: videoOutput
                source: player
                anchors.fill: parent
            }
        }
    }

    // Component representing a picture content
    Component
    {
        id: pictureComponent

        Image
        {
            property VideoFunction picture: null

            opacity: picture ? picture.intensity : 1.0

            onPictureChanged:
            {
                if (picture.customGeometry.width !== 0 && picture.customGeometry.height !== 0)
                {
                    if (picture.fullscreen)
                    {
                        x = picture.customGeometry.x
                        y = picture.customGeometry.y
                    }
                    width = picture.customGeometry.width
                    height = picture.customGeometry.height
                }
                else
                    anchors.fill = parent

                source = picture.sourceUrl.indexOf("://") !== -1 ? picture.sourceUrl : "file://" + picture.sourceUrl
                console.log("QML picture source: " + source)
            }

            transform: [
                Rotation
                {
                    origin.x: width / 2
                    origin.y: picture.rotation.x > 0 ? height : 0
                    axis { x: 1; y: 0; z: 0 }
                    angle: picture.rotation.x
                },
                Rotation
                {
                    origin.x: picture.rotation.y > 0 ? 0 : width
                    origin.y: height / 2
                    axis { x: 0; y: 1; z: 0 }
                    angle: picture.rotation.y
                },
                Rotation
                {
                    origin.x: width / 2
                    origin.y: height / 2
                    axis { x: 0; y: 0; z: 1 }
                    angle: picture.rotation.z
                }
            ]
        }
    }
}
