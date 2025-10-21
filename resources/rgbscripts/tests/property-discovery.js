/*
  Q Light Controller Plus
  RGB Script Property Discovery Tool
  
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

const fs = require('fs');
const path = require('path');

// Load test suite for property analysis functions
const testSuiteContent = fs.readFileSync(path.join(__dirname, 'test-suite.js'), 'utf8');
eval(testSuiteContent);

function getAllRGBScripts() {
  const scriptsDir = path.join(__dirname, '..');
  const files = fs.readdirSync(scriptsDir);
  
  return files.filter(file => {
    return file.endsWith('.js') && 
           !file.startsWith('test-') &&
           !fs.statSync(path.join(scriptsDir, file)).isDirectory();
  }).sort();
}

function analyzeAllScripts() {
  console.log('🔍 RGB Script Property Discovery Tool');
  console.log('=====================================\n');
  
  const scripts = getAllRGBScripts();
  const analysis = {
    totalScripts: scripts.length,
    scriptsWithProperties: 0,
    scriptsWithoutProperties: 0,
    totalProperties: 0,
    propertyTypes: {},
    scriptAnalysis: {}
  };
  
  for (const script of scripts) {
    const scriptPath = path.join(__dirname, '..', script);
    const content = fs.readFileSync(scriptPath, 'utf8');
    
    // Extract script name and author
    const nameMatch = content.match(/algo\.name\s*=\s*["']([^"']+)["']/);
    const authorMatch = content.match(/algo\.author\s*=\s*["']([^"']+)["']/);
    
    const properties = RGBTestSuite.analyzeScriptProperties(content);
    
    const scriptInfo = {
      filename: script,
      name: nameMatch ? nameMatch[1] : script.replace('.js', ''),
      author: authorMatch ? authorMatch[1] : 'Unknown',
      propertyCount: properties.length,
      properties: properties,
      hasProperties: properties.length > 0
    };
    
    analysis.scriptAnalysis[script] = scriptInfo;
    
    if (properties.length > 0) {
      analysis.scriptsWithProperties++;
      analysis.totalProperties += properties.length;
      
      // Count property types
      for (const prop of properties) {
        const type = prop.type || 'unknown';
        analysis.propertyTypes[type] = (analysis.propertyTypes[type] || 0) + 1;
      }
    } else {
      analysis.scriptsWithoutProperties++;
    }
  }
  
  return analysis;
}

function generateConfigurationReport(analysis) {
  console.log('📊 PROPERTY ANALYSIS SUMMARY');
  console.log('============================');
  console.log(`Total Scripts: ${analysis.totalScripts}`);
  console.log(`Scripts with Properties: ${analysis.scriptsWithProperties}`);
  console.log(`Scripts without Properties: ${analysis.scriptsWithoutProperties}`);
  console.log(`Total Properties Found: ${analysis.totalProperties}`);
  console.log(`Average Properties per Script: ${(analysis.totalProperties / analysis.scriptsWithProperties).toFixed(1)}`);
  
  console.log('\n📈 Property Types Distribution:');
  for (const [type, count] of Object.entries(analysis.propertyTypes)) {
    console.log(`  ${type}: ${count} properties`);
  }
  
  console.log('\n🎯 SCRIPTS WITH PROPERTIES:');
  console.log('===========================');
  
  const scriptsWithProps = Object.values(analysis.scriptAnalysis)
    .filter(script => script.hasProperties)
    .sort((a, b) => b.propertyCount - a.propertyCount);
  
  for (const script of scriptsWithProps) {
    console.log(`\n📄 ${script.name} (${script.filename})`);
    console.log(`   Author: ${script.author}`);
    console.log(`   Properties: ${script.propertyCount}`);
    
    for (const prop of script.properties) {
      const display = prop.display || prop.name;
      const type = prop.type || 'unknown';
      const values = prop.values || 'none';
      console.log(`     • ${display} (${prop.name}): ${type} [${values}]`);
    }
  }
  
  console.log('\n🔧 SCRIPTS WITHOUT PROPERTIES:');
  console.log('==============================');
  
  const scriptsWithoutProps = Object.values(analysis.scriptAnalysis)
    .filter(script => !script.hasProperties)
    .sort((a, b) => a.name.localeCompare(b.name));
  
  for (const script of scriptsWithoutProps) {
    console.log(`  • ${script.name} (${script.filename}) - ${script.author}`);
  }
}

function generateTestConfigurations(analysis) {
  console.log('\n🧪 GENERATING TEST CONFIGURATIONS');
  console.log('=================================');
  
  const generatedConfigs = {};
  
  for (const [filename, scriptInfo] of Object.entries(analysis.scriptAnalysis)) {
    if (scriptInfo.hasProperties) {
      const baseName = filename.replace('.js', '').toLowerCase();
      const configs = RGBTestSuite.generatePropertyTestConfigs(scriptInfo.properties);
      
      generatedConfigs[baseName] = configs;
      
      console.log(`\n📋 ${scriptInfo.name} (${configs.length} configurations):`);
      for (const config of configs.slice(0, 5)) { // Show first 5
        const propCount = Object.keys(config.props).length;
        console.log(`   • ${config.name} (${propCount} properties)`);
      }
      if (configs.length > 5) {
        console.log(`   ... and ${configs.length - 5} more configurations`);
      }
    }
  }
  
  return generatedConfigs;
}

function saveConfigurationsToFile(generatedConfigs) {
  const configContent = `// Auto-generated script configurations
// Generated on ${new Date().toISOString()}

var autoGeneratedConfigs = ${JSON.stringify(generatedConfigs, null, 2)};

// Export for use in test suite
if (typeof module !== 'undefined' && module.exports) {
  module.exports = autoGeneratedConfigs;
}
`;

  const outputPath = path.join(__dirname, 'auto-generated-configs.js');
  fs.writeFileSync(outputPath, configContent);
  
  console.log(`\n💾 Configurations saved to: ${outputPath}`);
  console.log(`📊 Generated ${Object.keys(generatedConfigs).length} script configurations`);
  
  const totalConfigs = Object.values(generatedConfigs)
    .reduce((sum, configs) => sum + configs.length, 0);
  console.log(`🎯 Total test configurations: ${totalConfigs}`);
}

// Main execution
if (require.main === module) {
  try {
    const analysis = analyzeAllScripts();
    generateConfigurationReport(analysis);
    const generatedConfigs = generateTestConfigurations(analysis);
    saveConfigurationsToFile(generatedConfigs);
    
    console.log('\n✅ Property discovery completed successfully!');
    
  } catch (error) {
    console.error('❌ Error during property discovery:', error.message);
    process.exit(1);
  }
}

module.exports = {
  analyzeAllScripts,
  generateConfigurationReport,
  generateTestConfigurations,
  saveConfigurationsToFile,
  getAllRGBScripts
};
