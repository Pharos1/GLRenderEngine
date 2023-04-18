//#ifndef SHADER_H
//#define SHADER_H
//
//#include <glad/glad.h>
//
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <iostream>
//
//class Shader
//{
//public:
//    unsigned int ID;
//    // constructor generates the shader on the fly
//    // ------------------------------------------------------------------------
//    Shader(const char* vertexPath, const char* fragmentPath)
//    {
//        // 1. retrieve the vertex/fragment source code from filePath
//        std::string vertexCode;
//        std::string fragmentCode;
//        std::ifstream vShaderFile;
//        std::ifstream fShaderFile;
//        // ensure ifstream objects can throw exceptions:
//        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
//        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
//        try 
//        {
//            // open files
//            vShaderFile.open(vertexPath);
//            fShaderFile.open(fragmentPath);
//            std::stringstream vShaderStream, fShaderStream;
//            // read file's buffer contents into streams
//            vShaderStream << vShaderFile.rdbuf();
//            fShaderStream << fShaderFile.rdbuf();
//            // close file handlers
//            vShaderFile.close();
//            fShaderFile.close();
//            // convert stream into string
//            vertexCode   = vShaderStream.str();
//            fragmentCode = fShaderStream.str();
//        }
//        catch (std::ifstream::failure& e)
//        {
//            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
//        }
//        const char* vShaderCode = vertexCode.c_str();
//        const char * fShaderCode = fragmentCode.c_str();
//        // 2. compile shaders
//        unsigned int vertex, fragment;
//        // vertex shader
//        vertex = glCreateShader(GL_VERTEX_SHADER);
//        glShaderSource(vertex, 1, &vShaderCode, NULL);
//        glCompileShader(vertex);
//        checkCompileErrors(vertex, "VERTEX");
//        // fragment Shader
//        fragment = glCreateShader(GL_FRAGMENT_SHADER);
//        glShaderSource(fragment, 1, &fShaderCode, NULL);
//        glCompileShader(fragment);
//        checkCompileErrors(fragment, "FRAGMENT");
//        // shader Program
//        ID = glCreateProgram();
//        glAttachShader(ID, vertex);
//        glAttachShader(ID, fragment);
//        glLinkProgram(ID);
//        checkCompileErrors(ID, "PROGRAM");
//        // delete the shaders as they're linked into our program now and no longer necessary
//        glDeleteShader(vertex);
//        glDeleteShader(fragment);
//    }
//    // activate the shader
//    // ------------------------------------------------------------------------
//    void use() 
//    { 
//        glUseProgram(ID); 
//    }
//    // utility uniform functions
//    // ------------------------------------------------------------------------
//    void setBool(const std::string &name, bool value) const
//    {         
//        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
//    }
//    // ------------------------------------------------------------------------
//    void setInt(const std::string &name, int value) const
//    { 
//        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
//    }
//    // ------------------------------------------------------------------------
//    void setFloat(const std::string &name, float value) const
//    { 
//        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
//    }
//
//private:
//    // utility function for checking shader compilation/linking errors.
//    // ------------------------------------------------------------------------
//    void checkCompileErrors(unsigned int shader, std::string type)
//    {
//        int success;
//        char infoLog[1024];
//        if (type != "PROGRAM")
//        {
//            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//            if (!success)
//            {
//                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
//                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//            }
//        }
//        else
//        {
//            glGetProgramiv(shader, GL_LINK_STATUS, &success);
//            if (!success)
//            {
//                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
//                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//            }
//        }
//    }
//};
//#endif

#pragma once

#ifndef SHADER
#define SHADER

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<GLM/glm.hpp>

class Shader {
private:
	unsigned int ID;

	void checkCompileErrors(GLuint shader, std::string type) {
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << std::endl;
			}
		}
	}
public:
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
		std::string vertexContent;
		std::string fragmentContent;
		std::string geometryContent;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);

			std::stringstream vShaderStream, fShaderStream;

			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			vertexContent = vShaderStream.str();
			fragmentContent = fShaderStream.str();
			if (geometryPath != nullptr)
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryContent = gShaderStream.str();
			}
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}

		//Define shaders
		const char* vShaderContent = vertexContent.c_str();
		const char* fShaderContent = fragmentContent.c_str();
		const char* gShaderContent = geometryContent.c_str();

		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		GLuint geometry = glCreateShader(GL_GEOMETRY_SHADER);
		int success;
		char infoLog[512];

		//Vertex Shader
		glShaderSource(vertex, 1, &vShaderContent, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		//Fragment Shader
		glShaderSource(fragment, 1, &fShaderContent, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");

		if (geometryPath != nullptr) {
			const char* gShaderCode = geometryContent.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY");
		}


		//Define shader program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		if (geometryPath != nullptr) glAttachShader(ID, geometry);
		glLinkProgram(ID);

		//Check for errors
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << std::endl << infoLog << std::endl;
		}

		//Clear the shader after they are linked
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		glDeleteShader(geometry);
	};
	Shader() {};
	int getID() { return ID; }
	void use() { glUseProgram(ID); };
	void unuse() { glDeleteProgram(ID); };
	void set1b(const std::string& name, bool value)  const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); };
	void set1i(const std::string& name, int value)   const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); };
	void set1ui(const std::string& name, unsigned int value)   const { glUniform1ui(glGetUniformLocation(ID, name.c_str()), (int)value); };
	void set1f(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), (float)value); };
	void set2f(const std::string& name, float x, float y) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); }
	void set3f(const std::string& name, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); }
	void set4f(const std::string& name, float x, float y, float z, float w) const { glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); }
	void setMat4fv(const std::string& name, GLfloat* mat4) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat4); }
	void setMat4(const std::string& name, glm::mat4 mat4) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4)); }
	void setVec2(const std::string& name, glm::vec2 vec2) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), vec2.x, vec2.y); }
	void setVec3(const std::string& name, glm::vec3 vec3) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), vec3.x, vec3.y, vec3.z); }
	void setVec4(const std::string& name, glm::vec4 vec4) const { glUniform4f(glGetUniformLocation(ID, name.c_str()), vec4.x, vec4.y, vec4.z, vec4.w); }
};

#endif
