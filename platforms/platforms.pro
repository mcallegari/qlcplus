TEMPLATE = subdirs

#android:    SUBDIRS += android
#ios:        SUBDIRS += ios
unix:!macx:!android: SUBDIRS += linux
macx:       SUBDIRS += macos
win32:      SUBDIRS += windows

