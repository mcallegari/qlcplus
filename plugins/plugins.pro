TEMPLATE = subdirs

!android:!ios {

 SUBDIRS              += dmxusb
 SUBDIRS              += peperoni
 SUBDIRS              += udmx
 SUBDIRS              += midi
 unix {
   packagesExist(libola libolaserver) {
       SUBDIRS        += ola
   }
 }
 !macx:!win32:SUBDIRS += dmx4linux
 SUBDIRS              += velleman
 SUBDIRS              += enttecwing
 SUBDIRS              += hid
 !macx:!win32:SUBDIRS += spi

 greaterThan(QT_MAJOR_VERSION, 4) {
    #!macx:!win32:SUBDIRS += uart
    #!macx:!win32:SUBDIRS += gpio
 }
}

SUBDIRS              += artnet
SUBDIRS              += E1.31
SUBDIRS              += loopback
SUBDIRS              += osc
