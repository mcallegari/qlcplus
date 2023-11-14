/*
  Q Light Controller Plus
  verticalline.js

  Copyright (c) Aidan Young
  Based on work by Massimo Callegari and Branson Matheson

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
    algo.name = "Vertical Line";
    algo.author = "Aidan Young";
    algo.acceptColors = 1;

    algo.properties = new Array();
    algo.vlinePlace = 0;
    algo.vlineCustom = 0;
    algo.vlinePixels = 0
    algo.vlineRight = 0;

    algo.properties.push("name:vlinePlace|type:list|display:Placement|values:Top,Bottom,Custom|write:setPlace|read:getPlace");
    algo.properties.push("name:vlineCustom|type:range|display:Custom Value|values:0,10000|write:setCustom|read:getCustom");
    algo.properties.push("name:vlinePixels|type:range|display:Remove Pixels|values:0,10000|write:setPixels|read:getPixels")
    algo.properties.push("name:vlineRight|type:range|display:Move Right|values:0,10000|write:setRight|read:getRight")

    algo.setPlace = function (_vlinePlace){
        if (_vlinePlace === "Top"){
            algo.vlinePlace = 0;
        }
        else if (_vlinePlace === "Bottom"){
            algo.vlinePlace = 1;
        }
        else if (_vlinePlace === "Custom"){
            algo.vlinePlace = 2;
        }
    }
    algo.getPlace = function (){
        if(algo.vlinePlace === 0){
            return "Top";
        }
        else if(algo.vlinePlace === 1){
            return "Bottom";
        }
        else if (algo.vlinePlace === 2){
            return "Custom";
        }
    }

    algo.setCustom = function(_custom){
        algo.vlineCustom = _custom
    }; algo.getCustom = function(){
        return algo.vlineCustom
    };
    
    algo.setPixels = function(_pixels){
        algo.vlinePixels = _pixels
    }; algo.getPixels = function(){
        return algo.vlinePixels
    };

    algo.setRight = function(_right){
        algo.vlineRight = _right
    }; algo.getRight = function(){
        return algo.vlineRight
    };

    algo.rgbMap = function (width, height, rgb, step){
    var map = new Array(height);
        for (var y = 0; y < height; y++)
        {
            map[y] = new Array(width);
            } 
            for (var x = 0 + algo.vlinePixels; x < width; x++){
                if(algo.vlinePlace === 0){
                    map[0][x - algo.vlineRight] = rgb;
                }else if (algo.vlinePlace === 1){
                    map[y-1][x - algo.vlineRight] = rgb;
                }else if(algo.vlinePlace === 2){
                    map[algo.vlineCustom - 1][x - algo.vlineRight] = rgb;
                 }else{map[y][x]=0}}
                 
return map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return width;
    };


    // Development tool access
    testAlgo = algo;
    return algo;
}
)();