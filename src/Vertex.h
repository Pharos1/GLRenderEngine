#ifndef VERTEX
#define VERTEX

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    //glm::vec3 Bitangent;

    Vertex(glm::vec3 pos = glm::vec3(0.f), glm::vec3 norm = glm::vec3(0.f), glm::vec2 texCoords = glm::vec2(0.f), glm::vec3 tangent = glm::vec3(0.f)){//, glm::vec3 tangent = glm::vec3(1.f, 0.f, 0.f)) {//, glm::vec3 bitangent = glm::vec3(0.f, 1.f, 0.f)) {
        Position = pos;
        Normal = norm;
        TexCoords = texCoords;
        Tangent = tangent;
        //Bitangent = bitangent;

        //Note: The resulting tangent and bitangent vector should have a value of (1,0,0) and (0,1,0)
        //      respectively that together with the normal (0,0,1) forms an orthogonal TBN matrix. Visualized on the plane, the TBN vectors would look like this:
    }
};
#endif