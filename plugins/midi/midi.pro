TEMPLATE = subdirs

macx:SUBDIRS       += macx
unix:!macx:SUBDIRS += alsa
win32:SUBDIRS      += win32
