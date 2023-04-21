/*
Sooner
 probably make the fuking mesh have normal maps too and do everything needed in the shaders
In mesh.h i should make texture type checking so i dont mess something up in the mesh object construction
 Make if normal map has something in it then use tbn but otherwise, don't
 Later on
 Make better hdr and use better tone mapping techniques
In texture class make it so i can list options with the "|" operator and learn how it works for listing like in post proccess options for assimp
make a modelMat in the mesh class and when drawing just send it to the shader and then draw. If no matrix is assigned to the mesh then just skip this step
Make missing textures appear with pink and white squares
Make textures not to strech but to start repeating if the (for example, plane) is too long
Make Material class for the textures and make a function for setting to shader and edit mesh and model classes to work with that mat class

 At the end
do something with the tangent space calculations to increase performance cuz now its damn slow(IM SURE)
Fix the fuking performance issues as there is a lot to improve. Especialy in the shaders.
To be able to use screen resizing i would need to update some things when the size changes(e.g. gBuffer, quadRenderBuffer)
Make main.cpp more clear like making a func for glInit and such
Check does the irradiance work with the pbr

To learn:
the fuking way the gpu works (Link: http://people.maths.ox.ac.uk/~gilesm/old/pp10/lec2_2x2.pdf)
learn how the pbr actually works, and especially the IBL part.

Good papers to implement:
https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf

Issues:
Model.h is not updated to work with pbr
Resizing fucks up with the screen rendering and it gets streched

Gamebreaking issues:
In mesh.h skybox class, draw func i have everything messed up with ids and textures.
*/
