include(../../variables.pri)

TEMPLATE = subdirs
TARGET = modtemplates

modtemplates.files += Exponential_Simple.qxmt
modtemplates.files += Exponential_Deep.qxmt
modtemplates.files += Exponential_Medium.qxmt
modtemplates.files += Exponential_Shallow.qxmt
modtemplates.files += Invert.qxmt
modtemplates.files += Linear.qxmt
modtemplates.files += Logarithmic_Deep.qxmt
modtemplates.files += Logarithmic_Medium.qxmt
modtemplates.files += Logarithmic_Shallow.qxmt
modtemplates.files += Threshold.qxmt
modtemplates.files += Preheat_5_Percent.qxmt


modtemplates.path = $$INSTALLROOT/$$MODIFIERSTEMPLATEDIR
INSTALLS += modtemplates
