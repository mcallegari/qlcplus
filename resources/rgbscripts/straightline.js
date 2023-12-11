/*
  Q Light Controller Plus
  straightline.js

  Copyright (c) Aidan Young
  Based on work by Massimo Callegari, Branson Matheson, Jano Svitok, Doug Puckett, and Heikki Junnila

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
        algo.hlineStretch = 0;

        algo.vlineCustom = 0;
        algo.vlinePixels = 0;
        algo.vlineUp = 0;
        algo.vlinePlace = 0;
        algo.vlineEffect = 0;
        algo.vlineStretch = 0;

        algo.lineOrientation = 0;

        algo.properties.push("name:lineOrientation|type:list|display:Orientation|values:Horizontal (H),Vertical (V)|write:setOrientation|read:getOrientation");

        algo.properties.push("name:hlinePlace|type:list|display:H Placement|values:Top,Middle,Bottom,Custom|write:setHPlace|read:getHPlace");
        algo.properties.push("name:hlineEffect|type:list|display:H Effect|values:None,Fill,Fill from Center,One By One,Noise,Even/Odd|write:setHEffect|read:getHEffect");
        algo.properties.push("name:hlineCustom|type:range|display:H Custom Value|values:0,10000|write:setHCustom|read:getHCustom");
        algo.properties.push("name:hlinePixels|type:range|display:H Remove Pixels|values:0,10000|write:setHPixels|read:getHPixels");
        algo.properties.push("name:hlineLeft|type:range|display:H Move Left|values:0,10000|write:setLeft|read:getLeft");

        algo.properties.push("name:vlinePlace|type:list|display:V Placement|values:Left,Middle,Right,Custom|write:setVPlace|read:getHPlace");
        algo.properties.push("name:vlineEffect|type:list|display:V Effect|values:None,Fill,Fill from Center,One By One,Noise,Even/Odd|write:setVEffect|read:getVEffect");
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
            } else if (_effect === "One By One") {
                algo.hlineEffect = 2;
            } else if (_effect === "Fill from Center") {
                algo.hlineEffect = 3;
            } else if (_effect === "Noise") {
                algo.hlineEffect = 4;
            } else if (_effect === "Even/Odd") {
                algo.hlineEffect = 5;
            }
        }
        algo.getHEffect = function () {
            if (algo.hlineEffect === 0) {
                return "None";
            } else if (algo.hlineEffect === 1) {
                return "Fill";
            } else if (algo.hlineEffect === 2) {
                return "One By One";
            } else if (algo.hlineEffect === 3) {
                return "Fill from Center";
            } else if (algo.hlineEffect === 4) {
                return "Noise";
            } else if (algo.hlineEffect === 5) {
                return "Even/Odd";
            }
        }

        algo.setVEffect = function (_effect) {
            if (_effect === "None") {
                algo.vlineEffect = 0;
            } else if (_effect === "Fill") {
                algo.vlineEffect = 1;
            } else if (_effect === "One By One") {
                algo.vlineEffect = 2;
            } else if (_effect === "Fill from Center") {
                algo.vlineEffect = 3;
            } else if (_effect === "Noise") {
                algo.vlineEffect = 4;
            } else if (_effect === "Even/Odd") {
                algo.vlineEffect = 5;
            }
        }
        algo.getVEffect = function () {
            if (algo.vlineEffect === 0) {
                return "None";
            } else if (algo.vlineEffect === 1) {
                return "Fill";
            } else if (algo.vlineEffect === 2) {
                return "One By One";
            } else if (algo.vlineEffect === 3) {
                return "Fill from Center";
            } else if (algo.vlineEffect === 4) {
                return "Noise";
            } else if (algo.vlineEffect === 5) {
                return "Even/Odd";
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

        var dCounter = 0;
        var isEven = 0;
        algo.rgbMap = function (width, height, rgb, step) {
            dCounter = 0;
            var map = new Array(height);
            if (algo.lineOrientation === 0) { //horizontal
                for (var y = 0; y < height; y++) {
                    map[y] = new Array(width);
                }
                for (var x = 0; x < width; x++) {
                    var center = (algo.rgbMapStepCount(width - algo.hlinePixels, height) - 1);
                    var effect;
                    var color;
                    isEven = (((width >= algo.hlinePixels) + 1) % 2 === 0);
                    if (algo.hlineEffect === 1) { // fill
                        effect = (x - algo.hlinePixels <= step);
                    } else if (algo.hlineEffect === 2) { //one by one
                        var xx = step % width;
                        effect = (x - algo.hlinePixels <= xx) && (x - algo.hlinePixels >= xx);
                    } else if (algo.hlineEffect === 3) { // fill from center
                        effect = x <= center + step + (isEven ? 1 : 0) && x - algo.hlinePixels >= center - step;
                    } else if (algo.hlineEffect === 4) { // noise
                        var r = (rgb >> 16) & 0x00FF;  // split color of user selected color
                        var g = (rgb >> 8) & 0x00FF;
                        var b = rgb & 0x00FF;

                        // create random color level from 1 to 255
                        var colorLevel = Math.floor(Math.random() * 255);

                        // Assign random color value to temp variables
                        var rr = colorLevel;
                        var gg = colorLevel;
                        var bb = colorLevel;

                        // Limit each color element to the maximum for chosen color or make 0 if below 0
                        if (rr > r) { rr = r; }
                        if (rr < 0) { rr = 0; }
                        if (gg > g) { gg = g; }
                        if (gg < 0) { gg = 0; }
                        if (bb > b) { bb = b; }
                        if (bb < 0) { bb = 0; }

                        var cColor = (rr << 16) + (gg << 8) + bb;   // put rgb parts back together

                        var vDiv = Math.random() * 5; //medium noise reduction

                        dCounter += 1;              // counter for noise trigger
                        if (dCounter >= vDiv) {     // compare counter to user noise amount selection value
                            dCounter = 0;           // clear the counter
                            color = cColor;
                        }

                    } else if (algo.hlineEffect === 5) { // even/odd
                        var i = step;
                        var xx = i % 2;
                        if ((i % 2) === 0) {
                            effect = (x - algo.hlinePixels <= xx) && (x - algo.hlinePixels >= xx);
                        } else {
                            effect = (x - algo.hlinePixels >= xx) && (x - algo.hlinePixels <= xx);
                        }
                        i++;
                    }
                    else { //none
                        effect = 1;
                    }

                    if (algo.hlineEffect !== 4) {
                        color = rgb;
                    }


                    if (algo.hlinePlace === 0) { // top
                        map[0][x - algo.hlineLeft] = (x >= algo.hlinePixels) && (effect) && color;
                    } else if (algo.hlinePlace === 1) { // middle
                        var middle = (Math.floor((parseInt(width) + 1) / 2) - 1);
                        map[middle][x - algo.hlineLeft] = (x >= algo.hlinePixels) && (effect) && color;
                        if (height % 2 === 0) {
                            map[middle + 1][x - algo.hlineLeft] = (y >= algo.hlinePixels) && (effect) && color;
                        }
                    } else if (algo.hlinePlace === 2) { // bottom
                        map[y - 1][x - algo.hlineLeft] = (x >= algo.hlinePixels) && (effect) && color;
                    } else if (algo.hlinePlace === 3) { // custom
                        map[algo.hlineCustom - 1][x - algo.hlineLeft] = (x >= algo.hlinePixels) && (effect) && color;
                    } else { map[y][x] = 0 }
                }
            }

            else if (algo.lineOrientation === 1) { //vertical
                dCounter = 0;
                for (var y = 0; y < height; y++) {
                    map[y] = new Array(width);
                    for (var x = 0; x < width; x++) {
                        var effect;
                        var yUp = y - algo.vlineUp;
                        var center = (algo.rgbMapStepCount(width, height - algo.vlinePixels) - 1);
                        isEven = (((height >= algo.vlinePixels) + 1) % 2 === 0);
                        if (algo.vlineEffect === 1) { //fill
                            effect = (y - algo.vlinePixels <= step);
                        } else if (algo.vlineEffect === 2) { //one by one
                            var yy = step % height;
                            effect = (y - algo.vlinePixels <= yy) && (y - algo.vlinePixels >= yy);
                        } else if (algo.vlineEffect === 3) { //fill from center
                            effect = y <= center + step + (isEven ? 1 : 0) && y >= center - step;
                        } else if (algo.vlineEffect === 4) { //noise
                            var r = (rgb >> 16) & 0x00FF;  // split color of user selected color
                            var g = (rgb >> 8) & 0x00FF;
                            var b = rgb & 0x00FF;

                            // create random color level from 1 to 255
                            var colorLevel = Math.floor(Math.random() * 255);

                            // Assign random color value to temp variables
                            var rr = colorLevel;
                            var gg = colorLevel;
                            var bb = colorLevel;

                            // Limit each color element to the maximum for chosen color or make 0 if below 0
                            if (rr > r) { rr = r; }
                            if (rr < 0) { rr = 0; }
                            if (gg > g) { gg = g; }
                            if (gg < 0) { gg = 0; }
                            if (bb > b) { bb = b; }
                            if (bb < 0) { bb = 0; }

                            var cColor = (rr << 16) + (gg << 8) + bb;   // put rgb parts back together

                            var vDiv = Math.random() * 5; //medium noise reduction

                            dCounter += 1;              // counter for noise trigger
                            if (dCounter >= vDiv) {     // compare counter to user noise amount selection value
                                dCounter = 0;           // clear the counter
                            }
                        } else if (algo.vlineEffect === 5) { // even/odd

                        }
                        else {
                            effect = 1;
                        }

                        if (algo.vlineEffect !== 4) {
                            color = rgb;
                        } else { color = cColor; }

                        /* if (yUp >= 0 && yUp < height) { */
                            if (algo.vlinePlace === 0) { // left
                                map[yUp][0] = (y >= algo.vlinePixels) && effect && color;
                            } else if (algo.vlinePlace === 1) { //middle
                                var middle = (Math.floor((parseInt(width) + 1) / 2) - 1);
                                map[yUp][middle] = (y >= algo.vlinePixels) && (effect) && rgb;
                                if (width % 2 === 0) {
                                    map[yUp][middle + 1] = (y >= algo.vlinePixels) && (effect) && rgb;
                                }
                            } else if (algo.vlinePlace === 2) { // Right
                                map[yUp][width - 1] = (y >= algo.vlinePixels) && (effect) && rgb;
                            } else if (algo.vlinePlace === 3) { // custom
                                map[yUp][algo.vlineCustom - 1] = (y >= algo.vlinePixels) && (effect) && rgb;
                            } else {
                                map[yUp][x] = 0;
                            }
                        /* } */
                    }
                }
            }

            return map;
        };

        algo.rgbMapStepCount = function (width, height) {
            if (algo.lineOrientation === 0 && algo.hlineEffect === 0 || algo.lineOrientation === 0 && algo.hlineEffect === 4) { //horizontal + noise
                return width;
            }
            else if (algo.lineOrientation === 0 && algo.hlineEffect === 1 || algo.lineOrientation === 0 && algo.hlineEffect === 2) { //fill + one by one
                return width - algo.hlinePixels;
            }
            else if (algo.lineOrientation === 0 && algo.hlineEffect === 3) { //fill from center
                return Math.floor((parseInt(width) + 1) / 2) - algo.hlinePixels;
            }
            else if (algo.lineOrientation === 0 && algo.hlineEffect === 5) { // even/odd
                return width;
            }

            else if (algo.lineOrientation === 1 && algo.vlineEffect === 0 || algo.lineOrientation === 1 && algo.vlineEffect === 4) { //vertical + noise
                return height;
            }
            else if (algo.lineOrientation === 1 && algo.vlineEffect === 2 || algo.lineOrientation === 1 && algo.vlineEffect === 1) {
                //fill + one by one
                return height - algo.vlinePixels;
            }
            else if (algo.lineOrientation === 1 && algo.vlineEffect === 3) { //fill from center
                return Math.floor((parseInt((height)) + 1) / 2);
            }
        };


        // Development tool access
        testAlgo = algo;
        return algo;
    }
)();