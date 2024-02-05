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

import "."

CustomPopupDialog
{
    id: popupRoot

    visible: true
    width: mainView.width / 2
    title: qsTr("Disclaimer")
    standardButtons: Dialog.Ok

    contentItem:
        GridLayout
        {
            columnSpacing: UISettings.iconSizeMedium

            Text
            {
                color: "red"
                font.family: "FontAwesome"
                font.pointSize: 50
                text: FontAwesome.fa_warning
            }

            Text
            {
                color: UISettings.fgMain
                linkColor: "#8AC800"
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                text: "<h3>!! Warning !!</h3>\n" +
                      "This is a very preliminary version of QLC+ 5.<br>" +
                      "It's incomplete, it will crash, it will corrupt your existing projects.<br>" +
                      "At this stage, you can look around, play with it, but be aware:<br>" +
                      "<b><u>it is NOT ready for production</u></b>.<br><br>" +
                      "The development progress report can be found at " +
                      "<a href='https://drive.google.com/open?id=1J1BK0pYCsLVBfLpDZ-GqpNgUzbgTwmhf9zOjg4J_MWg'>" +
                      "this link</a>.<br>" +
                      "Reports of what is marked as 'Work in progress' or 'Missing'<br>" +
                      " will be ignored. You've been warned."
                onLinkActivated: Qt.openUrlExternally(link)

                MouseArea
                {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            Text
            {
                color: "red"
                font.family: "FontAwesome"
                font.pointSize: 50
                text: FontAwesome.fa_warning
            }
        }
}
