#!/bin/bash

#############################################################################
# Engine tests
#############################################################################

CURRUSER=$(whoami)
TESTPREFIX=""
SLEEPCMD=""
HAS_XSERVER="0"

if [ "$CURRUSER" == "buildbot" ] || [ "$CURRUSER" == "abuild" ]; then
  if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [ $(which xvfb-run) == "" ]; then
      echo "xvfb-run not found in this system. Please install with: sudo apt-get install xvfb"
      exit
    fi
    TESTPREFIX="xvfb-run"
    HAS_XSERVER="1"
    # if we're running as build slave, set a sleep time to start/stop xvfb between tests
    SLEEPCMD="sleep 1"
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "We're on OSX. Any prefix needed ?"
  fi

else

  if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    XPID=$(pidof X)
    if [ ! ${#XPID} -gt 0 ]; then
      XPID=$(pidof Xorg)
    fi
    if [ ${#XPID} -gt 0 ]; then
      HAS_XSERVER="1"
    fi

    # no X server ? Let's look for xvfb. This is how Travis is setup
    if [ -n "$TRAVIS" ]; then
        HAS_XSERVER="1"
    fi
  fi
fi

# run xmllint on fixture definitions
pushd .
cd resources/fixtures/scripts
VALIDATION_ERRORS=$(./check)
popd
echo $VALIDATION_ERRORS
if [ "${VALIDATION_ERRORS}" ]; then
    echo "Fixture definitions are not valid. Please fix before commit."
    exit 1
fi

TESTDIR=engine/test
TESTS=$(find ${TESTDIR} -maxdepth 1 -mindepth 1 -type d)
for test in ${TESTS}
do
    # Ignore .git
    if [ $(echo ${test} | grep ".git") ]; then
        continue
    fi

    # Isolate just the test name
    test=$(echo ${test} | sed 's/engine\/test\///')

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

if [ "$HAS_XSERVER" -eq "1" ]; then

TESTDIR=ui/test
TESTS=$(find ${TESTDIR} -maxdepth 1 -mindepth 1 -type d)
for test in ${TESTS}
do
    # Ignore .git
    if [ $(echo ${test} | grep ".git") ]; then
        continue
    fi

    # Isolate just the test name
    test=$(echo ${test} | sed 's/ui\/test\///')

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

fi

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

if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "Skip Velleman test (not supported on OSX)"
else
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
fi

#############################################################################
# MIDI tests
#############################################################################

$SLEEPCMD
pushd .
cd plugins/midi/test
$TESTPREFIX ./test.sh
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "${RESULT} MIDI unit tests failed. Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# ArtNet tests
#############################################################################

$SLEEPCMD
pushd .
cd plugins/artnet/test
$TESTPREFIX ./test.sh
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "${RESULT} ArtNet unit tests failed. Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Final judgment
#############################################################################

echo "Unit tests passed."
