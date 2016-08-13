TEMPLATE = subdirs

#android:    SUBDIRS += android
#ios:        SUBDIRS += ios
unix:!macx: SUBDIRS += linux
macx:       SUBDIRS += macos
win32:      SUBDIRS += windows

