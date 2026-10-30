#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Types.h"
#include "Material.h"

class T_OBJ
{
public:
    std::vector<T_OBJVertex> vertices;
    Material material;       
};

void Get_TOBJ(const char* path, T_OBJ& obj, const char* baseDir = nullptr);