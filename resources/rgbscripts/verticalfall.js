/*
  Q Light Controller Plus
  verticalfall.js

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
        algo.apiVersion = 1;
        algo.name = "Vertical fall";
        algo.author = "Massimo Callegari";
	algo.acceptColors = 1;
        
        var util = new Object;
        util.initialized = false;
        util.color = 0xFF0000;
        util.fallObject = new Array();
        util.objYPos = new Array();
        util.objmap = new Array();
	
	util.initialize = function(rgb, width, height)
	{
            var r = (rgb >> 16) & 0x00FF;
            var g = (rgb >> 8) & 0x00FF;
            var b = rgb & 0x00FF;
            
            var rStep = (r / height);
            var gStep = (g / height);
            var bStep = (b / height);
            
            objYPos = new Array(width);
            for (var i = 0; i < width; i++)
                objYPos[i] = -1;

            fallObject = new Array(height);
            fallObject[0] = rgb;
            fallObject[height - 1] = 0;
            for (var f = 1; f < height - 1; f++)
            {
                var stepRGB = (rStep * (height - f - 1)) << 16;
                stepRGB += (gStep * (height - f - 1)) << 8;
                stepRGB += (bStep * (height - f - 1));
                fallObject[f] = stepRGB;
            }
            
            objmap = new Array(height);
            for (var y = 0; y < height; y++)
            {
                objmap[y] = new Array(width);
                for (var x = 0; x < width; x++)
                    objmap[y][x] = 0;
            }

            util.color = rgb;
	    util.initialized = true;
	}
	
	util.getNextStep = function(width, height)
        {
            for (var x = 0; x < width; x++)
            {
                if (objYPos[x] == -1)
                {
                    // this decides the amount of falling objects
                    var seed = Math.floor(Math.random()*100)
                    if (seed > 80)
                        objYPos[x] = 0;
                }
                
                if (objYPos[x] >= 0)
                {
                    var yPos = objYPos[x];
                    for (var i = 0; i < height; i++)
                    {
                        if (yPos < height)
                            objmap[yPos][x] = fallObject[i];
                        yPos--;
                        if (yPos == -1)
                            break;
                    }
                    objYPos[x]++;
                }
                if (objYPos[x] == height * 2)
                    objYPos[x] = -1;
            }
            return objmap;
        }

        algo.rgbMap = function(width, height, rgb, step)
        {
            if (util.initialized == false || util.color != rgb)
                util.initialize(rgb, width, height);

            return util.getNextStep(width, height);
        }

        algo.rgbMapStepCount = function(width, height)
        {
            return 1;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
