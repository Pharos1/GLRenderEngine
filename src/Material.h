#ifndef MATERIAL
#define MATERIAL

#include <iostream>

#include "Shader.h"
#include "Texture.h"

class Material {
public:
	Texture albedo;
	Texture normal;
	Texture metallic;
	Texture roughness;
	Texture AO; //Ambient occlusion
	float shininess = 32.f;

	bool initialized = false;

	Material(std::string albedo, std::string normal = "", std::string metallic = "", std::string roughness = "", std::string AO = "") {
		loadTextures(albedo, normal, metallic, roughness, AO);

		this->initialized = true;
	}
	Material() {};

	static void setIndices(Shader& shader, std::string variableName = "material") {
		shader.use();

		shader.set1i(variableName + ".albedo", 0);
		shader.set1i(variableName + ".normal", 1);
		shader.set1i(variableName + ".metallic", 2);
		shader.set1i(variableName + ".roughness", 3);
		shader.set1i(variableName + ".AO", 4);
	}
	void bind(Shader& shader) {
		shader.use();

		if (!this->initialized) {
			std::cout << "ERROR::MATERIAL.H::MATERIAL IS BEING BOUND BUT IT HAS YET TO BE INITIALIZED" << std::endl;
			return;
		}

		if (albedo.initialized) albedo.bind(0); //If it was loaded then you can bind it
		if (normal.initialized) normal.bind(1);
		if (metallic.initialized) metallic.bind(2);
		if (roughness.initialized) roughness.bind(3);
		if (AO.initialized) AO.bind(4);

		shader.set1f("material.shininess", shininess);
	}
	void unbind() {
		if (albedo.initialized) albedo.unbind(0); //If it was loaded then you can bind it
		if (normal.initialized) normal.unbind(1);
		if (metallic.initialized) metallic.unbind(2);
		if (roughness.initialized) roughness.unbind(3);
		if (AO.initialized) AO.unbind(4);
	}
	void loadTextures(std::string albedo = "", std::string normal = "", std::string metallic = "", std::string roughness = "", std::string AO = "") {
		if (albedo != "") this->albedo = Texture(albedo);
		if (normal != "") this->normal = Texture(normal);
		if (metallic != "") this->metallic = Texture(metallic);
		if (roughness != "") this->roughness = Texture(roughness);
		if (AO != "") this->AO = Texture(AO);

		this->initialized = true;
	}
};

#endif