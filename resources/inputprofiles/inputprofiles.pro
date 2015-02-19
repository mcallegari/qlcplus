include(../../variables.pri)

TEMPLATE = subdirs
TARGET = profiles

profiles.files += Akai-APC20.qxi
profiles.files += Akai-APC40.qxi
profiles.files += Akai-APCMini.qxi
profiles.files += Behringer-BCF2000.qxi
profiles.files += Behringer-BCF2000inMackieControlMode.qxi
profiles.files += Behringer-LC2412.qxi
profiles.file  += Elation-MIDIcon.qxi
profiles.files += Enttec-PlaybackWing.qxi
profiles.files += Enttec-ShortcutWing.qxi
profiles.files += KORG-nanoKONTROL.qxi
profiles.files += KORG-nanoKONTROL2.qxi
profiles.files += KORG-nanoPAD.qxi
profiles.files += KORG-nanoPAD2.qxi
profiles.files += Logitech-WingManAttack2.qxi
profiles.files += Mixxx-MIDI.qxi
profiles.files += Novation-Launchpad.qxi
profiles.files += Generic-MIDI.qxi
profiles.files += ShowTec-ShowMaster24.qxi
profiles.files += TouchOSC-Mix16.qxi

profiles.path = $$INSTALLROOT/$$INPUTPROFILEDIR
INSTALLS += profiles
