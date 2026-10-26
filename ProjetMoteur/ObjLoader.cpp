#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "ObjLoader.h"
#include <iostream>

void Get_TOBJ(const char* path, T_OBJ& obj)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, path);

    if (!success) {
        std::cerr << "Erreur chargement OBJ : " << warn << std::endl;
        return;
    }

    obj.vertices.clear();

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                T_OBJVertex v_out{};

                // Positions
                v_out.position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                v_out.position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                v_out.position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                v_out.position.w = 1.0f;

                // Normales
                if (idx.normal_index >= 0) {
                    v_out.nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    v_out.ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    v_out.nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // UVs (Textures)
                if (idx.texcoord_index >= 0) {
                    v_out.tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    v_out.ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                // Couleur par défaut
                v_out.r = 0.8f; v_out.g = 0.8f; v_out.b = 0.8f;

                obj.vertices.push_back(v_out);
            }
            index_offset += fv;
        }
    }
}