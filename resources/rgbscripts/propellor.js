/*
  Q Light Controller Plus
  propellor.js

  Copyright (c) Christoph Klenge

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
    algo.name = "Propellor";
    algo.author = "Christoph Klenge";
    algo.acceptColors = 2;

    algo.properties = new Array();
    algo.barWidth = 2;
    algo.properties.push("name:barWidth|type:range|display:Bar Width|values:1,8|write:setBarWidth|read:getBarWidth");
    algo.direction = 1;
    algo.properties.push("name:direction|type:list|display:Direction|values:Clockwise,Counterclockwise|write:setDirection|read:getDirection");
    algo.centerGap = 0;
    algo.properties.push("name:centerGap|type:range|display:Center Gap|values:0,8|write:setCenterGap|read:getCenterGap");
    	
  algo.setBarWidth = function(barWidth)
    {
      algo.barWidth = barWidth;
    };
  algo.getBarWidth = function()
    {
      return algo.barWidth;
    };

  algo.setDirection = function(direction)
    {
      if (direction == "Clockwise"){algo.direction = 1;}
      else if (direction == "Counterclockwise"){algo.direction = -1;}
      else {algo.direction = 1;}
    };
  algo.getDirection = function()
    {
      if (direction == 1){return"Clockwise";}
      else if (direction == -1){return"Counterclockwise";}
      return "none";
    };

  algo.setCenterGap = function(centerGap)
    {
      algo.centerGap = centerGap;
    };
  algo.getCenterGap = function()
    {
      return algo.centerGap;
    };
  

  algo.rgbMap = function(width, height, rgb, step)
    {   
      var map = new Array(height);
      for (var y = 0; y < height; y++)
      {
          map[y] = new Array();
          for (var x = 0; x < width; x++)
          {
            angle = algo.direction*step*(2*3.14)/algo.rgbMapStepCount(width, height)
            verticalOffset = (height-1)/2-y
            horizontalOffset = (width-1)/2-x
            distance = Math.sqrt(horizontalOffset*horizontalOffset+ verticalOffset*verticalOffset)
            normalDistance = Math.abs(horizontalOffset*Math.cos(angle)+verticalOffset*Math.sin(angle))
            if (normalDistance <= algo.barWidth/2 && distance >= algo.centerGap/2)
              map[y][x] = rgb;
            else 
              map[y][x]  = 0;
          }
      }
      return map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return Math.ceil(Math.max(width, height)/4)*8
    };

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)();
