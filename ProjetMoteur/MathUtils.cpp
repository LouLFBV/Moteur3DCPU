#include "MathUtils.h"
#include <cmath>

float Lerp(float a, float b, float t)
{
    return (1.0f - t) * a + t * b;
}

float PerspLerp(float a, float b, float wa, float wb, float t)
{
    float num = (1 - t) * a / wa + t * b / wb;
    float den = (1 - t) / wa + t / wb;
    return num / den;
}

void PerspectiveCorrect(const float invW[3], float l[3])
{
    float s = l[0] * invW[0] + l[1] * invW[1] + l[2] * invW[2];
    for (int i = 0; i < 3; i++) {
        l[i] = l[i] * invW[i] / s;
    }
}

glm::vec4 MultiplicationMatrixVector(const glm::mat4& matrix, glm::vec4 vector)
{
    return matrix * vector;
}
Vertex RotateVertex(Vertex v, float a)
{
    float x = v.position.x;
    float y = v.position.y;
    v.position.x = x * std::cos(a) - y * std::sin(a);
    v.position.y = x * std::sin(a) + y * std::cos(a);
    return v;
}

float SimpleNoise(float x, float z) {
    float f1 = std::sin(x * 0.1f) * std::cos(z * 0.1f) * 5.0f;
    float f2 = std::sin(x * 0.3f + 1.5f) * std::cos(z * 0.3f + 2.0f) * 2.0f;
    float f3 = std::sin(x * 0.8f) * std::cos(z * 0.8f) * 0.5f;
    return f1 + f2 + f3; 
}