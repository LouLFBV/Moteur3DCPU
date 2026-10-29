#include "Texture.h"
#include <cmath>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::string& filepath) {
    LoadFromFile(filepath);
}

bool Texture::LoadFromFile(const std::string& filepath) {
    unsigned char* data = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channels, 4);
    if (!data) return false;

    m_pixels.resize(m_width * m_height);

    for (int i = 0; i < m_width * m_height; ++i) {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = data[i * 4 + 3];

        // Format ARGB 32-bit
        m_pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    stbi_image_free(data);
    return true;
}

uint32_t Texture::Sample(float u, float v) const {
    if (m_pixels.empty()) return 0xFFFFFFFF; // Blanc par défaut si pas de texture

    // Wrap / Repeat (repeat la texture si UV < 0 ou > 1)
    u = u - std::floor(u);
    v = v - std::floor(v);

    // Inversion de V si l'origine UV est en bas à gauche
    v = 1.0f - v;

    int x = static_cast<int>(u * (m_width - 1));
    int y = static_cast<int>(v * (m_height - 1));

    x = std::clamp(x, 0, m_width - 1);
    y = std::clamp(y, 0, m_height - 1);

    return m_pixels[y * m_width + x];
}