/*
  Q Light Controller Plus
  plasmacolors.js

  Copyright (c) Nathan Durnan

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
	var colorPaletteList = 
		"White," +
		"Cream," +
		"Pink," +
		"Rose," +
		"Coral," +
		"Dim Red," +
		"Red," +
		"Orange," +
		"Dim Orange," +
		"Goldenrod," +
		"Gold," +
		"Yellow," +
		"Dim Yellow," +
		"Lime," +
		"Pale Green," +
		"Dim Green," +
		"Green," +
		"Seafoam," +
		"Turquoise," +
		"Teal," +
		"Cyan," +
		"Electric Blue," +
		"Blue," +
		"Dim Blue," +
		"Pale Blue," +
		"Indigo," +
		"Purple," +
		"Violet," +
		"Magenta," +
		"Hot Pink," +
		"Deep Pink," +
		"OFF";
	var colorPalette = new Array(
		0xFFFFFF, // White
		0xFFFF7F, // Cream
		0xFF7F7F, // Pink
		0x7F3F3F, // Rose
		0x7F3F1F, // Coral
		0x7F0000, // Dim Red
		0xFF0000, // Red
		0xFF3F00, // Orange
		0x7F1F00, // Dim Orange
		0x7F3F00, // Goldenrod
		0xFF7F00, // Gold
		0xFFFF00, // Yellow
		0x7F7F00, // Dim Yellow
		0x7FFF00, // Lime
		0x3F7F00, // Pale Green
		0x007F00, // Dim Green
		0x00FF00, // Green
		0x00FF3F, // Seafoam
		0x007F3F, // Turquoise
		0x007F7F, // Teal
		0x00FFFF, // Cyan
		0x007FFF, // Electric Blue
		0x0000FF, // Blue
		0x00007F, // Dim Blue
		0x1F1F7F, // Pale Blue
		0x1F00BF, // Indigo
		0x3F00BF, // Purple
		0x7F007F, // Violet
		0xFF00FF, // Magenta
		0xFF003F, // Hot Pink
		0x7F001F, // Deep Pink
		0x000000); // OFF

	var algo = new Object;
	algo.apiVersion = 2;
	algo.name = "Plasma (Colors)";
	algo.author = "Nathan Durnan";
	algo.acceptColors = 0;
	algo.properties = new Array();
	algo.rstepcount = 0;
	algo.gstepcount = 50;
	algo.bstepcount = 100;
	algo.color1Index = 0;
	algo.properties.push(
		"name:color1Index|type:list|display:Color 1|" +
		"values:" + colorPaletteList + "|" +
		"write:setColor1|read:getColor1");
	algo.color2Index = 6;
	algo.properties.push(
		"name:color2Index|type:list|display:Color 2|" +
		"values:" + colorPaletteList + "|" +
		"write:setColor2|read:getColor2");
	algo.color3Index = 16;
	algo.properties.push(
		"name:color3Index|type:list|display:Color 3|" +
		"values:" + colorPaletteList + "|" +
		"write:setColor3|read:getColor3");
	algo.color4Index = 22;
	algo.properties.push(
		"name:color4Index|type:list|display:Color 4|" +
		"values:" + colorPaletteList + "|" +
		"write:setColor4|read:getColor4");
	algo.color5Index = 31;
	algo.properties.push(
		"name:color5Index|type:list|display:Color 5|" +
		"values:" + colorPaletteList + "|" +
		"write:setColor5|read:getColor5");
	algo.presetSize = 5;
	algo.properties.push(
		"name:presetSize|type:range|display:Size|" +
		"values:1,20|write:setSize|read:getSize");
	algo.ramp = 15;
	algo.properties.push(
		"name:ramp|type:range|display:Ramp|" +
		"values:10,30|write:setRamp|read:getRamp");
	algo.stepsize = 25;
	algo.properties.push(
		"name:stepsize|type:range|display:Speed|" +
		"values:1,50|write:setStep|read:getStep");
	algo.colorIndex = new Array(
		algo.color1Index,
		algo.color2Index,
		algo.color3Index,
		algo.color4Index,
		algo.color5Index);

	var util = new Object;
	util.initialized = false;
	util.gradientData = new Array();
	util.colorArray = new Array();

	algo.setColor = function(_index, _preset)
	{
		if      (_preset == "White"        ) algo.colorIndex[_index] =  0;
		else if (_preset == "Cream"        ) algo.colorIndex[_index] =  1;
		else if (_preset == "Pink"         ) algo.colorIndex[_index] =  2;
		else if (_preset == "Rose"         ) algo.colorIndex[_index] =  3;
		else if (_preset == "Coral"        ) algo.colorIndex[_index] =  4;
		else if (_preset == "Dim Red"      ) algo.colorIndex[_index] =  5;
		else if (_preset == "Red"          ) algo.colorIndex[_index] =  6;
		else if (_preset == "Orange"       ) algo.colorIndex[_index] =  7;
		else if (_preset == "Dim Orange"   ) algo.colorIndex[_index] =  8;
		else if (_preset == "Goldenrod"    ) algo.colorIndex[_index] =  9;
		else if (_preset == "Gold"         ) algo.colorIndex[_index] = 10;
		else if (_preset == "Yellow"       ) algo.colorIndex[_index] = 11;
		else if (_preset == "Dim Yellow"   ) algo.colorIndex[_index] = 12;
		else if (_preset == "Lime"         ) algo.colorIndex[_index] = 13;
		else if (_preset == "Pale Green"   ) algo.colorIndex[_index] = 14;
		else if (_preset == "Dim Green"    ) algo.colorIndex[_index] = 15;
		else if (_preset == "Green"        ) algo.colorIndex[_index] = 16;
		else if (_preset == "Seafoam"      ) algo.colorIndex[_index] = 17;
		else if (_preset == "Turquoise"    ) algo.colorIndex[_index] = 18;
		else if (_preset == "Teal"         ) algo.colorIndex[_index] = 19;
		else if (_preset == "Cyan"         ) algo.colorIndex[_index] = 20;
		else if (_preset == "Electric Blue") algo.colorIndex[_index] = 21;
		else if (_preset == "Blue"         ) algo.colorIndex[_index] = 22;
		else if (_preset == "Dim Blue"     ) algo.colorIndex[_index] = 23;
		else if (_preset == "Pale Blue"    ) algo.colorIndex[_index] = 24;
		else if (_preset == "Indigo"       ) algo.colorIndex[_index] = 25;
		else if (_preset == "Purple"       ) algo.colorIndex[_index] = 26;
		else if (_preset == "Violet"       ) algo.colorIndex[_index] = 27;
		else if (_preset == "Magenta"      ) algo.colorIndex[_index] = 28;
		else if (_preset == "Hot Pink"     ) algo.colorIndex[_index] = 29;
		else if (_preset == "Deep Pink"    ) algo.colorIndex[_index] = 30;
		else algo.colorIndex[_index] = 31;
		return algo.colorIndex[_index];
	}
	algo.getColor = function(_index)
	{
		if      (algo.colorIndex[_index] ==  0) return "White";
		else if (algo.colorIndex[_index] ==  1) return "Cream";
		else if (algo.colorIndex[_index] ==  2) return "Pink";
		else if (algo.colorIndex[_index] ==  3) return "Rose";
		else if (algo.colorIndex[_index] ==  4) return "Coral";
		else if (algo.colorIndex[_index] ==  5) return "Dim Red";
		else if (algo.colorIndex[_index] ==  6) return "Red";
		else if (algo.colorIndex[_index] ==  7) return "Orange";
		else if (algo.colorIndex[_index] ==  8) return "Dim Orange";
		else if (algo.colorIndex[_index] ==  9) return "Goldenrod";
		else if (algo.colorIndex[_index] == 10) return "Gold";
		else if (algo.colorIndex[_index] == 11) return "Yellow";
		else if (algo.colorIndex[_index] == 12) return "Dim Yellow";
		else if (algo.colorIndex[_index] == 13) return "Lime";
		else if (algo.colorIndex[_index] == 14) return "Pale Green";
		else if (algo.colorIndex[_index] == 15) return "Dim Green";
		else if (algo.colorIndex[_index] == 16) return "Green";
		else if (algo.colorIndex[_index] == 17) return "Seafoam";
		else if (algo.colorIndex[_index] == 18) return "Turquoise";
		else if (algo.colorIndex[_index] == 19) return "Teal";
		else if (algo.colorIndex[_index] == 20) return "Cyan";
		else if (algo.colorIndex[_index] == 21) return "Electric Blue";
		else if (algo.colorIndex[_index] == 22) return "Blue";
		else if (algo.colorIndex[_index] == 23) return "Dim Blue";
		else if (algo.colorIndex[_index] == 24) return "Pale Blue";
		else if (algo.colorIndex[_index] == 25) return "Indigo";
		else if (algo.colorIndex[_index] == 26) return "Purple";
		else if (algo.colorIndex[_index] == 27) return "Violet";
		else if (algo.colorIndex[_index] == 28) return "Magenta";
		else if (algo.colorIndex[_index] == 29) return "Hot Pink";
		else if (algo.colorIndex[_index] == 30) return "Deep Pink";
		else return "OFF";
	}

	algo.setColor1 = function(_preset)
	{
		algo.color1Index = algo.setColor(0, _preset);
		util.initialize();
	}
	algo.getColor1 = function()
	{
		return algo.getColor(0);
	}

	algo.setColor2 = function(_preset)
	{
		algo.color2Index = algo.setColor(1, _preset);
		util.initialize();
	}
	algo.getColor2 = function()
	{
		return algo.getColor(1);
	}

	algo.setColor3 = function(_preset)
	{
		algo.color3Index = algo.setColor(2, _preset);
		util.initialize();
	}
	algo.getColor3 = function()
	{
		return algo.getColor(2);
	}

	algo.setColor4 = function(_preset)
	{
		algo.color4Index = algo.setColor(3, _preset);
		util.initialize();
	}
	algo.getColor4 = function()
	{
		return algo.getColor(3);
	}

	algo.setColor5 = function(_preset)
	{
		algo.color5Index = algo.setColor(4, _preset);
		util.initialize();
	}
	algo.getColor5 = function()
	{
		return algo.getColor(4);
	}

	algo.setSize = function(_size)
	{
		algo.presetSize = _size;
		util.initialize();
	}
	algo.getSize = function()
	{
		return algo.presetSize;
	}

	algo.setRamp = function(_ramp)
	{
		algo.ramp = _ramp;
		util.initialize();
	}
	algo.getRamp = function()
	{
		return algo.ramp;
	}

	algo.setStep = function(_step)
	{
		algo.stepsize = _step;
		util.initialize();
	}
	algo.getStep = function()
	{
		return algo.stepsize;
	}

	util.initialize = function()
	{
		// calculate the gradient for the selected preset
		// with the given width
		var gradIdx = 0;
		util.gradientData = new Array();
		util.colorArray = new Array();
		for (var j = 0; j < algo.colorIndex.length; j++)
		{
			util.colorArray[j] = colorPalette[algo.colorIndex[j]];
		}
		for (var i = 0; i < util.colorArray.length; i++)
		{
			var sColor = util.colorArray[i];
			var eColor = util.colorArray[i + 1];
			if (eColor == undefined)
				eColor = util.colorArray[0];

			util.gradientData[gradIdx++] = sColor;
			var sr = (sColor >> 16) & 0x00FF;
			var sg = (sColor >> 8) & 0x00FF;
			var sb = sColor & 0x00FF;
			var er = (eColor >> 16) & 0x00FF;
			var eg = (eColor >> 8) & 0x00FF;
			var eb = eColor & 0x00FF;

			var stepR = ((er - sr) / 300);
			var stepG = ((eg - sg) / 300);
			var stepB = ((eb - sb) / 300);

			for (var s = 1; s < 300; s++)
			{
				var gradR = Math.floor(sr + (stepR * s)) & 0x00FF;
				var gradG = Math.floor(sg + (stepG * s)) & 0x00FF;
				var gradB = Math.floor(sb + (stepB * s)) & 0x00FF;
				var gradRGB = (gradR << 16) + (gradG << 8) + gradB;
				util.gradientData[gradIdx++] = gradRGB;
			}
		}
		util.initialized = true;
	}
	// This is a port of Ken Perlin's Java code. The
	// original Java code is at http://cs.nyu.edu/%7Eperlin/noise/.
	// Note that in this version, a number from 0 to 1 is returned.
	util.noise = function(x, y, z)
	{

		var p = new Array(512);
		var permutation = [ 151,160,137,91,90,15,131,13,201,95,96,53,194,233, 7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190, 6,148,247,120,234,75, 0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20, 125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229, 122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161,1, 216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100, 109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212, 207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152, 2,44,154, 163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232, 178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,49,192,214, 31,181,199,106,157,184, 84,204,176,115, 121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180];
		
		for (var i=0; i < 256 ; i++) 
			p[256+i] = p[i] = permutation[i]; 

		var X = Math.floor(x) & 255,                  // FIND UNIT CUBE THAT
			Y = Math.floor(y) & 255,                  // CONTAINS POINT.
			Z = Math.floor(z) & 255;
			x -= Math.floor(x);                       // FIND RELATIVE X,Y,Z
			y -= Math.floor(y);                       // OF POINT IN CUBE.
			z -= Math.floor(z);
		var u = fade(x),                              // COMPUTE FADE CURVES
			v = fade(y),                              // FOR EACH OF X,Y,Z.
			w = fade(z);
		var A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z, // HASH COORDINATES OF
			B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z; // THE 8 CUBE CORNERS,

		return scale(lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
			                grad(p[BA  ], x-1, y  , z   )),    // BLENDED
			        lerp(u, grad(p[AB  ], x  , y-1, z   ),     // RESULTS
			                grad(p[BB  ], x-1, y-1, z   ))),   // FROM  8
			lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),     // CORNERS
			                grad(p[BA+1], x-1, y  , z-1 )),    // OF CUBE
			        lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
			                grad(p[BB+1], x-1, y-1, z-1 )))));
	}
	function fade(t) { return t * t * t * (t * (t * 6 - 15) + 10); }
	function lerp( t, a, b) { return a + t * (b - a); }
	function grad(hash, x, y, z) {
		var h = hash & 15;                   // CONVERT LO 4 BITS OF HASH CODE
		var u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
			v = h<4 ? y : h==12||h==14 ? x : z;
		return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
	} 
	function scale(n) { return (1 + n)/2; }
	


	algo.rgbMap = function(width, height, rgb, step)
	{
		if (util.initialized == false)
			util.initialize();

		var size = algo.presetSize/2;  // set a scaling value
		var speed = Math.pow(100 , (algo.stepsize / 50))  // create a more uniform speed control
		algo.bstepcount += (speed / 500)
		algo.bstepcount = (algo.bstepcount % 256)  // A rolling step count for the noise function
		var square = width>height ? width : height; // keep the patten square
		
		var map = new Array(height);
		for (var y = 0; y < height; y++)
		{
			map[y] = new Array();

			for (var x = 0; x < width; x++)
			{
				var nx = x / square;  // Normalize nx & ny to 0 - 1
				var ny = y / square;
				var n = util.noise( size*nx, size*ny, algo.bstepcount );
				var gradStep = Math.round( Math.pow(n , (algo.ramp/10)) * util.gradientData.length);
				map[y][x] = util.gradientData[gradStep];
			}
		}

		return map;
	}

	algo.rgbMapStepCount = function(width, height)
	{
		if (util.initialized == false)
		util.initialize();

		return width * height;  // This make no difference to the script ;-)
	}

	// Development tool access
	testAlgo = algo;

	return algo;
}
)()
