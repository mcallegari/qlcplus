include(../../variables.pri)

TEMPLATE = subdirs

docs.path = $$INSTALLROOT/$$DOCSDIR/html
docs.files = \
             addeditfixtures.html \
             addresstool.html \
             addrgbpanel.html \
             addvcbuttonmatrix.html \
             artnetplugin.html \
             audiotriggers.html \
             audio-input-output.html \
             capabilityeditor.html \
             capabilitywizard.html \
             channeleditor.html \
             channelsgroupeditor.html \
             channelproperties.html \
             chasereditor.html \
             collectioneditor.html \
             commandlineparameters.html \
             concept.html \
             dmxdump.html \
             dmxusbplugin.html \
             e131plugin.html \
             efxeditor.html \
             fixturedefinitioneditor.html \
             fixtureeditor.html \
             fixturegroupeditor.html \
             fixturemanager.html \
             fixturemonitor.html \
             fixturesremap.html \
             functionmanager.html \
             functionwizard.html \
             guicustomstyles.html \
             headeditor.html \
             hidplugin.html \
             howto-add-fixtures.html \
             howto-input-profiles.html \
             howto-input-output-mapping.html \
             index.html \
             kioskmode.html \
             liveedit.html \
             loopbackplugin.html \
             mainwindow.html \
             midiplugin.html \
             modeeditor.html \
             olaplugin.html \
             oscplugin.html \
             parameterstuning.html \
             peperonioutput.html \
             questionsandanswers.html \
             rgbmatrixeditor.html \
             rgbscriptapi.html \
             sceneeditor.html \
             selectfunction.html \
             selectfixture.html \
             selectinputchannel.html \
             scripteditor.html \
             showeditor.html \
             showmanager.html \
             simpledesk.html \
             tutorial.html \
             tutorial-bcf-lc2412.html \
             tutorial-multipage.html \
             tutorial-soundcontrol.html \
             udmxoutput.html \
             vcbutton.html \
             vcbuttonmatrix.html \
             vccuelist.html \
             vcframe.html \
             vclabel.html \
             vcmatrix.html \
             vcsoloframe.html \
             vcspeeddial.html \
             vcslider.html \
             vcslidermatrix.html \
             vcstylingplacement.html \
             vcxypad.html \
             vellemanoutput.html \
             virtualconsole.html \
             webinterface.html
             
imgs.path = $$INSTALLROOT/$$DOCSDIR/html/images
imgs.files += images/channelmodifier.png \
              images/efx-general.png \
              images/efx-movement.png \
              images/fixremap.png \
              images/locale.png \
              images/mainwindow.png \
              images/tutorial1_1.png \
              images/tutorial1_2.png \
              images/tutorial1_3.png \
              images/multipage1.png \
              images/multipage2.png \
              images/multipage3.png \
              images/multipage4.png \
              images/pan-tilt.png

INSTALLS += docs imgs
