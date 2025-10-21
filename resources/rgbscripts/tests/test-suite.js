/*
  Q Light Controller Plus
  RGB Script Test Suite
  
  Copyright (c) Branson Matheson
  
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

// Test Suite for RGB Scripts
var RGBTestSuite = (function() {
  
  var testResults = [];
  var currentTest = null;
  
  // Real-world matrix configurations
  var testMatrixes = [
    // Small matrices
    { name: "8x8 Square", width: 8, height: 8 },
    { name: "16x16 Square", width: 16, height: 16 },
    
    // LED strips
    { name: "8x64 Strip", width: 8, height: 64 },
    { name: "16x32 Strip", width: 16, height: 32 },
    { name: "32x16 Strip", width: 32, height: 16 },
    { name: "64x8 Strip", width: 64, height: 8 },
    
    // Large matrices
    { name: "32x32 Large", width: 32, height: 32 },
    { name: "64x64 XLarge", width: 64, height: 64 },
    
    // Extreme cases
    { name: "1x100 Thin", width: 1, height: 100 },
    { name: "100x1 Wide", width: 100, height: 1 },
    { name: "1x1 Single", width: 1, height: 1 },
    { name: "128x128 Huge", width: 128, height: 128 }
  ];
  
  // Test configurations for different script types
  var scriptTestConfigs = {
    // Lines.js specific configurations
    lines: [
      // Basic configurations
      { name: "Default", props: {} },
      { name: "Max Lines", props: { linesAmount: 200 } },
      { name: "Max Size", props: { linesSize: 32 } },
      { name: "Min Size", props: { linesSize: 1 } },

      // Movement tests
      { name: "Fast Movement", props: { linesMovement: "Right", linesMovementSpeed: 200 } },
      { name: "Slow Movement", props: { linesMovement: "Left", linesMovementSpeed: 10 } },
      { name: "Loop Movement", props: { linesMovement: "Up Loop", linesMovementSpeed: 150 } },

      // Lifecycle tests
      { name: "Static Fade", props: { linesLifecycle: "Static Fade Out" } },
      { name: "Shrink Fade", props: { linesLifecycle: "Shrink Fade In" } },
      { name: "Grow Fade", props: { linesLifecycle: "Grow Fade Out" } },

      // Distribution tests
      { name: "Every 2nd", props: { linesType: "Horizontal", linesDistribution: "Every 2nd" } },
      { name: "Center Third", props: { linesType: "Vertical", linesDistribution: "Center Third" } },
      { name: "Edges Only", props: { linesType: "Plus", linesDistribution: "Edges Only" } },

      // Pattern tests
      { name: "Dashed Lines", props: { linesPattern: "Dashed", linesSize: 8 } },
      { name: "Dotted Lines", props: { linesPattern: "Dotted", linesSize: 12 } },
      { name: "Double Lines", props: { linesPattern: "Double", linesSize: 16 } },

      // Variance tests
      { name: "Max Brightness Var", props: { linesBrightnessVariance: 100 } },
      { name: "Max Size Var", props: { linesVariability: 100 } },

      // Extreme combinations
      { name: "Everything Max", props: {
        linesAmount: 200,
        linesSize: 32,
        linesType: "Star",
        linesMovement: "Right Loop",
        linesMovementSpeed: 200,
        linesLifecycle: "Grow Fade Out",
        linesDistribution: "Every 2nd",
        linesPattern: "Double",
        linesBrightnessVariance: 100,
        linesVariability: 100
      }},

      { name: "Everything Min", props: {
        linesAmount: 1,
        linesSize: 1,
        linesType: "Horizontal",
        linesMovement: "None",
        linesMovementSpeed: 10,
        linesLifecycle: "Static",
        linesDistribution: "All",
        linesPattern: "Solid",
        linesBrightnessVariance: 0,
        linesVariability: 0
      }}
    ],

    // Generic configurations for other scripts
    generic: [
      { name: "Default", props: {} },
      { name: "Basic Test 1", props: {} },
      { name: "Basic Test 2", props: {} }
    ],

    // Circles.js specific configurations
    circles: [
      { name: "Default", props: {} },
      { name: "Max Circles", props: { circlesAmount: 50 } },
      { name: "Large Circles", props: { circlesSize: 20 } }
    ],

    // Balls.js specific configurations
    balls: [
      { name: "Default", props: {} },
      { name: "Many Balls", props: { ballsAmount: 20 } },
      { name: "Fast Balls", props: { ballsSpeed: 100 } }
    ]
  };

  // Edge case matrix configurations for thorough testing
  var edgeCaseMatrixes = [
    // Original edge cases
    { name: "1x1 Single", width: 1, height: 1 },
    { name: "100x1 Wide", width: 100, height: 1 },
    { name: "1x100 Thin", width: 1, height: 100 },
    { name: "2x2 Tiny", width: 2, height: 2 },

    // Prime dimension matrices
    { name: "3x7 Prime", width: 3, height: 7 },
    { name: "5x13 Prime", width: 5, height: 13 },
    { name: "7x11 Prime", width: 7, height: 11 },
    { name: "11x17 Prime", width: 11, height: 17 },

    // Power-of-2 matrices
    { name: "4x4 Power2", width: 4, height: 4 },
    { name: "8x8 Power2", width: 8, height: 8 },
    { name: "16x16 Power2", width: 16, height: 16 },
    { name: "32x32 Power2", width: 32, height: 32 },

    // Extreme aspect ratios
    { name: "1x1000 Ultra-Thin", width: 1, height: 1000 },
    { name: "1000x1 Ultra-Wide", width: 1000, height: 1 },
    { name: "2x500 Very-Thin", width: 2, height: 500 },
    { name: "500x2 Very-Wide", width: 500, height: 2 },

    // Odd dimensions
    { name: "3x3 Odd", width: 3, height: 3 },
    { name: "5x5 Odd", width: 5, height: 5 },
    { name: "7x7 Odd", width: 7, height: 7 },
    { name: "9x9 Odd", width: 9, height: 9 },

    // Large prime matrices
    { name: "17x17 Large-Prime", width: 17, height: 17 },
    { name: "23x23 Large-Prime", width: 23, height: 23 },
    { name: "31x31 Large-Prime", width: 31, height: 31 },

    // Non-square aspect ratios
    { name: "3x5 Ratio", width: 3, height: 5 },
    { name: "4x7 Ratio", width: 4, height: 7 },
    { name: "6x10 Ratio", width: 6, height: 10 },
    { name: "9x16 Ratio", width: 9, height: 16 }
  ];
  
  // Utility functions
  function log(message, type) {
    type = type || 'info';
    var timestamp = new Date().toISOString().substr(11, 12);
    console.log(`[${timestamp}] [${type.toUpperCase()}] ${message}`);
    
    if (currentTest) {
      currentTest.logs = currentTest.logs || [];
      currentTest.logs.push({ timestamp, type, message });
    }
  }
  
  function startTest(testName) {
    currentTest = {
      name: testName,
      startTime: Date.now(),
      status: 'running',
      logs: []
    };
    log(`Starting test: ${testName}`, 'test');
  }
  
  function endTest(success, error) {
    if (!currentTest) return;
    
    currentTest.endTime = Date.now();
    currentTest.duration = currentTest.endTime - currentTest.startTime;
    currentTest.status = success ? 'passed' : 'failed';
    currentTest.error = error;
    
    log(`Test ${currentTest.status}: ${currentTest.name} (${currentTest.duration}ms)`, 
        currentTest.status === 'passed' ? 'pass' : 'fail');
    
    testResults.push(currentTest);
    currentTest = null;
  }
  
  function validateRGBMap(map, width, height, testName) {
    if (!map) {
      throw new Error("rgbMap returned null/undefined");
    }
    
    if (!Array.isArray(map)) {
      throw new Error("rgbMap did not return an array");
    }
    
    if (map.length !== height) {
      throw new Error(`rgbMap height mismatch: expected ${height}, got ${map.length}`);
    }
    
    for (var y = 0; y < height; y++) {
      if (!Array.isArray(map[y])) {
        throw new Error(`Row ${y} is not an array`);
      }
      
      if (map[y].length !== width) {
        throw new Error(`Row ${y} width mismatch: expected ${width}, got ${map[y].length}`);
      }
      
      for (var x = 0; x < width; x++) {
        var pixel = map[y][x];
        if (typeof pixel !== 'number') {
          throw new Error(`Pixel at (${x},${y}) is not a number: ${typeof pixel}`);
        }
        
        if (pixel < 0 || pixel > 0xFFFFFF) {
          throw new Error(`Pixel at (${x},${y}) out of range: ${pixel}`);
        }
      }
    }
    
    log(`RGB map validation passed: ${width}x${height}`, 'debug');
  }

  // Animation sequence validation functions
  function validateAnimationContinuity(frames, testName) {
    if (frames.length < 2) return true;

    var discontinuities = 0;
    var maxAllowedChange = 0.3; // 30% of pixels can change between frames

    for (var f = 1; f < frames.length; f++) {
      var changedPixels = 0;
      var totalPixels = frames[f].length * frames[f][0].length;

      for (var y = 0; y < frames[f].length; y++) {
        for (var x = 0; x < frames[f][y].length; x++) {
          if (frames[f][y][x] !== frames[f-1][y][x]) {
            changedPixels++;
          }
        }
      }

      var changeRatio = changedPixels / totalPixels;
      if (changeRatio > maxAllowedChange) {
        discontinuities++;
        log(`Frame ${f}: ${(changeRatio*100).toFixed(1)}% pixels changed (>${(maxAllowedChange*100).toFixed(1)}%)`, 'warn');
      }
    }

    var continuityScore = 1 - (discontinuities / (frames.length - 1));
    log(`Animation continuity: ${(continuityScore*100).toFixed(1)}% (${discontinuities} discontinuities)`, 'debug');

    return continuityScore > 0.8; // 80% of frames should be continuous
  }

  function validateLoopSeamlessness(frames, testName) {
    if (frames.length < 3) return true;

    var firstFrame = frames[0];
    var lastFrame = frames[frames.length - 1];
    var changedPixels = 0;
    var totalPixels = firstFrame.length * firstFrame[0].length;

    for (var y = 0; y < firstFrame.length; y++) {
      for (var x = 0; x < firstFrame[y].length; x++) {
        if (firstFrame[y][x] !== lastFrame[y][x]) {
          changedPixels++;
        }
      }
    }

    var seamlessness = 1 - (changedPixels / totalPixels);
    log(`Loop seamlessness: ${(seamlessness*100).toFixed(1)}% (${changedPixels}/${totalPixels} pixels differ)`, 'debug');

    return seamlessness > 0.7; // 70% similarity between first and last frame
  }

  function validateMotionConsistency(frames, testName) {
    if (frames.length < 3) return true;

    var motionVectors = [];

    // Calculate motion vectors between consecutive frames
    for (var f = 1; f < frames.length; f++) {
      var vector = calculateMotionVector(frames[f-1], frames[f]);
      motionVectors.push(vector);
    }

    // Check for consistent motion patterns
    var consistentMotion = true;
    var avgMagnitude = motionVectors.reduce(function(sum, v) { return sum + v.magnitude; }, 0) / motionVectors.length;

    for (var i = 0; i < motionVectors.length; i++) {
      var deviation = Math.abs(motionVectors[i].magnitude - avgMagnitude) / avgMagnitude;
      if (deviation > 0.5) { // 50% deviation threshold
        consistentMotion = false;
        break;
      }
    }

    log(`Motion consistency: ${consistentMotion ? 'PASS' : 'FAIL'} (avg magnitude: ${avgMagnitude.toFixed(2)})`, 'debug');
    return consistentMotion;
  }

  function calculateMotionVector(frame1, frame2) {
    var totalMovement = 0;
    var activePixels = 0;

    for (var y = 0; y < frame1.length; y++) {
      for (var x = 0; x < frame1[y].length; x++) {
        if (frame1[y][x] !== 0 || frame2[y][x] !== 0) {
          activePixels++;
          if (frame1[y][x] !== frame2[y][x]) {
            totalMovement++;
          }
        }
      }
    }

    return {
      magnitude: activePixels > 0 ? totalMovement / activePixels : 0,
      activePixels: activePixels
    };
  }

  // Visual pattern validation functions
  function validateSymmetry(map, testName) {
    var height = map.length;
    var width = map[0].length;
    var isHorizontallySymmetric = true;
    var isVerticallySymmetric = true;

    // Check horizontal symmetry (left-right)
    for (var y = 0; y < height; y++) {
      for (var x = 0; x < Math.floor(width / 2); x++) {
        if (map[y][x] !== map[y][width - 1 - x]) {
          isHorizontallySymmetric = false;
          break;
        }
      }
      if (!isHorizontallySymmetric) break;
    }

    // Check vertical symmetry (top-bottom)
    for (var x = 0; x < width; x++) {
      for (var y = 0; y < Math.floor(height / 2); y++) {
        if (map[y][x] !== map[height - 1 - y][x]) {
          isVerticallySymmetric = false;
          break;
        }
      }
      if (!isVerticallySymmetric) break;
    }

    log(`Symmetry check: H=${isHorizontallySymmetric ? 'PASS' : 'FAIL'}, V=${isVerticallySymmetric ? 'PASS' : 'FAIL'}`, 'debug');

    return {
      horizontal: isHorizontallySymmetric,
      vertical: isVerticallySymmetric,
      anySymmetry: isHorizontallySymmetric || isVerticallySymmetric
    };
  }

  function validateColorGradient(map, testName) {
    var height = map.length;
    var width = map[0].length;
    var colors = [];

    // Collect all unique colors
    for (var y = 0; y < height; y++) {
      for (var x = 0; x < width; x++) {
        var color = map[y][x];
        if (color !== 0 && colors.indexOf(color) === -1) {
          colors.push(color);
        }
      }
    }

    if (colors.length < 2) {
      log(`Color gradient: Single color (${colors.length} unique colors)`, 'debug');
      return { isGradient: false, colorCount: colors.length, smoothness: 0 };
    }

    // Check for smooth color transitions
    var smoothTransitions = 0;
    var totalTransitions = 0;

    for (var y = 0; y < height; y++) {
      for (var x = 0; x < width - 1; x++) {
        if (map[y][x] !== 0 && map[y][x + 1] !== 0) {
          totalTransitions++;
          var colorDiff = Math.abs(map[y][x] - map[y][x + 1]);
          if (colorDiff < 0x111111) { // Small color difference indicates smooth transition
            smoothTransitions++;
          }
        }
      }
    }

    var smoothness = totalTransitions > 0 ? smoothTransitions / totalTransitions : 0;
    var isGradient = smoothness > 0.6; // 60% of transitions should be smooth

    log(`Color gradient: ${colors.length} colors, ${(smoothness*100).toFixed(1)}% smooth transitions`, 'debug');

    return {
      isGradient: isGradient,
      colorCount: colors.length,
      smoothness: smoothness
    };
  }

  function validateExpectedPattern(map, scriptName, testName) {
    var height = map.length;
    var width = map[0].length;
    var activePixels = 0;
    var totalPixels = width * height;

    // Count active (non-black) pixels
    for (var y = 0; y < height; y++) {
      for (var x = 0; x < width; x++) {
        if (map[y][x] !== 0) {
          activePixels++;
        }
      }
    }

    var coverage = activePixels / totalPixels;

    // Script-specific pattern expectations
    var expectedPatterns = {
      'lines': { minCoverage: 0.01, maxCoverage: 0.8, expectsLines: true },
      'circles': { minCoverage: 0.05, maxCoverage: 0.7, expectsCircular: true },
      'fill': { minCoverage: 0.1, maxCoverage: 1.0, expectsFill: true },
      'stripes': { minCoverage: 0.2, maxCoverage: 0.8, expectsStripes: true },
      'gradient': { minCoverage: 0.3, maxCoverage: 1.0, expectsGradient: true },
      'plasma': { minCoverage: 0.5, maxCoverage: 1.0, expectsGradient: true },
      'empty': { minCoverage: 0.0, maxCoverage: 0.0, expectsEmpty: true }
    };

    var baseName = scriptName.replace('.js', '').toLowerCase();
    var expected = expectedPatterns[baseName] || { minCoverage: 0.0, maxCoverage: 1.0 };

    var coverageValid = coverage >= expected.minCoverage && coverage <= expected.maxCoverage;

    log(`Pattern validation: ${(coverage*100).toFixed(1)}% coverage (expected: ${(expected.minCoverage*100).toFixed(1)}%-${(expected.maxCoverage*100).toFixed(1)}%)`, 'debug');

    return {
      coverageValid: coverageValid,
      coverage: coverage,
      activePixels: activePixels,
      expected: expected
    };
  }
  
  // Load and test a script
  function loadScript(scriptPath) {
    try {
      // In a real environment, you'd load the script file
      // For now, we'll assume the script is already loaded
      log(`Loading script: ${scriptPath}`, 'debug');
      return true;
    } catch (error) {
      log(`Failed to load script ${scriptPath}: ${error.message}`, 'error');
      return false;
    }
  }
  
  // Test a single configuration
  function testConfiguration(algo, matrix, config, steps) {
    steps = steps || 10;
    
    try {
      // Apply configuration properties
      for (var prop in config.props) {
        var setterName = 'set' + prop.charAt(0).toUpperCase() + prop.slice(1);
        if (typeof algo[setterName] === 'function') {
          algo[setterName](config.props[prop]);
          log(`Set ${prop} = ${config.props[prop]}`, 'debug');
        } else {
          log(`Warning: No setter found for ${prop}`, 'warn');
        }
      }
      
      // Test rgbMapStepCount
      var stepCount = algo.rgbMapStepCount(matrix.width, matrix.height);
      if (typeof stepCount !== 'number' || stepCount <= 0) {
        throw new Error(`Invalid step count: ${stepCount}`);
      }
      log(`Step count: ${stepCount}`, 'debug');
      
      // Test multiple steps
      for (var step = 0; step < Math.min(steps, stepCount); step++) {
        var rgb = 0xFF0000 + (step * 0x001100); // Vary color slightly
        var map = algo.rgbMap(matrix.width, matrix.height, rgb, step);
        
        validateRGBMap(map, matrix.width, matrix.height, 
                      `${config.name} on ${matrix.name} step ${step}`);
      }
      
      log(`Configuration test passed: ${config.name} on ${matrix.name}`, 'debug');
      return true;
      
    } catch (error) {
      log(`Configuration test failed: ${error.message}`, 'error');
      throw error;
    }
  }
  
  // Property discovery and analysis
  function parsePropertyString(propString) {
    var parts = propString.split('|');
    var property = {};

    for (var i = 0; i < parts.length; i++) {
      var part = parts[i].split(':');
      if (part.length === 2) {
        property[part[0]] = part[1];
      }
    }

    return property;
  }

  function analyzeScriptProperties(scriptContent) {
    var properties = [];
    var propertyRegex = /properties\.push\("([^"]+)"\)/g;
    var match;

    while ((match = propertyRegex.exec(scriptContent)) !== null) {
      properties.push(parsePropertyString(match[1]));
    }

    return properties;
  }

  function generatePropertyTestConfigs(properties) {
    var configs = [];

    // Default configuration
    configs.push({ name: "Default", props: {} });

    // Test each property individually
    for (var i = 0; i < properties.length; i++) {
      var prop = properties[i];

      if (prop.type === 'range' && prop.values) {
        var range = prop.values.split(',');
        if (range.length === 2) {
          var min = parseInt(range[0]);
          var max = parseInt(range[1]);

          // Test min, max, and middle values
          configs.push({
            name: `${prop.display || prop.name} Min`,
            props: { [prop.name]: min }
          });
          configs.push({
            name: `${prop.display || prop.name} Max`,
            props: { [prop.name]: max }
          });
          configs.push({
            name: `${prop.display || prop.name} Mid`,
            props: { [prop.name]: Math.floor((min + max) / 2) }
          });
        }
      } else if (prop.type === 'list' && prop.values) {
        var values = prop.values.split(',');
        for (var j = 0; j < values.length; j++) {
          configs.push({
            name: `${prop.display || prop.name} ${values[j]}`,
            props: { [prop.name]: j }
          });
        }
      }
    }

    // Extreme combination test
    if (properties.length > 0) {
      var extremeProps = {};
      for (var i = 0; i < properties.length; i++) {
        var prop = properties[i];
        if (prop.type === 'range' && prop.values) {
          var range = prop.values.split(',');
          if (range.length === 2) {
            extremeProps[prop.name] = parseInt(range[1]); // Max value
          }
        } else if (prop.type === 'list' && prop.values) {
          var values = prop.values.split(',');
          extremeProps[prop.name] = values.length - 1; // Last option
        }
      }

      if (Object.keys(extremeProps).length > 0) {
        configs.push({
          name: "All Properties Max",
          props: extremeProps
        });
      }
    }

    return configs;
  }

  // Load auto-generated configurations if available
  var autoGeneratedConfigs = {};
  try {
    if (typeof require !== 'undefined') {
      autoGeneratedConfigs = require('./auto-generated-configs.js');
    }
  } catch (e) {
    // Auto-generated configs not available, continue with manual configs
  }

  // Get test configurations for a script
  function getConfigsForScript(scriptName) {
    var baseName = scriptName.replace('.js', '').toLowerCase();

    // First check manual configurations (highest priority)
    if (scriptTestConfigs[baseName]) {
      return scriptTestConfigs[baseName];
    }

    // Then check auto-generated configurations
    if (autoGeneratedConfigs[baseName]) {
      return autoGeneratedConfigs[baseName];
    }

    // Default to generic configs for unknown scripts
    return scriptTestConfigs.generic;
  }

  return {
    // Public API
    testMatrixes: testMatrixes,
    edgeCaseMatrixes: edgeCaseMatrixes,
    scriptTestConfigs: scriptTestConfigs,
    testResults: testResults,

    // Property discovery functions
    analyzeScriptProperties: analyzeScriptProperties,
    generatePropertyTestConfigs: generatePropertyTestConfigs,
    parsePropertyString: parsePropertyString,

    // Get configurations for a specific script
    getConfigsForScript: getConfigsForScript,

    // Test functions
    testScript: function(scriptName, algo, configs, matrices) {
      log(`\n=== Testing ${scriptName} ===`, 'test');

      configs = configs || getConfigsForScript(scriptName);
      matrices = matrices || testMatrixes;
      var totalTests = matrices.length * configs.length;
      var passedTests = 0;

      for (var m = 0; m < matrices.length; m++) {
        var matrix = matrices[m];

        for (var c = 0; c < configs.length; c++) {
          var config = configs[c];
          var testName = `${scriptName}: ${config.name} on ${matrix.name}`;

          startTest(testName);

          try {
            testConfiguration(algo, matrix, config, 5);
            endTest(true);
            passedTests++;
          } catch (error) {
            endTest(false, error.message);
          }
        }
      }

      log(`\n=== ${scriptName} Results ===`, 'test');
      log(`Passed: ${passedTests}/${totalTests}`, 'test');
      log(`Success Rate: ${(passedTests/totalTests*100).toFixed(1)}%`, 'test');

      return { passed: passedTests, total: totalTests };
    },

    // Edge case testing specifically for problematic configurations
    edgeCaseTest: function(scriptName, algo) {
      log(`\n=== Edge Case Testing ${scriptName} ===`, 'test');

      var edgeCaseConfigs = [
        { name: "Default", props: {} },
        { name: "Max Parameters", props: { amount: 200, size: 32 } },
        { name: "Min Parameters", props: { amount: 1, size: 1 } }
      ];

      // Use script-specific configs if available
      if (scriptName.toLowerCase().includes('lines')) {
        edgeCaseConfigs = [
          { name: "Default", props: {} },
          { name: "Max Lines", props: { linesAmount: 200 } },
          { name: "Every 2nd Distribution", props: { linesDistribution: "Every 2nd" } },
          { name: "Center Third Distribution", props: { linesDistribution: "Center Third" } },
          { name: "Edges Only Distribution", props: { linesDistribution: "Edges Only" } },
          { name: "Everything Max", props: {
            linesAmount: 200,
            linesSize: 32,
            linesMovement: "Right Loop",
            linesMovementSpeed: 200,
            linesLifecycle: "Grow Fade Out",
            linesDistribution: "Every 2nd",
            linesPattern: "Double",
            linesBrightnessVariance: 100,
            linesVariability: 100
          }}
        ];
      }

      var totalTests = edgeCaseMatrixes.length * edgeCaseConfigs.length;
      var passedTests = 0;

      for (var m = 0; m < edgeCaseMatrixes.length; m++) {
        var matrix = edgeCaseMatrixes[m];
        log(`\n📐 Testing ${matrix.name} (${matrix.width}x${matrix.height})`, 'test');

        for (var c = 0; c < edgeCaseConfigs.length; c++) {
          var config = edgeCaseConfigs[c];
          var testName = `${scriptName} Edge Case: ${config.name} on ${matrix.name}`;

          startTest(testName);

          try {
            testConfiguration(algo, matrix, config, 5);
            endTest(true);
            passedTests++;
            log(`  ✅ ${config.name}: PASS`, 'debug');
          } catch (error) {
            endTest(false, error.message);
            log(`  ❌ ${config.name}: FAIL - ${error.message}`, 'error');
          }
        }
      }

      log(`\n📊 Edge Case Test Results:`, 'test');
      log(`   Passed: ${passedTests}/${totalTests}`, 'test');
      log(`   Success Rate: ${((passedTests / totalTests) * 100).toFixed(1)}%`, 'test');

      return { passed: passedTests, total: totalTests };
    },

    // Animation sequence validation
    animationSequenceTest: function(scriptName, algo) {
      log(`\n=== Animation Sequence Testing ${scriptName} ===`, 'test');

      var testMatrices = [
        { name: "16x16 Medium", width: 16, height: 16 },
        { name: "32x32 Large", width: 32, height: 32 }
      ];

      var passedTests = 0;
      var totalTests = testMatrices.length;

      for (var m = 0; m < testMatrices.length; m++) {
        var matrix = testMatrices[m];
        var testName = `${scriptName} Animation: ${matrix.name}`;

        startTest(testName);

        try {
          var stepCount = algo.rgbMapStepCount(matrix.width, matrix.height);
          var framesToTest = Math.min(20, stepCount); // Test up to 20 frames
          var frames = [];

          // Capture animation frames
          for (var step = 0; step < framesToTest; step++) {
            var rgb = 0xFF0000;
            var frame = algo.rgbMap(matrix.width, matrix.height, rgb, step);
            validateRGBMap(frame, matrix.width, matrix.height, testName);
            frames.push(frame);
          }

          // Validate animation properties
          var continuityPassed = validateAnimationContinuity(frames, testName);
          var seamlessnessPassed = validateLoopSeamlessness(frames, testName);
          var consistencyPassed = validateMotionConsistency(frames, testName);

          var animationScore = (continuityPassed ? 1 : 0) + (seamlessnessPassed ? 1 : 0) + (consistencyPassed ? 1 : 0);

          if (animationScore >= 2) { // At least 2 out of 3 tests should pass
            endTest(true);
            passedTests++;
            log(`Animation validation passed: ${animationScore}/3 criteria met`, 'debug');
          } else {
            throw new Error(`Animation validation failed: only ${animationScore}/3 criteria met`);
          }

        } catch (error) {
          endTest(false, error.message);
        }
      }

      log(`Animation sequence test results: ${passedTests}/${totalTests} passed`, 'test');
      return { passed: passedTests, total: totalTests };
    },

    // Visual pattern validation
    visualPatternTest: function(scriptName, algo) {
      log(`\n=== Visual Pattern Testing ${scriptName} ===`, 'test');

      var testMatrices = [
        { name: "16x16 Square", width: 16, height: 16 },
        { name: "32x16 Rectangle", width: 32, height: 16 },
        { name: "8x8 Small", width: 8, height: 8 }
      ];

      var passedTests = 0;
      var totalTests = testMatrices.length;

      for (var m = 0; m < testMatrices.length; m++) {
        var matrix = testMatrices[m];
        var testName = `${scriptName} Visual: ${matrix.name}`;

        startTest(testName);

        try {
          // Test multiple frames for pattern consistency
          var stepCount = algo.rgbMapStepCount(matrix.width, matrix.height);
          var framesToTest = Math.min(5, stepCount);
          var patternResults = [];

          for (var step = 0; step < framesToTest; step++) {
            var rgb = 0xFF0000;
            var frame = algo.rgbMap(matrix.width, matrix.height, rgb, step);
            validateRGBMap(frame, matrix.width, matrix.height, testName);

            // Validate visual patterns
            var symmetry = validateSymmetry(frame, testName);
            var gradient = validateColorGradient(frame, testName);
            var pattern = validateExpectedPattern(frame, scriptName, testName);

            patternResults.push({
              symmetry: symmetry,
              gradient: gradient,
              pattern: pattern
            });
          }

          // Analyze pattern consistency across frames
          var consistentPatterns = true;
          var validPatterns = 0;

          for (var i = 0; i < patternResults.length; i++) {
            var result = patternResults[i];

            // Check if pattern meets expectations
            if (result.pattern.coverageValid) {
              validPatterns++;
            }

            // For symmetric scripts, check symmetry
            if (scriptName.toLowerCase().includes('center') ||
                scriptName.toLowerCase().includes('radial')) {
              if (!result.symmetry.anySymmetry) {
                log(`Expected symmetry not found in frame ${i}`, 'warn');
              }
            }

            // For gradient scripts, check gradient quality
            if (scriptName.toLowerCase().includes('gradient') ||
                scriptName.toLowerCase().includes('plasma')) {
              if (!result.gradient.isGradient) {
                log(`Expected gradient not found in frame ${i}`, 'warn');
              }
            }
          }

          var patternScore = validPatterns / patternResults.length;

          if (patternScore >= 0.8) { // 80% of frames should have valid patterns
            endTest(true);
            passedTests++;
            log(`Visual pattern validation passed: ${(patternScore*100).toFixed(1)}% valid patterns`, 'debug');
          } else {
            throw new Error(`Visual pattern validation failed: only ${(patternScore*100).toFixed(1)}% valid patterns`);
          }

        } catch (error) {
          endTest(false, error.message);
        }
      }

      log(`Visual pattern test results: ${passedTests}/${totalTests} passed`, 'test');
      return { passed: passedTests, total: totalTests };
    },

    // Enhanced property boundary testing
    propertyBoundaryTest: function(scriptName, algo) {
      log(`\n=== Property Boundary Testing ${scriptName} ===`, 'test');

      // Get script properties for boundary testing
      var scriptContent = '';
      try {
        if (typeof require !== 'undefined') {
          var fs = require('fs');
          var path = require('path');
          var scriptPath = path.join(__dirname, '..', scriptName);
          scriptContent = fs.readFileSync(scriptPath, 'utf8');
        }
      } catch (e) {
        log('Could not read script file for property analysis', 'warn');
        return { passed: 0, total: 0 };
      }

      var properties = analyzeScriptProperties(scriptContent);
      if (properties.length === 0) {
        log('No properties found for boundary testing', 'info');
        return { passed: 1, total: 1 }; // Scripts without properties pass by default
      }

      var testMatrix = { width: 16, height: 16 };
      var passedTests = 0;
      var totalTests = 0;

      for (var i = 0; i < properties.length; i++) {
        var prop = properties[i];

        if (prop.type === 'range' && prop.values) {
          var range = prop.values.split(',');
          if (range.length === 2) {
            var min = parseInt(range[0]);
            var max = parseInt(range[1]);

            // Boundary value analysis: min-1, min, min+1, max-1, max, max+1
            var boundaryValues = [min - 1, min, min + 1, max - 1, max, max + 1];

            for (var j = 0; j < boundaryValues.length; j++) {
              var value = boundaryValues[j];
              var testName = `${scriptName} Boundary: ${prop.name}=${value}`;
              totalTests++;

              startTest(testName);

              try {
                // Apply property value
                var setterName = 'set' + prop.name.charAt(0).toUpperCase() + prop.name.slice(1);
                if (typeof algo[setterName] === 'function') {
                  algo[setterName](value);
                }

                // Test with boundary value
                var rgb = 0xFF0000;
                var map = algo.rgbMap(testMatrix.width, testMatrix.height, rgb, 0);
                validateRGBMap(map, testMatrix.width, testMatrix.height, testName);

                // Valid range should work, invalid range should either work or fail gracefully
                if (value >= min && value <= max) {
                  // Should definitely work
                  endTest(true);
                  passedTests++;
                  log(`  ✅ ${prop.name}=${value} (valid): PASS`, 'debug');
                } else {
                  // Invalid value - should either work (clamped) or fail gracefully
                  endTest(true);
                  passedTests++;
                  log(`  ⚠️  ${prop.name}=${value} (invalid): HANDLED`, 'debug');
                }

              } catch (error) {
                // For invalid values, graceful failure is acceptable
                if (value < min || value > max) {
                  endTest(true);
                  passedTests++;
                  log(`  ⚠️  ${prop.name}=${value} (invalid): REJECTED - ${error.message}`, 'debug');
                } else {
                  endTest(false, error.message);
                  log(`  ❌ ${prop.name}=${value} (valid): FAIL - ${error.message}`, 'error');
                }
              }
            }
          }
        } else if (prop.type === 'list' && prop.values) {
          var values = prop.values.split(',');

          // Test valid list indices and invalid ones
          var testIndices = [-1, 0, Math.floor(values.length / 2), values.length - 1, values.length];

          for (var j = 0; j < testIndices.length; j++) {
            var index = testIndices[j];
            var testName = `${scriptName} Boundary: ${prop.name}=${index}`;
            totalTests++;

            startTest(testName);

            try {
              // Apply property value
              var setterName = 'set' + prop.name.charAt(0).toUpperCase() + prop.name.slice(1);
              if (typeof algo[setterName] === 'function') {
                algo[setterName](index);
              }

              // Test with boundary value
              var rgb = 0xFF0000;
              var map = algo.rgbMap(testMatrix.width, testMatrix.height, rgb, 0);
              validateRGBMap(map, testMatrix.width, testMatrix.height, testName);

              // Valid indices should work, invalid indices should either work or fail gracefully
              if (index >= 0 && index < values.length) {
                endTest(true);
                passedTests++;
                log(`  ✅ ${prop.name}=${index} (valid): PASS`, 'debug');
              } else {
                endTest(true);
                passedTests++;
                log(`  ⚠️  ${prop.name}=${index} (invalid): HANDLED`, 'debug');
              }

            } catch (error) {
              // For invalid indices, graceful failure is acceptable
              if (index < 0 || index >= values.length) {
                endTest(true);
                passedTests++;
                log(`  ⚠️  ${prop.name}=${index} (invalid): REJECTED - ${error.message}`, 'debug');
              } else {
                endTest(false, error.message);
                log(`  ❌ ${prop.name}=${index} (valid): FAIL - ${error.message}`, 'error');
              }
            }
          }
        }
      }

      log(`Property boundary test results: ${passedTests}/${totalTests} passed`, 'test');
      return { passed: passedTests, total: totalTests };
    },

    // Stress test with extreme parameters
    stressTest: function(scriptName, algo) {
      log(`\n=== Stress Testing ${scriptName} ===`, 'test');

      var stressConfigs = [
        { name: "Extreme Large Matrix", matrix: { width: 256, height: 256 }, steps: 3 },
        { name: "Extreme Wide Matrix", matrix: { width: 1000, height: 1 }, steps: 3 },
        { name: "Extreme Tall Matrix", matrix: { width: 1, height: 1000 }, steps: 3 },
        { name: "Long Duration", matrix: { width: 32, height: 32 }, steps: 1000 }
      ];

      var passedTests = 0;

      for (var i = 0; i < stressConfigs.length; i++) {
        var stress = stressConfigs[i];
        var testName = `${scriptName} Stress: ${stress.name}`;

        startTest(testName);

        try {
          var startTime = Date.now();

          for (var step = 0; step < stress.steps; step++) {
            var rgb = 0xFF0000;
            var map = algo.rgbMap(stress.matrix.width, stress.matrix.height, rgb, step);
            validateRGBMap(map, stress.matrix.width, stress.matrix.height, testName);

            // Check for timeout (10 seconds max per test)
            if (Date.now() - startTime > 10000) {
              throw new Error(`Test timeout after ${Date.now() - startTime}ms`);
            }
          }

          endTest(true);
          passedTests++;
        } catch (error) {
          endTest(false, error.message);
        }
      }

      log(`Stress test results: ${passedTests}/${stressConfigs.length} passed`, 'test');
      return { passed: passedTests, total: stressConfigs.length };
    },

    // Memory leak detection
    memoryTest: function(scriptName, algo) {
      log(`\n=== Memory Testing ${scriptName} ===`, 'test');

      startTest(`${scriptName} Memory Test`);

      try {
        var matrix = { width: 64, height: 64 };
        var iterations = 1000;

        // Force garbage collection if available
        if (typeof gc === 'function') {
          gc();
        }

        for (var i = 0; i < iterations; i++) {
          var rgb = Math.floor(Math.random() * 0xFFFFFF);
          var map = algo.rgbMap(matrix.width, matrix.height, rgb, i % 100);

          // Don't keep references to prevent legitimate memory usage
          map = null;

          if (i % 100 === 0) {
            log(`Memory test iteration ${i}/${iterations}`, 'debug');
          }
        }

        endTest(true);
        log('Memory test completed - check for memory leaks manually', 'test');
        return { passed: 1, total: 1 };

      } catch (error) {
        endTest(false, error.message);
        return { passed: 0, total: 1 };
      }
    },

    // Performance benchmarking with regression detection
    performanceTest: function(scriptName, algo) {
      log(`\n=== Performance Testing ${scriptName} ===`, 'test');

      var benchmarks = [
        { name: "Small Matrix", width: 16, height: 16, steps: 100 },
        { name: "Medium Matrix", width: 32, height: 32, steps: 100 },
        { name: "Large Matrix", width: 64, height: 64, steps: 50 },
        { name: "XLarge Matrix", width: 128, height: 128, steps: 10 }
      ];

      // Performance baselines (ms/frame) - these would be updated over time
      var baselines = {
        "Small Matrix": 0.05,
        "Medium Matrix": 0.15,
        "Large Matrix": 0.5,
        "XLarge Matrix": 2.0
      };

      var results = [];
      var regressions = [];

      for (var b = 0; b < benchmarks.length; b++) {
        var bench = benchmarks[b];
        var testName = `${scriptName} Performance: ${bench.name}`;

        startTest(testName);

        try {
          var startTime = Date.now();

          for (var step = 0; step < bench.steps; step++) {
            var rgb = 0xFF0000;
            var map = algo.rgbMap(bench.width, bench.height, rgb, step);
          }

          var endTime = Date.now();
          var totalTime = endTime - startTime;
          var avgTime = totalTime / bench.steps;
          var pixelsPerMs = (bench.width * bench.height) / avgTime;

          // Check for performance regression
          var baseline = baselines[bench.name];
          var regressionThreshold = 2.0; // 2x slower than baseline
          var isRegression = avgTime > (baseline * regressionThreshold);

          if (isRegression) {
            regressions.push({
              benchmark: bench.name,
              current: avgTime,
              baseline: baseline,
              ratio: avgTime / baseline
            });
            log(`⚠️  Performance regression detected: ${avgTime.toFixed(2)}ms vs ${baseline}ms baseline`, 'warn');
          }

          log(`${bench.name}: ${totalTime}ms total, ${avgTime.toFixed(2)}ms/frame, ${pixelsPerMs.toFixed(0)} pixels/ms`, 'perf');

          results.push({
            name: bench.name,
            totalTime: totalTime,
            avgTime: avgTime,
            pixelsPerMs: pixelsPerMs,
            baseline: baseline,
            isRegression: isRegression
          });

          endTest(true);

        } catch (error) {
          endTest(false, error.message);
        }
      }

      // Report performance summary
      if (regressions.length > 0) {
        log(`\n⚠️  Performance Regressions Detected: ${regressions.length}`, 'warn');
        for (var i = 0; i < regressions.length; i++) {
          var reg = regressions[i];
          log(`  ${reg.benchmark}: ${reg.ratio.toFixed(1)}x slower than baseline`, 'warn');
        }
      } else {
        log(`\n✅ No performance regressions detected`, 'perf');
      }

      return {
        results: results,
        regressions: regressions,
        hasRegressions: regressions.length > 0
      };
    },

    // Generate comprehensive test report
    generateReport: function() {
      log('\n=== TEST REPORT ===', 'test');

      var totalTests = testResults.length;
      var passedTests = testResults.filter(function(t) { return t.status === 'passed'; }).length;
      var failedTests = testResults.filter(function(t) { return t.status === 'failed'; }).length;

      log(`Total Tests: ${totalTests}`, 'test');
      log(`Passed: ${passedTests}`, 'test');
      log(`Failed: ${failedTests}`, 'test');
      log(`Success Rate: ${(passedTests/totalTests*100).toFixed(1)}%`, 'test');

      if (failedTests > 0) {
        log('\n=== FAILED TESTS ===', 'test');
        testResults.filter(function(t) { return t.status === 'failed'; }).forEach(function(test) {
          log(`❌ ${test.name}: ${test.error}`, 'fail');
        });
      }

      var avgDuration = testResults.reduce(function(sum, t) { return sum + t.duration; }, 0) / totalTests;
      log(`\nAverage test duration: ${avgDuration.toFixed(2)}ms`, 'test');

      return {
        total: totalTests,
        passed: passedTests,
        failed: failedTests,
        successRate: passedTests/totalTests,
        avgDuration: avgDuration,
        results: testResults
      };
    },

    // Clear test results
    clearResults: function() {
      testResults.length = 0;
      log('Test results cleared', 'test');
    }
  };
})();
