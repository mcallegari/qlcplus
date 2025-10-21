#!/usr/bin/env node

/*
  Test Any RGB Script
  
  This script tests the ability to load and test any RGB script
  in the directory, not just the predefined ones.
  
  Usage: node test-any-script.js [script-name]
*/

const fs = require('fs');
const path = require('path');

// Load test suite
eval(fs.readFileSync(path.join(__dirname, 'test-suite.js'), 'utf8'));

function getAllScripts() {
  const scriptsDir = path.join(__dirname, '..');
  const files = fs.readdirSync(scriptsDir);
  
  return files.filter(file => {
    return file.endsWith('.js') && 
           !file.startsWith('test-') &&
           !fs.statSync(path.join(scriptsDir, file)).isDirectory();
  }).sort();
}

function testAnyScript(scriptName) {
  console.log(`🧪 Testing ${scriptName}...`);
  
  try {
    // Clear any existing testAlgo
    if (typeof testAlgo !== 'undefined') {
      delete testAlgo;
    }
    
    // Load script
    const scriptPath = path.join(__dirname, '..', scriptName);
    const scriptContent = fs.readFileSync(scriptPath, 'utf8');
    eval(scriptContent);
    
    if (typeof testAlgo === 'undefined') {
      console.log(`⚠️  ${scriptName} does not expose testAlgo (not an RGB script)`);
      return false;
    }
    
    console.log(`✅ Successfully loaded ${scriptName}`);
    
    // Run basic test
    const results = RGBTestSuite.testScript(scriptName, testAlgo);
    console.log(`📊 Results: ${results.passed}/${results.total} tests passed`);
    
    return results.passed === results.total;
    
  } catch (error) {
    console.log(`❌ Error testing ${scriptName}: ${error.message}`);
    return false;
  }
}

function main() {
  const args = process.argv.slice(2);
  const scriptToTest = args[0];
  
  if (!scriptToTest) {
    console.log('🎨 Test Any RGB Script');
    console.log('======================\n');
    
    const allScripts = getAllScripts();
    console.log(`Found ${allScripts.length} JavaScript files:\n`);
    
    let rgbScripts = 0;
    let totalPassed = 0;
    let totalTests = 0;
    
    allScripts.forEach((script, index) => {
      console.log(`${index + 1}. Testing ${script}...`);
      
      if (testAnyScript(script)) {
        rgbScripts++;
      }
      
      console.log(''); // Empty line for readability
    });
    
    console.log(`\n📈 Summary:`);
    console.log(`   Total files tested: ${allScripts.length}`);
    console.log(`   Valid RGB scripts: ${rgbScripts}`);
    console.log(`   Success rate: ${((rgbScripts / allScripts.length) * 100).toFixed(1)}%`);
    
  } else {
    // Test specific script
    if (!scriptToTest.endsWith('.js')) {
      console.error('❌ Script name must end with .js');
      process.exit(1);
    }
    
    const scriptPath = path.join(__dirname, '..', scriptToTest);
    if (!fs.existsSync(scriptPath)) {
      console.error(`❌ Script not found: ${scriptToTest}`);
      process.exit(1);
    }
    
    console.log(`🎯 Testing specific script: ${scriptToTest}\n`);
    const success = testAnyScript(scriptToTest);
    
    if (success) {
      console.log('\n🎉 Test completed successfully!');
      process.exit(0);
    } else {
      console.log('\n⚠️  Test completed with issues.');
      process.exit(1);
    }
  }
}

if (require.main === module) {
  main();
}

module.exports = { testAnyScript, getAllScripts };
