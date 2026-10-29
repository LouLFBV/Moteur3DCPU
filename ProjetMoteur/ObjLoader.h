#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Types.h"
#include "Texture.h" 

class T_OBJ
{
public:
    std::vector<T_OBJVertex> vertices;
    Texture texture;       
    bool hasTexture = false;
};

void Get_TOBJ(const char* path, T_OBJ& obj, const char* baseDir = nullptr);