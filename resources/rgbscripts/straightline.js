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
    function () {
        var algo = new Object;
        algo.apiVersion = 2;
        algo.name = "Straight Line";
        algo.author = "Aidan Young";
        algo.acceptColors = 2;

        algo.properties = new Array();
        algo.hlinePlace = 0;
        algo.hlineCustom = 0;
        algo.hlinePixels = 0
        algo.hlineLeft = 0;
        algo.hlineEffect = 0;

        algo.vlineCustom = 0;
        algo.vlinePixels = 0
        algo.vlineUp = 0;
        algo.vlinePlace = 0;
        algo.vlineEffect = 0;

        algo.lineOrientation = 0;

        algo.properties.push("name:lineOrientation|type:list|display:Orientation|values:Horizontal (H),Vertical (V)|write:setOrientation|read:getOrientation");

        algo.properties.push("name:hlinePlace|type:list|display:H Placement|values:Top,Middle,Bottom,Custom|write:setHPlace|read:getHPlace");
        algo.properties.push("name:hlineEffect|type:list|display:H Effect|values:None,Fill,One By One|write:setHEffect|read:getHEffect");
        algo.properties.push("name:hlineCustom|type:range|display:H Custom Value|values:0,10000|write:setHCustom|read:getHCustom");
        algo.properties.push("name:hlinePixels|type:range|display:H Remove Pixels|values:0,10000|write:setHPixels|read:getHPixels");
        algo.properties.push("name:hlineLeft|type:range|display:H Move Left|values:0,10000|write:setLeft|read:getLeft");

        algo.properties.push("name:vlinePlace|type:list|display:V Placement|values:Left,Middle,Right,Custom|write:setVPlace|read:getHPlace");
        algo.properties.push("name:vlineEffect|type:list|display:V Effect|values:None,Fill,One By One|write:setVEffect|read:getVEffect");
        algo.properties.push("name:vlineCustom|type:range|display:V Custom Value|values:0,10000|write:setVCustom|read:getVCustom");
        algo.properties.push("name:vlinePixels|type:range|display:V Remove Pixels|values:0,10000|write:setVPixels|read:getVPixels");
        algo.properties.push("name:vlineUp|type:range|display:V Move Up|values:0,10000|write:setUp|read:getUp");

        algo.setOrientation = function (_orientation) {
            if (_orientation == "Vertical (V)") {
                algo.lineOrientation = 1;
            } else if (_orientation == "Horizontal (H)") {
                algo.lineOrientation = 0;
            }
            algo.getOrientation = function () {
                if (algo.lineOrientation === 1) {
                    return "Vertical (V)";
                } if (algo.lineOrientation === 0) {
                    return "Horizontal (H)";
                }
            }
        }

        algo.setHPlace = function (_hlinePlace) {
            if (_hlinePlace === "Top") {
                algo.hlinePlace = 0;
            }
            else if (_hlinePlace === "Middle") {
                algo.hlinePlace = 1;
            }
            else if (_hlinePlace === "Bottom") {
                algo.hlinePlace = 2;
            }
            else if (_hlinePlace === "Custom") {
                algo.hlinePlace = 3;
            }
        }
        algo.getHPlace = function () {
            if (algo.hlinePlace === 0) {
                return "Top";
            }
            else if (algo.hlinePlace === 2) {
                return "Bottom";
            }
            else if (algo.hlinePlace === 1) {
                return "Middle";
            }
            else if (algo.hlinePlace === 3) {
                return "Custom";
            }
        }

        algo.setVPlace = function (_vlinePlace) {
            if (_vlinePlace === "Left") {
                algo.vlinePlace = 0;
            }
            else if (_vlinePlace === "Middle") {
                algo.vlinePlace = 1;
            }
            else if (_vlinePlace === "Right") {
                algo.vlinePlace = 2;
            }
            else if (_vlinePlace === "Custom") {
                algo.vlinePlace = 3;
            }
        }
        algo.getVPlace = function () {
            if (algo.vlinePlace === 0) {
                return "Left";
            }
            else if (algo.vlinePlace === 1) {
                return "Middle";
            }
            else if (algo.vlinePlace === 2) {
                return "Right";
            }
            else if (algo.vlinePlace === 2) {
                return "Custom";
            }
        }

        algo.setHEffect = function (_effect) {
            if (_effect === "None") {
                algo.hlineEffect = 0;
            } else if (_effect === "Fill") {
                algo.hlineEffect = 1;
            } else if (_effect === "One By One"){
                algo.hlineEffect = 2;
            }
        }
        algo.getHEffect = function () {
            if (algo.hlineEffect === 0) {
                return "None";
            } else if (algo.hlineEffect === 1) {
                return "Fill";
            } else if (algo.hlineEffect === 2){
                return "One By One";
            }
        }

        algo.setVEffect = function (_effect) {
            if (_effect === "None") {
                algo.vlineEffect = 0;
            } else if (_effect === "Fill") {
                algo.vlineEffect = 1;
            } else if (_effect === "One By One"){
                algo.vlineEffect = 2;
            }
        }
        algo.getVEffect = function () {
            if (algo.vlineEffect === 0) {
                return "None";
            } else if (algo.vlineEffect === 1) {
                return "Fill";
            } else if (algo.vlineEffect === 2){
                return "One By One";
            }
        }

        algo.setHCustom = function (_custom) {
            algo.hlineCustom = _custom
        }; algo.getHCustom = function () {
            return algo.hlineCustom
        };

        algo.setHPixels = function (_pixels) {
            algo.hlinePixels = _pixels
        }; algo.getHPixels = function () {
            return algo.hlinePixels
        };

        algo.setVCustom = function (_custom) {
            algo.vlineCustom = _custom
        }; algo.getVCustom = function () {
            return algo.vlineCustom
        };

        algo.setVPixels = function (_pixels) {
            algo.vlinePixels = _pixels
        }; algo.getVPixels = function () {
            return algo.vlinePixels
        };

        algo.setLeft = function (_Left) {
            algo.hlineLeft = _Left
        }; algo.getLeft = function () {
            return algo.hlineLeft
        };

        algo.setUp = function (_up) {
            algo.vlineUp = _up;
        }; algo.getUp = function () {
            return algo.vlineUp;
        };


        algo.rgbMap = function (width, height, rgb, step) {
            var map = new Array(height);
            if (algo.lineOrientation === 0) { //horizontal
                for (var y = 0; y < height; y++) {
                    map[y] = new Array(width);
                }
                for (var x = 0 + algo.hlinePixels; x < width; x++) {

                    var effect;
                    if (algo.hlineEffect === 1) {
                        effect = (x <= step);
                    }
                    else {
                        effect = 1;
                    }

                    if (algo.hlinePlace === 0) { // top
                        map[0][x - algo.hlineLeft] = (effect) && rgb;
                    } else if (algo.hlinePlace === 1) { // middle
                        var middleY = (height / 2)
                        map[middleY][x - algo.hlineLeft] = (effect) && rgb;
                    } else if (algo.hlinePlace === 2) { // bottom
                        map[y - 1][x - algo.hlineLeft] = (effect) && rgb;
                    } else if (algo.hlinePlace === 3) { // custom
                        map[algo.hlineCustom - 1][x - algo.hlineLeft] = (effect) && rgb;
                    } else { map[y][x] = 0 }
                }
            }

            else if (algo.lineOrientation === 1) { //vertical
                for (var y = 0; y < height; y++) {
                    map[y] = new Array(width);

                    for (var x = 0; x < width; x++) {
                        var yUp = y - algo.vlineUp;
                        var effect;
                        if (algo.vlineEffect === 1) { //fill
                            effect = ((yUp <= step - algo.vlinePixels)); //to do --- figure out a way for the effect to conform to how big/small the line is (if possible)
                        } else if (algo.vlineEffect === 2) { //one by one
                            effect = (yUp <= step);
                            }
                        else {
                            effect = 1;
                        }


                        if (yUp >= 0 && yUp < height) {
                            if (algo.vlinePlace === 0) { // left
                                map[yUp][0] = (y >= algo.vlinePixels) && effect && rgb;
                            } else if (algo.vlinePlace === 1) { //middle
                                var middleX = (width / 2)
                                map[yUp][middleX] = (y >= algo.vlinePixels) && (effect) && rgb;
                            } else if (algo.vlinePlace === 2) { // Right
                                map[yUp][width - 1] = (y >= algo.vlinePixels) && (effect) && rgb;
                            } else if (algo.vlinePlace === 3) { // custom
                                map[yUp][algo.vlineCustom - 1] = (y >= algo.vlinePixels) && (effect) && rgb;
                            } else {
                                map[yUp][x] = 0;
                            }
                        }
                    }
                }
            }

            return map;
        };

        algo.rgbMapStepCount = function (width, height) {
            if (algo.lineOrientation === 0) {
                return width;
            }
            else if (algo.lineOrientation === 1  && algo.vlineEffect === 0) {
                return height;
            }
            else if (algo.lineOrientation === 1 && algo.vlineEffect === 2){
                return;
            }
        };


        // Development tool access
        testAlgo = algo;
        return algo;
    }
)();