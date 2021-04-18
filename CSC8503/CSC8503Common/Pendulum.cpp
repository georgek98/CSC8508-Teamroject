#include "Pendulum.h"
using namespace NCL;
using namespace CSC8503;
using namespace Maths;

bool print(tweeny::tween<int>&, int x)
{
	printf("%d\n", x); return false;
}
Pendulum::Pendulum(float timeToSwing, bool isSwingingLeft)
{
	speed = timeToSwing;
	name = "Pendulum";
	this->isSwingingLeft = isSwingingLeft;

	angle = isSwingingLeft ? -45 : 45;
	Quaternion q = transform.GetOrientation();
	float zAngle = q.ToEuler().z;
	tween = tweeny::from(zAngle).to(angle).during(200).onStep([this](float outAngle)
		{
			this->SetOrientation(PxQuat(Maths::DegreesToRadians(outAngle), PxVec3(0, 0, 1)));
			return false;
		});
	tween.via(0, tweeny::easing::cubicOut);
}

void Pendulum::FixedUpdate(float fixedDT)
{
	if (tween.progress() < 1.0f)
	{
		tween.step(1);
	}
	else
	{
		isSwingingLeft = !isSwingingLeft;
		angle = isSwingingLeft ? -45 : 45;
		Quaternion q = transform.GetOrientation();
		float zAngle = q.ToEuler().z;
		tween = tweeny::from(zAngle).to(angle).during(200).onStep([this](float outAngle)
			{
				this->SetOrientation(PxQuat(Maths::DegreesToRadians(outAngle), PxVec3(0, 0, 1)));
				return false;
			});
		tween.via(0, tweeny::easing::cubicOut);
	}
}
