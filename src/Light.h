#pragma once
#ifndef LIGHT
#define LIGHT

#include<GLM/glm.hpp>
#include "Shader.h"

struct Light {
    glm::vec3 diffuse;
    float intensity;
};
class DirLight : public Light {
public:
    glm::vec3 dir;

    DirLight(glm::vec3 dir = glm::vec3(0.f), glm::vec3 diffuse = glm::vec3(0.f), float intensity = 1.f) {
        this->dir = dir;

        //this->ambient = ambient;
        this->diffuse = diffuse;
        //this->specular = specular;

        this->intensity = intensity;
    }
    void set(Shader& shader, std::string lightName) {
        shader.setVec3(lightName + ".direction", dir);

        //shader.setVec3(lightName + ".ambient", ambient);
        shader.setVec3(lightName + ".diffuse", diffuse);

        shader.set1f(lightName + ".intensity", intensity);
        //shader.setVec3(lightName + ".specular", specular);
    }
};
class PointLight : public Light {
public:
    glm::vec3 pos;

    float constant;
    float linear;
    float quadratic;

    PointLight(glm::vec3 pos = glm::vec3(0.f), glm::vec3 diffuse = glm::vec3(0.f), float intensity = 1.f) {//, float constant = 1.f, float linear = .014f, float quadratic = .000007f) {
        this->pos = pos;

        //this->constant = constant;
        //this->linear = linear;
        //this->quadratic = quadratic;

        //this->ambient = ambient;
        this->diffuse = diffuse;
        //this->specular = specular;

        this->intensity = intensity;
    }
    void set(Shader& shader, std::string lightName) {
        shader.setVec3(lightName + ".position", pos);

        //shader.setVec3(lightName + ".ambient", ambient);
        shader.setVec3(lightName + ".diffuse", diffuse);
        //shader.setVec3(lightName + ".specular", specular);

        //shader.set1f(lightName + ".constant", constant);
        //shader.set1f(lightName + ".linear", linear);
        //shader.set1f(lightName + ".quadratic", quadratic);

        shader.set1f(lightName + ".intensity", intensity);
    }
};
class SpotLight : public Light {
public:
    glm::vec3 pos;
    glm::vec3 dir;

    //float constant;
    //float linear;
    //float quadratic;

    float cutOff;
    float outerCutOff;

    float cosCutOff;
    float cosOuterCutOff;

    SpotLight(glm::vec3 pos = glm::vec3(0.f), glm::vec3 dir = glm::vec3(0.f), glm::vec3 diffuse = glm::vec3(0.f), float intensity = 1.f, float cutOff = 0.f, float outerCutOff = 0.f) {//, float constant = 1.f, float linear = .09f, float quadratic = .032f) {
        this->pos = pos;
        this->dir = dir;

        //this->constant = constant;
        //this->linear = linear;
        //this->quadratic = quadratic;

        //this->ambient = ambient;
        this->diffuse = diffuse;
        //this->specular = specular;

        this->cutOff = cutOff;
        this->outerCutOff = outerCutOff;

        this->intensity = intensity;

        updateCosCutOff();
        updateCosOuterCutOff();
    }
    void updateCosCutOff() {
        if (cutOff > outerCutOff)
            std::cout << "WARNING::LIGHT.H::SPOTLIGHT::Inner cut off is bigger than the outer cut off!" << std::endl;
        cosCutOff = glm::cos(glm::radians(cutOff));
    }
    void updateCosOuterCutOff() {
        if (cutOff > outerCutOff)
            std::cout << "WARNING::LIGHT.H::SPOTLIGHT::Inner cut off is bigger than the outer cut off!" << std::endl;
        cosOuterCutOff = glm::cos(glm::radians(outerCutOff)); 
    }
    void set(Shader& shader, std::string lightName) {
        shader.setVec3(lightName + ".position", pos);
        shader.setVec3(lightName + ".direction", dir);

        //shader.setVec3(lightName + ".ambient", ambient);
        shader.setVec3(lightName + ".diffuse", diffuse);
        //shader.setVec3(lightName + ".specular", specular);

        //shader.set1f(lightName + ".constant", constant);
        //shader.set1f(lightName + ".linear", linear);
        //shader.set1f(lightName + ".quadratic", quadratic);

        updateCosCutOff();
        updateCosOuterCutOff();

        shader.set1f(lightName + ".cutOff", cosCutOff);
        shader.set1f(lightName + ".outerCutOff", cosOuterCutOff);

        shader.set1f(lightName + ".intensity", intensity);
    }
};
#endif