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
        algo.linePlace = 0;
        algo.lineCustom = 0;
        algo.linePixels = 0
        algo.hlineLeft = 0;
        algo.lineEffect = 0;
        algo.vlineUp = 0;

        algo.lineOrientation = 0;

        algo.properties.push("name:lineOrientation|type:list|display:Orientation|values:Horizontal (H),Vertical (V)|write:setOrientation|read:getOrientation");

        algo.properties.push("name:lineEffect|type:list|display:Effect|values:None,Fill,Fill from Center,One By One,Noise,Even/Odd|write:setHEffect|read:getHEffect");
        algo.properties.push("name:linePlace|type:list|display:Placement|values:Top/Left,Middle,Bottom/Right,Custom|write:setHPlace|read:getHPlace");
        algo.properties.push("name:lineCustom|type:range|display:Custom Value|values:0,10000|write:setHCustom|read:getHCustom");
        algo.properties.push("name:linePixels|type:range|display:Remove Pixels|values:0,10000|write:setHPixels|read:getHPixels");
        algo.properties.push("name:hlineLeft|type:range|display:H Move Left|values:0,10000|write:setLeft|read:getLeft");
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
            if (_hlinePlace === "Top/Left") {
                algo.linePlace = 0;
            }
            else if (_hlinePlace === "Middle") {
                algo.linePlace = 1;
            }
            else if (_hlinePlace === "Bottom/Right") {
                algo.linePlace = 2;
            }
            else if (_hlinePlace === "Custom") {
                algo.linePlace = 3;
            }
        }
        algo.getHPlace = function () {
            if (algo.linePlace === 0) {
                return "Top/Left";
            }
            else if (algo.linePlace === 2) {
                return "Bottom/Right";
            }
            else if (algo.linePlace === 1) {
                return "Middle";
            }
            else if (algo.linePlace === 3) {
                return "Custom";
            }
        }

        algo.setHEffect = function (_effect) {
            if (_effect === "None") {
                algo.lineEffect = 0;
            } else if (_effect === "Fill") {
                algo.lineEffect = 1;
            } else if (_effect === "One By One") {
                algo.lineEffect = 2;
            } else if (_effect === "Fill from Center") {
                algo.lineEffect = 3;
            } else if (_effect === "Noise") {
                algo.lineEffect = 4;
            } else if (_effect === "Even/Odd") {
                algo.lineEffect = 5;
            }
        }
        algo.getHEffect = function () {
            if (algo.lineEffect === 0) {
                return "None";
            } else if (algo.lineEffect === 1) {
                return "Fill";
            } else if (algo.lineEffect === 2) {
                return "One By One";
            } else if (algo.lineEffect === 3) {
                return "Fill from Center";
            } else if (algo.lineEffect === 4) {
                return "Noise";
            } else if (algo.lineEffect === 5) {
                return "Even/Odd";
            }
        }

        algo.setHCustom = function (_custom) {
            algo.lineCustom = _custom
        }; algo.getHCustom = function () {
            return algo.lineCustom
        };

        algo.setHPixels = function (_pixels) {
            algo.linePixels = _pixels
        }; algo.getHPixels = function () {
            return algo.linePixels
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
                    var center = (algo.rgbMapStepCount(width - algo.linePixels, height) - 1);
                    var effect;
                    var color;
                    isEven = (((width >= algo.linePixels) + 1) % 2 === 0);
                    if (algo.lineEffect === 1) { // fill
                        effect = (x - algo.linePixels <= step);
                    } else if (algo.lineEffect === 2) { //one by one
                        var xx = step % width;
                        effect = (x - algo.linePixels <= xx) && (x - algo.linePixels >= xx);
                    } else if (algo.lineEffect === 3) { // fill from center
                        effect = x <= center + step + (isEven ? 1 : 0) && x - algo.linePixels >= center - step;
                    } else if (algo.lineEffect === 4) { // noise
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

                    } else if (algo.lineEffect === 5) { // even/odd
                        var i = step;
                        effect = (x % 2 === 0 && i % 2 === 0) || (x % 2 !== 0 && i % 2 !== 0);
                        i++;
                    }
                    else { //none
                        effect = 1;
                    }

                    if (algo.lineEffect !== 4) {
                        color = rgb;
                    }


                    if (algo.linePlace === 0) { // top
                        map[0][x - algo.hlineLeft] = (x >= algo.linePixels) && (effect) && color;
                    } else if (algo.linePlace === 1) { // middle
                        var middle = (Math.floor((parseInt(width) + 1) / 2) - 1);
                        map[middle][x - algo.hlineLeft] = (x >= algo.linePixels) && (effect) && color;
                        if (height % 2 === 0) {
                            map[middle + 1][x - algo.hlineLeft] = (y >= algo.linePixels) && (effect) && color;
                        }
                    } else if (algo.linePlace === 2) { // bottom
                        map[y - 1][x - algo.hlineLeft] = (x >= algo.linePixels) && (effect) && color;
                    } else if (algo.linePlace === 3) { // custom
                        map[algo.lineCustom - 1][x - algo.hlineLeft] = (x >= algo.linePixels) && (effect) && color;
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
                        var center = (algo.rgbMapStepCount(width, height - algo.linePixels) - 1);
                        isEven = (((height >= algo.linePixels) + 1) % 2 === 0);
                        if (algo.lineEffect === 1) { //fill
                            effect = (y - algo.linePixels <= step);
                        } else if (algo.lineEffect === 2) { //one by one
                            var yy = step % height;
                            effect = (y - algo.linePixels <= yy) && (y - algo.linePixels >= yy);
                        } else if (algo.lineEffect === 3) { //fill from center
                            effect = y <= center + step + (isEven ? 1 : 0) && y >= center - step;
                        } else if (algo.lineEffect === 4) { //noise
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
                        } else if (algo.lineEffect === 5) { // even/odd
                            var i = step;
                            effect = (y % 2 === 0 && i % 2 === 0) || (y % 2 !== 0 && i % 2 !== 0);
                            i++;
                        }
                        else {
                            effect = 1;
                        }

                        if (algo.lineEffect !== 4) {
                            color = rgb;
                        } else { color = cColor; }

                        if (yUp >= 0 && yUp < height) {
                            if (algo.linePlace === 0) { // left
                                map[yUp][0] = (y >= algo.linePixels) && effect && color;
                            } else if (algo.linePlace === 1) { //middle
                                var middle = (Math.floor((parseInt(width) + 1) / 2) - 1);
                                map[yUp][middle] = (y >= algo.linePixels) && (effect) && rgb;
                                if (width % 2 === 0) {
                                    map[yUp][middle + 1] = (y >= algo.linePixels) && (effect) && rgb;
                                }
                            } else if (algo.linePlace === 2) { // Right
                                map[yUp][width - 1] = (y >= algo.linePixels) && (effect) && rgb;
                            } else if (algo.linePlace === 3) { // custom
                                map[yUp][algo.lineCustom - 1] = (y >= algo.linePixels) && (effect) && rgb;
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
            if (algo.lineOrientation === 0 && algo.lineEffect === 0 || algo.lineOrientation === 0 && algo.lineEffect === 4) { //horizontal + noise
                return width;
            }
            else if (algo.lineOrientation === 0 && algo.lineEffect === 1 || algo.lineOrientation === 0 && algo.lineEffect === 2) { //fill + one by one
                return width - algo.linePixels;
            }
            else if (algo.lineOrientation === 0 && algo.lineEffect === 3) { //fill from center
                return Math.floor((parseInt(width) + 1) / 2) - algo.linePixels;
            }
            else if (algo.lineOrientation === 0 && algo.lineEffect === 5 || algo.lineOrientation === 1 && algo.lineEffect === 5) { // even/odd (both vertical and horizontal)
                return 2;
            }

            else if (algo.lineOrientation === 1 && algo.lineEffect === 0 || algo.lineOrientation === 1 && algo.lineEffect === 4) { //vertical + noise
                return height;
            }
            else if (algo.lineOrientation === 1 && algo.lineEffect === 2 || algo.lineOrientation === 1 && algo.lineEffect === 1) {
                //fill + one by one
                return height - algo.linePixels;
            }
            else if (algo.lineOrientation === 1 && algo.lineEffect === 3) { //fill from center
                return Math.floor((parseInt((height)) + 1) / 2);
            }
        };


        // Development tool access
        testAlgo = algo;
        return algo;
    }
)();