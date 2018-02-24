include(../../variables.pri)

TEMPLATE = subdirs

docs.path = $$INSTALLROOT/$$DOCSDIR/html
docs.files = html_en_EN/*.html

imgs.path = $$INSTALLROOT/$$DOCSDIR/images
imgs.files += images/*.png

INSTALLS += docs imgs
