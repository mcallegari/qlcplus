/*
  Q Light Controller Plus
  ColorToolBasic.qml

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

Rectangle
{
    id: rootBox
    width: 330
    height: 370
    color: "#444"
    border.color: "#222"
    border.width: 2

    property color selectedColor

    signal colorChanged(real r, real g, real b, real white, real amber, real uv)
    signal released()

    onSelectedColorChanged:
    {
        colorChanged(selectedColor.r, selectedColor.g, selectedColor.b, 0, 0, 0)
    }

    property var baseColors: [ 0xFF0000, 0xFF9900, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0x9900FF, 0xFF00FF ]

    function getHTMLColor(r, g, b)
    {
        return "#" + r.toString(16) + g.toString(16) + b.toString(16);
    }

    function getBaseHTMLColor(index)
    {
        var bcStr = baseColors[index].toString(16);
        return "#" + "000000".substr(0, 6 - bcStr.length) + bcStr;
    }

    function getShadedColor(colIndex, index)
    {
        var bcStr = baseColors[colIndex].toString(16);
        var htmlColor = "#" + "000000".substr(0, 6 - bcStr.length) + bcStr;
        if (index < 3)
            return Qt.lighter(htmlColor, 1 + (0.20 * (3 - index)))
        else
            return Qt.darker(htmlColor, 1 + (0.66 * (index - 2)))
    }

    Rectangle
    {
        x: 40
        y: 5
        width: parent - 10
        height: 42

        Row
        {
            Repeater
            {
                model: 8
                delegate:
                    Rectangle
                    {
                        width: 40
                        height: 40
                        border.width: 1
                        border.color: "#222"
                        color:  getHTMLColor(index * 36, index * 36, index * 36)
                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked:
                            {
                                selectedColor = color
                                rootBox.released()
                            }
                        }
                    }
            }
        }
    }

    Rectangle
    {
        x: 40
        y: 52
        width: parent - 10
        height: 42

        Row
        {
            Repeater
            {
                model: 8
                delegate:
                    Rectangle
                    {
                        width: 40
                        height: 40
                        border.width: 1
                        border.color: "#222"
                        color: getBaseHTMLColor(index)

                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked:
                            {
                                selectedColor = color
                                rootBox.released()
                            }
                        }
                    }
            }
        }
    }

    Rectangle
    {
        x: 40
        y: 100
        width: parent - 10
        height: 42 * 6

        Row
        {
            Repeater
            {
                id: baseColorColumn
                model: 8
                delegate:
                    Column
                    {
                        property int colIndex: index
                        Repeater
                        {
                            id: colorShades
                            model: 6
                            delegate:
                                Rectangle
                                {
                                    width: 40
                                    height: 40
                                    border.width: 1
                                    border.color: "#222"
                                    color: getShadedColor(colIndex, index)

                                    MouseArea
                                    {
                                        anchors.fill: parent
                                        onClicked:
                                        {
                                            selectedColor = color
                                            rootBox.released()
                                        }
                                    }
                                }
                        }
                    }
            }
        }

    }

    Row
    {
        x: 40
        y: 350
        spacing: 20
        RobotoText
        {
            label: qsTr("Selected color");
        }
        Rectangle
        {
            width: 70
            height: 40
            color: selectedColor
        }
    }


}

