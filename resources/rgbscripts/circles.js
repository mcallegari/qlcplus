/*
  Q Light Controller Plus
  circles.js

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

// Development tool access
var testAlgo;

(
  function()
  {
    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Circles";
    algo.author = "Massimo Callegari";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.circlesAmount = 3;
    algo.properties.push("name:circlesAmount|type:range|display:Amount|values:1,10|write:setAmount|read:getAmount");
    algo.fadeMode = 0;
    algo.properties.push("name:fadeMode|type:list|display:Fade Mode|values:Don't Fade,Fade In,Fade Out|write:setFade|read:getFade");
    //algo.fillCircles = 0;
    //algo.properties.push("name:fillCircles|type:list|display:Fill circles|values:No,Yes|write:setFill|read:getFill");

    var util = new Object;
    util.pixelMap = new Array();
    util.initialized = false;
    util.circlesMaxSize = 0;
    
    var circles = new Array();
    
    function Circle(x, y, step)
    {
      this.xCenter = x;
      this.yCenter = y;
      this.step = step;
    }
    
    algo.setAmount = function(_amount)
    {
      algo.circlesAmount = _amount;
      util.initialized = false;
    }
    
    algo.getAmount = function()
    {
      return algo.circlesAmount;
    }
    
    algo.setFade = function(_fade)
    {
      if (_fade == "Fade In")
	algo.fadeMode = 1;
      else if (_fade == "Fade Out")
	algo.fadeMode = 2;
      else
	algo.fadeMode = 0;
    }

    algo.getFade = function()
    {
      if (algo.fadeMode == 1)
	return "Fade In";
      else if (algo.fadeMode == 2)
	return "Fade Out";
      else
	return "Don't Fade";
    }
/*
    algo.setFill = function(_fill)
    {
      if (_fill == "Yes")
	algo.fillCircles = 1;
      else
	algo.fillCircles = 0;
    }

    algo.getFill = function()
    {
      if (algo.fillCircles == 1)
	return "Yes";
      else
	return "No";
    }
*/
    util.initialize = function(size)
    {
      if (size > 0)
	util.circlesMaxSize = size;

      circles = new Array();
      for (var i = 0; i < algo.circlesAmount; i++)
	circles[i] = new Circle(-1, -1, 0);

      util.initialized = true;
    }
    
    util.getColor = function(step, rgb)
    {
      if (algo.fadeMode == 0)
	return rgb;
      else
      {
	var r = (rgb >> 16) & 0x00FF;
	var g = (rgb >> 8) & 0x00FF;
	var b = rgb & 0x00FF;
	
	var stepCount = Math.floor(util.circlesMaxSize / 2);
	var fadeStep = step;
	if (algo.fadeMode == 2)
	  fadeStep = stepCount - step;

	var newR = (r / stepCount) * fadeStep;
	var newG = (g / stepCount) * fadeStep;
	var newB = (b / stepCount) * fadeStep;
	var newRGB = (newR << 16) + (newG << 8) + newB;
	return newRGB;
      }
    }

    util.drawPixel = function(cx, cy, color, width, height)
    {
      //cx = cx.toFixed(0);
      //cy = cy.toFixed(0);
      cx = Math.round(cx);
      cy = Math.round(cy);
      if (cx >= 0 && cx < width && cy >= 0 && cy < height)
	util.pixelMap[cy][cx] = color;
    }

    util.getNextStep = function(width, height, rgb)
    {
        // create an empty, black pixelMap
      	util.pixelMap = new Array(height);
	for (var y = 0; y < height; y++)
	{
	  util.pixelMap[y] = new Array(width);
	  for (var x = 0; x < width; x++)
	    util.pixelMap[y][x] = 0;
	}
	
	for (var i = 0; i < algo.circlesAmount; i++)
	{
	  var color = util.getColor(circles[i].step, rgb);
	  //alert("Circle " + i + " xCenter: " + circles[i].xCenter + " color: " + color.toString(16));
	  if (circles[i].xCenter == -1)
	  {
	    var seed = Math.floor(Math.random()*100)
            if (seed > 50) continue;
	    circles[i].xCenter = Math.floor(Math.random() * width);
	    circles[i].yCenter = Math.floor(Math.random() * height);
	    util.pixelMap[circles[i].yCenter][circles[i].xCenter] = color;
	  }
	  else
	  {
	    /*
	    var radius = circles[i].step;
	    var radius2 = radius * radius;
	    for (var x = -radius; x <= radius; x++) 
	    {
	      var y = Math.floor(Math.sqrt(radius2 - x*x) + 0.5);
	      var cx = circles[i].xCenter + x;
	      var cy1 = circles[i].yCenter + y;
	      var cy2 = circles[i].yCenter - y;
	      if (cx >= 0 && cx < width)
	      {
		if (cy1 >= 0 && cy1 < height)
		  map[cy1][cx] = color;
		if (cy2 >= 0 && cy2 < height)
		  map[cy2][cx] = color;
	      }
	    }
	    */
	    var l = circles[i].step * Math.cos(Math.PI / 4);
	    var radius2 = circles[i].step * circles[i].step;
	    l = l.toFixed(0);

	    for (var x = 0; x <= l; x++) 
	    {
	      var y = Math.sqrt(radius2 - (x * x));

	      util.drawPixel(circles[i].xCenter + x, circles[i].yCenter + y, color, width, height);
	      util.drawPixel(circles[i].xCenter + x, circles[i].yCenter - y, color, width, height);
	      util.drawPixel(circles[i].xCenter - x, circles[i].yCenter + y, color, width, height);
	      util.drawPixel(circles[i].xCenter - x, circles[i].yCenter - y, color, width, height);

	      util.drawPixel(circles[i].xCenter + y, circles[i].yCenter + x, color, width, height);
	      util.drawPixel(circles[i].xCenter + y, circles[i].yCenter - x, color, width, height);
	      util.drawPixel(circles[i].xCenter - y, circles[i].yCenter + x, color, width, height);
	      util.drawPixel(circles[i].xCenter - y, circles[i].yCenter - x, color, width, height);
	    }
	  }

	  circles[i].step++;
	  if (circles[i].step >= (util.circlesMaxSize / 2))
	  {
	    circles[i].xCenter = -1;
	    circles[i].yCenter = -1;
	    circles[i].step = 0;
	  }
	}
	
	return util.pixelMap;
    }

    algo.rgbMap = function(width, height, rgb, step)
    {
      	if (util.initialized == false)
	{
	  if (height < width)
	    util.initialize(height);
	  else
	    util.initialize(width);
	}

	return util.getNextStep(width, height, rgb);
    }

    algo.rgbMapStepCount = function(width, height)
    {
      return width;
    }

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)()
