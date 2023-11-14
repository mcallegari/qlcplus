/*
  Q Light Controller Plus
  horizontalline.js

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
    algo.name = "Horizonal Line";
    algo.author = "Aidan Young";
    algo.acceptColors = 1;

    algo.properties = new Array();
    algo.hlinePlace = 0;
    algo.hlineCustom = 0;
    algo.hlinePixels = 0
    algo.hlineUp = 0;

    algo.properties.push("name:hlinePlace|type:list|display:Placement|values:Left,Right,Custom|write:setPlace|read:getPlace");
    algo.properties.push("name:hlineCustom|type:range|display:Custom Value|values:0,10000|write:setCustom|read:getCustom");
    algo.properties.push("name:hlinePixels|type:range|display:Remove Pixels|values:0,10000|write:setPixels|read:getPixels")
    algo.properties.push("name:hlineUp|type:range|display:Move Up|values:0,10000|write:setUp|read:getUp")
    algo.setPlace = function (_hlinePlace){
        if (_hlinePlace === "Left"){
            algo.hlinePlace = 0;
        }
        else if (_hlinePlace === "Right"){
            algo.hlinePlace = 1;
        }
        else if (_hlinePlace === "Custom"){
            algo.hlinePlace = 2;
        }
    }
    algo.getPlace = function (){
        if(algo.hlinePlace === 0){
            return "Left";
        }
        else if(algo.hlinePlace === 1){
            return "Right";
        }
        else if (algo.hlinePlace === 2){
            return "Custom";
        }
    }

    algo.setCustom = function(_custom){
        algo.hlineCustom = _custom
    }; algo.getCustom = function(){
        return algo.hlineCustom
    };
    
    algo.setPixels = function(_pixels){
        algo.hlinePixels = _pixels
    }; algo.getPixels = function(){
        return algo.hlinePixels
    };

    algo.setUp = function(_up){
        algo.hlineUp = _up;
    }; algo.getUp = function(){
        return algo.hlineUp;
    };

    algo.rgbMap = function (width, height, rgb, step) {
        var map = new Array(height);
        for (var y = 0; y < height; y++) {
            map[y] = new Array(width);
            for (var x = 0; x < width; x++) {
                var yUp = y - algo.hlineUp;
                if (yUp >= 0 && yUp < height) {
                    if (algo.hlinePlace === 0) { // left
                        map[yUp][0] = (y >= algo.hlinePixels) && rgb; 
                    } else if (algo.hlinePlace === 1) { // right
                        map[yUp][width - 1] = (y >= algo.hlinePixels) && rgb; 
                    } else if (algo.hlinePlace === 2) { // custom
                        map[yUp][algo.hlineCustom - 1] = (y >= algo.hlinePixels) && rgb; 
                    } else {
                        map[yUp][x] = 0;
                    }}}}
        return map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return height;
    };


    // Development tool access
    testAlgo = algo;
    return algo;
}
)();