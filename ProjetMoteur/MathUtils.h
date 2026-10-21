#pragma once
#include "Types.h"
#include <glm/glm.hpp>

float Lerp(float a, float b, float t);
float PerspLerp(float a, float b, float wa, float wb, float t);
void PerspectiveCorrect(const float invW[3], float l[3]);
glm::vec4 MultiplicationMatrixVector(const glm::mat4& matrix, glm::vec4 vector);
Vertex RotateVertex(Vertex v, float a);