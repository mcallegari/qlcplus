#!/usr/bin/env node

/**
 * Comprehensive RGB Script Tester
 * Tests ALL RGB scripts in the directory automatically
 */

const fs = require('fs');
const path = require('path');

// Load test suite
eval(fs.readFileSync(path.join(__dirname, 'test-suite.js'), 'utf8'));

function getAllRGBScripts() {
  const scriptsDir = path.join(__dirname, '..');
  const files = fs.readdirSync(scriptsDir);

  return files.filter(file => {
    return file.endsWith('.js') &&
           file !== 'test-suite.js' &&
           file !== 'run-tests.js' &&
           file !== 'test-all-scripts.js' &&
           !file.startsWith('test-') &&
           !fs.statSync(path.join(scriptsDir, file)).isDirectory();
  }).sort();
}

function testScript(scriptName) {
  console.log(`\n${'='.repeat(50)}`);
  console.log(`🧪 TESTING: ${scriptName}`);
  console.log(`${'='.repeat(50)}`);
  
  try {
    // Clear any previous algorithm
    if (typeof testAlgo !== 'undefined') {
      delete testAlgo;
    }
    
    // Load script
    const scriptPath = path.join(__dirname, '..', scriptName);
    const scriptContent = fs.readFileSync(scriptPath, 'utf8');
    eval(scriptContent);
    
    if (typeof testAlgo === 'undefined') {
      console.log(`❌ SKIP: ${scriptName} - No testAlgo found`);
      return { status: 'skipped', reason: 'No testAlgo' };
    }
    
    // Test basic functionality
    const startTime = Date.now();
    const results = RGBTestSuite.testScript(scriptName, testAlgo);
    const duration = Date.now() - startTime;
    
    console.log(`✅ PASS: ${scriptName}`);
    console.log(`   Tests: ${results.passed}/${results.total}`);
    console.log(`   Success Rate: ${(results.successRate * 100).toFixed(1)}%`);
    console.log(`   Duration: ${duration}ms`);
    
    return {
      status: 'passed',
      passed: results.passed,
      total: results.total,
      successRate: results.successRate,
      duration: duration
    };
    
  } catch (error) {
    console.log(`❌ FAIL: ${scriptName} - ${error.message}`);
    return { 
      status: 'failed', 
      error: error.message,
      duration: 0
    };
  }
}

function generateReport(results) {
  console.log(`\n${'='.repeat(60)}`);
  console.log(`📊 COMPREHENSIVE RGB SCRIPT TEST REPORT`);
  console.log(`${'='.repeat(60)}`);
  
  const passed = results.filter(r => r.status === 'passed');
  const failed = results.filter(r => r.status === 'failed');
  const skipped = results.filter(r => r.status === 'skipped');
  
  console.log(`\n📈 SUMMARY:`);
  console.log(`   Total Scripts: ${results.length}`);
  console.log(`   ✅ Passed: ${passed.length}`);
  console.log(`   ❌ Failed: ${failed.length}`);
  console.log(`   ⏭️  Skipped: ${skipped.length}`);
  console.log(`   🎯 Success Rate: ${((passed.length / results.length) * 100).toFixed(1)}%`);
  
  if (passed.length > 0) {
    console.log(`\n✅ PASSED SCRIPTS:`);
    passed.forEach(result => {
      console.log(`   ${result.script}: ${result.passed}/${result.total} tests (${(result.successRate * 100).toFixed(1)}%)`);
    });
  }
  
  if (failed.length > 0) {
    console.log(`\n❌ FAILED SCRIPTS:`);
    failed.forEach(result => {
      console.log(`   ${result.script}: ${result.error}`);
    });
  }
  
  if (skipped.length > 0) {
    console.log(`\n⏭️  SKIPPED SCRIPTS:`);
    skipped.forEach(result => {
      console.log(`   ${result.script}: ${result.reason}`);
    });
  }
  
  const totalDuration = results.reduce((sum, r) => sum + (r.duration || 0), 0);
  console.log(`\n⏱️  Total Test Duration: ${totalDuration}ms`);
  
  // Performance ranking
  const performanceRanked = passed
    .filter(r => r.duration > 0)
    .sort((a, b) => a.duration - b.duration);
    
  if (performanceRanked.length > 0) {
    console.log(`\n🏃 PERFORMANCE RANKING (fastest to slowest):`);
    performanceRanked.forEach((result, index) => {
      const emoji = index === 0 ? '🥇' : index === 1 ? '🥈' : index === 2 ? '🥉' : '  ';
      console.log(`   ${emoji} ${result.script}: ${result.duration}ms`);
    });
  }
}

function main() {
  console.log('🎨 RGB Script Comprehensive Tester');
  console.log('===================================');
  
  const scripts = getAllRGBScripts();
  console.log(`Found ${scripts.length} RGB scripts to test:`);
  scripts.forEach(script => console.log(`  - ${script}`));
  
  const results = [];
  
  scripts.forEach(script => {
    const result = testScript(script);
    result.script = script;
    results.push(result);
  });
  
  generateReport(results);
  
  // Exit code based on results
  const failedCount = results.filter(r => r.status === 'failed').length;
  process.exit(failedCount > 0 ? 1 : 0);
}

if (require.main === module) {
  main();
}

module.exports = { getAllRGBScripts, testScript, generateReport };
