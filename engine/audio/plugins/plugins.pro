TEMPLATE = subdirs
CONFIG  += ordered

!android:!ios {
system(pkg-config --exists mad) {
  SUBDIRS += mad
}

system(pkg-config --exists sndfile) {
  SUBDIRS += sndfile
}
}
