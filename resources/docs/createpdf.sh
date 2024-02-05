#
# QLC+ documentation PDF creation through wkhtmltopdf utility downloaded from
# https://wkhtmltopdf.org/downloads.html
#

set -e

WKHTMLTOPDF=$(which wkhtmltopdf)

test -d "$1"
cd "$1"
test -f "index_pdf.html"

$WKHTMLTOPDF \
  --footer-center "Page [page]" \
  --image-quality 100 \
  --enable-external-links \
  --enable-javascript \
  --javascript-delay 1000 \
  --enable-local-file-access \
  --allow . \
  pdf_cover.html \
  index_pdf.html \
  concept.html \
  questionsandanswers.html \
  mainwindow.html \
  fixturemonitor.html \
  addresstool.html \
  dmxdump.html \
  liveedit.html \
  fixturemanager.html \
  addeditfixtures.html \
  addrgbpanel.html \
  fixturegroupeditor.html \
  channelsgroupeditor.html \
  channelproperties.html \
  fixturesremap.html \
  howto-add-fixtures.html \
  functionmanager.html \
  sceneeditor.html \
  chasereditor.html \
  showeditor.html \
  efxeditor.html \
  collectioneditor.html \
  rgbmatrixeditor.html \
  rgbscriptapi.html \
  scripteditor.html \
  audioeditor.html \
  videoeditor.html \
  functionwizard.html \
  selectfunction.html \
  selectfixture.html \
  showmanager.html \
  virtualconsole.html \
  vcframe.html \
  vcsoloframe.html \
  vcbutton.html \
  vcbuttonmatrix.html \
  vcslider.html \
  vcslidermatrix.html \
  vcmatrix.html \
  vcspeeddial.html \
  vccuelist.html \
  vcxypad.html \
  vclabel.html \
  audiotriggers.html \
  selectinputchannel.html \
  vcstylingplacement.html \
  simpledesk.html \
  howto-input-output-mapping.html \
  howto-input-profiles.html \
  audio-input-output.html \
  supported-input-devices.html \
  artnetplugin.html \
  dmxusbplugin.html \
  e131plugin.html \
  hidplugin.html \
  midiplugin.html \
  olaplugin.html \
  os2lplugin.html \
  oscplugin.html \
  peperonioutput.html \
  udmxoutput.html \
  vellemanoutput.html \
  loopbackplugin.html \
  fixturedefinitioneditor.html \
  modeeditor.html \
  tutorial.html \
  tutorial-multipage.html \
  tutorial-soundcontrol.html \
  tutorial-bcf-lc2412.html \
  commandlineparameters.html \
  kioskmode.html \
  webinterface.html \
  disable_apple_ftdi_driver.html \
  parameterstuning.html \
  guicustomstyles.html \
  QLC+.pdf

cd -
mv $1/QLC+.pdf QLC+_manual_$1.pdf
