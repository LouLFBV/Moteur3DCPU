#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "ObjLoader.h"
#include <iostream>
#include <string>
#include <utility>

// Construit le chemin complet d'une texture à partir du dossier de base.
static std::string GetTexturePath(const char* baseDir, const std::string& textureName)
{
    if (!baseDir || textureName.empty()) {
        return textureName;
    }

    // Ne pas préfixer un chemin absolu.
    const bool isAbsolute =
        textureName[0] == '/' ||
        textureName[0] == '\\' ||
        (textureName.size() > 1 && textureName[1] == ':');

    if (isAbsolute) {
        return textureName;
    }

    std::string path = baseDir;

    if (!path.empty() && path.back() != '/' && path.back() != '\\') {
        path += '/';
    }

    return path + textureName;
}

void Get_TOBJ(const char* path, T_OBJ& obj, const char* baseDir)
{
    if (!path || path[0] == '\0') {
        std::cerr << "Erreur : chemin OBJ invalide." << std::endl;
        return;
    }

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;

    const bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, path, baseDir, true);

    if (!warn.empty()) {
        std::cerr << "Avertissement OBJ : " << warn << std::endl;
    }

    if (!success) {
        std::cerr << "Erreur chargement OBJ " << std::endl;
        return;
    }

    // On travaille sur un objet temporaire pour conserver l'ancien
    // modèle si une erreur survient pendant l'extraction des sommets.
    T_OBJ loadedObj;
    loadedObj.GetMaterial().Reset();

    if (!materials.empty()) {
        const tinyobj::material_t& mat = materials[0];

        loadedObj.GetMaterial().ambient = glm::vec3(
            mat.ambient[0],
            mat.ambient[1],
            mat.ambient[2]
        );

        loadedObj.GetMaterial().diffuse = glm::vec3(
            mat.diffuse[0],
            mat.diffuse[1],
            mat.diffuse[2]
        );

        loadedObj.GetMaterial().specular = glm::vec3(
            mat.specular[0],
            mat.specular[1],
            mat.specular[2]
        );

        if (mat.shininess > 0.0f) {
            loadedObj.GetMaterial().shininess = mat.shininess;
        }

        if (!mat.diffuse_texname.empty()) {
            const std::string texturePath =
                GetTexturePath(baseDir, mat.diffuse_texname);

            if (loadedObj.GetMaterial().texture.LoadFromFile(texturePath)) {
                loadedObj.GetMaterial().hasTexture = true;

                std::cout
                    << "Texture chargee via MTL : "
                    << texturePath
                    << std::endl;
            }
            else {
                std::cerr
                    << "Impossible de charger l'image de texture : "
                    << texturePath
                    << std::endl;
            }
        }
    }
    for (const tinyobj::shape_t& shape : shapes) {
        size_t indexOffset = 0;

        for (unsigned char faceVertexCount : shape.mesh.num_face_vertices)
        {
            const size_t vertexCount = static_cast<size_t>(faceVertexCount);

            if (indexOffset + vertexCount > shape.mesh.indices.size()) {
                std::cerr << "Erreur : indices OBJ invalides." << std::endl;
                return;
            }

            for (size_t v = 0; v < vertexCount; ++v) {
                const tinyobj::index_t& index = shape.mesh.indices[indexOffset + v];

                if (index.vertex_index < 0 || static_cast<size_t>(index.vertex_index) * 3 + 2 >= attrib.vertices.size()) {
                    std::cerr << "Erreur : indice de position OBJ invalide." << std::endl;
                    return;
                }

                T_OBJVertex vertex{};

                // Positions.
                const size_t positionIndex = static_cast<size_t>(index.vertex_index) * 3;

                vertex.position = glm::vec4(
                    attrib.vertices[positionIndex],
                    attrib.vertices[positionIndex + 1],
                    attrib.vertices[positionIndex + 2],
                    1.0f
                );

                // Normales.
                if (index.normal_index >= 0) {
                    const size_t normalIndex = static_cast<size_t>(index.normal_index) * 3;

                    if (normalIndex + 2 >= attrib.normals.size()) {
                        std::cerr << "Erreur : indice de normale OBJ invalide." << std::endl;
                        return;
                    }

                    vertex.nx = attrib.normals[normalIndex];
                    vertex.ny = attrib.normals[normalIndex + 1];
                    vertex.nz = attrib.normals[normalIndex + 2];
                }

                // Coordonnées UV.
                if (index.texcoord_index >= 0) {
                    const size_t texcoordIndex = static_cast<size_t>(index.texcoord_index) * 2;

                    if (texcoordIndex + 1 >= attrib.texcoords.size()) {
                        std::cerr << "Erreur : indice UV OBJ invalide." << std::endl;
                        return;
                    }

                    vertex.tx = attrib.texcoords[texcoordIndex];
                    vertex.ty = attrib.texcoords[texcoordIndex + 1];
                }

                // Couleur blanche par défaut.
                vertex.r = 1.0f;
                vertex.g = 1.0f;
                vertex.b = 1.0f;

                loadedObj.GetVertices().push_back(vertex);
            }

            indexOffset += vertexCount;
        }
    }

    if (loadedObj.GetVertices().empty()) {
        std::cerr << "Erreur : le fichier OBJ ne contient aucun sommet." << std::endl;
        return;
    }

    obj = std::move(loadedObj);
}

const std::vector<T_OBJVertex>& T_OBJ::GetVertices() const
{
    return m_vertices;
}

std::vector<T_OBJVertex>& T_OBJ::GetVertices()
{
    return m_vertices;
}

const Material& T_OBJ::GetMaterial() const
{
    return m_material;
}

Material& T_OBJ::GetMaterial()
{
    return m_material;
}