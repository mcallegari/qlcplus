include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoni

INCLUDEPATH += peperoni
INCLUDEPATH += ../../interfaces
DEPENDPATH  += peperoni
CONFIG      += plugin
QMAKE_LFLAGS += -shared

# Headers
HEADERS += peperoni/usbdmx-dynamic.h \
           peperonidevice.h \
           peperoni.h

# Sources
SOURCES += peperoni/usbdmx-dynamic.cpp \
           peperonidevice.cpp \
           peperoni.cpp

HEADERS += ../../interfaces/qlcioplugin.h

TRANSLATIONS += Peperoni_fi_FI.ts
TRANSLATIONS += Peperoni_de_DE.ts
TRANSLATIONS += Peperoni_es_ES.ts
TRANSLATIONS += Peperoni_fr_FR.ts
TRANSLATIONS += Peperoni_it_IT.ts
TRANSLATIONS += Peperoni_nl_NL.ts

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
