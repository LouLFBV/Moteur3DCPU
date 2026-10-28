#include "FragmentProcessor.h"


void FragmentProcessor::SetFragmentShader(PixelShader shader)
{
    m_fragmentShader = shader;
}

uint32_t FragmentProcessor::ProcessFragment(const BordLigne& pixel) const
{
    if (m_fragmentShader) {
        return m_fragmentShader(pixel);
    }

    return 0xFFFF00FF;
}
