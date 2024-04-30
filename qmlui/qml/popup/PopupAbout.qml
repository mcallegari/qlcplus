/*
  Q Light Controller Plus
  PopupAbout.qml

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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 2
    title: qsTr("Information")
    standardButtons: Dialog.Close

    contentItem:
        GridLayout
        {
            columnSpacing: UISettings.iconSizeMedium

            Image
            {
                source: "qrc:/qlcplus.svg"
                width: UISettings.iconSizeDefault * 3
                height: width
                sourceSize: Qt.size(width, height)
            }

            Text
            {
                color: UISettings.fgMain
                linkColor: "#8AC800"
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                text: "<h3>" + qlcplus.appName + "<br>" + qlcplus.appVersion + "</h3>\n" +
                      "Copyright Ⓒ <b>Heikki Junnila, Massimo Callegari</b> " + qsTr("and contributors") + "<br>" +
                      qsTr("Website") + ": <a href='https://www.qlcplus.org'>https://www.qlcplus.org</a><br><br>" +
                      qsTr("This application is licensed under the terms of the") +
                      " <a href='https://www.apache.org/licenses/LICENSE-2.0'>" +
                      qsTr("Apache 2.0 license") + "</a>."
                onLinkActivated: Qt.openUrlExternally(link)
                Layout.fillWidth: true
                wrapMode: Text.WordWrap

                MouseArea
                {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }
        }
}
