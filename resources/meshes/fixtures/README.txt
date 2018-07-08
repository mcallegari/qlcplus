** Notes for fixture meshes creation 

If you use Blender to create meshes, just keep in mind:
- Blender works with a Z-Up coordinate system, so you need to rotate the model in a way that Y is the up direction. Once the model is saved in DAE format, change the <up_axis> tag to be Y_UP
- Blender tends to accumulate transformations. When you're done with modeling, select each mesh of the model, hit CTRL-A and reset one by one rotation (must be 0,0,0) and scale (must be 1,1,1)
