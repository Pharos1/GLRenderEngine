#ifndef MODEL
#define MODEL

//#include <glad/glad.h> 
#include <GLAD/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL2/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"
#include "Material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;
//Todo: Some files might not work because they are made with materials and not textures so I have to make the model class extract material information from the mtl file and use them in our application.

class Model
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Texture*> loadedTextures;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<MaterialMesh>    meshes;
    string directory;

    // constructor, expects a filepath to a 3D model.
    Model(string const& path){
        loadModel(path);

        //for (int i = 0; i < textures_loaded.size(); i++) {
        //    std::cout << (this->directory + '/' + std::string(textures_loaded[i].path)).c_str();
        //    std::cout << " :: ";
        //    std::cout << textures_loaded[i].fileName;
        //    std::cout << " :: ";
        //    std::cout << textures_loaded[i].getID();
        //    std::cout << " :: ";
        //    std::cout << textures_loaded[i].type << std::endl;
        //}
    }
    ~Model() { //Delete all textures at the end
        for (int i = 0; i < textures_loaded.size(); i++) {
            textures_loaded[i].deleteTexture();
        }

        for (int i = 0; i < loadedTextures.size(); i++) {
            loadedTextures[i]->deleteTexture();
        }
    }
    Model() {};

    // draws the model, and thus all its meshes
    void Draw(Shader& shader){
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path){
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph); //aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){ // if is Not Zero
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene){
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++){
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene, node));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++){
            processNode(node->mChildren[i], scene);
        }

    }

    MaterialMesh processMesh(aiMesh* mesh, const aiScene* scene, aiNode* node){
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        //vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++){
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // normals
            if (mesh->HasNormals()){
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]){ // does the mesh contain texture coordinates?{
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                //vector.x = mesh->mBitangents[i].x;
                //vector.y = mesh->mBitangents[i].y;
                //vector.z = mesh->mBitangents[i].z;
                //vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++){
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        MaterialMesh finalMesh(vertices, indices);
        //Transformation matrix
        
        //finalMesh.transformation =glm::mat4(node->mTransformation.a1, node->mTransformation.a2, node->mTransformation.a3, node->mTransformation.a4,
        //                                    node->mTransformation.b1, node->mTransformation.b2, node->mTransformation.b3, node->mTransformation.b4,
        //                                    node->mTransformation.c1, node->mTransformation.c2, node->mTransformation.c3, node->mTransformation.c4,
        //                                    node->mTransformation.d1, node->mTransformation.d2, node->mTransformation.d3, node->mTransformation.d4);
        //std::cout << node->mTransformation.a1 << node->mTransformation.a2 << node->mTransformation.a3 << node->mTransformation.a4 << std::endl
        //    << node->mTransformation.b1 << node->mTransformation.b2 << node->mTransformation.b3 << node->mTransformation.b4 << std::endl
        //    << node->mTransformation.c1 << node->mTransformation.c2 << node->mTransformation.c3 << node->mTransformation.c4 << std::endl
        //    << node->mTransformation.d1 << node->mTransformation.d2 << node->mTransformation.d3 << node->mTransformation.d4 << std::endl << "::" << std::endl;

        
        // process materials
        if (scene->HasMaterials()) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
            // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
            // Same applies to other texture as the following list summarizes:
            // diffuse: texture_diffuseN
            // specular: texture_specularN

            // 1. diffuse maps
            //vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            //textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            //// 2. specular maps
            //vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            //textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            //
            //vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
            //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            Material meshMaterial;

            loadMaterial(&meshMaterial.albedo, material, aiTextureType_DIFFUSE);
            loadMaterial(&meshMaterial.normal, material, aiTextureType_HEIGHT);
            loadMaterial(&meshMaterial.metallic, material, aiTextureType_METALNESS);
            loadMaterial(&meshMaterial.roughness, material, aiTextureType_DIFFUSE_ROUGHNESS);
            loadMaterial(&meshMaterial.AO, material, aiTextureType_LIGHTMAP);

            meshMaterial.initialized = true;

            finalMesh.material = meshMaterial;
        }
        return finalMesh;
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName){
        vector<Texture> textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++){
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++){
                if (std::strcmp(textures_loaded[j].fileName.data(), str.C_Str()) == 0){
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip){   // if texture hasn't been loaded already, load it
                Texture texture((this->directory + '/' + std::string(str.C_Str())).c_str()); //For the second parameter I can use just str.C_Str() but I have to test

                //texture.TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                //texture.fileName = str.C_Str(); //Setting it from the texture class
                
                textures.push_back(texture);

                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }
    void loadMaterial(Texture* materialTexture, aiMaterial* material, aiTextureType type) {
        aiString texturePath;
        material->GetTexture(type, 0, &texturePath);

        std::string path = this->directory + "/" + texturePath.C_Str();

        bool skip = false;

        for (int i = 0; i < textures_loaded.size(); i++) {
            if (textures_loaded[i].path == path) {
                skip = true;

                *materialTexture = textures_loaded[i];

                break;
            }
        }
        if (!skip && std::strcmp(texturePath.C_Str(), "") != 0) {
            *materialTexture = Texture(path);
            textures_loaded.push_back(*materialTexture);
        }
    }
};
#endif
