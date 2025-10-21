#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

// Load test suite
eval(fs.readFileSync(path.join(__dirname, 'test-suite.js'), 'utf8'));

// Load lines.js script
eval(fs.readFileSync(path.join(__dirname, '..', 'lines.js'), 'utf8'));

console.log('🔍 Testing lines.js Edge Cases');
console.log('==============================');

// Test the specific configurations that were failing
const edgeCaseMatrixes = [
  { name: "1x1 Single", width: 1, height: 1 },
  { name: "100x1 Wide", width: 100, height: 1 },
  { name: "1x100 Thin", width: 1, height: 100 },
  { name: "2x2 Tiny", width: 2, height: 2 }
];

const edgeCaseConfigs = [
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

let totalTests = 0;
let passedTests = 0;

// Test each matrix with each configuration
for (const matrix of edgeCaseMatrixes) {
  console.log(`\n📐 Testing ${matrix.name} (${matrix.width}x${matrix.height})`);
  
  for (const config of edgeCaseConfigs) {
    totalTests++;
    
    try {
      // Reset algorithm
      if (typeof testAlgo !== 'undefined') {
        delete testAlgo;
      }
      eval(fs.readFileSync(path.join(__dirname, '..', 'lines.js'), 'utf8'));
      
      // Apply configuration
      for (const [prop, value] of Object.entries(config.props)) {
        const setter = `set${prop.charAt(0).toUpperCase() + prop.slice(1)}`;
        if (typeof testAlgo[setter] === 'function') {
          testAlgo[setter](value);
        }
      }
      
      // Test multiple steps
      const stepCount = testAlgo.rgbMapStepCount(matrix.width, matrix.height);
      
      for (let step = 0; step < Math.min(stepCount, 5); step++) {
        const rgbMap = testAlgo.rgbMap(matrix.width, matrix.height, 0xFF0000, step);
        
        // Validate RGB map structure
        if (!Array.isArray(rgbMap) || rgbMap.length !== matrix.height) {
          throw new Error(`Invalid RGB map height: expected ${matrix.height}, got ${rgbMap.length}`);
        }
        
        for (let y = 0; y < matrix.height; y++) {
          if (!Array.isArray(rgbMap[y]) || rgbMap[y].length !== matrix.width) {
            throw new Error(`Invalid RGB map width at row ${y}: expected ${matrix.width}, got ${rgbMap[y].length}`);
          }
          
          for (let x = 0; x < matrix.width; x++) {
            const pixel = rgbMap[y][x];
            if (typeof pixel !== 'number' || pixel < 0 || pixel > 0xFFFFFF) {
              throw new Error(`Invalid pixel value at [${y}][${x}]: ${pixel}`);
            }
          }
        }
      }
      
      console.log(`  ✅ ${config.name}: PASS`);
      passedTests++;
      
    } catch (error) {
      console.log(`  ❌ ${config.name}: FAIL - ${error.message}`);
    }
  }
}

console.log(`\n📊 Edge Case Test Results:`);
console.log(`   Passed: ${passedTests}/${totalTests}`);
console.log(`   Success Rate: ${((passedTests / totalTests) * 100).toFixed(1)}%`);

if (passedTests === totalTests) {
  console.log(`\n🎉 All edge case tests PASSED! The fixes are working correctly.`);
  process.exit(0);
} else {
  console.log(`\n⚠️  Some edge case tests failed. Review the fixes.`);
  process.exit(1);
}
