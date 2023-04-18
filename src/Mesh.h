#ifndef MESH
#define MESH

#include <GLAD/glad.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"

#include <string>
#include <vector>
using namespace std;

std::vector<Vertex> skyboxVerts = {
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)), //BACK
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)),

        Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)), //FRONT
        Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)),

        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)), //RIGHT
        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),

        Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)), //LEFT
        Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),

        Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)), //BOTTOM
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),

        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)), //TOP
        Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.f),  glm::vec2(0.0f, 1.0f)),
    };

//Todo: specular textures stay forever in the frag shader so i should learn more and maybe zero its index at the end of the draw call.
class Mesh {
public:
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;
    /*
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }
    */
    /*
    void Draw(Shader& shader){
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++){
            string number;
            string name = textures[i].type;


            if (name == "texture_diffuse") number = std::to_string(diffuseNr++);
            else if (name == "texture_specular") number = std::to_string(specularNr++);

            //std::cout << ("material." + name + number).c_str();

            shader.set1i(("material." + name + number).c_str(), i);
            //std::cout << number;
            //glUniform1i(glGetUniformLocation(shader.getID(), ("material." + name + number).c_str()), i);
            //std::cout << textures.size() << std::endl;
            textures[i].bind(i);
        }

        // draw mesh
        glBindVertexArray(VAO);
        //std::cout << indices.size() << std::endl;
        if (indices.size() != 0) {
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    */
    unsigned int VBO, EBO;
    void setupMesh(){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        //Vertex attribute pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(3);

        //glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        //glEnableVertexAttribArray(4);
    }
};

class ClassicMesh : public Mesh {
public:
    ClassicMesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        for (int i = 0; i < textures.size(); i++) {
            std::cout << textures[i].path;
            std::cout << " :: ";
            std::cout << textures[i].getID();
            std::cout << " :: ";
            std::cout << textures[i].type << std::endl;
        }

        setupMesh();
    }
    ClassicMesh() {};

    void Draw(Shader& shader, bool enabled = true) {
        if (!enabled) return;
        shader.use();
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            string number;
            string name = textures[i].type;


            if (name == "texture_diffuse") number = std::to_string(diffuseNr++);
            else if (name == "texture_specular") number = std::to_string(specularNr++);
            else number = "1";
            //std::cout << ("material." + name + number).c_str() << std::endl;

            shader.set1i(("material." + name + number).c_str(), i);
            //std::cout << number;
            //glUniform1i(glGetUniformLocation(shader.getID(), ("material." + name + number).c_str()), i);
            //std::cout << textures.size() << std::endl;
            textures[i].bind(i);
        }

        // draw mesh
        glBindVertexArray(VAO);
        //std::cout << indices.size() << std::endl;
        if (indices.size() != 0) {
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
};
class Skybox : public Mesh {
public:
    CubemapTexture texture;

    Skybox(std::vector<std::string> textureFaces) {
        this->vertices = skyboxVerts;

        texture = CubemapTexture(textureFaces);

        setupMesh();
    }
    Skybox() {};

    void Draw(Shader& shader, glm::mat4 view, glm::mat4 proj) { //Todo: its a bit of a loss to set proj everytime so it will be better to be set only on setup and make a function for setting it
        //textures
        unsigned int thisID = texture.getID();

        texture.bind(thisID);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glBindTexture(GL_TEXTURE_CUBE_MAP, thisID);

        shader.use();
        shader.set1i("skybox", thisID);

        shader.setMat4("view", glm::mat4(glm::mat3(view)));
        shader.setMat4("proj", proj);



        // draw mesh
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);

        //Default the options
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
};
class RenderQuad : public Mesh{
public:
    RenderQuad(vector<Vertex> vertices, vector<unsigned int> indices) {
        this->vertices = vertices;
        this->indices = indices;

        setupMesh();
    }
    RenderQuad() {};

    void Draw(Shader& shader, vector<unsigned int> textureIDs) {
        // draw mesh
        shader.use();
        shader.set1b("renderQuad", true);

        for(int i = 0; i < textureIDs.size(); i++){
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        }

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(VAO);

        if (indices.size() != 0) {
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);

        //Default options
        glEnable(GL_DEPTH_TEST);
        shader.set1b("renderQuad", false);
    }
};
#endif
