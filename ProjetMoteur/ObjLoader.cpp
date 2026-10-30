#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "ObjLoader.h"
#include <iostream>

void Get_TOBJ(const char* path, T_OBJ& obj, const char* baseDir)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, path, baseDir);

    if (!success) {
        std::cerr << "Erreur chargement OBJ : " << warn << " " << err << std::endl;
        return;
    }

    obj.vertices.clear();
    obj.material.Reset();

    // 1. Détection et chargement automatique de la texture et du material depuis le .mtl
    if (!materials.empty()) {
        const auto& mat = materials[0];

        // Remplissage des composantes Blinn-Phong
        obj.material.ambient = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
        obj.material.diffuse = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        obj.material.specular = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);

        if (mat.shininess > 0.0f) {
            obj.material.shininess = mat.shininess;
        }
        // Chargement automatique de la texture diffuse (map_Kd)
        if (!mat.diffuse_texname.empty()) {
            std::string texturePath = mat.diffuse_texname;
            if (baseDir) {
                texturePath = std::string(baseDir) + "/" + texturePath;
            }

            if (obj.material.texture.LoadFromFile(texturePath)) {
                obj.material.hasTexture = true;
                std::cout << "Texture chargee via MTL : " << texturePath << std::endl;
            }
            else {
                std::cerr << "Impossible de charger l'image de texture : " << texturePath << std::endl;
            }
        }
    }

    // 2. Extraction classique des sommets
    for (size_t s = 0; s < shapes.size(); ++s) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
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
                v_out.r = 1.0f;
                v_out.g = 1.0f;
                v_out.b = 1.0f;

                obj.vertices.push_back(v_out);
            }
            index_offset += fv;
        }
    }
}