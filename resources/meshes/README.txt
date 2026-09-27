** Notes for the 3D view meshes

The meshes are split in three folders:

- fixtures: the models the 3D view draws the fixtures themselves with, chosen
  automatically from the type of the fixture. See the notes in that folder for
  how to author one.
- stage: the stage furniture. The trusses the Theatre and Rock stages are
  assembled from live here, and so do the curtains.
- generic: the primitive shapes, i.e. a cube, a sphere, a plane and so on.

Everything but the fixture meshes is offered to the user by the "Custom items"
section of the 3D view settings, whose file dialog opens on this folder, so
anything dropped anywhere below it shows up there.

* Tileable meshes

A mesh whose file name ends with _tile_<N>m is a repeating section <N> metres
wide along the X axis, and is handled differently by the 3D view: its X scale
is a section count rather than a stretch factor, so setting the X scale to
400% draws four copies of the section side by side instead of one section
stretched to four times its width. The Y and Z scales still stretch, as usual.

Such a mesh must therefore be modelled so that copies of it abut seamlessly:
its bounding box has to be exactly <N> metres wide, and the geometry along the
two X faces of that box has to match. See stage/curtain_tile_1m.obj, whose fold
profile is periodic and reaches the same extremum on both edges of the
section.
