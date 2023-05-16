#pragma once
#ifndef TEXTURE
#define TEXTURE


#include<iostream>
#include<string>
#include<vector>

//#include<GLAD/glad.h>
#include <GLAD/gl.h>
#include <GLFW/glfw3.h>
#include <SOIL2/stb_image.h>
#include <SOIL2/SOIL2.h>

//Todo: maybe make a framebuffer class and make cubemap class and make other classes that inherit from the Texture class and probably shadowMap and rbo

class Texture {
public:
	int width;
	int height;
	GLuint glType;
	int nrChannels;

	std::string type;
	std::string path = "";

	GLuint id = 0;

	GLuint getID() const { return this->id; }
	void bind(const GLint texture_unit) {
		if (!id) {
			std::cout << "ERROR::TEXTURE.H::TEXTURE NOT INITIALIZED" << std::endl;
			return;
		}

		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(this->glType, this->id);
	}
	void unbind(const GLint texture_unit) {
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(this->glType, 0);
	}
	void deleteTexture() {
		glDeleteTextures(1, &this->id);
	}
	void loadTexture(std::string path, bool invertY = false, std::string type = "texture_diffuse", GLenum glType = GL_TEXTURE_2D) {
		//Note: glGenTexture generates n number of texture ids and sends them to the second parameter
		//Note: glActiveTexture sets the texture unit that glBindTexture will bind to(starting from 0)
		//Note: glBindTexture sets the texture id(sec parameter) to the texture unit(from glActiveTexture) if glActive texture wasn't called before it is bind to GL_TEXTURE0 
		if (path == "") return;

		this->path = path;
		this->type = type;
		this->glType = glType;

		if (!this->id) glGenTextures(1, &id);
		std::cout << id << std::endl;
		this->id = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGBA, this->id, invertY ? SOIL_FLAG_INVERT_Y : 0);

		if (this->id) {
			glTexParameteri(glType, GL_TEXTURE_WRAP_S, GL_REPEAT); //Note: When using transparency its good to use GL_CLAMP_TO_EDGE instead of GL_REPEAT to prevent interpolation of colors on the top of the texture
			glTexParameteri(glType, GL_TEXTURE_WRAP_T, GL_REPEAT); //and here also
			glTexParameteri(glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glGenerateMipmap(glType);
		}
		else
			std::cout << "ERROR::SOIL LAST RESULT: '" << SOIL_last_result() << "' while loading: " << path << std::endl;

		glActiveTexture(GL_TEXTURE0); //Unbind
		glBindTexture(glType, 0);
	}

	Texture(std::string path, bool invertY = false, std::string type = "texture_diffuse", GLenum glType = GL_TEXTURE_2D) {
		loadTexture(path, invertY, type, glType);
	}
	Texture() {};
	~Texture() {
		if (!id) return;

		std::cout << "TEXTURE::DELETED::PATH: " << this->path << " :ID'" << this->id << "'" << std::endl;
		this->deleteTexture();
	}
};
class CubemapTexture : public Texture {
public:
	CubemapTexture(std::vector<std::string> faces) {
		if(!this->id) glGenTextures(1, &this->id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);

		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &this->width, &this->height, &this->nrChannels, 0);
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);
			}
			else {
				std::cout << "ERROR::TEXTURE.H::CUBEMAP FACE FAILED TO LOAD: " << faces[i] << std::endl;
			}
			stbi_image_free(data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	CubemapTexture() {};
};
class HDRMap : public Texture{
public:
	void loadHDRMap(std::string path, GLuint glType = GL_TEXTURE_2D) {
		stbi_set_flip_vertically_on_load(true);

		this->glType = glType;

		if(!id) glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
			std::cout << "Failed to load HDR image." << std::endl;
	}
	HDRMap(std::string path, GLuint glType = GL_TEXTURE_2D) {
		loadHDRMap(path, glType);
	}
	HDRMap() {};
};
#endif