#ifndef MESH
#define MESH

//#include <GLAD/glad.h>
#include <GLAD/gl.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"
#include "Material.h"

#include <string>
#include <vector>

std::vector<glm::vec3> skyboxVerts = { //Note: those are cube verts but the faces are flipped
		glm::vec3(-1.f, -1.f, -1.f), //BACK
		glm::vec3( 1.f, -1.f, -1.f),
		glm::vec3( 1.f,  1.f, -1.f),
		glm::vec3( 1.f,  1.f, -1.f),
		glm::vec3(-1.f,  1.f, -1.f),
		glm::vec3(-1.f, -1.f, -1.f),
								   
		glm::vec3( 1.f,  1.f,  1.f), //FRONT
		glm::vec3( 1.f, -1.f,  1.f),
		glm::vec3(-1.f, -1.f,  1.f),
		glm::vec3(-1.f, -1.f,  1.f),
		glm::vec3(-1.f,  1.f,  1.f),
		glm::vec3( 1.f,  1.f,  1.f),
								   
		glm::vec3(-1.f, -1.f, -1.f), //RIGHT
		glm::vec3(-1.f,  1.f, -1.f),
		glm::vec3(-1.f,  1.f,  1.f),
		glm::vec3(-1.f,  1.f,  1.f),
		glm::vec3(-1.f, -1.f,  1.f),
		glm::vec3(-1.f, -1.f, -1.f),
								   
		glm::vec3( 1.f,  1.f,  1.f), //LEFT
		glm::vec3( 1.f,  1.f, -1.f),
		glm::vec3( 1.f, -1.f, -1.f),
		glm::vec3( 1.f, -1.f, -1.f),
		glm::vec3( 1.f, -1.f,  1.f),
		glm::vec3( 1.f,  1.f,  1.f),
								   
		glm::vec3( 1.f, -1.f,  1.f), //BOTTOM
		glm::vec3( 1.f, -1.f, -1.f),
		glm::vec3(-1.f, -1.f, -1.f),
		glm::vec3(-1.f, -1.f, -1.f),
		glm::vec3(-1.f, -1.f,  1.f),
		glm::vec3( 1.f, -1.f,  1.f),
								   
		glm::vec3(-1.f,  1.f, -1.f), //TOP
		glm::vec3( 1.f,  1.f, -1.f),
		glm::vec3( 1.f,  1.f,  1.f),
		glm::vec3( 1.f,  1.f,  1.f),
		glm::vec3(-1.f,  1.f,  1.f),
		glm::vec3(-1.f,  1.f, -1.f),
	};

class Mesh {
public:
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
	unsigned int VAO, VBO, EBO;

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
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
	}
};
class ClassicMesh : public Mesh {
public:
	ClassicMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices = {}) {
		this->vertices = vertices;
		this->indices = indices;

		setupMesh();
	}
	ClassicMesh() {};

	void Draw(Shader& shader) {
		shader.use();
		glBindVertexArray(VAO);

		if (indices.empty())
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		else
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
};
class MaterialMesh : public Mesh {
public:
	Material material;
	Material* currentMaterial = &material;
	
	MaterialMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices = {}, Material material = Material()) {
		this->vertices = vertices;
		this->indices = indices;
		this->material = material;

		setupMesh();
	}
	MaterialMesh() {};
	MaterialMesh(const MaterialMesh& other) : Mesh(other){ //Copy constructor
		if (currentMaterial == &other.material)
			currentMaterial = &material;
		else
			currentMaterial = other.currentMaterial;
	}

	void Draw(Shader& shader) {
		shader.use();

		currentMaterial->bind(shader);
		glBindVertexArray(VAO);
		
		if (indices.empty())
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		else
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		currentMaterial->unbind();
	}
};
class Skybox { //TODO: EVERYTHING HERE IS MESSED UP AND HAS TO BE FIXED
private:
	unsigned int VAO, VBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, skyboxVerts.size() * 3 * sizeof(float), &skyboxVerts[0], GL_STATIC_DRAW);

		//Vertex attribute pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);
	}
public:
	std::vector<glm::vec3> vertices;
	CubemapTexture texture;

	Skybox(std::vector<std::string> textureFaces) {
		this->vertices = skyboxVerts;

		texture.loadCubemap(textureFaces);

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
class HDRSkybox{
public:
	HDRMap texture;
	HDRMap* texturePtr = &texture;
	unsigned int VAO, VBO;

	HDRSkybox() {};
	HDRSkybox(const HDRSkybox& other) {
		if (texturePtr == &other.texture)
			texturePtr = &texture;
		else
			texturePtr = other.texturePtr;

		VAO = other.VAO;
		VBO = other.VBO;
	}

	void setup() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, skyboxVerts.size() * 3 * sizeof(float), &skyboxVerts[0], GL_STATIC_DRAW);

		//Vertex attribute pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);
	}
	void Draw(Shader& shader) {
		glDepthFunc(GL_LEQUAL);

		shader.use();

		texturePtr->bind(0);
		glBindVertexArray(VAO);
		
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		texturePtr->unbind(0);

		glDepthFunc(GL_LESS);
	}
};
class RenderQuad : public Mesh{
public:
	RenderQuad(std::vector<Vertex> vertices, std::vector<unsigned int> indices = {}) {
		this->vertices = vertices;
		this->indices = indices;

		setupMesh();
	}
	RenderQuad() {};

	void Draw(Shader& shader, std::vector<unsigned int> textureIDs, int offset = 0) {
		// draw mesh
		shader.use();

		for(int i = 0; i < textureIDs.size(); i++){
			glActiveTexture(GL_TEXTURE0 + i + offset);
			glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
		}

		glDepthFunc(GL_ALWAYS);
		glBindVertexArray(VAO);

		if (indices.empty())
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		else
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		glDepthFunc(GL_LESS);
		glBindVertexArray(0);
	}
};
#endif
