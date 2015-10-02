include(../../variables.pri)

TEMPLATE = subdirs

docs.path = $$INSTALLROOT/$$DOCSDIR/html
docs.files = *.html

imgs.path = $$INSTALLROOT/$$DOCSDIR/html/images
imgs.files += images/*.png

INSTALLS += docs imgs
