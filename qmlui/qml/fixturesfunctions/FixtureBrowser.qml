/*
  Q Light Controller Plus
  FixtureBrowser.qml

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
import QtQuick.Controls 1.1

Rectangle {
    id: rectangle2
    anchors.fill: parent
    color: "transparent"

    Rectangle {
        id: searchBox
        y: 8
        z: 1
        height: 30
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        color: "#303030"
        radius: 10
        border.width: 2

        Image {
            id: borderImage1
            y: 3
            width: 24
            height: 24
            anchors.left: parent.left
            anchors.leftMargin: 6
            source: "qrc:/search.svg"
            sourceSize: Qt.size(24, 24)
        }

        TextEdit {
            id: textEdit1
            y: 3
            height: 24
            color: "#ffffff"
            text: qsTr("")
            font.pixelSize: 18
        }
    }

    ListView {
        id: manufacturerList
        //objectName: "fixtureList"
        x: 8
        z: 0
        anchors.top: searchBox.bottom
        anchors.topMargin: 6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        focus: true

        model: fixtureBrowser.manufacturers
        delegate: FixtureDelegate {
            onClicked: {
                fixtureBrowser.manufacturer = modelData
                mfText.label = modelData
                //console.log("Pressed:" + modelData)
                manufacturerList.visible = false
                fixtureList.model = fixtureBrowser.models
                fixtureList.currentIndex = -1
                fixtureArea.visible = true
            }
        }
    }

    Rectangle {
        id: fixtureArea
        visible: false
        color: "transparent"

        anchors.top: searchBox.bottom
        anchors.topMargin: 6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8

        Rectangle {
            id: manufBackLink
            height: 40
            z: 1
            anchors.right: parent.right
            anchors.left: parent.left
            color: "#333"

            Image {
                id: leftArrow
                rotation: 180
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:///arrow-right.svg"
                height: 26
                width: 18
            }

            RobotoText {
                id: mfText
                anchors.left: leftArrow.right
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                fontSize: 18
                fontBold: true
                labelColor: "#888"

            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    fixtureArea.visible = false
                    fxPropsRect.visible = false
                    manufacturerList.visible = true
                }
                onEntered: manufBackLink.color = "#444"
                onExited: manufBackLink.color = "#333"
            }
        }

        ListView {
            id: fixtureList
            x: 8
            z: 0
            anchors.top: manufBackLink.bottom
            anchors.topMargin: 6
            anchors.bottom: fxPropsRect.visible ? fxPropsRect.top : parent.bottom
            anchors.bottomMargin: 6
            anchors.right: parent.right
            anchors.left: parent.left
            highlight: Rectangle { color: "#0978FF"; radius: 5 }
            highlightMoveVelocity: 800
            focus: true

            delegate: FixtureDelegate {
                id: dlg
                visibleArrow: false
                manufacturer: fixtureBrowser.manufacturer
                onClicked: {
                    fixtureList.currentIndex = index
                    fixtureBrowser.model = modelData
                    fxPropsRect.fxManufacturer = fixtureBrowser.manufacturer
                    fxPropsRect.fxModel = modelData
                    fxPropsRect.fxName = modelData
                    fxPropsRect.fxModes = fixtureBrowser.modes
                    fxPropsRect.visible = true
                }
            }
        }

        FixtureProperties {
            id: fxPropsRect
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            visible: false
        }
    }
}
