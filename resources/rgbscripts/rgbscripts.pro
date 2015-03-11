include(../../variables.pri)

TEMPLATE = subdirs
TARGET   = scripts

scripts.files += balls.js
scripts.files += circles.js
scripts.files += evenodd.js
scripts.files += fill.js
scripts.files += fillfromcenter.js
scripts.files += fillunfill.js
scripts.files += fillunfillfromcenter.js
scripts.files += fillunfillsquaresfromcenter.js
scripts.files += gradient.js
scripts.files += opposite.js
scripts.files += plasma.js
scripts.files += randomcolumn.js
scripts.files += randomfillcolumn.js
scripts.files += randomfillrow.js
scripts.files += randomfillsingle.js
scripts.files += randomrow.js
scripts.files += randomsingle.js
scripts.files += singlerandom.js
scripts.files += squares.js
scripts.files += squaresfromcenter.js
scripts.files += stripes.js
scripts.files += stripesfromcenter.js
scripts.files += verticalfall.js

scripts.path = $$INSTALLROOT/$$RGBSCRIPTDIR
INSTALLS    += scripts
