include(../variables.pri)

TEMPLATE = subdirs

docs.path = $$INSTALLROOT/$$DOCSDIR/html
docs.files = \
             addeditfixtures.html \
             addvcbuttonmatrix.html \
             capabilityeditor.html \
             capabilitywizard.html \
             channeleditor.html \
             chasereditor.html \
             collectioneditor.html \
             commandlineparameters.html \
             concept.html \
             efxeditor.html \
             enttecdmxusboutput.html \
             fixturedefinitioneditor.html \
             fixtureeditor.html \
             fixturegroupeditor.html \
             fixturemanager.html \
             fixturemonitor.html \
             functionmanager.html \
             functionwizard.html \
             headeditor.html \
             howto-add-fixtures.html \
             howto-input-devices.html \
             howto-input-mapping.html \
             howto-output-mapping.html \
             index.html \
             modeeditor.html \
             peperonioutput.html \
             rgbmatrixeditor.html \
             rgbscriptapi.html \
             sceneeditor.html \
             selectfunction.html \
             selectfixture.html \
             selectinputchannel.html \
             simpledesk.html \
             tutorial.html \
             tutorial.png \
             udmxoutput.html \
             vcbutton.html \
             vcbuttonmatrix.html \
             vccuelist.html \
             vcframe.html \
             vclabel.html \
             vcsoloframe.html \
             vcslider.html \
             vcslidermatrix.html \
             vcstylingplacement.html \
             vcxypad.html \
             vellemanoutput.html \
             virtualconsole.html
INSTALLS += docs
