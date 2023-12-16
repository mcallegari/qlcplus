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

var devtool = Object;
devtool.gridwidth = null;
devtool.gridheight = null;
devtool.gridsize = null;
devtool.currentStep = 0;
devtool.testTimer = null;
devtool.alternate = false;

devtool.initDefinitions = function()
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

devtool.writeSelectOptions = function(item)
{
    var opt = document.createElement("option");
    var t = document.createTextNode(item);
    if (this.currentValue === item) {
        opt.selected = "selected";
    }
    opt.setAttribute("value", item);
    opt.appendChild(t);
    this.inputElement.appendChild(opt);
}

devtool.addPropertyTableEntry = function(property)
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
    var writeFunction = property[keys.indexOf("write")][1];
    if (name !== "") {
        var storedValue = localStorage.getItem(name);
        if (storedValue !== null) {
            window.testAlgo[writeFunction](storedValue);
        }
    }
    var readFunction = property[keys.indexOf("read")][1];

    var nameCell = row.insertCell(-1);
    var t = document.createTextNode(displayName);
    nameCell.appendChild(t);

    var currentValue = window.testAlgo[readFunction]();

    var formCell = row.insertCell(-1);
    if (typeProperty === "list") {
        input = document.createElement("select");
        input.name = name;
        input.id = name;
        input.setAttribute("onChange", "devtool.writeFunction('" + writeFunction + "', '" + name + "', this.value); devtool.setStep(0); devtool.writeCurrentStep()");
        var selectOption = new Object();
        selectOption.currentValue = currentValue;
        selectOption.readFunction = readFunction;
        selectOption.inputElement = input;
        values.forEach(devtool.writeSelectOptions, selectOption);
        formCell.appendChild(input);
    } else if (typeProperty === "range") {
        input = document.createElement("input");
        input.type = "number";
        input.required = "required";
        input.name = name;
        input.setAttribute("value", currentValue);
        input.id = name;
        input.min = values[0];
        input.max = values[1];
        input.setAttribute("onChange", "devtool.writeFunction('" + writeFunction + "', '" + name + "', this.value); devtool.setStep(0); devtool.writeCurrentStep()");
        formCell.appendChild(input);
    } else if (typeProperty === "integer") {
        input = document.createElement("input");
        input.type = "number";
        input.required = "required";
        input.name = name;
        input.setAttribute("value", currentValue);
        input.id = name;
        input.setAttribute("onChange", "devtool.writeFunction('" + writeFunction + "', '" + name + "', this.value); devtool.setStep(0); devtool.writeCurrentStep()");
        formCell.appendChild(input);
    } else { // string
        input = document.createElement("input");
        input.type = "text";
        input.name = name;
        input.setAttribute("value", currentValue);
        input.id = name;
        input.setAttribute("onChange", "devtool.writeFunction('" + writeFunction + "', '" + name + "', this.value); devtool.setStep(0); devtool.writeCurrentStep()");
        formCell.appendChild(input);
    }
}

devtool.initProperties = function()
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
    properties.forEach(devtool.addPropertyTableEntry);
}

devtool.initPixelColors = function()
{
    var acceptColors = parseInt(testAlgo.acceptColors, 10);
    if (isNaN(acceptColors)) {
      acceptColors = 0;
    }
    var pixelColorChooser = document.getElementById("pixelColorChooser");
    pixelColorChooser.hidden = (acceptColors === 0);

    var color2Chooser = document.getElementById("color2Chooser");
    color2Chooser.hidden = (acceptColors <= 2);

    var color3Chooser = document.getElementById("color3Chooser");
    color3Chooser.hidden = (acceptColors <= 3);

    var color4Chooser = document.getElementById("color4Chooser");
    color4Chooser.hidden = (acceptColors <= 4);

    var color5Chooser = document.getElementById("color5Chooser");
    color5Chooser.hidden = (acceptColors <= 5);
}

devtool.initSpeedValue = function()
{
    var speed = localStorage.getItem("devtool.speed");
    if (speed === null) {
        speed = 500;
    }
    document.getElementById("speed").value = speed;
}

devtool.initAlternateValue = function()
{
    var alternate = localStorage.getItem("devtool.alternate");
    if (alternate === true) {
      alternate = true;
    } else {
      alternate = false;
    }
    document.getElementById("alternate").checked = alternate;
}

devtool.initColorValues = function()
{
    var color1 = localStorage.getItem("devtool.color1");
    if (color1 === null || Number.isNaN(parseInt("0x" + color1, 16))) {
      color1 = "ff0000";
    }
    document.getElementById("color1").value = color1;
    var color2 = localStorage.getItem("devtool.color2");
    if (color2 === null || color2 === "" || Number.isNaN(parseInt("0x" + color2, 16))) {
      document.getElementById("color2").value = "";
    } else {
      document.getElementById("color2").value = color2;
    }
    var color3 = localStorage.getItem("devtool.color3");
    if (color3 === null || color2 === "" || Number.isNaN(parseInt("0x" + color3, 16))) {
      document.getElementById("color3").value = "";
    } else {
      document.getElementById("color3").value = color3;
    }
    var color4 = localStorage.getItem("devtool.color4");
    if (color4 === null || color4 === "" || Number.isNaN(parseInt("0x" + color4, 16))) {
      document.getElementById("color4").value = "";
    } else {
      document.getElementById("color4").value = color2;
    }
    var color5 = localStorage.getItem("devtool.color5");
    if (color5 === null || color5 === "" || Number.isNaN(parseInt("0x" + color5, 16))) {
      document.getElementById("color5").value = "";
    } else {
      document.getElementById("color5").value = color25;
    }
}

devtool.initGridSize = function()
{
    devtool.gridwidth = localStorage.getItem("devtool.gridwidth");
    if (devtool.gridwidth === null) {
        devtool.gridwidth = 15;
    }
    document.getElementById("gridwidth").value = devtool.gridwidth;
    devtool.gridheight = localStorage.getItem("devtool.gridheight");
    if (devtool.gridheight === null) {
        devtool.gridheight = 15;
    }
    document.getElementById("gridheight").value = devtool.gridheight;
    devtool.gridsize = localStorage.getItem("devtool.gridsize");
    if (devtool.gridsize === null) {
        devtool.gridsize = 20;
    }
    document.getElementById("gridsize").value = devtool.gridsize;
}

devtool.getRgbFromColorInt = function(color)
{
    var red = color >> 16;
    var green = (color >> 8) - red * 256;
    var blue = color - red * 256 * 256 - green * 256;
    return [red, green, blue];
}

devtool.getColor1Int = function()
{
    var colorInput = document.getElementById("color1");
    return parseInt(colorInput.value, 16);
}

devtool.getColor2Int = function()
{
    var colorInput = document.getElementById("color2");
    return parseInt(colorInput.value, 16);
}

devtool.getColor3Int = function()
{
    var colorInput = document.getElementById("color3");
    return parseInt(colorInput.value, 16);
}

devtool.getColor4Int = function()
{
    var colorInput = document.getElementById("color4");
    return parseInt(colorInput.value, 16);
}

devtool.getColor5Int = function()
{
    var colorInput = document.getElementById("color5");
    return parseInt(colorInput.value, 16);
}

devtool.getCurrentColorInt = function()
{
    var primaryColor = devtool.getcolor1Int();
    var secondaryColor = devtool.getcolor2Int();

    if (testAlgo.acceptColors === 0 || Number.isNaN(primaryColor)) {
        return null;
    }

    if (testAlgo.acceptColors === 1 || Number.isNaN(secondaryColor) || devtool.stepCount() <= 1) {
        return primaryColor;
    }

    var primaryColorRgb = devtool.getRgbFromColorInt(primaryColor);
    var secondaryColorRgb = devtool.getRgbFromColorInt(secondaryColor);

    var primaryFactor = (devtool.stepCount() - devtool.currentStep - 1) / (devtool.stepCount() - 1);
    var secondaryFactor = devtool.currentStep / (devtool.stepCount() - 1);

    var red = Math.round(primaryColorRgb[0] * primaryFactor + secondaryColorRgb[0] * secondaryFactor);
    var green = Math.round(primaryColorRgb[1] * primaryFactor + secondaryColorRgb[1] * secondaryFactor);
    var blue = Math.round(primaryColorRgb[2] * primaryFactor + secondaryColorRgb[2] * secondaryFactor);

    return red * 256 * 256 + green * 256 + blue;
}

devtool.writeCurrentStep = function()
{
    devtool.currentStep = parseInt(document.getElementById("currentStep").value); // currentStep may have been changed manually

    var map = document.getElementById("map");
    for (var i = map.rows.length - 1; i >= 0; i--) {
        map.deleteRow(i);
    }

    var color1Rgb = devtool.getColor1Int();
    var color2Rgb = devtool.getColor2Int();
    var color3Rgb = devtool.getColor3Int();
    var color4Rgb = devtool.getColor4Int();
    var color5Rgb = devtool.getColor5Int();
    var rawColors = [color1Rgb, color2Rgb, color3Rgb, color4Rgb, color5Rgb];

    var rgb;
    if (testAlgo.apiVersion > 2) {
      rgb = testAlgo.rgbMap(devtool.gridwidth, devtool.gridheight, devtool.getCurrentColorInt(), devtool.currentStep, rawColors);
    } else {
      rgb = testAlgo.rgbMap(devtool.gridwidth, devtool.gridheight, devtool.getCurrentColorInt(), devtool.currentStep);
    }

    for (var y = 0; y < devtool.gridheight; y++)
    {
        var row = map.insertRow(y);

        for (var x = 0; x < devtool.gridwidth; x++)
        {
            var cell = row.insertCell(x);
            var rgbStr = rgb[y][x].toString(16);
            while (rgbStr.length !== 6) {
                rgbStr = "0" + rgbStr;
            }
            rgbStr = "#" + rgbStr;
            cell.style.backgroundColor = rgbStr;
            cell.style.height = devtool.gridsize + "px";
            cell.style.width = devtool.gridsize + "px";
            cell.title = "(" + x + ", " + y + "): " + rgbStr + " – " + cell.style.backgroundColor; // rgbStr will be #rrggbb whereas the cell style will be rgb(255, 255, 255)
        }
    }
}

devtool.setStep = function(step)
{
    devtool.currentStep = step;
    document.getElementById("currentStep").value = devtool.currentStep;
    devtool.writeCurrentStep();
}

// Do not cache this value - the program also doesn't
// and there are scripts changing this number on any parameter change
devtool.stepCount = function()
{
    return testAlgo.rgbMapStepCount(devtool.gridwidth, devtool.gridheight);
}

devtool.updateStepCount = function()
{
    document.getElementById("stepCount").value = devtool.stepCount();
    document.getElementById("currentStep").max = devtool.stepCount() - 1;
}

devtool.onGridSizeUpdated = function()
{
    devtool.gridwidth = parseInt(document.getElementById("gridwidth").value);
    localStorage.setItem("devtool.gridwidth", devtool.gridwidth);

    devtool.gridheight = parseInt(document.getElementById("gridheight").value);
    localStorage.setItem("devtool.gridheight", devtool.gridheight);

    devtool.gridsize = parseInt(document.getElementById("gridsize").value);
    localStorage.setItem("devtool.gridsize", devtool.gridsize);

    devtool.updateStepCount();

    devtool.writeCurrentStep();
}

devtool.onColorChange = function()
{
    var color1 = parseInt("0x" + document.getElementById("color1").value).toString(16);
    localStorage.setItem("devtool.color1", color1);
    var color2 = parseInt("0x" + document.getElementById("color2").value).toString(16);
    if (color2 === "NaN") { // Evaluation of the string.
      localStorage.setItem("devtool.color2", "");
    } else {
      localStorage.setItem("devtool.color2", color2);
    }
    var color3 = parseInt("0x" + document.getElementById("color3").value).toString(16);
    if (color3 === "NaN") { // Evaluation of the string.
      localStorage.setItem("devtool.color3", "");
    } else {
      localStorage.setItem("devtool.color3", color3);
    }
    var color4 = parseInt("0x" + document.getElementById("color4").value).toString(16);
    if (color4 === "NaN") { // Evaluation of the string.
      localStorage.setItem("devtool.color4", "");
    } else {
      localStorage.setItem("devtool.color4", color4);
    }
    var color5 = parseInt("0x" + document.getElementById("color5").value).toString(16);
    if (color5 === "NaN") { // Evaluation of the string.
      localStorage.setItem("devtool.color5", "");
    } else {
      localStorage.setItem("devtool.color5", color5);
    }
    devtool.writeCurrentStep();
}

devtool.startTest = function(inc)
{
    var speed = document.getElementById("speed").value;
    window.clearInterval(devtool.testTimer); // avoid multiple timers running simultaneously
    if (inc > 0) {
      devtool.testTimer = window.setInterval("devtool.nextStep()", speed);
    } else {
      devtool.testTimer = window.setInterval("devtool.previousStep()", speed);
    }
    localStorage.setItem("devtool.timerRunning", inc);
}

devtool.stopTest = function()
{
    window.clearInterval(devtool.testTimer);
    localStorage.setItem("devtool.timerRunning", 0);
}

devtool.initTestStatus = function()
{
    let timerStatus = localStorage.getItem("devtool.timerRunning");
    if (timerStatus === null || parseInt(timerStatus) !== 0) {
        devtool.startTest(parseInt(timerStatus));
    }
}

devtool.init = function()
{
    if (typeof testAlgo === "undefined") {
        return;
    }
    devtool.initDefinitions();
    devtool.initGridSize();
    devtool.setStep(0);
    devtool.initSpeedValue();
    devtool.initColorValues();
    devtool.initProperties();
    devtool.initPixelColors();
    devtool.onGridSizeUpdated();
    devtool.writeCurrentStep();
    devtool.initTestStatus();
}

devtool.handleLoadError = function()
{
    return;
}

devtool.onFilenameUpdated = function(filename)
{
    if (filename === "") {
        return;
    }

    localStorage.setItem("devtool.filename", filename);

    var script = document.getElementById("algoScript");
    if (script === null) {
        script = document.createElement("script");
        script.id = "algoScript";
        script.addEventListener("load", () => devtool.init(), false);
        script.addEventListener("error", () => devtool.handleLoadError(), false);
        script.type = "text/javascript";
        script.src = filename;
        document.head.appendChild(script);
    } else {
        script.src = filename;
    }
    // devtool.init(); // is called onload.
}

devtool.loadAlgoFile = function()
{
    var storedValue = localStorage.getItem("devtool.filename");
    if (storedValue === null) {
        storedValue = "evenodd.js";
    }
    document.getElementById("filename").value = storedValue;
    devtool.onFilenameUpdated(storedValue);
}

devtool.writeFunction = function(functionName, propertyName, value)
{
    window.testAlgo[functionName](value);
    localStorage.setItem(propertyName, value);
    devtool.updateStepCount();
}

devtool.onAlternateChanged = function()
{
    var alternate = document.getElementById("alternate").checked;
    localStorage.setItem("devtool.alternate", alternate);
    devtool.alternate = alternate;
}

devtool.onSpeedChanged = function()
{
    var speed = document.getElementById("speed").value;
    localStorage.setItem("devtool.speed", speed);
    devtool.initTestStatus();
}

devtool.nextStep = function()
{
    if (devtool.currentStep + 1 < devtool.stepCount()) {
        devtool.setStep(devtool.currentStep + 1);
    } else {
        let timerStatus = localStorage.getItem("devtool.timerRunning");
        let alternate = document.getElementById("alternate").checked;
        if (timerStatus == "1" && alternate) {
            devtool.startTest(-1);
        } else {
            devtool.setStep(0);
        }
    }
}

devtool.previousStep = function()
{
    if (devtool.currentStep > 0 && (devtool.currentStep - 1) < devtool.stepCount()) {
        devtool.setStep(devtool.currentStep - 1);
    } else {
        let timerStatus = localStorage.getItem("devtool.timerRunning");
        let alternate = document.getElementById("alternate").checked;
        if (timerStatus == "-1" && alternate) {
            devtool.startTest(1);
        } else {
            devtool.setStep(devtool.stepCount() - 1); // last step
        }
    }
}
