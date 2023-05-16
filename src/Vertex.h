#ifndef VERTEX
#define VERTEX

#include <GLM/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    //glm::vec3 Bitangent;

    Vertex(glm::vec3 position = glm::vec3(0.f), glm::vec3 normal = glm::vec3(0.f), glm::vec2 texCoord = glm::vec2(0.f), glm::vec3 tangent = glm::vec3(0.f)){//, glm::vec3 tangent = glm::vec3(1.f, 0.f, 0.f)) {//, glm::vec3 bitangent = glm::vec3(0.f, 1.f, 0.f)) {
        this->position = position;
        this->normal = normal;
        this->texCoord = texCoord;
        this->tangent = tangent;
        //Bitangent = bitangent;

        //Note: The resulting tangent and bitangent vector should have a value of (1,0,0) and (0,1,0)
        //      respectively that together with the normal (0,0,1) forms an orthogonal TBN matrix. Visualized on the plane, the TBN vectors would look like this:
    }
};
#endif