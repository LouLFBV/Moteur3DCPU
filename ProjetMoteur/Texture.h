#pragma once

#include <vector>
#include <string>
#include <cstdint>

class Texture
{
public:
    Texture() = default;
    explicit Texture(const std::string& filepath);

    bool LoadFromFile(const std::string& filepath);

    uint32_t Sample(float u, float v) const;

    int GetWidth() const;
    int GetHeight() const;

    uint32_t GetPixel(int x, int y) const;

private:
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;

    std::vector<uint32_t> m_pixels;
};
