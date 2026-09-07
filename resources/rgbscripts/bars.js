/*
  Q Light Controller Plus
  Bars (ANIMATED - Configurable Width & Gap)

  FIXED: Configurable properties now use the "type:range|display:Label|values:min,max" 
  syntax confirmed to work in QLC+ scripts like propellor.js.
  
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
    algo.name = "Bars";
    algo.author = "Google Gemini AI (via markzvo)";

    // Initialize internal variables first, as seen in propellor.js and stripes.js
    algo.orientation = 0; 
    algo.barWidth = 2;  // Thickness of the 'ON' stripe
    algo.gapWidth = 2;  // Thickness of the 'OFF' gap

    // --- Properties Definition ---
    algo.properties = new Array();
    
    // Property 1: Orientation 
    algo.properties.push("name:orientation|type:list|display:Orientation|values:Horizontal,Vertical|write:setOrientation|read:getOrientation");
    
    // Property 2: Bar Thickness (FIXED: Using type:range and values:min,max)
    // Range type is used for integer sliders, similar to propellor.js
    algo.properties.push("name:barWidth|type:range|display:Bar Thickness|values:1,30|write:setBarWidth|read:getBarWidth");

    // Property 3: Gap Thickness (FIXED: Using type:range and values:min,max)
    algo.properties.push("name:gapWidth|type:range|display:Gap Thickness|values:1,30|write:setGapWidth|read:getGapWidth");


    // --- Orientation Accessors ---
    algo.setOrientation = function(_orientation)
    {
      if (_orientation === "Vertical") { algo.orientation = 1; }
      else { algo.orientation = 0; }
    };

    algo.getOrientation = function()
    {
      if (algo.orientation === 1) { return "Vertical"; }
      else { return "Horizontal"; }
    };
    
    // --- Bar Width Accessors ---
    algo.setBarWidth = function(_barWidth)
    {
      // The value comes in as a string and must be parsed
      algo.barWidth = parseInt(_barWidth);
      if (algo.barWidth < 1) algo.barWidth = 1; // Safety check
    };

    algo.getBarWidth = function()
    {
      return algo.barWidth;
    };

    // --- Gap Width Accessors ---
    algo.setGapWidth = function(_gapWidth)
    {
      // The value comes in as a string and must be parsed
      algo.gapWidth = parseInt(_gapWidth);
      if (algo.gapWidth < 1) algo.gapWidth = 1; // Safety check
    };

    algo.getGapWidth = function()
    {
      return algo.gapWidth;
    };


    // --- Main Mapping Function ---
    algo.rgbMap = function(width, height, rgb, step)
    {
      var map = new Array(height);
      
      var w = algo.barWidth;  // Stripe ON width
      var g = algo.gapWidth;  // Stripe OFF width
      var cycle = w + g;      // Total cycle length

      for (var y = 0; y < height; y++)
      {
          map[y] = new Array();
          for (var x = 0; x < width; x++)
          {
            var isStripe = false;
            var position;
            
            // Determine the coordinate to use based on orientation
            if (algo.orientation === 1) { // Vertical Stripes: pattern repeats and scrolls on X
                position = x;
            } else { // Horizontal Stripes: pattern repeats and scrolls on Y
                position = y;
            }
            
            // Animated Check: Shift the pattern by 'step'
            // If ((current position + animation offset) MOD cycle length) is less than the bar width, it's ON.
            if (((position + step) % cycle) < w) {
                isStripe = true;
            }

            if (isStripe) {
                map[y][x] = rgb; // Stripe color (current color from QLC+ function)
            } else {
                map[y][x] = 0;   // Gap color (black/off)
            }
          }
      }

      return map;
    };

    // --- Step Count Function ---
    // The number of steps is equal to the total pattern cycle length (bar + gap).
    algo.rgbMapStepCount = function(width, height)
    {
      // Return the dynamically calculated cycle length
      return algo.barWidth + algo.gapWidth; 
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
