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
var testTimer;
var timerRunning;

function initDefinitions()
{
    document.getElementById("apiversion").value = testAlgo.apiVersion;
    document.getElementById("name").value = testAlgo.name;
    document.getElementById("author").value = testAlgo.author;
    if (typeof testAlgo.acceptColors !== "undefined") {
        document.getElementById("acceptColors").value = testAlgo.acceptColors;
    } else {
        document.getElementById("acceptColors").value = "2 (Default)";
    }
}

function writeSelectOptions(item)
{
    var opt = document.createElement("option");
    var t = document.createTextNode(item);
    if (window.testAlgo[this.readFunction]() === item) {
        opt.selected = "selected";
    }
    opt.setAttribute("value", item);
    opt.appendChild(t);
    this.inputElement.appendChild(opt);
}

function addPropertyTableEntry(property)
{
    var table = document.getElementById("properties");
    var row = table.insertRow(-1);
    var i = 0;
    var keys = new Array();
    var input;

    for (i = 0; i < property.length; i++) {
        keys.push(property[i][0]);
    }

    var name = "";
    if (keys.indexOf("name") >= 0) {
        name = property[keys.indexOf("name")][1];
    }
    var displayName = name;
    if (keys.indexOf("display") >= 0) {
        displayName = property[keys.indexOf("display")][1];
    }
    var typeProperty = "string";
    if (keys.indexOf("type") >= 0) {
        // list: defines a list of strings that will be displayed by the QLC+ RGB Matrix Editor
        // range: defined a range of integer values that this property can handle
        // integer: an integer value that QLC+ can exchange with the script
        // string: a string that QLC+ can exchange with the script
        typeProperty = property[keys.indexOf("type")][1];
    }
    var values = Array(0, 0);
    if (keys.indexOf("values") >= 0) {
        values = property[keys.indexOf("values")][1].split(",");
    }
    var writeFunction = "";
    if (keys.indexOf("write") >= 0) {
        writeFunction = property[keys.indexOf("write")][1];
        if (name !== "") {
            var storedValue = localStorage.getItem(name);
            if (storedValue !== null) {
                window.testAlgo[writeFunction](storedValue);
            }
        }
    }
    var readFunction = "";
    if (keys.indexOf("read") >= 0) {
        readFunction = property[keys.indexOf("read")][1];
    }

    var nameCell = row.insertCell(-1);
    var t = document.createTextNode(displayName);
    nameCell.appendChild(t);

    var formCell = row.insertCell(-1);
    if (typeProperty === "list") {
        input = document.createElement("select");
        input.name = name;
        input.id = name;
        input.setAttribute("onChange", "writeFunction('" + writeFunction + "', '" + name + "', this.value); setStep(0); writeCurrentStep()");
        var selectOption = new Object();
        selectOption.readFunction = readFunction;
        selectOption.inputElement = input;
        values.forEach(writeSelectOptions, selectOption);
        formCell.appendChild(input);
    } else if (typeProperty === "range") {
        input = document.createElement("input");
        input.type = "number";
        input.required = "required";
        input.name = name;
        input.setAttribute("value", window.testAlgo[name]);
        input.id = name;
        input.min = values[0];
        input.max = values[1];
        input.setAttribute("onChange", "writeFunction('" + writeFunction + "', '" + name + "', this.value); setStep(0); writeCurrentStep()");
        formCell.appendChild(input);
    } else if (typeProperty === "integer") {
        input = document.createElement("input");
        input.type = "number";
        input.required = "required";
        input.name = name;
        input.setAttribute("value", window.testAlgo[name]);
        input.id = name;
        input.setAttribute("onChange", "writeFunction('" + writeFunction + "', '" + name + "', this.value); setStep(0); writeCurrentStep()");
        formCell.appendChild(input);
    } else { // string
        input = document.createElement("input");
        input.type = "text";
        input.name = name;
        input.setAttribute("value", window.testAlgo[name]);
        input.id = name;
        input.setAttribute("onChange", "writeFunction('" + writeFunction + "', '" + name + "', this.value); setStep(0); writeCurrentStep()");
        formCell.appendChild(input);
    }
}

function initProperties()
{
    var table = document.getElementById("properties");
    var properties = Array();
    var property = Array();
    var i = 0;
    var entry = 0;

    // Cleanup the table before updating its contents
    for (i = table.rows.length - 1; i >= 0; i--) {
        table.deleteRow(i);
    }

    // Algo properties not supported by versions prior to 2.
    if (testAlgo.apiVersion < 2) {
        return;
    }

    // Get the properties
    for (i = 0; i < testAlgo.properties.length; i++)
    {
        var propDef = testAlgo.properties[i];
        var propKeyValue = propDef.split("|");
        property = Array();

        for (entry = 0; entry < propKeyValue.length; entry++) {
            var keyValue = propKeyValue[entry].split(":");
            var key = keyValue[0];
            keyValue.shift();
            property.push(Array(key, keyValue.join(":")));
        }
        properties.push(property);
    }
    // Write the properties
    properties.forEach(addPropertyTableEntry);
}

function initPixelColors()
{
    var pixelColorChooser = document.getElementById("pixelColorChooser");
    pixelColorChooser.hidden = testAlgo.acceptColors === 0;

    var secondaryColorChooser = document.getElementById("secondaryColorChooser");
    secondaryColorChooser.hidden = testAlgo.acceptColors === 1;
}

function initSpeedValue()
{
    var speed = localStorage.getItem("speed");
    if (speed === null) {
        speed = 500;
    }
    document.getElementById("speed").value = speed;
}

function getRgbFromColorInt(color)
{
    var red = color >> 16;
    var green = (color >> 8) - red * 256;
    var blue = color - red * 256 * 256 - green * 256;
    return [red, green, blue];
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
            cell.style.height = "20px";
            cell.style.width = "20px";
            cell.title = "(" + x + ", " + y + "): " + rgbStr + " – " + cell.style.backgroundColor; // rgbStr will be #rrggbb whereas the cell style will be rgb(255, 255, 255)
        }
    }
}

function setStep(step) {
    currentStep = step;
    document.getElementById("currentStep").value = currentStep;
    writeCurrentStep();
}

function onGridSizeUpdated()
{
    width = parseInt(document.getElementById("width").value);
    height = parseInt(document.getElementById("height").value);

    stepCount = testAlgo.rgbMapStepCount(width, height);
    document.getElementById("stepCount").value = stepCount;
    document.getElementById("currentStep").max = stepCount - 1;

    setStep(0);
    writeCurrentStep();
}

function startTest()
{
    var speed = document.getElementById("speed").value;
    window.clearInterval(testTimer); // avoid multiple timers running simultaneously
    testTimer = window.setInterval("nextStep()", speed);
    localStorage.setItem("timerRunning", 1);
}

function stopTest()
{
    window.clearInterval(testTimer);
    localStorage.setItem("timerRunning", 0);
}

function initTestStatus()
{
    var timerStatus = localStorage.getItem("timerRunning");
    if (timerStatus === null || parseInt(timerStatus) === 1) {
        startTest();
    }
}

function init()
{
    if (typeof testAlgo === "undefined") {
        return;
    }
    initDefinitions();
    initSpeedValue();
    initProperties();
    initPixelColors();
    onGridSizeUpdated();
    writeCurrentStep();
    initTestStatus();
}

function handleLoadError()
{
    return;
}

function onFilenameUpdated(filename)
{
    if (filename === "") {
        return;
    }

    localStorage.setItem("filename", filename);

    var script = document.getElementById("algoScript");
    if (script === null) {
        script = document.createElement("script");
        script.id = "algoScript";
        script.addEventListener("load", () => init(), false);
        script.addEventListener("error", () => handleLoadError(), false);
        script.type = "text/javascript";
        script.src = filename;
        document.head.appendChild(script);
    } else {
        script.src = filename;
    }
    // init(); // is called onload.
}

function loadAlgoFile()
{
    var storedValue = localStorage.getItem("filename");
    if (storedValue === null) {
        storedValue = "evenodd.js";
    }
    document.getElementById("filename").value = storedValue;
    onFilenameUpdated(storedValue);
}

function writeFunction(functionName, propertyName, value)
{
    window.testAlgo[functionName](value);
    localStorage.setItem(propertyName, value);
}

function onSpeedChanged()
{
    var speed = document.getElementById("speed").value;
    localStorage.setItem("speed", speed);
    initTestStatus();
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
