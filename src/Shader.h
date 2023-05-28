#ifndef SHADER
#define SHADER

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<GLM/glm.hpp>

//Note: Shaders remove inactive uniforms i.e. uniforms that don't contribute to the final result.
class Shader {
private:
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
	unsigned int ID = 0;
	void loadShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
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

			if (geometryPath != nullptr){
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
		GLuint geometry = (geometryPath != nullptr) ? glCreateShader(GL_GEOMETRY_SHADER) : 0;
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
		if(!ID) ID = glCreateProgram(); //If it doesn't have an ID just give it

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
		glDetachShader(ID, vertex);
		glDetachShader(ID, fragment);
		if (geometryPath != nullptr) glDetachShader(ID, geometry);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr) glDeleteShader(geometry);
	}
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
		loadShader(vertexPath, fragmentPath, geometryPath);
	};
	Shader() {};
	~Shader() { deleteProgram(); };

	int getID() { return ID; };
	void use() { glUseProgram(ID); };
	void unuse() { glUseProgram(0); };
	void deleteProgram() { glDeleteProgram(ID); };

	void set1b(const std::string& name, bool value) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniform1i(location, (int)value);
		else
			return;// std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void set1i(const std::string& name, int value) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniform1i(location, (int)value);
		else
			return; //std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void set1ui(const std::string& name, unsigned int value) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniform1ui(location, (int)value);
		else
			return; //std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void set1f(const std::string& name, float value) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniform1f(location, (float)value);
		else
			return;// std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void setMat3(const std::string& name, glm::mat3 mat3) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat3));
		else
			return; //std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void setMat4(const std::string& name, glm::mat4 mat4) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat4));
		else
			return; //std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void setVec2(const std::string& name, glm::vec2 vec2) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniform2f(location, vec2.x, vec2.y);
		else
			return; //std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void setVec3(const std::string& name, glm::vec3 vec3) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniform3f(glGetUniformLocation(ID, name.c_str()), vec3.x, vec3.y, vec3.z);
		else
			return; //std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
	void setVec4(const std::string& name, glm::vec4 vec4) {
		int location = glGetUniformLocation(ID, name.c_str());

		if (location != -1)
			glUniform4f(glGetUniformLocation(ID, name.c_str()), vec4.x, vec4.y, vec4.z, vec4.w);
		else
			return; //std::cout << "WARNING::SHADER.H::An active uniform corresponding to the name '" << name << "' was not found!\n";
	}
};

#endif
