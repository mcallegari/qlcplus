function init()
{
    var w = document.getElementById("width");
    var h = document.getElementById("height");

    if (!w.value)
        w.value = 15;
    if (!h.value)
        h.value = 15;

    updateProperties();
    updateStepCount();
    writeCurrentStep();
}

function updateStepCount()
{
    var w = document.getElementById("width");
    var h = document.getElementById("height");
    var stepCount = document.getElementById("stepcount");
    var currentStep = document.getElementById("currentstep");

    if (w && h && stepCount && currentStep)
    {
        stepCount.value = testAlgo.rgbMapStepCount(w.value, h.value);
        currentStep.value = -1;
        nextStep();
        updateProperties();
    }
    else
    {
        alert("Width, Height or Result element not found!");
    }
}

function updateProperties()
{
    var apiVersion = document.getElementById("apiversion");
    var name = document.getElementById("name");
    var author = document.getElementById("author");

    if (apiVersion)
        apiVersion.value = testAlgo.apiVersion;
    if (name)
        name.value = testAlgo.name;
    if (author)
        author.value = testAlgo.author;    
}

function nextStep()
{
    var stepCount = document.getElementById("stepcount");
    var currentStep = document.getElementById("currentstep");

    if (stepCount && currentStep)
    {
        var steps = parseInt(stepCount.value);
        var current = parseInt(currentStep.value);

        var next;
        if ((current + 1) < steps)
            next = current + 1;
        else
            next = 0;

        currentStep.value = next;
        writeCurrentStep();
    }
    else
    {
        alert("stepcount or currentstep element not found!");
    }
}

function previousStep()
{
    var stepCount = document.getElementById("stepcount");
    var currentStep = document.getElementById("currentstep");

    if (stepcount && currentStep)
    {
        var steps = parseInt(stepCount.value);
        var current = parseInt(currentStep.value);

        var next;
        if (current > 0)
            next = current - 1;
        else
            next = steps - 1;

        currentStep.value = next;
        writeCurrentStep();
    }
    else
    {
        alert("stepcount or currentstep element not found!");
    }
}

function writeCurrentStep()
{
    var map = document.getElementById("map");
    var w = document.getElementById("width");
    var h = document.getElementById("height");
    var currentStep = document.getElementById("currentstep");
    var stepCount = document.getElementById("stepcount");
    var bicolor = document.getElementById("bicolor");

    var currentRgb = parseInt(document.getElementById("color1").value, 16);

    if (bicolor.checked)
    {
        var stepCountMinusOne = parseInt(stepCount.value) - 1;
        stepCountMinusOne = stepCountMinusOne == 0 ? 1 : stepCountMinusOne;
        var currentR = ((stepCountMinusOne - parseInt(currentStep.value)) / stepCountMinusOne) * 255;
        var currentG = 0;
        var currentB = (parseInt(currentStep.value) / stepCountMinusOne) * 255;
        currentRgb = (Math.round(currentR) * 256 * 256 + Math.round(currentG) * 256 + Math.round(currentB)).toString(16);
        currentRgb = "000000".substr(0, 6 - currentRgb.length) + currentRgb;
    }

    if (w && h && map && currentStep)
    {
        var width = parseInt(w.value);
        var height = parseInt(h.value);
        var step = parseInt(currentStep.value);

        for (var i = map.rows.length - 1; i >= 0; i--)
            map.deleteRow(i);

        var rgb = testAlgo.rgbMap(width, height, currentRgb, step);

        for (var y = 0; y < height; y++)
        {
            var row = map.insertRow(y);

            for (var x = 0; x < width; x++)
            {
                var cell = row.insertCell(x);
		var rgbStr = rgb[y][x].toString(16);
		while (rgbStr.length != 6)
		  rgbStr = "0" + rgbStr;
                cell.style.backgroundColor = rgbStr;
                cell.style.height = 20;
                cell.style.width = 20;
            }
        }
    }
    else
    {
        alert("map element not found!");
    }
}
