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
    { name: "1x1 Single", width: 1, height: 1 },
    { name: "100x1 Wide", width: 100, height: 1 },
    { name: "1x100 Thin", width: 1, height: 100 },
    { name: "2x2 Tiny", width: 2, height: 2 }
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
  
  // Get test configurations for a script
  function getConfigsForScript(scriptName) {
    var baseName = scriptName.replace('.js', '').toLowerCase();

    if (scriptTestConfigs[baseName]) {
      return scriptTestConfigs[baseName];
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

    // Performance benchmarking
    performanceTest: function(scriptName, algo) {
      log(`\n=== Performance Testing ${scriptName} ===`, 'test');

      var benchmarks = [
        { name: "Small Matrix", width: 16, height: 16, steps: 100 },
        { name: "Medium Matrix", width: 32, height: 32, steps: 100 },
        { name: "Large Matrix", width: 64, height: 64, steps: 50 },
        { name: "XLarge Matrix", width: 128, height: 128, steps: 10 }
      ];

      var results = [];

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

          log(`${bench.name}: ${totalTime}ms total, ${avgTime.toFixed(2)}ms/frame, ${pixelsPerMs.toFixed(0)} pixels/ms`, 'perf');

          results.push({
            name: bench.name,
            totalTime: totalTime,
            avgTime: avgTime,
            pixelsPerMs: pixelsPerMs
          });

          endTest(true);

        } catch (error) {
          endTest(false, error.message);
        }
      }

      return results;
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
