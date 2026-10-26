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
    for (int i = 0; i < 3; i++) l[i] = l[i] * invW[i] / s;
}

glm::vec4 MultiplicationMatrixVector(const glm::mat4& matrix, glm::vec4 vector)
{
    float in[4] = { vector.x, vector.y, vector.z, vector.w };
    float out[4] = { 0, 0, 0, 0 };

    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            out[row] += matrix[row][column] * in[column];

    return { out[0], out[1], out[2], out[3] };
}

Vertex RotateVertex(Vertex v, float a)
{
    float x = v.position.x;
    float y = v.position.y;
    v.position.x = x * std::cos(a) - y * std::sin(a);
    v.position.y = x * std::sin(a) + y * std::cos(a);
    return v;
}