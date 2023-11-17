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
    algo.vlinePlace = 0;
    algo.vlineCustom = 0;
    algo.vlinePixels = 0
    algo.vlineRight = 0;

    algo.hlineCustom = 0;
    algo.hlinePixels = 0
    algo.hlineUp = 0;
    algo.hlinePlace = 0;
    algo.lineOrientation = 0;

    algo.properties.push("name:lineOrientation|type:list|display:Orientation|values:Vertical (V),Horizontal (H),Both|write:setOrientation|read:getOrientation");

    algo.properties.push("name:vlinePlace|type:list|display:V Placement|values:Top,Bottom,Custom|write:setVPlace|read:getVPlace");
    algo.properties.push("name:vlineCustom|type:range|display:V Custom Value|values:0,10000|write:setvCustom|read:getvCustom");
    algo.properties.push("name:vlinePixels|type:range|display:V Remove Pixels|values:0,10000|write:setvPixels|read:getvPixels");
    algo.properties.push("name:vlineRight|type:range|display:V Move Right|values:0,10000|write:setRight|read:getRight");
    
    algo.properties.push("name:hlinePlace|type:list|display:H Placement|values:Left,Right,Custom|write:setHPlace|read:getHPlace");
    algo.properties.push("name:hlineCustom|type:range|display:H Custom Value|values:0,10000|write:sethCustom|read:gethCustom");
    algo.properties.push("name:hlinePixels|type:range|display:H Remove Pixels|values:0,10000|write:sethPixels|read:gethPixels");
    algo.properties.push("name:hlineUp|type:range|display:H Move Up|values:0,10000|write:setUp|read:getUp");

    algo.setOrientation = function (_orientation){
        if(_orientation == "Vertical (V)"){
            algo.lineOrientation = 0;
        }else if (_orientation == "Horizontal (H)"){
            algo.lineOrientation = 1;
        }else if(_orientation == "Both"){
            algo.lineOrientation = 2;
        }
    }
    algo.getOrientation = function (){
        if(algo.lineOrientation === 0){
            return "Vertical (V)";
        } if(algo.lineOrientation === 1){
            return "Horizontal (H)";
        } if(algo.lineOrientation === 2){
            return "Both";
        }
    }

    algo.setVPlace = function (_vlinePlace){
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
    algo.getVPlace = function (){
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

    algo.setHPlace = function (_hlinePlace){
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
    algo.getHPlace = function (){
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

    algo.setvCustom = function(_custom){
        algo.vlineCustom = _custom
    }; algo.getvCustom = function(){
        return algo.vlineCustom
    };
    
    algo.setvPixels = function(_pixels){
        algo.vlinePixels = _pixels
    }; algo.getvPixels = function(){
        return algo.vlinePixels
    };

    algo.sethCustom = function(_custom){
        algo.hlineCustom = _custom
    }; algo.gethCustom = function(){
        return algo.hlineCustom
    };
    
    algo.sethPixels = function(_pixels){
        algo.hlinePixels = _pixels
    }; algo.gethPixels = function(){
        return algo.hlinePixels
    };

    algo.setRight = function(_right){
        algo.vlineRight = _right
    }; algo.getRight = function(){
        return algo.vlineRight
    };

    algo.setUp = function(_up){
        algo.hlineUp = _up;
    }; algo.getUp = function(){
        return algo.hlineUp;
    };


    algo.rgbMap = function (width, height, rgb, step){
    var map = new Array(height);
    if(algo.lineOrientation === 0){ //vertical
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
                }
    else if(algo.lineOrientation === 1){ //horizontal
        for (var y = 0; y < height; y++) {
            map[y] = new Array(width);
            for (var x = 0; x < width; x++) {
                var yUp = y - algo.hlineUp;
                if (yUp >= 0 && yUp < height) {
                    if (algo.hlinePlace === 0) { // left
                        map[yUp][0] = (y >= algo.hlinePixels) && rgb; 
                    } else if (algo.hlinePlace === 1) { // Right
                        map[yUp][width - 1] = (y >= algo.hlinePixels) && rgb; 
                    } else if (algo.hlinePlace === 2) { // custom
                        map[yUp][algo.hlineCustom - 1] = (y >= algo.hlinePixels) && rgb; 
                    } else {
                        map[yUp][x] = 0;
                    }}}}
                }
    else if (algo.lineOrientation === 2){ //both
        for (var y = 0; y < height; y++) {
            map[y] = new Array(width);

            for (var x = 0 + algo.vlinePixels; x < width; x++){ //both vertical
                if(algo.vlinePlace === 0){
                    map[0][x - algo.vlineRight] = rgb;
                }else if (algo.vlinePlace === 1){
                    map[y-1][x - algo.vlineRight] = rgb;
                }else if(algo.vlinePlace === 2){
                    map[algo.vlineCustom - 1][x - algo.vlineRight] = rgb;
                 }else{map[y][x]=0}}

                 for (var x = 0; x < width; x++) { //both horizontal
                    var yUp = y - algo.hlineUp;
                    if (yUp >= 0 && yUp < height) {
                        if (algo.hlinePlace === 0) { // left
                            map[yUp][0] = (y >= algo.hlinePixels) && rgb; 
                        } else if (algo.hlinePlace === 1) { // Right
                            map[yUp][width - 1] = (y >= algo.hlinePixels) && rgb; 
                        } else if (algo.hlinePlace === 2) { // custom
                            map[yUp][algo.hlineCustom - 1] = (y >= algo.hlinePixels) && rgb; 
                        } else {
                            map[yUp][x] = 0;
                        }}}
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