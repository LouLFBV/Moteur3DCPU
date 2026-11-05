#pragma once

#include "Types.h"
#include "ObjLoader.h"
#include <vector>

struct GameObject
{
    T_OBJ objModel;

    glm::mat4 lastModelMatrix = glm::mat4(1.0f);
    bool hasLastModelMatrix = false;
    bool loaded = false;

    Transform transform{};
    glm::mat4 modelMatrix{ 1.0f };

    std::vector<Vertex> gpuVertices;

    void UpdateTransform()
    {
        modelMatrix = transform.GetModelMatrix();
    }
};