#!/bin/bash

#############################################################################
# Engine tests
#############################################################################

CURRUSER=`whoami`
TESTPREFIX=""
SLEEPCMD=""

if [ "$CURRUSER" == "buildbot" ]; then
  if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [ `which xvfb-run` == "" ]; then
      echo "xvfb-run not found in this system. Please install with: sudo apt-get install xvfb"
      exit
    fi
    TESTPREFIX="xvfb-run"
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "We're on OSX. Any prefix needed ?"
  fi
fi

# unfortunately the Raspberry Pi isn't fast enough to start/stop
# xvfb in time between the tests. So give him a bit of breath !
if [[ "$OSTYPE" == "linux-gnueabihf" ]]; then
  SLEEPCMD="sleep 1"
fi

TESTDIR=engine/test
TESTS=`find ${TESTDIR} -maxdepth 1 -mindepth 1 -type d`
for test in ${TESTS}
do
    # Ignore .git
    if [ `echo ${test} | grep ".git"` ]; then
        continue
    fi

    # Isolate just the test name
    test=`echo ${test} | sed 's/engine\/test\///'`

    $SLEEPCMD
    # Execute the test
    pushd .
    cd ${TESTDIR}/${test}
    $TESTPREFIX ./test.sh
    RESULT=${?}
    popd
    if [ ${RESULT} != 0 ]; then
        echo "${RESULT} Engine unit tests failed. Please fix before commit."
        exit ${RESULT}
    fi
done

#############################################################################
# UI tests
#############################################################################

TESTDIR=ui/test
TESTS=`find ${TESTDIR} -maxdepth 1 -mindepth 1 -type d`
for test in ${TESTS}
do
    # Ignore .git
    if [ `echo ${test} | grep ".git"` ]; then
        continue
    fi

    # Isolate just the test name
    test=`echo ${test} | sed 's/ui\/test\///'`

    $SLEEPCMD
    # Execute the test
    pushd .
    cd ${TESTDIR}/${test}
    DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../../engine/src:../../src \
        LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../engine/src:../../src $TESTPREFIX ./${test}_test
    RESULT=${?}
    popd
    if [ ${RESULT} != 0 ]; then
        echo "${RESULT} UI unit tests failed. Please fix before commit."
        exit ${RESULT}
    fi
done

#############################################################################
# Enttec wing tests
#############################################################################

$SLEEPCMD
pushd .
cd plugins/enttecwing/test
$TESTPREFIX ./test.sh
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "${RESULT} Enttec wing unit tests failed. Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Velleman test
#############################################################################

$SLEEPCMD
pushd .
cd plugins/velleman/test
$TESTPREFIX ./test.sh
RESULT=$?
if [ $RESULT != 0 ]; then
    echo "Velleman unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# MIDI tests
#############################################################################
#pushd .
#cd plugins/midi/common/test
#DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
#	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./common_test
#RESULT=$?
#if [ $RESULT != 0 ]; then
#    echo "MIDI Input common unit test failed ($RESULT). Please fix before commit."
#    exit $RESULT
#fi
#popd

#############################################################################
# Final judgment
#############################################################################

echo "Unit tests passed."
