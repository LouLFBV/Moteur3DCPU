#pragma once

#include <vector>
#include <glm/glm.hpp>

struct T_OBJVertex
{
    glm::vec4 position;
    float nx, ny, nz;
    float tx, ty;
    float r, g, b;
};

class T_OBJ
{
public:
    std::vector<T_OBJVertex> vertices;
};

void Get_TOBJ(const char* path, T_OBJ& obj);