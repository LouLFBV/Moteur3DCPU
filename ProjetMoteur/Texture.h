#pragma once
#include <vector>
#include <string>
#include <cstdint>

class Texture {
public:
    Texture() = default;
    Texture(const std::string& filepath);

    bool LoadFromFile(const std::string& filepath);

    // Échantillonne la couleur 32-bit (ARGB/RGBA) en coordonnées UV [0.0, 1.0]
    uint32_t Sample(float u, float v) const;

private:
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    std::vector<uint32_t> m_pixels; // Stockage des couleurs 0xAARRGGBB
};