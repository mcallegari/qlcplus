TEMPLATE = subdirs
CONFIG  += ordered
!android {
SUBDIRS += src
SUBDIRS += test
}