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
import QtMultimedia 5.14

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
    property var mediaItems: []

    function addVideo(vContent)
    {
        var item = videoComponent.createObject(ctxRoot, { "video": vContent });
        if (videoComponent.status !== Component.Ready)
            console.log("Video component is not ready !!")
        mediaArray.push(vContent.id)
        mediaItems.push(item)
    }

    function addPicture(pContent)
    {
        var item = pictureComponent.createObject(ctxRoot, { "picture": pContent });
        if (pictureComponent.status !== Component.Ready)
            console.log("Picture component is not ready !!")
        mediaArray.push(pContent.id)
        mediaItems.push(item)
    }

    function removeContent(id)
    {
        var cIdx = mediaArray.indexOf(id)
        if (cIdx > -1)
        {
            mediaItems[cIdx].stopPlayback()
            mediaArray.splice(cIdx, 1)
            mediaItems.splice(cIdx, 1)
        }

        console.log("Removing content with ID: " + id + ", count: " + mediaArray.length)

        if (mediaArray.length == 0)
            videoContent.destroyContext()
    }

    function translateUrl(url)
    {
        if (url.indexOf("://") !== -1)
            return url
        if (Qt.platform.os == "windows")
            return "file:///" + url
        else
            return "file://" + url
    }

    // Component representing a video content
    Component
    {
        id: videoComponent

        Rectangle
        {
            objectName: "media-" + video.id
            color: "black"
            opacity: video.intensity
            z: video.zIndex

            property VideoFunction video: null
            property alias volume: player.volume
            property vector3d rotation: video.rotation
            property rect geometry: video.customGeometry

            function stopPlayback()
            {
                player.stop()
            }

            onVideoChanged:
            {
                if (geometry.width !== 0 && geometry.height !== 0)
                {
                    if (video.fullscreen)
                    {
                        x = Qt.binding(function() { return geometry.x })
                        y = Qt.binding(function() { return geometry.y })
                    }
                    width = Qt.binding(function() { return geometry.width })
                    height = Qt.binding(function() { return geometry.height })
                }
                else
                    anchors.fill = parent

                player.source = translateUrl(video.sourceUrl)
                console.log("QML video source: " + player.source)
            }

            transform: [
                Rotation
                {
                    origin.x: width / 2
                    origin.y: rotation.x > 0 ? height : 0
                    axis { x: 1; y: 0; z: 0 }
                    angle: rotation.x
                },
                Rotation
                {
                    origin.x: rotation.y > 0 ? 0 : width
                    origin.y: height / 2
                    axis { x: 0; y: 1; z: 0 }
                    angle: rotation.y
                },
                Rotation
                {
                    origin.x: width / 2
                    origin.y: height / 2
                    axis { x: 0; y: 0; z: 1 }
                    angle: rotation.z
                }
            ]

            MediaPlayer
            {
                id: player
                //source: "sourceURL"
                autoPlay: true
                volume: video.intensity
                /* Qt 6.8
                audioOutput:
                    AudioOutput {
                        volume: video.intensity
                    }
                */

                onStopped:
                {
                    if (video.runOrder === QLCFunction.Loop)
                    {
                        console.log("Video loop")
                        player.play()
                    }
                    else
                    {
                        console.log("Video stopped")
                        ctxRoot.removeContent(video.id)
                    }
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
            objectName: "media-" + picture.id
            opacity: picture.intensity
            z: picture.zIndex

            property VideoFunction picture: null
            property vector3d rotation: picture.rotation
            property rect geometry: picture.customGeometry

            function stopPlayback() { }

            onPictureChanged:
            {
                if (geometry.width !== 0 && geometry.height !== 0)
                {
                    if (picture.fullscreen)
                    {
                        x = Qt.binding(function() { return geometry.x })
                        y = Qt.binding(function() { return geometry.y })
                    }
                    width = Qt.binding(function() { return geometry.width })
                    height = Qt.binding(function() { return geometry.height })
                }
                else
                    anchors.fill = parent

                source = translateUrl(picture.sourceUrl)
                console.log("QML picture source: " + source)
            }

            transform: [
                Rotation
                {
                    origin.x: width / 2
                    origin.y: rotation.x > 0 ? height : 0
                    axis { x: 1; y: 0; z: 0 }
                    angle: rotation.x
                },
                Rotation
                {
                    origin.x: rotation.y > 0 ? 0 : width
                    origin.y: height / 2
                    axis { x: 0; y: 1; z: 0 }
                    angle: rotation.y
                },
                Rotation
                {
                    origin.x: width / 2
                    origin.y: height / 2
                    axis { x: 0; y: 0; z: 1 }
                    angle: rotation.z
                }
            ]
        }
    }
}
