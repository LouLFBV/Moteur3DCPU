#include "Texture.h"

#include <cmath>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::string& filepath)
{
    LoadFromFile(filepath);
}

bool Texture::LoadFromFile(const std::string& filepath)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data = stbi_load(
        filepath.c_str(),
        &width,
        &height,
        &channels,
        4
    );

    if (!data) {
        return false;
    }

    std::vector<uint32_t> pixels(width * height);

    for (int i = 0; i < width * height; ++i)
    {
        uint32_t r = data[i * 4 + 0];
        uint32_t g = data[i * 4 + 1];
        uint32_t b = data[i * 4 + 2];
        uint32_t a = data[i * 4 + 3];

        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    stbi_image_free(data);

    m_width = width;
    m_height = height;
    m_channels = channels;
    m_pixels = std::move(pixels);

    return true;
}

uint32_t Texture::Sample(float u, float v) const
{
    if (m_pixels.empty()){
        return 0xFFFFFFFF;
    }

    // Répétition des coordonnées UV
    u -= std::floor(u);
    v -= std::floor(v);

    // Inversion de V si l'origine UV est en bas à gauche
    v = 1.0f - v;

    int x = static_cast<int>(u * (m_width - 1));
    int y = static_cast<int>(v * (m_height - 1));

    return m_pixels[y * m_width + x];
}

int Texture::GetWidth() const
{
    return m_width;
}

int Texture::GetHeight() const
{
    return m_height;
}

uint32_t Texture::GetPixel(int x, int y) const
{
    // En dehors de l'image : transparence
    if (m_pixels.empty() ||
        x < 0 || x >= m_width ||
        y < 0 || y >= m_height){
        return 0x00000000;
    }

    return m_pixels[y * m_width + x];
}
