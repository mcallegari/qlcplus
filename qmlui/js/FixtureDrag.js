/*
  Q Light Controller Plus
  FixtureDrag.js

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

var itemComponent = null;
var draggedItem = null;
var startingMouse;
var posnInWindow;
var manufacturer, model, mode, name;
var universeIndex, address, channels, quantity, gap;

function initProperties()
{
    manufacturer = fixtureBrowser.selectedManufacturer;
    model = fixtureBrowser.selectedModel;
    mode = fixtureBrowser.selectedMode;
    name = fixtureBrowser.fixtureName;
    universeIndex = fxPropsRect.fxUniverseIndex;
    address = fxPropsRect.fxAddress - 1;
    channels = fixtureBrowser.modeChannelsCount;
    quantity = fxPropsRect.fxQuantity;
    gap = fxPropsRect.fxGap;
    console.log("mf: " + manufacturer + ", mdl: " + model + ", mode: " + mode);
    console.log("addr: " + address + ", ch: " + channels);
}

// Creation is split into two functions due to an asynchronous wait while
// possible external files are loaded.

function createItem()
{
    if (itemComponent.status === Component.Ready && draggedItem == null)
    {
        draggedItem = itemComponent.createObject(mainView,
                  {"x": posnInWindow.x, "y": posnInWindow.y, "z": 10,
                   "manufacturer": manufacturer, "model": model,
                   "address": address, "universe": universeIndex,
                   "channels": channels, "quantity": quantity, "gap": gap });
    }
    else if (itemComponent.status === Component.Error)
    {
        draggedItem = null;
        console.log("error creating component");
        console.log(itemComponent.errorString());
    }
}

function loadComponent()
{
    if (itemComponent != null) // component has been previously loaded
    {
        createItem();
        return;
    }

    itemComponent = Qt.createComponent("FixtureDragItem.qml");
    createItem();
}

function handleDrag(mouse)
{
    if (draggedItem == null)
    {
        posnInWindow = fxDraggableItem.mapToItem(mainView, 0, 0);
        loadComponent();
    }

    draggedItem.x = mouse.x + posnInWindow.x - 5;
    draggedItem.y = mouse.y + posnInWindow.y - 5;
}

function endDrag(mouse)
{
    if (draggedItem == null)
        return;

    var currContext = previewLoader.item.contextName;
    var offset = 0;
    console.log("[FixtureDrag] Current context: " + currContext);

    if (currContext === "2D")
        offset = View2D.gridPosition.x;

    var x = draggedItem.x - leftSidePanel.width - offset;
    var y = draggedItem.y - previewLoader.y - viewToolbar.height;

    console.log("[FixtureDrag] Item x: " + x + ", y: " + y);

    if (x >= 0 && y >= 0)
    {
        fixtureManager.addFixture(manufacturer, model, mode, name, universeIndex,
                                  draggedItem.address, channels, quantity, gap, x, y);
    }

    draggedItem.destroy();
    draggedItem = null;
}

