@xmllint --noout --schema ../../schemas/fixture.xsd $(find .. -name *.qxf) 2>&1 > NUL | grep -v " validates$"
@xmllint --noout --schema ../../schemas/fixturesmap.xsd ../FixturesMap.xml 2>&1 > NUL | grep -v " validates$"

