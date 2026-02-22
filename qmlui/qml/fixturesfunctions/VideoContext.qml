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

import QtQuick
import QtMultimedia

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

    Keys.onEscapePressed: stopAllContentFromUI()

    function addVideo(vContent, fadeIn, fadeOut)
    {
        var item = videoComponent.createObject(ctxRoot,
                                               { "video": vContent,
                                                 "fadeIn": (fadeIn > 0 ? fadeIn : 0),
                                                 "fadeOut": (fadeOut > 0 ? fadeOut : 0) });
        if (videoComponent.status !== Component.Ready)
            console.log("Video component is not ready !!")

        if (item.fadeIn > 0)
            item.requestFadeIn(item.fadeIn)

        mediaArray.push(vContent.id)
        mediaItems.push(item)
    }

    function addPicture(pContent, fadeIn, fadeOut)
    {
        var item = pictureComponent.createObject(ctxRoot,
                                                 { "picture": pContent,
                                                   "fadeIn": (fadeIn > 0 ? fadeIn : 0),
                                                   "fadeOut": (fadeOut > 0 ? fadeOut : 0) });
        if (pictureComponent.status !== Component.Ready)
            console.log("Picture component is not ready !!")

        if (item.fadeIn > 0)
            item.requestFadeIn(item.fadeIn)

        mediaArray.push(pContent.id)
        mediaItems.push(item)
    }

    function pauseContent(id, enable)
    {
        var cIdx = mediaArray.indexOf(id)
        if (cIdx > -1)
        {
            if (enable)
                mediaItems[cIdx].pausePlayback()
            else
                mediaItems[cIdx].resumePlayback()
        }
    }

    function cleanupItem(item)
    {
        var cIdx = mediaItems.indexOf(item)
        if (cIdx > -1)
        {
            var id = mediaArray[cIdx]
            mediaItems[cIdx].stopPlayback()
            mediaItems[cIdx].destroy()
            mediaArray.splice(cIdx, 1)
            mediaItems.splice(cIdx, 1)
        }

        if (mediaArray.length === 0)
            videoContent.destroyContext()
    }

    function removeContent(id)
    {
        var cIdx = mediaArray.indexOf(id)
        if (cIdx === -1)
            return

        var item = mediaItems[cIdx]
        if (item.removalRequested)
            return

        item.removalRequested = true
        if (item.fadeOut > 0)
            item.requestFadeOut(item.fadeOut)
        else
            cleanupItem(item)
    }

    function cleanupContent(id)
    {
        var cIdx = mediaArray.indexOf(id)
        if (cIdx > -1)
            cleanupItem(mediaItems[cIdx])
    }

    function stopAllContentFromUI()
    {
        // Iterate over a copy because stopFromUI() triggers async removal.
        var items = mediaItems.slice()
        for (var i = 0; i < items.length; i++)
        {
            var item = items[i]
            if (item && item.video)
                item.video.stopFromUI()
            else if (item && item.picture)
                item.picture.stopFromUI()
        }
    }

    function translateUrl(url)
    {
        if (url.indexOf("://") !== -1)
            return url

        if (Qt.platform.os === "windows")
            return "file:///" + url
        else
            return "file://" + url
    }

    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onPressed: ctxRoot.forceActiveFocus()
    }

    // Component representing a video content
    Component
    {
        id: videoComponent

        Rectangle
        {
            id: mediaRect
            objectName: "media-" + video.id
            color: "black"
            opacity: video ? effectiveIntensity * fadeMultiplier : 1.0
            z: video ? video.zIndex : 1

            property VideoFunction video: null
            property alias volume: player.audioOutput.volume
            property vector3d rotation: video.rotation
            property rect geometry: video.customGeometry
            property int fadeIn: 0
            property int fadeOut: 0
            property real fadeMultiplier: 1.0
            property int fadeState: 0 // 0 idle, 1 fade in, 2 fade out
            property bool removalRequested: false
            property real frozenIntensity: -1.0
            property real effectiveIntensity: (fadeState === 2 && frozenIntensity >= 0.0) ?
                                                  frozenIntensity :
                                                  (video ? video.intensity : 1.0)

            function requestFadeIn(duration)
            {
                if (fadeState === 1)
                    return

                if (duration <= 0)
                {
                    fadeState = 0
                    fadeMultiplier = 1.0
                    return
                }

                fadeState = 1
                frozenIntensity = -1.0
                fadeMultiplier = 0.0
                fadeAnim.to = 1.0
                fadeAnim.duration = duration
                fadeAnim.start()
            }

            function requestFadeOut(duration)
            {
                if (fadeState === 2)
                    return

                if (duration <= 0)
                {
                    frozenIntensity = video ? video.intensity : 1.0
                    fadeState = 0
                    fadeMultiplier = 0.0
                    ctxRoot.cleanupItem(mediaRect)
                    return
                }

                frozenIntensity = video ? video.intensity : 1.0
                fadeState = 2
                fadeAnim.to = 0.0
                fadeAnim.duration = duration
                fadeAnim.start()
            }

            function stopPlayback()
            {
                player.stop()
            }

            function pausePlayback()
            {
                player.pause()
            }

            function resumePlayback()
            {
                player.play()
            }

            NumberAnimation on fadeMultiplier
            {
                id: fadeAnim
                running: false
                onStopped:
                {
                    var state = mediaRect.fadeState
                    mediaRect.fadeState = 0
                    mediaRect.frozenIntensity = -1.0
                    if (state === 2)
                        ctxRoot.cleanupItem(mediaRect)
                }
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
                autoPlay: true
                audioOutput:
                    AudioOutput {
                        volume: mediaRect.effectiveIntensity * mediaRect.fadeMultiplier
                    }

                videoOutput: pVideoOutput

                onMediaStatusChanged:
                {
                    if (mediaStatus == MediaPlayer.EndOfMedia)
                    {
                        if (mediaRect.video.runOrder === QLCFunction.Loop)
                        {
                            player.play()
                        }
                        else
                        {
                            mediaRect.video.stopFromUI()
                        }
                    }
                }
            }

            VideoOutput
            {
                id: pVideoOutput
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
            id: pictureItem
            objectName: "media-" + picture.id
            opacity: picture ? effectiveIntensity * fadeMultiplier : 1.0
            z: picture ? picture.zIndex : 1

            property VideoFunction picture: null
            property vector3d rotation: picture.rotation
            property rect geometry: picture.customGeometry
            property int fadeIn: 0
            property int fadeOut: 0
            property real fadeMultiplier: 1.0
            property int fadeState: 0 // 0 idle, 1 fade in, 2 fade out
            property bool removalRequested: false
            property real frozenIntensity: -1.0
            property real effectiveIntensity: (fadeState === 2 && frozenIntensity >= 0.0) ?
                                                  frozenIntensity :
                                                  (picture ? picture.intensity : 1.0)

            function requestFadeIn(duration)
            {
                if (fadeState === 1)
                    return

                if (duration <= 0)
                {
                    fadeState = 0
                    fadeMultiplier = 1.0
                    return
                }

                fadeState = 1
                frozenIntensity = -1.0
                fadeMultiplier = 0.0
                picFadeAnim.to = 1.0
                picFadeAnim.duration = duration
                picFadeAnim.start()
            }

            function requestFadeOut(duration)
            {
                if (fadeState === 2)
                    return

                if (duration <= 0)
                {
                    frozenIntensity = picture ? picture.intensity : 1.0
                    fadeState = 0
                    fadeMultiplier = 0.0
                    ctxRoot.cleanupItem(pictureItem)
                    return
                }

                frozenIntensity = picture ? picture.intensity : 1.0
                fadeState = 2
                picFadeAnim.to = 0.0
                picFadeAnim.duration = duration
                picFadeAnim.start()
            }

            function stopPlayback() { }
            function pausePlayback() { }
            function resumePlayback() { }

            NumberAnimation on fadeMultiplier
            {
                id: picFadeAnim
                running: false
                onStopped:
                {
                    var state = pictureItem.fadeState
                    pictureItem.fadeState = 0
                    pictureItem.frozenIntensity = -1.0
                    if (state === 2)
                        ctxRoot.cleanupItem(pictureItem)
                }
            }

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
