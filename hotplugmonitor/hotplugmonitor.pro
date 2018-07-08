TEMPLATE = subdirs
CONFIG  += ordered
!android:!ios {
SUBDIRS += src
SUBDIRS += test
}
