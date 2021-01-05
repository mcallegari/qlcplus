unix:!macx:include(../../../variables.pri)

TEMPLATE = subdirs

macx:SUBDIRS       += macx
unix:!macx:SUBDIRS += alsa
win32:SUBDIRS      += win32

TRANSLATIONS += MIDI_de_DE.ts
TRANSLATIONS += MIDI_es_ES.ts
TRANSLATIONS += MIDI_fi_FI.ts
TRANSLATIONS += MIDI_fr_FR.ts
TRANSLATIONS += MIDI_it_IT.ts
TRANSLATIONS += MIDI_nl_NL.ts
TRANSLATIONS += MIDI_cz_CZ.ts
TRANSLATIONS += MIDI_pt_BR.ts
TRANSLATIONS += MIDI_ca_ES.ts
TRANSLATIONS += MIDI_ja_JP.ts

unix:!macx {
   metainfo.path   = $$METAINFODIR
   metainfo.files += org.qlcplus.QLCPlus.midi.metainfo.xml
   INSTALLS       += metainfo
}
