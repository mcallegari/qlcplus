include(../variables.pri)

TEMPLATE = subdirs
TARGET   = scripts

scripts.files += evenodd.js
scripts.files += fill.js
scripts.files += fillcolumnsfromcenter.js
scripts.files += fillrowsfromcenter.js
scripts.files += fillsquaresfromcenter.js
scripts.files += fillunfillcolumnsfromcenter.js
scripts.files += fillunfillcolumns.js
scripts.files += fillunfillrowsfromcenter.js
scripts.files += fillunfillrows.js
scripts.files += fillunfillsquaresfromcenter.js
scripts.files += fullcolumnsfromcenter.js
scripts.files += fullrowsfromcenter.js
scripts.files += oppositecolumns.js
scripts.files += oppositerows.js
scripts.files += randomcolumn.js
scripts.files += randomfillcolumn.js
scripts.files += randomfillrow.js
scripts.files += randomfillsingle.js
scripts.files += randomrow.js
scripts.files += randomsingle.js
scripts.files += singlerandom.js
scripts.files += squaresfromcenter.js
scripts.files += stripes.js
scripts.files += verticalfall.js

scripts.path = $$INSTALLROOT/$$RGBSCRIPTDIR
INSTALLS    += scripts
