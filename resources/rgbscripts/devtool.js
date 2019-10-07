/*
  Q Light Controller Plus
  devtool.js

  Copyright (c) Heikki Junnila

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

var width;
var height;
var stepCount;
var currentStep;

function init()
{
    initProperties();
    initPixelColors();
    onGridSizeUpdated();
    writeCurrentStep();
}

function initProperties()
{
    document.getElementById("apiversion").value = testAlgo.apiVersion;
    document.getElementById("name").value = testAlgo.name;
    document.getElementById("author").value = testAlgo.author;
}

function initPixelColors()
{
    var pixelColorChooser = document.getElementById("pixelColorChooser");
    pixelColorChooser.hidden = testAlgo.acceptColors === 0;

    var secondaryColorChooser = document.getElementById("secondaryColorChooser");
    secondaryColorChooser.hidden = testAlgo.acceptColors === 1;
}

function onGridSizeUpdated()
{
    width = parseInt(document.getElementById("width").value);
    height = parseInt(document.getElementById("height").value);

    stepCount = testAlgo.rgbMapStepCount(width, height);
    document.getElementById("stepCount").value = stepCount;
    document.getElementById("currentStep").max = stepCount - 1;

    setStep(0);
}

function nextStep()
{
    if (currentStep + 1 < stepCount) {
        setStep(currentStep + 1);
    }
    else {
        setStep(0);
    }
}

function previousStep()
{
    if (currentStep > 0) {
        setStep(currentStep - 1);
    } else {
        setStep(stepCount - 1); // last step
    }
}

function setStep(step) {
    currentStep = step;
    document.getElementById("currentStep").value = currentStep;
    writeCurrentStep();
}

function writeCurrentStep()
{
    currentStep = parseInt(document.getElementById("currentStep").value); // currentStep may have been changed manually

    var map = document.getElementById("map");
    for (var i = map.rows.length - 1; i >= 0; i--) {
        map.deleteRow(i);
    }
    var rgb = testAlgo.rgbMap(width, height, getCurrentColorInt(), currentStep);

    for (var y = 0; y < height; y++)
    {
        var row = map.insertRow(y);

        for (var x = 0; x < width; x++)
        {
            var cell = row.insertCell(x);
            var rgbStr = rgb[y][x].toString(16);
            while (rgbStr.length !== 6) {
                rgbStr = "0" + rgbStr;
            }
            rgbStr = "#" + rgbStr;
            cell.style.backgroundColor = rgbStr;
            cell.style.height = 20;
            cell.style.width = 20;
            cell.title = "(" + x + ", " + y + "): " + rgbStr + " – " + cell.style.backgroundColor; // rgbStr will be #rrggbb whereas the cell style will be rgb(255, 255, 255)
        }
    }
}

function getCurrentColorInt()
{
    var primaryColorInput = document.getElementById("primaryColor");
    var primaryColor = parseInt(primaryColorInput.value, 16);
    var secondaryColorInput = document.getElementById("secondaryColor");
    var secondaryColor = parseInt(secondaryColorInput.value, 16);

    if (testAlgo.acceptColors === 0 || Number.isNaN(primaryColor)) {
        return null;
    }

    if (testAlgo.acceptColors === 1 || Number.isNaN(secondaryColor) || stepCount <= 1) {
        return primaryColor;
    }

    var primaryColorRgb = getRgbFromColorInt(primaryColor);
    var secondaryColorRgb = getRgbFromColorInt(secondaryColor);

    var primaryFactor = (stepCount - currentStep - 1) / (stepCount - 1);
    var secondaryFactor = currentStep / (stepCount - 1);

    var red = Math.round(primaryColorRgb[0] * primaryFactor + secondaryColorRgb[0] * secondaryFactor);
    var green = Math.round(primaryColorRgb[1] * primaryFactor + secondaryColorRgb[1] * secondaryFactor);
    var blue = Math.round(primaryColorRgb[2] * primaryFactor + secondaryColorRgb[2] * secondaryFactor);

    return red * 256 * 256 + green * 256 + blue;
}

function getRgbFromColorInt(color)
{
    var red = color >> 16;
    var green = (color >> 8) - red * 256;
    var blue = color - red * 256 * 256 - green * 256;
    return [red, green, blue];
}
