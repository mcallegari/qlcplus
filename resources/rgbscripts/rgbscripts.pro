include(../../variables.pri)

TEMPLATE = subdirs
TARGET   = scripts

scripts.files += balls.js
scripts.files += ballscolors.js
scripts.files += circles.js
scripts.files += evenodd.js
scripts.files += fill.js
scripts.files += fillfromcenter.js
scripts.files += fillunfill.js
scripts.files += fillunfillfromcenter.js
scripts.files += fillunfillsquaresfromcenter.js
scripts.files += gradient.js
scripts.files += noise.js
scripts.files += onebyone.js
scripts.files += opposite.js
scripts.files += plasma.js
scripts.files += plasmacolors.js
scripts.files += randomcolumn.js
scripts.files += randomfillcolumn.js
scripts.files += randomfillrow.js
scripts.files += randomfillsingle.js
scripts.files += randompixelperrow.js
scripts.files += randompixelperrowmulticolor.js
scripts.files += randomrow.js
scripts.files += randomsingle.js
scripts.files += squares.js
scripts.files += squaresfromcenter.js
scripts.files += starfield.js
scripts.files += stripes.js
scripts.files += stripesfromcenter.js
scripts.files += strobe.js
scripts.files += verticalfall.js
scripts.files += waves.js

scripts.path = $$INSTALLROOT/$$RGBSCRIPTDIR
INSTALLS    += scripts
