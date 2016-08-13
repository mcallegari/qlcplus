include(../../variables.pri)

TEMPLATE = subdirs
TARGET   = icons

desktop.path   = $$INSTALLROOT/share/applications/
desktop.files += qlcplus.desktop qlcplus-fixtureeditor.desktop
INSTALLS      += desktop

icons.path   = $$INSTALLROOT/share/pixmaps/
icons.files += ../../resources/icons/png/qlcplus.png ../../resources/icons/png/qlcplus-fixtureeditor.png
INSTALLS    += icons

mime.path   = $$INSTALLROOT/share/mime/packages
mime.files += qlcplus.xml
INSTALLS   += mime

appdata.path   = $$INSTALLROOT/share/appdata/
appdata.files += qlcplus-fixtureeditor.appdata.xml qlcplus.appdata.xml
INSTALLS      += appdata

manpages.path = $$INSTALLROOT/$$MANDIR
manpages.files += *.1
INSTALLS += manpages

# This is nowadays run by dpkg (TODO: rpm)
#MIMEUPDATE    = $$system("which update-mime-database")
#mimeupdate.commands = $$MIMEUPDATE /usr/share/mime
#mimeupdate.path = /usr/share/mime
#INSTALLS += mimeupdate

samples.files += ../Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples
