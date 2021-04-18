#pragma once
#include "Vector3.h"
#include "Vector4.h"
using namespace NCL::Maths;

namespace NCL
{
	class Light
	{
	public:

		Light(Vector3 position, Vector4 color, float radius = 5, Vector3 direction = Vector3(0,0,0))
		{
			this->radius     = radius;
			this->lightPos   = position;
			this->lightColour = color;
			this->lightDir = direction;
		}

		Vector3 GetPosition() const { return lightPos; }
		void SetPosition(const Vector3& val) { lightPos = val; }

		Vector3 GetDirection() const { return lightDir; }
		void SetDirection(const Vector3& val) { lightDir = val; }

		float GetRadius() const { return radius; }
		void SetRadius(float val) { radius = val; }

		Vector4 GetColour() const { return lightColour; }
		void SetColour(const Vector4& val) { lightColour = val; }

	protected:

		float radius;
		Vector4 lightColour;
		Vector3 lightPos;
		Vector3 lightDir;
	};

}

