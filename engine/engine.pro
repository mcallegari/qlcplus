TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS += src
!android:!ios {
  SUBDIRS += test
}
