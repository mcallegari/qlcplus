include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = uart

QT += widgets
QT += serialport

INCLUDEPATH += ../interfaces
CONFIG      += plugin

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += UART_de_DE.ts
TRANSLATIONS += UART_es_ES.ts
TRANSLATIONS += UART_fi_FI.ts
TRANSLATIONS += UART_fr_FR.ts
TRANSLATIONS += UART_it_IT.ts
TRANSLATIONS += UART_nl_NL.ts
TRANSLATIONS += UART_cz_CZ.ts
TRANSLATIONS += UART_pt_BR.ts
TRANSLATIONS += UART_ca_ES.ts
TRANSLATIONS += UART_ja_JP.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += uartplugin.h \
           uartwidget.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += uartplugin.cpp \
           uartwidget.cpp

