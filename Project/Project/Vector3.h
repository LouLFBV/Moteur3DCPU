#pragma once
#include <cstddef>
#include <cmath>

const double PI = std::acos(-1.0);

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3() : x(0), y(0), z(0) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	~Vector3() = default;


	void Add(const Vector3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
	}

	void Sub(const Vector3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}

	float Dot(const Vector3& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	Vector3 Cross(const Vector3& other) const
	{
		return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	float Norm() const
	{
		return std::sqrt((x * x + y * y + z * z));
	}

	void Normalize()
	{
		if (Norm() == 0) return;
		x /= Norm();
		y /= Norm();
		z /= Norm();
	}

	static float DistanceBetween(Vector3 playerPos, Vector3 targetPos)
	{
		return Vector3(targetPos.x - playerPos.x, targetPos.y - playerPos.y, targetPos.z - playerPos.z).Norm();
	}

	static bool IsInView(Vector3 playerPos, Vector3 forward, Vector3 targetPos, std::size_t angleDeg)
	{
		Vector3 directionToTarget = LookDirection(playerPos, targetPos);
		float dotProduct = forward.Dot(directionToTarget);

		float angleRad = angleDeg * PI / 180.0;

		return dotProduct >= std::cos(angleRad);
	}

	static Vector3 LookDirection(Vector3 position, Vector3 target)
	{
		float newX = (target.x - position.x);
		float newY = (target.y - position.y);
		float newZ = (target.z - position.z);

		Vector3 vectorDirection = Vector3(newX, newY, newZ);
		vectorDirection.Normalize();

		return vectorDirection;
	}

	static void ComputeLocalBasis(const Vector3& forwardInput, const Vector3& worldUp, Vector3& outRight, Vector3& outUp, Vector3& outForward)
	{
		outForward = forwardInput;
		outForward.Normalize();

		outRight = worldUp.Cross(outForward);
		outRight.Normalize();

		outUp = outForward.Cross(outRight);
		outUp.Normalize();
	}
};