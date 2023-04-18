//Sooner
// probably make the fuking mesh have normal maps too and do everything needed in the shaders
//In mesh.h i should make texture type checking so i dont mess something up in the mesh object construction
// Make if normal map has something in it then use tbn but otherwise, don't
// Later on
// Make better hdr and use better tone mapping techniques
//In texture class make it so i can list options with the "|" operator and learn how it works for listing like in post proccess options for assimp
//maybe implements shadows later on as now I will skip that part cuz they are not that important.
//make a modelMat in the mesh class and when drawing just send it to the shader and then draw. If no matrix is assigned to the mesh then just skip this step
//make a texture class for cubemaps, RBOs, frabuffers, shadowMap and others
//Make missing textures appear with pink and white squares
//Make textures not to strech but to start repeating if the (for example, plane) is too long
// At the end
//do something with the tangent space calculations to increase performance cuz now its damn slow(IM SURE)
//Fix the fuking performance issues as there is a lot to improve. Especialy in the shaders.
//To be able to use screen resizing i would need to update some things when the size changes(e.g. gBuffer, quadRenderBuffer)

//Make bit flags for HDREnabled, Motion blur enabled, Bloom enabled and such.
//Make a big shader for post processing and feed it with the bit flags from the upper line

//To learn:
//I have to learn the fucking frame and render buffers cuz i hate them and i dont know them well
//Also the fuking way the gpu works (Link: http://people.maths.ox.ac.uk/~gilesm/old/pp10/lec2_2x2.pdf)

//WHY NOT
//point light shadows just because i hate the miserable dir light shadow map, also make it able to be disabled from the gui

//Good papers to implement
//https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf