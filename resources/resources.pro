TEMPLATE = subdirs

!qmlui: SUBDIRS += docs
SUBDIRS += fixtures
SUBDIRS += gobos
SUBDIRS += inputprofiles
SUBDIRS += miditemplates
SUBDIRS += modifierstemplates
SUBDIRS += rgbscripts
SUBDIRS += samples
qmlui {
  SUBDIRS += colorfilters
  SUBDIRS += meshes
}

