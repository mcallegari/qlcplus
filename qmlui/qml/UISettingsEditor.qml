/*
  Q Light Controller Plus
  PopupUISettings.qml

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

Rectangle
{
    id: editorRoot
    anchors.fill: parent
    color: "transparent"

    property real origItemHeight: { origItemHeight = UISettings.listItemHeight }
    property real origIconMedium: { origIconMedium = UISettings.iconSizeMedium }
    property real origTextSizeDefault: { origTextSizeDefault = UISettings.textSizeDefault}
    property real origIconDefault: {origIconDefault = UISettings.iconSizeDefault}

    onVisibleChanged:
    {
        origItemHeight = UISettings.listItemHeight
        origIconMedium = UISettings.iconSizeMedium
        origTextSizeDefault = UISettings.textSizeDefault
        sfRestore.origScaleFactor = qlcplus.uiScaleFactor
    }

    ColorTool
    {
        id: colorTool
        z: 2
        x: editorGrid.width
        //parent: mainView
        visible: false
        showPalette: false

        property Item rectItem
        property Item selectedItem

        onColorChanged:
        {
            rectItem.color = Qt.rgba(r, g, b, 1.0)
            selectedItem.updateColor(Qt.rgba(r, g, b, 1.0))
        }
        onClose: visible = false
    }

    CustomPopupDialog
    {
        id: messagePopup
        standardButtons: Dialog.Ok
        onAccepted: close()
    }

    Component
    {
        id: colorSelector

        RowLayout
        {
            height: origIconMedium

            property color origColor
            property Item pItem

            function init(item, original, col)
            {
                pItem = item
                origColor = original
                colorRect.color = col
            }

            Rectangle
            {
                id: colorRect
                height: origIconMedium
                width: height * 4
                border.color: csMouseArea.containsMouse ? "white" : "gray"
                border.width: 2

                MouseArea
                {
                    id: csMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked:
                    {
                        colorTool.rectItem = colorRect
                        colorTool.selectedItem = pItem
                        colorTool.visible = true
                    }
                }
            }

            IconButton
            {
                width: origIconMedium
                height: width
                imgSource: "qrc:/undo.svg"
                tooltip: qsTr("Reset to default")

                onClicked:
                {
                    pItem.updateColor(origColor)
                    colorRect.color = origColor
                }
            }
        }
    }

    GridLayout
    {
        id: editorGrid
        columnSpacing: origIconMedium
        rowSpacing: 10
        columns: 4
        z: 1

        // Row 1
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Scaling factor")
        }
        RowLayout
        {
            Slider
            {
                id: sfSlider
                orientation: Qt.Horizontal
                from: 0.3
                to: 2
                value: UISettings.scalingFactor
                wheelEnabled: true
                onMoved: uiManager.setModified("scalingFactor", value)
            }

            RobotoText
            {
                height: origItemHeight
                fontSize: origTextSizeDefault
                label: sfSlider.value.toFixed(2) + "x"
            }

            IconButton
            {
                id: sfRestore
                width: origIconMedium
                height: width
                imgSource: "qrc:/undo.svg"
                tooltip: qsTr("Reset to default")

                property real origScaleFactor

                onClicked: uiManager.setModified("scalingFactor", 1.0)
            }
        }

        Rectangle
        {
            Layout.columnSpan: 2
            height: origItemHeight
            color: "transparent"
        }

        // Row 2
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Background darker")
        }
        Loader
        {
            property string kName: "bgStronger"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Background dark")
        }
        Loader
        {
            property string kName: "bgStrong"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 3
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Background medium")
        }
        Loader
        {
            property string kName: "bgMedium"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Background light")
        }
        Loader
        {
            property string kName: "bgLight"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 4
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Background lighter")
        }
        Loader
        {
            property string kName: "bgLighter"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Controls background")
        }
        Loader
        {
            property string kName: "bgControl"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 5
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Foreground main")
        }
        Loader
        {
            property string kName: "fgMain"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Foreground medium")
        }
        Loader
        {
            property string kName: "fgMedium"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 6
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Foreground light")
        }
        Loader
        {
            property string kName: "fgLight"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Toolbar gradient start")
        }
        Loader
        {
            property string kName: "toolbarStartMain"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 7
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Sub-toolbar gradient start")
        }
        Loader
        {
            property string kName: "toolbarStartSub"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Toolbar gradient end")
        }
        Loader
        {
            property string kName: "toolbarEnd"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 8
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Toolbar hover gradient start")
        }
        Loader
        {
            property string kName: "toolbarHoverStart"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }


        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Toolbar hover gradient end")
        }
        Loader
        {
            property string kName: "toolbarHoverEnd"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 9
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Toolbar selection")
        }
        Loader
        {
            property string kName: "toolbarSelectionMain"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }


        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Sub-toolbar selection")
        }
        Loader
        {
            property string kName: "toolbarSelectionSub"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 10
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Section header")
        }
        Loader
        {
            property string kName: "sectionHeader"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Section header divider")
        }
        Loader
        {
            property string kName: "sectionHeaderDiv"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 11
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Item highlight")
        }
        Loader
        {
            property string kName: "highlight"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Item highlight pressed")
        }
        Loader
        {
            property string kName: "highlightPressed"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 12
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Item hover")
        }
        Loader
        {
            property string kName: "hover"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Item selection")
        }
        Loader
        {
            property string kName: "selection"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        //Row 13
        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("VC Frame drop area")
        }
        Loader
        {
            property string kName: "activeDropArea"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        RobotoText
        {
            height: origItemHeight
            fontSize: origTextSizeDefault
            label: qsTr("Item dark border")
        }
        Loader
        {
            property string kName: "borderColorDark"
            sourceComponent: colorSelector
            onLoaded: item.init(this, uiManager.getDefault(kName),
                                      uiManager.getModified(kName))

            function updateColor(col)
            {
                uiManager.setModified(kName, col)
            }
        }

        GenericButton
        {
            Layout.columnSpan: 4
            Layout.alignment: Qt.AlignHCenter
            width: origIconMedium * 10
            height: origIconDefault
            fontSize: origTextSizeDefault
            label: qsTr("Save to file")
            onClicked:
            {
                var fPath = uiManager.userConfFilepath()
                if (uiManager.saveSettings() === true)
                {
                    messagePopup.title = qsTr("Operation completed")
                    messagePopup.message = qsTr("File successfully saved to:" + "<br>" + fPath)
                }
                else
                {
                    messagePopup.title = qsTr("Error")
                    messagePopup.message = qsTr("Unable to save file:" + "<br>" + fPath)
                }
                messagePopup.open()
            }

            Image
            {
                x: parent.width * 0.05
                anchors.verticalCenter: parent.verticalCenter
                width: parent.height * 0.75
                height: width
                source: "qrc:/filesave.svg"
                sourceSize: Qt.size(width, height)
            }
        }
    }
}
