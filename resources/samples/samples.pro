include(../../variables.pri)

TEMPLATE = subdirs
TARGET   = samples

samples.files += *.qxw
samples.path = $$SAMPLESDIR
INSTALLS += samples

