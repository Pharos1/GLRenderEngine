#pragma once
#ifndef TEXTURE
#define TEXTURE


#include<iostream>
#include<string>
#include<vector>

#include<GLAD/glad.h>
#include<GLFW/glfw3.h>
#include<SOIL/stb_image.h>

//Todo: maybe make a framebuffer class and make cubemap class and make other classes that inherit from the Texture class and probably shadowMap and rbo

class Texture {
public:
	int width;
	int height;
	GLuint glType;
	int nrChannels;

	std::string type;
	std::string path;
	std::string fileName;

	GLuint id;

	GLuint getID() const { return this->id; }
	void bind(const GLint texture_unit) {
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(this->glType, this->id);
	}
	void unbind() {
		glActiveTexture(0);
		glBindTexture(this->glType, 0);
	}
	void deleteTexture() {
		glDeleteTextures(1, &this->id);
	}
};
class ClassicTexture : public Texture {

public:
	ClassicTexture(std::string path, std::string type = "texture_diffuse", GLenum glType = GL_TEXTURE_2D) {
		//Note: glGenTexture generates n number of texture ids and sends them to the second parameter
		//Note: glActiveTexture sets the texture unit that glBindTexture will bind to(starting from 0)
		//Note: glBindTexture sets the texture id(sec parameter) to the texture unit(from glActiveTexture) if glActive texture wasn't called before it is bind to GL_TEXTURE0 

		this->path = path;
		this->fileName = path.substr(path.find_last_of('/') + 1);
		this->type = type;
		this->glType = glType;

		//stbi_set_flip_vertically_on_load(true); //Note: if flip uvs is on in ModelLoading and this is on then it will mess up the textures

		unsigned char* image = stbi_load(path.c_str(), &this->width, &this->height, &nrChannels, STBI_rgb_alpha);

		glGenTextures(1, &this->id);
		glBindTexture(glType, this->id);


		glTexParameteri(glType, GL_TEXTURE_WRAP_S, GL_REPEAT); //Note: When using transparency its good to use GL_CLAMP_TO_EDGE instead of GL_REPEAT to prevent interpolation of colors on the top of the texture
		glTexParameteri(glType, GL_TEXTURE_WRAP_T, GL_REPEAT); //and here also
		glTexParameteri(glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (image) {
			glTexImage2D(glType, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image); //Note: if your textures are in sRGB you should use GL_SRGB or GL_SRGB_ALPHA for the internalType(the 3rd variable)
			glGenerateMipmap(glType);
		}
		else {
			std::cout << "ERROR::TEXTURE::TEXTURE_LOADING_FAILED: " << path << "\n";
		}

		glActiveTexture(0); //Unbind
		glBindTexture(glType, 0);
		stbi_image_free(image);
	}
	ClassicTexture() {};
};
class CubemapTexture : public Texture {
public:
	CubemapTexture(std::vector<std::string> faces) {
		glGenTextures(1, &this->id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);

		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &this->width, &this->height, &this->nrChannels, 0);
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);
				stbi_image_free(data);
			}
			else {
				std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	CubemapTexture() {};
};
class EquirectangularMap : public Texture{
public:
	EquirectangularMap(std::string path, GLuint glType = GL_TEXTURE_2D) {
		stbi_set_flip_vertically_on_load(true);

		this->glType = glType;

		float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);
		if (data){
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
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
};
#endif