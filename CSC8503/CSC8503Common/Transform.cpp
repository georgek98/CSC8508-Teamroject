/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Transform Implementation		 */
#include "Transform.h"

using namespace NCL::CSC8503;

Transform::Transform()
{
	//textureMatrix = matrix;
	textureMatrix = Matrix4::Scale(Vector3(1, 1, 1));
}

Transform::~Transform()
{
}

void Transform::UpdateMatrix()
{
	matrix = Matrix4::Translation(Vector3(pxPos)) * Matrix4(Quaternion(pxOrientation)) * Matrix4::Scale(Vector3(pxScale));
}

void Transform::SetScale(const PxVec3& worldScale)
{
	pxScale = worldScale;
	UpdateMatrix();
}

void Transform::SetPosition(const PxVec3& worldPos)
{
	pxPos = worldPos;
	//UpdateMatrix();
}

void Transform::SetOrientation(const PxQuat& newOr)
{
	pxOrientation = newOr;
	//UpdateMatrix();
}
void Transform::SetTextureScale(Vector3 scale)
{
	textureMatrix = Matrix4::Scale(scale);
}
