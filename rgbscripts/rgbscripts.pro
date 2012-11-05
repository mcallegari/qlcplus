include(../variables.pri)

TEMPLATE = subdirs
TARGET   = scripts

scripts.files += evenodd.js
scripts.files += fullcolumns.js
scripts.files += fullrows.js
scripts.files += oppositecolumns.js
scripts.files += oppositerows.js
scripts.files += singlerandom.js

scripts.path = $$INSTALLROOT/$$RGBSCRIPTDIR
INSTALLS    += scripts
