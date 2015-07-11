TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS += src
!android {
  SUBDIRS += test
}
