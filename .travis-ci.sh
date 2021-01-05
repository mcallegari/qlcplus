#!/bin/bash

# This script is triggered from the script section of .travis.yml
# It runs the appropriate commands depending on the task requested.

# Otherwise compile and check as normal
if [ "$TASK" = "compile" ]; then
  if [ "$QT" = "qt5" ]; then
    $QMAKE QMAKE_CXX=$CXX QMAKE_CC=$CC QMAKE_LINK=$CXX QMAKE_LINK_SHLIB=$CXX && make && ./unittest.sh
    exit $?  
  fi
  if [ "$QT" = "qt5qml" ]; then
    $QMAKE QMAKE_CXX=$CXX QMAKE_CC=$CC QMAKE_LINK=$CXX QMAKE_LINK_SHLIB=$CXX CONFIG+=qmlui && make
    exit $?  
  fi  
fi
if [ "$TASK" = "coverage" ]; then
$QMAKE CONFIG+=coverage QMAKE_CXX=$CXX QMAKE_CC=$CC QMAKE_LINK=$CXX QMAKE_LINK_SHLIB=$CXX && make && ./coverage.sh
exit $?
fi
