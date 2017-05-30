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

    property int videoCount: 0

    function addVideo(vContent)
    {
        videoComponent.createObject(ctxRoot,
                       {"video": vContent });
        if (videoComponent.status !== Component.Ready)
            console.log("Video component is not ready !!")
        videoCount++
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

            //transform: Rotation { origin.x: width / 2; origin.y: height / 2; axis { x: 0; y: 1; z: 0 } angle: -45 }

            MediaPlayer
            {
                id: player
                //source: "sourceURL"
                autoPlay: true

                onStopped:
                {
                    console.log("Video stopped")
                    videoCount--
                    if (videoCount == 0)
                        videoContent.destroyContext()
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
}
