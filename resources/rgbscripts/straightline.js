/*
  Q Light Controller Plus
  straightline.js

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
    algo.name = "Straight Line";
    algo.author = "Aidan Young";
    algo.acceptColors = 1;

    algo.properties = new Array();
    algo.hlinePlace = 0;
    algo.hlineCustom = 0;
    algo.hlinePixels = 0
    algo.hlineRight = 0;

    algo.vlineCustom = 0;
    algo.vlinePixels = 0
    algo.vlineUp = 0;
    algo.vlinePlace = 0;

    algo.lineOrientation = 0;

    algo.properties.push("name:lineOrientation|type:list|display:Orientation|values:Horizontal (H),Vertical (V),Both|write:setOrientation|read:getOrientation");

    algo.properties.push("name:hlinePlace|type:list|display:H Placement|values:Top,Bottom,Custom|write:setHPlace|read:getHPlace");
    algo.properties.push("name:hlineCustom|type:range|display:H Custom Value|values:0,10000|write:setHCustom|read:getHCustom");
    algo.properties.push("name:hlinePixels|type:range|display:H Remove Pixels|values:0,10000|write:setHPixels|read:getHPixels");
    algo.properties.push("name:hlineRight|type:range|display:H Move Right|values:0,10000|write:setRight|read:getRight");
    
    algo.properties.push("name:vlinePlace|type:list|display:V Placement|values:Left,Right,Custom|write:setVPlace|read:getHPlace");
    algo.properties.push("name:vlineCustom|type:range|display:V Custom Value|values:0,10000|write:setVCustom|read:getVCustom");
    algo.properties.push("name:vlinePixels|type:range|display:V Remove Pixels|values:0,10000|write:setVPixels|read:getVPixels");
    algo.properties.push("name:vlineUp|type:range|display:V Move Up|values:0,10000|write:setUp|read:getUp");

    algo.setOrientation = function (_orientation){
        if(_orientation == "Vertical (V)"){
            algo.lineOrientation = 1;
        }else if (_orientation == "Horizontal (H)"){
            algo.lineOrientation = 0;
        }else if(_orientation == "Both"){
            algo.lineOrientation = 2;
        }
    }
    algo.getOrientation = function (){
        if(algo.lineOrientation === 1){
            return "Vertical (V)";
        } if(algo.lineOrientation === 0){
            return "Horizontal (H)";
        } if(algo.lineOrientation === 2){
            return "Both";
        }
    }

    algo.setHPlace = function (_hlinePlace){
        if (_hlinePlace === "Top"){
            algo.hlinePlace = 0;
        }
        else if (_hlinePlace === "Bottom"){
            algo.hlinePlace = 1;
        }
        else if (_hlinePlace === "Custom"){
            algo.hlinePlace = 2;
        }
    }
    algo.getHPlace = function (){
        if(algo.hlinePlace === 0){
            return "Top";
        }
        else if(algo.hlinePlace === 1){
            return "Bottom";
        }
        else if (algo.hlinePlace === 2){
            return "Custom";
        }
    }

    algo.setVPlace = function (_vlinePlace){
        if (_vlinePlace === "Left"){
            algo.vlinePlace = 0;
        }
        else if (_vlinePlace === "Right"){
            algo.vlinePlace = 1;
        }
        else if (_vlinePlace === "Custom"){
            algo.vlinePlace = 2;
        }
    }
    algo.getVPlace = function (){
        if(algo.vlinePlace === 0){
            return "Left";
        }
        else if(algo.vlinePlace === 1){
            return "Right";
        }
        else if (algo.vlinePlace === 2){
            return "Custom";
        }
    }

    algo.setHCustom = function(_custom){
        algo.hlineCustom = _custom
    }; algo.getHCustom = function(){
        return algo.hlineCustom
    };
    
    algo.setHPixels = function(_pixels){
        algo.hlinePixels = _pixels
    }; algo.getHPixels = function(){
        return algo.hlinePixels
    };

    algo.setVCustom = function(_custom){
        algo.vlineCustom = _custom
    }; algo.getVCustom = function(){
        return algo.vlineCustom
    };
    
    algo.setVPixels = function(_pixels){
        algo.vlinePixels = _pixels
    }; algo.getVPixels = function(){
        return algo.vlinePixels
    };

    algo.setRight = function(_right){
        algo.hlineRight = _right
    }; algo.getRight = function(){
        return algo.hlineRight
    };

    algo.setUp = function(_up){
        algo.vlineUp = _up;
    }; algo.getUp = function(){
        return algo.vlineUp;
    };


    algo.rgbMap = function (width, height, rgb, step){
    var map = new Array(height); 
    if(algo.lineOrientation === 0){ //horizontal
        for (var y = 0; y < height; y++)
        {
            map[y] = new Array(width);
            } 
            for (var x = 0 + algo.hlinePixels; x < width; x++){
                if(algo.hlinePlace === 0){
                    map[0][x - algo.hlineRight] = rgb;
                }else if (algo.hlinePlace === 1){
                    map[y-1][x - algo.hlineRight] = rgb;
                }else if(algo.hlinePlace === 2){
                    map[algo.hlineCustom - 1][x - algo.hlineRight] = rgb;
                 }else{map[y][x]=0}}
                }
    else if(algo.lineOrientation === 1){ //vertical
        for (var y = 0; y < height; y++) {
            map[y] = new Array(width);
            for (var x = 0; x < width; x++) {
                var yUp = y - algo.vlineUp;
                if (yUp >= 0 && yUp < height) {
                    if (algo.vlinePlace === 0) { // left
                        map[yUp][0] = (y >= algo.vlinePixels) && rgb; 
                    } else if (algo.vlinePlace === 1) { // Right
                        map[yUp][width - 1] = (y >= algo.vlinePixels) && rgb; 
                    } else if (algo.vlinePlace === 2) { // custom
                        map[yUp][algo.vlineCustom - 1] = (y >= algo.vlinePixels) && rgb; 
                    } else {
                        map[yUp][x] = 0;
                    }}}}
                }
    else if (algo.lineOrientation === 2) { // both
    for (var y = 0; y < height; y++) {
        map[y] = new Array(width);
        var vy = y;

        for (var hx = 0 + algo.hlinePixels; hx < width; hx++) { // both horizontal
            if (algo.hlinePlace === 0) {
                map[0][hx - algo.hlineRight] = rgb;
            } else if (algo.hlinePlace === 1) {
                map[height - 1][hx - algo.hlineRight] = rgb;
            } else if (algo.hlinePlace === 2) {
                map[algo.hlineCustom - 1][hx - algo.hlineRight] = rgb;
            } else {
                map[y][hx] = 0;
            }

            for (var vx = 0; vx < width; vx++) { // both vertical
            var yUp = vy - algo.vlineUp;
            if (yUp >= 0 && yUp < height) {
                if (algo.vlinePlace === 0) { // left
                    map[yUp][0] = (vy >= algo.vlinePixels) && rgb;
                } else if (algo.vlinePlace === 1) { // Right
                    map[yUp][width - 1] = (vy >= algo.vlinePixels) && rgb;
                } else if (algo.vlinePlace === 2) { // custom
                    map[yUp][algo.vlineCustom - 1] = (vy >= algo.vlinePixels) && rgb;
                } else {
                    map[yUp][vx] = 0;
                }
            }
        }
        }
    }
}

                 
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