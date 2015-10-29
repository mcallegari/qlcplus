/*
  Q Light Controller Plus
  squares.js

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
    algo.name = "Squares";
    algo.author = "Massimo Callegari";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.squaresAmount = 3;
    algo.properties.push("name:squaresAmount|type:range|display:Amount|values:1,10|write:setAmount|read:getAmount");
    algo.fadeMode = 0;
    algo.properties.push("name:fadeMode|type:list|display:Fade Mode|values:Don't Fade,Fade In,Fade Out|write:setFade|read:getFade");
    algo.fillSquares = 0;
    algo.properties.push("name:fillSquares|type:list|display:Fill squares|values:No,Yes|write:setFill|read:getFill");

    var util = new Object;
    util.initialized = false;
    util.squaresMaxSize = 0;
    
    var squares = new Array();
    
    function Square(x, y, step)
    {
      this.xCenter = x;
      this.yCenter = y;
      this.step = step;
    }
    
    algo.setAmount = function(_amount)
    {
      algo.squaresAmount = _amount;
      util.initialized = false;
    }
    
    algo.getAmount = function()
    {
      return algo.squaresAmount;
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
    
    algo.setFill = function(_fill)
    {
      if (_fill == "Yes")
	algo.fillSquares = 1;
      else
	algo.fillSquares = 0;
    }

    algo.getFill = function()
    {
      if (algo.fillSquares == 1)
        return "Yes";
      else
        return "No";
    }

    util.initialize = function(size)
    {
      if (size > 0)
	util.squaresMaxSize = size;

      squares = new Array();
      for (var i = 0; i < algo.squaresAmount; i++)
	squares[i] = new Square(-1, -1, 0);

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
	
	var stepCount = Math.floor(util.squaresMaxSize / 2);
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

    util.getNextStep = function(width, height, rgb)
    {
        // create an empty, black map
      	var map = new Array(height);
	for (var y = 0; y < height; y++)
	{
	  map[y] = new Array();
	  for (var x = 0; x < width; x++)
	    map[y][x] = 0;
	}
	
	for (var i = 0; i < algo.squaresAmount; i++)
	{
	  var color = util.getColor(squares[i].step, rgb);
	  //alert("Square " + i + " xCenter: " + squares[i].xCenter + " color: " + color.toString(16));
	  if (squares[i].xCenter == -1)
	  {
	    var seed = Math.floor(Math.random()*100)
            if (seed > 50) continue;
	    squares[i].xCenter = Math.floor(Math.random() * width);
	    squares[i].yCenter = Math.floor(Math.random() * height);
	    map[squares[i].yCenter][squares[i].xCenter] = color;
	  }
	  else
	  {
	    var firstY = squares[i].yCenter - squares[i].step;
	    var side = (squares[i].step * 2) + 1;
	    for (var sy = firstY; sy <= (firstY + side); sy++)
	    {
	      if (sy < 0 || sy >= height) continue;

	      var firstX = squares[i].xCenter - squares[i].step;

	      for (var sx = firstX; sx <= firstX + side; sx++)
	      {
		if (sx < 0 || sx >= width) continue;
		if (sy == firstY || sy == firstY + side || algo.fillSquares == 1) 
		  map[sy][sx] = color;
		else
		{
		  if (sx == firstX || sx == firstX + side)
		    map[sy][sx] = color;
		}
	      }
	    }
	  }

	  squares[i].step++;
	  if (squares[i].step >= (util.squaresMaxSize / 2))
	  {
	    squares[i].xCenter = -1;
	    squares[i].yCenter = -1;
	    squares[i].step = 0;
	  }
	}
	
	return map;
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
      if (height < width)
        return height;
      else
        return width;
    }

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)()
