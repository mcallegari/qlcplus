include(../../variables.pri)

TEMPLATE = subdirs
TARGET   = scripts

scripts.files += alternate.js
scripts.files += balls.js
scripts.files += blinder.js
scripts.files += circles.js
scripts.files += circular.js
scripts.files += evenodd.js
scripts.files += fill.js
scripts.files += fillfromcenter.js
scripts.files += fillunfill.js
scripts.files += fillunfillfromcenter.js
scripts.files += fillunfillsquaresfromcenter.js
scripts.files += fireworks.js
scripts.files += flyingobjects.js
scripts.files += gradient.js
scripts.files += lines.js
scripts.files += marquee.js
scripts.files += noise.js
scripts.files += onebyone.js
scripts.files += opposite.js
scripts.files += plasma.js
scripts.files += randomcolumn.js
scripts.files += randomfillcolumn.js
scripts.files += randomfillrow.js
scripts.files += randomfillsingle.js
scripts.files += randompixelperrow.js
scripts.files += randompixelperrowmulticolor.js
scripts.files += randomrow.js
scripts.files += randomsingle.js
scripts.files += sinewave.js
scripts.files += snowbubbles.js
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
