TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS += audio
SUBDIRS += src
!android:!ios {
  SUBDIRS += test
}
