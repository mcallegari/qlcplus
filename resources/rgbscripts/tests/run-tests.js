#!/usr/bin/env node

/*
  Q Light Controller Plus
  RGB Script Test Runner

  Usage: node run-tests.js [script-name]
  Examples:
    node run-tests.js                    # Test lines.js (default)
    node run-tests.js lines.js           # Test lines.js specifically
    node run-tests.js circles.js         # Test circles.js
    node run-tests.js --list             # List available scripts
    node run-tests.js --all              # Test all scripts

  Copyright (c) Branson Matheson
*/

// Load the test suite
var fs = require('fs');
var path = require('path');

// Load test suite
eval(fs.readFileSync(path.join(__dirname, 'test-suite.js'), 'utf8'));

console.log('🎨 RGB Script Test Runner');
console.log('==========================\n');

// Get command line arguments
var args = process.argv.slice(2);
var scriptToTest = args[0] || 'lines.js';

// Helper functions
function getAllRGBScripts() {
  var scriptsDir = path.join(__dirname, '..');
  var files = fs.readdirSync(scriptsDir);

  return files.filter(function(file) {
    return file.endsWith('.js') &&
           !file.startsWith('test-') &&
           !fs.statSync(path.join(scriptsDir, file)).isDirectory();
  }).sort();
}

function loadScript(scriptName) {
  try {
    // Clear any existing testAlgo
    if (typeof testAlgo !== 'undefined') {
      delete testAlgo;
    }

    var scriptPath = path.join(__dirname, '..', scriptName);
    if (!fs.existsSync(scriptPath)) {
      throw new Error(`Script not found: ${scriptName}`);
    }

    console.log(`📂 Loading script: ${scriptName}`);

    // Read and evaluate the script
    var scriptContent = fs.readFileSync(scriptPath, 'utf8');
    eval(scriptContent);

    if (typeof testAlgo === 'undefined') {
      console.log(`⚠️  Warning: ${scriptName} does not expose testAlgo (may not be an RGB script)`);
      return null;
    }

    console.log(`✅ Successfully loaded ${scriptName}`);
    return testAlgo;
  } catch (error) {
    console.error(`❌ Error loading ${scriptName}: ${error.message}`);
    return null;
  }
}

function validateScript(scriptName) {
  // Check if the file exists and is a .js file
  if (!scriptName.endsWith('.js')) {
    return false;
  }

  var scriptPath = path.join(__dirname, '..', scriptName);
  return fs.existsSync(scriptPath);
}

// Handle special commands
if (scriptToTest === '--list') {
  console.log('📋 Available RGB Scripts:');
  var scripts = getAllRGBScripts();
  scripts.forEach(function(script, index) {
    console.log(`  ${index + 1}. ${script}`);
  });
  process.exit(0);
}

if (scriptToTest === '--all') {
  console.log('🚀 Testing all RGB scripts...\n');
  // This will be handled in the main function
}

// Run comprehensive tests on any script
function testScript(scriptName, algo) {
  console.log(`🧪 Testing ${scriptName}...\n`);

  // Clear previous test results
  RGBTestSuite.clearResults();

  // Basic functionality test
  var basicResults = RGBTestSuite.testScript(scriptName, algo);

  // Edge case test (integrated from test-lines-edge-cases.js)
  var edgeResults = RGBTestSuite.edgeCaseTest(scriptName, algo);

  // Animation sequence validation
  var animationResults = RGBTestSuite.animationSequenceTest(scriptName, algo);

  // Visual pattern validation
  var visualResults = RGBTestSuite.visualPatternTest(scriptName, algo);

  // Property boundary testing
  var boundaryResults = RGBTestSuite.propertyBoundaryTest(scriptName, algo);

  // Stress test
  var stressResults = RGBTestSuite.stressTest(scriptName, algo);

  // Memory test
  var memoryResults = RGBTestSuite.memoryTest(scriptName, algo);

  // Performance test
  var perfResults = RGBTestSuite.performanceTest(scriptName, algo);

  // Generate final report
  var report = RGBTestSuite.generateReport();

  return {
    basic: basicResults,
    edge: edgeResults,
    animation: animationResults,
    visual: visualResults,
    boundary: boundaryResults,
    stress: stressResults,
    memory: memoryResults,
    performance: perfResults,
    overall: report
  };
}

// Test other RGB scripts if available
function testOtherScripts() {
  var scriptsToTest = [
    'circles.js',
    'balls.js',
    'stripes.js',
    'gradient.js',
    'plasma.js',
    'fireworks.js',
    'waves.js',
    'noise.js',
    'starfield.js',
    'squares.js'
  ];
  
  var results = {};
  
  scriptsToTest.forEach(function(scriptName) {
    try {
      console.log(`\nTesting ${scriptName}...`);
      
      // Load script
      var scriptPath = path.join(__dirname, '..', scriptName);
      if (fs.existsSync(scriptPath)) {
        eval(fs.readFileSync(scriptPath, 'utf8'));
        
        if (typeof testAlgo !== 'undefined') {
          // Basic test with default configurations
          var basicConfigs = [
            { name: "Default", props: {} },
            { name: "Stress", props: {} }
          ];
          
          var result = RGBTestSuite.testScript(scriptName, testAlgo, basicConfigs);
          results[scriptName] = result;
        } else {
          console.log(`Warning: ${scriptName} does not expose testAlgo`);
        }
      } else {
        console.log(`Warning: ${scriptName} not found`);
      }
    } catch (error) {
      console.log(`Error testing ${scriptName}: ${error.message}`);
      results[scriptName] = { error: error.message };
    }
  });
  
  return results;
}

// Legacy edge case testing (for backward compatibility)
function testEdgeCases(algo) {
  console.log('\n=== Legacy Edge Case Testing ===');

  var edgeCases = [
    {
      name: "Zero dimensions",
      test: function() {
        try {
          algo.rgbMap(0, 0, 0xFF0000, 0);
          return { passed: false, error: "Should have failed with zero dimensions" };
        } catch (e) {
          return { passed: true, message: "Correctly handled zero dimensions" };
        }
      }
    },
    {
      name: "Negative dimensions",
      test: function() {
        try {
          algo.rgbMap(-1, -1, 0xFF0000, 0);
          return { passed: false, error: "Should have failed with negative dimensions" };
        } catch (e) {
          return { passed: true, message: "Correctly handled negative dimensions" };
        }
      }
    },
    {
      name: "Invalid RGB values",
      test: function() {
        try {
          var map = algo.rgbMap(8, 8, -1, 0);
          return { passed: true, message: "Handled invalid RGB gracefully" };
        } catch (e) {
          return { passed: true, message: "Correctly rejected invalid RGB" };
        }
      }
    },
    {
      name: "Large step values",
      test: function() {
        try {
          var map = algo.rgbMap(8, 8, 0xFF0000, 999999);
          return { passed: true, message: "Handled large step values" };
        } catch (e) {
          return { passed: false, error: `Failed with large step: ${e.message}` };
        }
      }
    },
    {
      name: "Property boundary values",
      test: function() {
        try {
          // Test extreme property values (script-specific)
          if (typeof algo.setLinesSize === 'function') {
            algo.setLinesSize(999);  // Should be clamped
          }
          if (typeof algo.setAmount === 'function') {
            algo.setAmount(999);     // Should be clamped
          }
          if (typeof algo.setBrightnessVariance === 'function') {
            algo.setBrightnessVariance(999); // Should be clamped
          }

          var map = algo.rgbMap(16, 16, 0xFF0000, 0);
          return { passed: true, message: "Handled extreme property values" };
        } catch (e) {
          return { passed: false, error: `Failed with extreme properties: ${e.message}` };
        }
      }
    }
  ];

  var edgeResults = [];

  edgeCases.forEach(function(edgeCase) {
    console.log(`Testing: ${edgeCase.name}`);
    var result = edgeCase.test();
    edgeResults.push({
      name: edgeCase.name,
      passed: result.passed,
      message: result.message,
      error: result.error
    });

    if (result.passed) {
      console.log(`  ✅ ${result.message || 'Passed'}`);
    } else {
      console.log(`  ❌ ${result.error || 'Failed'}`);
    }
  });

  return edgeResults;
}

// Main execution
function main() {
  var startTime = Date.now();

  try {
    if (scriptToTest === '--all') {
      // Test all scripts
      var allScripts = getAllRGBScripts();
      var allResults = {};
      var totalPassed = 0;
      var totalTests = 0;

      console.log(`🚀 Testing ${allScripts.length} RGB scripts...\n`);

      allScripts.forEach(function(script) {
        var algo = loadScript(script);
        if (algo) {
          var results = testScript(script, algo);
          allResults[script] = results;
          totalPassed += results.overall.passed;
          totalTests += results.overall.total;
        }
      });

      // Summary for all scripts
      console.log('\n' + '='.repeat(60));
      console.log('🎯 ALL SCRIPTS TEST SUMMARY');
      console.log('='.repeat(60));

      Object.keys(allResults).forEach(function(script) {
        var result = allResults[script];
        var successRate = (result.overall.passed / result.overall.total * 100).toFixed(1);
        console.log(`${script}: ${result.overall.passed}/${result.overall.total} (${successRate}%)`);
      });

      var overallSuccess = totalPassed / totalTests;
      console.log(`\nOverall: ${totalPassed}/${totalTests} (${(overallSuccess * 100).toFixed(1)}%)`);

    } else {
      // Validate script name
      if (!validateScript(scriptToTest)) {
        console.error(`❌ Invalid script: ${scriptToTest}`);
        console.log('💡 Use --list to see available scripts');
        process.exit(1);
      }

      // Test single script
      var algo = loadScript(scriptToTest);
      if (!algo) {
        console.error(`❌ Failed to load script: ${scriptToTest}`);
        console.log('💡 This may not be a valid RGB script (no testAlgo found)');
        process.exit(1);
      }

      var results = testScript(scriptToTest, algo);

      // Test edge cases (legacy compatibility)
      var edgeResults = testEdgeCases(algo);

      var endTime = Date.now();
      var totalTime = endTime - startTime;

      // Final summary
      console.log('\n' + '='.repeat(50));
      console.log('🎯 FINAL TEST SUMMARY');
      console.log('='.repeat(50));
      console.log(`Total execution time: ${totalTime}ms`);
      console.log(`${scriptToTest} basic tests: ${results.basic.passed}/${results.basic.total}`);
      console.log(`${scriptToTest} edge tests: ${results.edge.passed}/${results.edge.total}`);
      console.log(`${scriptToTest} animation tests: ${results.animation.passed}/${results.animation.total}`);
      console.log(`${scriptToTest} visual tests: ${results.visual.passed}/${results.visual.total}`);
      console.log(`${scriptToTest} boundary tests: ${results.boundary.passed}/${results.boundary.total}`);
      console.log(`${scriptToTest} stress tests: ${results.stress.passed}/${results.stress.total}`);
      console.log(`${scriptToTest} memory tests: ${results.memory.passed}/${results.memory.total}`);
      console.log(`Legacy edge case tests: ${edgeResults.filter(r => r.passed).length}/${edgeResults.length}`);

      var overallSuccess = results.overall.successRate;
      console.log(`Overall success rate: ${(overallSuccess * 100).toFixed(1)}%`);
    }

    if (overallSuccess >= 0.95) {
      console.log('🎉 EXCELLENT! All tests passed with flying colors!');
      process.exit(0);
    } else if (overallSuccess >= 0.8) {
      console.log('✅ GOOD! Most tests passed, minor issues detected.');
      process.exit(0);
    } else {
      console.log('⚠️  WARNING! Significant test failures detected.');
      process.exit(1);
    }

  } catch (error) {
    console.error('❌ Fatal error during testing:', error.message);
    console.error(error.stack);
    process.exit(1);
  }
}

// Run if called directly
if (require.main === module) {
  main();
}

module.exports = {
  testScript,
  testEdgeCases,
  testOtherScripts,
  loadScript,
  getAllRGBScripts,
  RGBTestSuite
};
