/*
  Q Light Controller Plus
  DMXAddressWidget.qml

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
import QtQuick.Layouts 1.2

Rectangle
{
    id: dipBox
    height: UISettings.bigItemHeight * 1.2
    width: UISettings.bigItemHeight * 3

    color: "red"

    property int dipWidth: ((width - (10 * dipRow.spacing)) / 10)

    property bool editable: true
    property bool flipHorizontally: false
    property bool flipVertically: false
    property int currentValue: 1

    signal valueChanged(int value)

    RobotoText
    {
        id: actText
        x: dipRow.x
        y: flipVertically ? parent.height - height - 3 : 3
        height: dipBox.height * 0.20
        fontSize: Math.min(UISettings.textSizeDefault , height - 2)
        label: "ON"
    }

    Row
    {
        id: dipRow
        spacing: 10
        y: flipVertically ? 3: parent.height - height - 3
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater
        {
            model: 10
            delegate:
                Column
                {
                    RobotoText
                    {
                        visible: flipVertically
                        width: dipBox.dipWidth
                        height: dipBox.height * 0.20
                        label: flipHorizontally ? 10 - index : index + 1
                        fontSize: Math.min(UISettings.textSizeDefault , height - 2)
                        textHAlign: Text.AlignHCenter
                    }

                    Rectangle
                    {
                        id:  dipBitBox
                        height: dipBox.height * 0.50
                        width: dipBox.dipWidth
                        color: UISettings.bgMedium

                        /* normalize the bit index to always reflect the value bit position */
                        property int bitIndex: flipHorizontally ? 9 - index : index
                        property bool isHigh: currentValue & (1 << bitIndex)

                        Rectangle
                        {
                            y: flipVertically ? (dipBitBox.isHigh ? parent.height - height : 0) : (dipBitBox.isHigh ? 0 : parent.height - height)
                            width: dipBox.dipWidth
                            height: dipBox.dipWidth
                        }

                        MouseArea
                        {
                            enabled: editable
                            anchors.fill: parent
                            onClicked:
                            {
                                if (dipBitBox.bitIndex === 9)
                                {
                                    dipBitBox.isHigh = !dipBitBox.isHigh
                                    return
                                }

                                var newBit = dipBitBox.isHigh ? 0 : 1
                                if (newBit)
                                    dipBox.valueChanged(currentValue | (newBit << dipBitBox.bitIndex))
                                else
                                    dipBox.valueChanged(currentValue & ~(1 << dipBitBox.bitIndex))
                            }
                        }
                    }
                    RobotoText
                    {
                        visible: !flipVertically
                        width: dipBox.dipWidth
                        height: dipBox.height * 0.20
                        label: flipHorizontally ? 10 - index : index + 1
                        fontSize: Math.min(UISettings.textSizeDefault , height - 2)
                        textHAlign: Text.AlignHCenter
                    }
                }
        }
    }
}
