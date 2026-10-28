#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Types.h"


class T_OBJ
{
public:
    std::vector<T_OBJVertex> vertices;
};

void Get_TOBJ(const char* path, T_OBJ& obj);