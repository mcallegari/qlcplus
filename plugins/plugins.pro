TEMPLATE = subdirs

SUBDIRS              += enttecdmxusb
SUBDIRS              += peperoni
SUBDIRS              += udmx
SUBDIRS              += midi
unix:SUBDIRS         += ola
!macx:!win32:SUBDIRS += dmx4linux
SUBDIRS              += velleman
SUBDIRS              += enttecwing
!macx:!win32:SUBDIRS += hid
SUBDIRS              += osc
SUBDIRS              += artnet
