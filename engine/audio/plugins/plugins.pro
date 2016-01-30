TEMPLATE = subdirs
CONFIG  += ordered

system(pkg-config --exists mad) {
  SUBDIRS += mad
}

system(pkg-config --exists sndfile) {
  SUBDIRS += sndfile
}