#!/bin/bash
#
# To measure unit test coverage, perform these steps:
# 0. export CCACHE_DISABLE=1        # (only if you use compiler cache)
# 1. qmake
# 2. make distclean
# 3. qmake CONFIG+=coverage
# 4. make -j8
# 5. ./coverage.sh ui|qmlui # OR
# 5. make lcov
#
# Human-readable HTML results are written under coverage/html.
#

set -e
ARCH=$(uname)
THISCMD=`basename "$0"`

TARGET=${1:-}

if [ "$TARGET" != "ui" ] && [ "$TARGET" != "qmlui" ]; then
  echo >&2 "Usage: $THISCMD ui|qmlui"
  exit 1
fi


#############################################################################
# Test directories to find coverage measurements from
#############################################################################

COUNT=0
test[$COUNT]="engine/src"
COUNT=$((COUNT+1))
if [ "$TARGET" == "ui" ]; then
    test[$COUNT]="ui/src"
COUNT=$((COUNT+1))
fi
test[$COUNT]="plugins/artnet/test"
COUNT=$((COUNT+1))
test[$COUNT]="plugins/enttecwing/src"
COUNT=$((COUNT+1))
#test[$COUNT]="plugins/midiinput/common/src"
#COUNT=$((COUNT+1))
if [ ${ARCH} != "Darwin" ]; then
    test[$COUNT]="plugins/velleman/src"
    COUNT=$((COUNT+1))
fi

# Number of tests
tlen=${#test[@]}

#############################################################################
# Functions
#############################################################################

# arg1:srcdir arg2:testname
function prepare {
    lcov -d ${1} -z || exit $?
    rm -f ${1}/moc_*.gcno ${1}/moc_*.gcda
    lcov -d ${1} -c -i -o coverage/${2}-base.info
}

# arg1:srcdir arg2:testname
function gather_data {
    lcov -d ${1} -c -o coverage/${2}-test.info
    lcov -a coverage/${2}-base.info -a coverage/${2}-test.info \
         -o coverage/${2}-merge.info
}

#############################################################################
# Initialization
#############################################################################

# Check if lcov is installed
if [ -z "$(which lcov)" ]; then
    echo "Unable to produce coverage results; can't find lcov."
fi

# Remove previous data
if [ -d coverage ]; then
    rm -rf coverage
fi

# Create directories for new coverage data
mkdir -p coverage/html

#############################################################################
# Preparation
#############################################################################

for ((i = 0; i < tlen; i++))
do
    prepare ${test[i]} $i || exit $?
done

#############################################################################
# Run unit tests
#############################################################################

./unittest.sh $TARGET
FAILED=$?
if [ ${FAILED} != 0 ]; then
    echo "Will not measure coverage because ${FAILED} unit tests failed."
    exit ${FAILED}
fi

#############################################################################
# Gather results
#############################################################################

for ((i = 0; i < tlen; i++))
do
    gather_data ${test[i]} $i
done

#############################################################################
# All combined and HTMLized
#############################################################################

for ((i = 0; i < tlen; i++))
do
    mergeargs="${mergeargs} -a coverage/${i}-merge.info"
done

lcov ${mergeargs} -o coverage/coverage.info

# Remove stuff that isn't part of QLC sources
lcov -r coverage/coverage.info *.h -o coverage/coverage.info # Q_OBJECT etc.
lcov -r coverage/coverage.info moc_* -o coverage/coverage.info
lcov -r coverage/coverage.info *usr* -o coverage/coverage.info
lcov -r coverage/coverage.info *_test* -o coverage/coverage.info
lcov -r coverage/coverage.info ui_* -o coverage/coverage.info
lcov -r coverage/coverage.info *Library* -o coverage/coverage.info # OSX

# Generate HTML report
genhtml -o coverage/html coverage/coverage.info
