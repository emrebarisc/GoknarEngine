#include "AxisObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

AxisObject::AxisObject() : ObjectBase()
{
	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* axisStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Axis.fbx");
	staticMeshComponent_->SetMesh(axisStaticMesh);

	SetWorldPosition(Vector3::ZeroVector);
	SetWorldScaling(Vector3{ 0.1f });
}

void AxisObject::BeginGame()
{
	//GOKNAR_INFO("F: {} L: {} U: {}", GetForwardVector().ToString(), GetLeftVector().ToString(), GetUpVector().ToString());

	// Integration test for quaternion from/to euler conversion
	//for (float k = -180.f; k <= 180.f; k += 12.5f)
	//{
	//	for (float j = -180.f; j <= 180.f; j += 12.5f)
	//	{
	//		for (float i = -180.f; i <= 180.f; i += 12.5f)
	//		{
	//			Vector3 testVector = Vector3{ i, j, k };
	//			Matrix rotationMatrix = Matrix::GetRotationMatrix(testVector);
	//			Quaternion quaternionFromMatrix(rotationMatrix);
	//			Quaternion quaternion = Quaternion::FromEuler(testVector);

	//			//if (!quaternionFromMatrix.Equals(quaternion))
	//			//{
	//			//	GOKNAR_INFO("quaternionFromMatrix: {} quaternion: {}", quaternionFromMatrix.ToString(), quaternion.ToString());
	//			//}

	//			Vector3 resultVector = quaternion.ToEuler();
	//			Matrix testVectorMatrix = quaternion.GetMatrix();
	//			Quaternion quaternion2 = Quaternion::FromEuler(resultVector);
	//			Matrix quaternion2Matrix = quaternion2.GetMatrix();

	//			if (testVectorMatrix.Equals(quaternion2Matrix)/* && 
	//				quaternion.Equals(quaternion2)*/)//  Vector3::Distance(testVector, resultVector) < 0.001f)
	//			{
	//				GOKNAR_INFO("CORRECT! Vector: {} Angle: {} Quaternion: {}", testVector.ToString(), resultVector.ToString(), quaternion.ToString());
	//			}
	//			else
	//			{

	//				GOKNAR_INFO("INCORRECT! Vector: {} Angle: {} Quaternion: {}", testVector.ToString(), resultVector.ToString(), quaternion.ToString());
	//			}
	//		}
	//	}
	//}
}

void AxisObject::Tick(float deltaTime)
{

}
