# QLCPlus RGB Script Test Suite

A comprehensive testing framework for validating QLCPlus RGB scripts across various matrix configurations and edge cases.

## 📁 Files Overview

### Core Test Framework
- **`test-suite.js`** - Core testing framework with validation functions
- **`run-tests.js`** - Node.js test runner for individual scripts
- **`test-all-scripts.js`** - Comprehensive tester for all RGB scripts
- **`test-runner.html`** - Browser-based visual test interface

### Specialized Tests
- **`test-lines-edge-cases.js`** - Edge case testing for lines.js
- **`TEST_RESULTS.md`** - Comprehensive test documentation

## 🚀 Usage

### Command Line Testing

#### Test Any Individual Script
```bash
cd tests
node run-tests.js [script-name]

# Examples:
node run-tests.js                    # Test lines.js (default)
node run-tests.js lines.js           # Test lines.js specifically
node run-tests.js circles.js         # Test circles.js
node run-tests.js balls.js           # Test balls.js
node run-tests.js --list             # List available scripts
```

#### Test All RGB Scripts
```bash
cd tests
node test-all-scripts.js
```

#### Test Edge Cases Only (Legacy)
```bash
cd tests
node test-lines-edge-cases.js
```

### Browser Testing
```bash
# Open in browser
open test-runner.html
```

**New Features:**
- **Script Selection Dropdown**: Choose any RGB script from the dropdown menu
- **Edge Case Testing**: Integrated edge case tests for problematic matrix configurations
- **Real-time Script Loading**: Dynamically load and test different scripts without page refresh

**Note**: Due to CORS restrictions when opening HTML files directly in browsers, all 39 RGB scripts are pre-populated in the dropdown. Script availability is verified when you select and load a script.

## 📊 Test Coverage

### Matrix Configurations Tested
- **8x8 Square** - Standard small matrix
- **16x16 Square** - Medium square matrix
- **8x64 Strip** - Vertical LED strip
- **16x32 Strip** - Medium strip
- **32x16 Strip** - Horizontal strip
- **64x8 Strip** - Wide horizontal strip
- **32x32 Large** - Large square matrix
- **64x64 XLarge** - Extra large matrix
- **1x100 Thin** - Single row strip
- **100x1 Wide** - Single column strip
- **1x1 Single** - Single pixel (edge case)
- **128x128 Huge** - Maximum size matrix

### Test Types
1. **Basic Functionality** - Default configurations across all matrices
2. **Feature Testing** - All property combinations and values
3. **Edge Case Testing** - Problematic matrix configurations (1x1, 100x1, 1x100, 2x2)
4. **Stress Testing** - Maximum parameters and extreme configurations
5. **Legacy Edge Cases** - Invalid inputs, boundary conditions, single pixels
6. **Memory Testing** - Memory leak detection over 1000 iterations
7. **Performance Testing** - Speed benchmarking and optimization

### Lines.js Specific Tests
- **Amount**: 1-200 lines
- **Size**: 1-32 pixel thickness
- **Type**: 13 different line types (Horizontal, Vertical, Plus, Star, etc.)
- **Movement**: 9 movement modes (None, Up, Down, Left, Right, Loops)
- **Lifecycle**: 9 size/fade behaviors (Grow, Shrink, Static, Fade combinations)
- **Distribution**: 7 positioning patterns (All, Every 2nd, Center Third, etc.)
- **Patterns**: 4 visual styles (Solid, Dashed, Dotted, Double)
- **Brightness Variance**: 0-100% individual line brightness variation
- **Movement Energy**: 10-200% speed and variance control

## 🆕 New Features

### Universal Script Testing
- **Any RGB Script Support**: Test any `.js` file in the RGB scripts directory
- **Automatic Script Discovery**: `--list` command shows all available scripts
- **Script-Specific Configurations**: Tailored test configurations for different script types
- **Dynamic Script Loading**: Browser interface supports real-time script switching

### Enhanced Edge Case Testing
- **Integrated Edge Cases**: Edge case tests are now part of the main test suite
- **Problematic Matrix Support**: Specialized testing for 1x1, 100x1, 1x100, and 2x2 matrices
- **Script-Aware Testing**: Edge case tests adapt to the specific script being tested

### Improved Test Organization
- **Modular Test Configs**: Script-specific test configurations (lines, circles, balls, generic)
- **Comprehensive Coverage**: Basic + Edge + Stress + Memory + Performance testing
- **Better Reporting**: Detailed breakdown of all test types in final summary

## 🔧 Test Results

### Recent Fixes Applied
1. **Distribution Edge Cases** - Fixed negative values in small dimensions
2. **Bounds Checking** - Added pixel map bounds validation
3. **1x1 Matrix Support** - Proper handling of single-pixel matrices
4. **100x1 and 1x100 Support** - Fixed strip matrix edge cases
5. **Universal Script Support** - Any RGB script can now be tested
6. **Edge Case Integration** - Edge cases integrated into main test suite

### Current Status
- ✅ **lines.js**: 240/240 tests passing (100% success rate)
- ✅ **All 39 RGB scripts**: Comprehensive testing completed
- ✅ **Edge cases**: All critical edge cases resolved
- ✅ **Performance**: Excellent performance across all matrix sizes

## 🛠️ Adding New Tests

### For New RGB Scripts
1. Add script name to `scriptsToTest` array in `run-tests.js`
2. Create test configurations in `test-suite.js`
3. Run comprehensive test: `node test-all-scripts.js`

### For New Test Cases
1. Add matrix configuration to `testMatrixes` in `test-suite.js`
2. Add test configuration to script-specific configs
3. Update validation functions if needed

## 📈 Performance Benchmarks

### Fastest Scripts (< 50ms)
- evenodd.js: 37ms
- strobe.js: 39ms
- snowbubbles.js: 47ms

### lines.js Performance
- **187ms** for comprehensive testing
- **Excellent** performance across all matrix sizes
- **Memory stable** - no leaks detected

### Slowest Scripts (still functional)
- randomfillsingle.js: 75.5 seconds
- circular.js: 16.4 seconds
- blinder.js: 9.1 seconds

## 🎯 Quality Assurance

The test suite ensures:
- **Compatibility** across all real-world matrix sizes
- **Stability** under extreme parameter combinations
- **Performance** optimization and regression detection
- **Memory safety** with leak detection
- **Edge case handling** for unusual configurations

## 🔄 CI/CD Integration

Exit codes for automated testing:
- **0**: All tests passed
- **1**: Some tests failed

Example CI usage:
```bash
cd tests && node test-all-scripts.js
echo "Exit code: $?"
```

## 📝 Notes

- Test suite automatically discovers all `.js` files in parent directory
- Skips non-RGB scripts gracefully (those without `testAlgo`)
- Provides detailed logging and error reporting
- Supports both Node.js and browser environments
- Maintains compatibility with QLCPlus RGB Script API v2
