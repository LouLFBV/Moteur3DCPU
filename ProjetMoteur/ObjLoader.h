#pragma once
#include <vector>
#include "Types.h"
#include "Material.h"

class T_OBJ
{
public:
    const std::vector<T_OBJVertex>& GetVertices() const;
    std::vector<T_OBJVertex>& GetVertices();

    const Material& GetMaterial() const;
    Material& GetMaterial();

private:
    std::vector<T_OBJVertex> m_vertices;
    Material m_material;
};

void Get_TOBJ(const char* path, T_OBJ& obj, const char* baseDir = nullptr);