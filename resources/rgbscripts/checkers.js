/*
  Q Light Controller Plus
  checkers.js

  Copyright (c) Branson Matheson
  Based on the work of Massimo Callegari and Tim Cullingworth

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
var testAlgo

;(function () {
  var colorPalette = new Object()
  colorPalette.collection = new Array(
    ['White', 0xffffff], //  0
    ['Cream', 0xffff7f], //  1
    ['Pink', 0xff7f7f], //  2
    ['Rose', 0x7f3f3f], //  3
    ['Coral', 0x7f3f1f], //  4
    ['Dim Red', 0x7f0000], //  5
    ['Red', 0xff0000], //  6
    ['Orange', 0xff3f00], //  7
    ['Dim Orange', 0x7f1f00], //  8
    ['Goldenrod', 0x7f3f00], //  9
    ['Gold', 0xff7f00], // 10
    ['Yellow', 0xffff00], // 11
    ['Dim Yellow', 0x7f7f00], // 12
    ['Lime', 0x7fff00], // 13
    ['Pale Green', 0x3f7f00], // 14
    ['Dim Green', 0x007f00], // 15
    ['Green', 0x00ff00], // 16
    ['Seafoam', 0x00ff3f], // 17
    ['Turquoise', 0x007f3f], // 18
    ['Teal', 0x007f7f], // 19
    ['Cyan', 0x00ffff], // 20
    ['Electric Blue', 0x007fff], // 21
    ['Blue', 0x0000ff], // 22
    ['Dim Blue', 0x00007f], // 23
    ['Pale Blue', 0x1f1f7f], // 24
    ['Indigo', 0x1f00bf], // 25
    ['Purple', 0x3f00bf], // 26
    ['Violet', 0x7f007f], // 27
    ['Magenta', 0xff00ff], // 28
    ['Hot Pink', 0xff003f], // 29
    ['Deep Pink', 0x7f001f], // 30
    ['Grey', 0xaaaaaa], // 31
    ['OFF', 0x000000]
  ) // 32
  colorPalette.makeSubArray = function (_index) {
    var _array = new Array()
    for (var i = 0; i < colorPalette.collection.length; i++) {
      _array.push(colorPalette.collection[i][_index])
    }
    return _array
  }
  colorPalette.names = colorPalette.makeSubArray(0)

  var util = new Object()
  util.initialized = false
  util.altCheck = 0
  util.width = 0
  util.height = 0

  var algo = new Object()
  algo.apiVersion = 2
  algo.name = 'Checkers'
  algo.author = 'Branson Matheson'
  algo.acceptColors = 0
  algo.properties = new Array()
  algo.checkWidth = 8
  algo.properties.push(
    'name:checkWidth|type:range|display:Check Width|values:1,100|write:setCheckWidth|read:getCheckWidth'
  )
  algo.checkHeight = 8
  algo.properties.push(
    'name:checkHeight|type:range|display:Check Height|values:1,100|write:setCheckHeight|read:getCheckHeight'
  )
  algo.checkDepth = 0
  algo.properties.push(
    'name:checkDepth|type:range|display:Check Depth|values:0,64|write:setCheckDepth|read:getCheckDepth'
  )
  algo.checkSlide = 0
  algo.properties.push(
    'name:checkSlide|type:list|display:Slide|values:None,Up,Down,Left,Right,UpRight,UpLeft,DownRight,DownLeft|write:setSlide|read:getSlide'
  )
  algo.setSlide = function (_slide) {
    if (_slide === 'Up') {
      algo.checkSlide = 1
    } else if (_slide === 'Down') {
      algo.checkSlide = 2
    } else if (_slide === 'Left') {
      algo.checkSlide = 3
    } else if (_slide === 'Right') {
      algo.checkSlide = 4
    } else if (_slide === 'UpRight') {
      algo.checkSlide = 5
    } else if (_slide === 'UpLeft') {
      algo.checkSlide = 6
    } else if (_slide === 'DownRight') {
      algo.checkSlide = 7
    } else if (_slide === 'DownLeft') {
      algo.checkSlide = 8
    } else {
      algo.checkSlide = 0
    }
  }

  algo.getSlide = function () {
    if (algo.checkSlide === 1) {
      return 'Up'
    } else if (algo.checkSlide === 2) {
      return 'Down'
    } else if (algo.checkSlide === 3) {
      return 'Left'
    } else if (algo.checkSlide === 4) {
      return 'Right'
    } else if (algo.checkSlide === 5) {
      return 'UpRight'
    } else if (algo.checkSlide === 6) {
      return 'UpLeft'
    } else if (algo.checkSlide === 7) {
      return 'DownRight'
    } else if (algo.checkSlide === 8) {
      return 'DownLeft'
    } else {
      return 'None'
    }
  }

  algo.checkAlternate = 0
  algo.properties.push(
    'name:checkAlternate|type:list|display:Alternate Colors|values:No,Yes|write:setAlternate|read:getAlternate'
  )
  algo.setAlternate = function (_slide) {
    if (_slide === 'Yes') {
      algo.checkAlternate = 1
    } else {
      algo.checkAlternate = 0
    }
  }

  algo.getAlternate = function () {
    if (algo.checkAlternate === 1) {
      return 'Yes'
    } else {
      return 'No'
    }
  }

  algo.checkOnColorIndex = 0
  algo.properties.push(
    'name:checkOnColor|type:list|display:CheckOnColor|' +
      'values:' +
      colorPalette.names.toString() +
      '|' +
      'write:setCheckOnColor|read:getCheckOnColor'
  )
  algo.checkOffColorIndex = 31
  algo.properties.push(
    'name:checkOffColor|type:list|display:CheckOffColor|' +
      'values:' +
      colorPalette.names.toString() +
      '|' +
      'write:setCheckOffColor|read:getCheckOffColor'
  )

  algo.colorIndex = new Array(algo.checkOnColorIndex, algo.checkOffColorIndex)

  algo.setColor = function (_index, _preset) {
    var i = colorPalette.names.indexOf(_preset)
    if (i === -1) {
      i = colorPalette.collection.length - 1
    }
    algo.colorIndex[_index] = i
    return algo.colorIndex[_index]
  }

  algo.getColor = function (_index) {
    var i = algo.colorIndex[_index]
    if (i < 0) {
      i = 0
    }
    if (i >= colorPalette.collection.length) {
      i = colorPalette.collection.length - 1
    }
    return colorPalette.collection[i][0]
  }

  algo.setCheckOnColor = function (_preset) {
    algo.checkOnColorIndex = algo.setColor(0, _preset)
    util.initialized = false
  }
  algo.getCheckOnColor = function () {
    return algo.getColor(0)
  }

  algo.setCheckOffColor = function (_preset) {
    algo.checkOffColorIndex = algo.setColor(1, _preset)
    util.initialized = false
  }
  algo.getCheckOffColor = function () {
    return algo.getColor(1)
  }

  function mod (n, m) {
    return ((n % m) + m) % m
  }

  function limit (n, m) {
    if (n <= 0) {
      return 0
    }
    if (n > m) {
      return m
    }
    return n
  }

  algo.setCheckWidth = function (_amount) {
    algo.checkWidth = _amount
    util.initialized = false
  }

  algo.getCheckWidth = function () {
    return algo.checkWidth
  }

  algo.setCheckHeight = function (_amount) {
    algo.checkHeight = _amount
    util.initialized = false
  }

  algo.getCheckHeight = function () {
    return algo.checkHeight
  }
  algo.setCheckDepth = function (_amount) {
    algo.checkDepth = _amount
    util.initialized = false
  }

  algo.getCheckDepth = function () {
    return algo.checkDepth
  }

  util.checkers = new Array()
  function Check (y, x, step, space) {
    // upper left corner
    this.x = x
    this.y = y
    this.step = step
    this.space = space
  }

  util.initialize = function (width, height) {
    util.checkers = new Array()
    var i = 0
    var r = 0

    console.log('portal with size: ' + height + ',' + width)
    var ch = Number(algo.checkHeight)
    var cw = Number(algo.checkWidth)
    console.log('creating checkers with size: ' + ch + ', ' + cw)
    // compute check grid 1 check bigger than the view portal
    // and add checks evenly across the grid
    util.gridy = ch * (Math.ceil(height / ch) + 1)
    util.gridx = cw * (Math.ceil(width / cw) + 1)
    console.log(
      'creating checker grid with size: ' + util.gridy + ',' + util.gridx
    )
    for (var y = 0; y < util.gridy; y += ch) {
      for (var x = 0; x < util.gridx; x += cw) {
        var space = 0
        if (Math.floor(r / 2) === r / 2) {
          if (Math.floor(i / 2) === i / 2) {
            space = 1
          }
        } else {
          space = 1
          if (Math.floor(i / 2) === i / 2) {
            space = 0
          }
        }
        // console.log('creating check: ' + i + ' pos: ' +y + ',' + x + ' space:' + space);
        util.checkers.push(new Check(y, x, 0, space))
        i++
      }
      r++
    }
    // reset initialization
    util.initialized = true
    util.width = width
    util.height = height
  }

  util.getColor = function (check, h, w) {
    var color = 0
    if (util.altCheck === 1) {
      if (check.space === 1) {
        color = colorPalette.collection[algo.checkOnColorIndex][1]
      }
      if (check.space === 0) {
        color = colorPalette.collection[algo.checkOffColorIndex][1]
      }
    } else {
      if (check.space === 0) {
        color = colorPalette.collection[algo.checkOnColorIndex][1]
      }
      if (check.space === 1) {
        color = colorPalette.collection[algo.checkOffColorIndex][1]
      }
    }
    if (algo.checkDepth > 0) {
      // calculate distance from center
      var hd = Math.abs(h - algo.checkHeight / 2) //actual center
      var wd = Math.abs(w - algo.checkWidth / 2) // actual center
      var ca = Math.floor((Math.sqrt(hd * hd + wd * wd) * algo.checkDepth) / 2)
      //var ca = Math.abs((hd*algo.checkDepth)+(wd*algo.checkDepth));
      // update color values
      var r = limit(((color >> 16) & 0x00ff) - ca, 0xff) // split colour in to
      var g = limit(((color >> 8) & 0x00ff) - ca, 0xff) // separate part
      var b = limit((color & 0x00ff) - ca, 0xff)

      // brigten colors by amounts based on distance
      var new_color = (r << 16) + (g << 8) + b
      color = new_color
      // console.log('r:' + r + ' h:' + h + ' hd:' + hd +' ca: ' + ca + ' old: ' + color + ' new: ' + new_color);
    }
    return color
  }

  util.getNextStep = function (width, height) {
    // create an empty, black map y by x
    var map = new Array(width)
    for (var y = 0; y <= height; y++) {
      map[y] = new Array()
      for (var x = 0; x <= width; x++) {
        map[y][x] = 0
      }
    }

    // checkers onto map
    for (var i = 0; i < util.checkers.length; i++) {
      var check = util.checkers[i]
      // if sliding add one square on the grid per step
      if (algo.checkSlide > 0) {
        if (
          algo.checkSlide == 1 ||
          algo.checkSlide == 5 ||
          algo.checkSlide == 6
        ) {
          check.y = mod(check.y - 1, util.gridy)
        }
        if (
          algo.checkSlide == 2 ||
          algo.checkSlide == 7 ||
          algo.checkSlide == 8
        ) {
          check.y = mod(check.y + 1, util.gridy)
        }
        if (
          algo.checkSlide == 3 ||
          algo.checkSlide == 6 ||
          algo.checkSlide == 8
        ) {
          check.x = mod(check.x - 1, util.gridx)
        }
        if (
          algo.checkSlide == 4 ||
          algo.checkSlide == 5 ||
          algo.checkSlide == 7
        ) {
          check.x = mod(check.x + 1, util.gridx)
        }
      }
      // if the check upper left hand corner is in the view port map it
      if (check.y <= height && check.x <= width) {
        for (var w = 0; w < algo.checkWidth; w++) {
          for (var h = 0; h < algo.checkHeight; h++) {
            // compute where pixel will be
            var actualY = mod(check.y + h, util.gridy)
            var actualX = mod(check.x + w, util.gridx)
            // if this pixel is in the view port .. map it
            if (actualY <= height && actualX <= width) {
              // if it's already mapped.. that's a problem
              if (map[actualY][actualX] != 0) {
                console.log(
                  'skipping pixel in check: ' +
                    i +
                    ': (' +
                    check.y +
                    '.' +
                    check.x +
                    ') pos: (' +
                    actualY +
                    ',' +
                    actualX +
                    ') color ' +
                    map[actualY][actualX]
                )
              } else {
                // if it's in the grid .. get color and map it
                var color = util.getColor(check, h, w)
                map[actualY][actualX] = color
              }
            }
          }
        }
      }
    }
    // alternate on each pass
    if (algo.checkAlternate === 1 && util.altCheck === 1) {
      util.altCheck = 0
    } else {
      util.altCheck = 1
    }

    return map
  }

  algo.rgbMap = function (width, height, rgb, step) {
    if (
      util.initialized === false ||
      width != util.width ||
      height != util.height
    ) {
      console.log('width: ' + width + ' height: ' + height)
      console.log('util.width: ' + util.width + ' util.height: ' + height)
      util.initialize(width, height)
    }
    var rgbmap = util.getNextStep(width, height)
    return rgbmap
  }

  algo.rgbMapStepCount = function (width, height) {
    var size = Math.floor((width * height) / algo.checkWidth)
    return size
  }

  // Development tool access
  testAlgo = algo

  return algo
})()
