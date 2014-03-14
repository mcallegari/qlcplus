TEMPLATE = subdirs

SUBDIRS              += dmxusb
SUBDIRS              += peperoni
SUBDIRS              += udmx
SUBDIRS              += midi
unix:SUBDIRS         += ola
!macx:!win32:SUBDIRS += dmx4linux
SUBDIRS              += velleman
SUBDIRS              += enttecwing
!macx:SUBDIRS        += hid
SUBDIRS              += osc
SUBDIRS              += artnet
SUBDIRS              += E1.31
!macx:!win32:SUBDIRS += spi
