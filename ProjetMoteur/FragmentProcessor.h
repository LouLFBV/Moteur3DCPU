#pragma once
#include <functional>
#include "Types.h"

using PixelShader = std::function<uint32_t(const BordLigne&)>;

class FragmentProcessor {
public:
    void SetFragmentShader(PixelShader shader);

    // Exécute le shader de fragment et gère le blending si besoin
    uint32_t ProcessFragment(const BordLigne& pixel) const;

private:
    PixelShader m_fragmentShader;
};