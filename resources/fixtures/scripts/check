#!/bin/sh
DIR=`dirname $0`
set -e

# Validate the fixtures against the XML scheme
xmllint --noout --schema "$DIR"/../../schemas/fixture.xsd $(find "$DIR"/.. -name *.qxf) >/dev/null 2>&1 
RES=$?
if [ $RES -ne 0 ]; then
	# Re-Validate to print the error cause
	xmllint --noout --schema "$DIR"/../../schemas/fixture.xsd $(find "$DIR"/.. -name *.qxf) 2>&1 | grep -v " validates$"
	exit $RES
else
	echo "Fixtures: OK"
fi

cd "$DIR"/.. && ./scripts/fixtures-tool.py --validate || exit $?
cd - >>/dev/null

# Validate the fixture map against the XML scheme
xmllint --noout --schema "$DIR"/../../schemas/fixturesmap.xsd "$DIR"/../FixturesMap.xml >/dev/null 2>&1
RES=$?
if [ $RES -ne 0 ]; then
	# Re-Validate to print the error cause
	xmllint --noout --schema "$DIR"/../../schemas/fixturesmap.xsd "$DIR"/../FixturesMap.xml 2>&1
	exit $RES
else
	echo "Fixturesmap: OK"
fi


