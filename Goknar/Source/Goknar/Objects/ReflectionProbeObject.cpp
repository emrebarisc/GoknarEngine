#include "pch.h"

#include "Goknar/Objects/ReflectionProbeObject.h"

#include "Goknar/Renderer/ReflectionProbe.h"

#include <cmath>

ReflectionProbeObject::ReflectionProbeObject(const ObjectInitializer& objectInitializer) :
	ObjectBase(objectInitializer)
{
	SetName("ReflectionProbeObject");

	reflectionProbe_ = new ReflectionProbe();
	SyncReflectionProbeState();
}

ReflectionProbeObject::~ReflectionProbeObject()
{
	delete reflectionProbe_;
}

ObjectBase* ReflectionProbeObject::Clone() const
{
	ReflectionProbeObject* clonedObject = new ReflectionProbeObject();
	CopyValuesTo(clonedObject);
	clonedObject->SetSize(size_);
	clonedObject->SetCaptureDistance(captureDistance_);
	clonedObject->SetIsActive(GetIsActive());

	return clonedObject;
}

void ReflectionProbeObject::PreInit()
{
	ObjectBase::PreInit();

	if (reflectionProbe_)
	{
		reflectionProbe_->PreInit();
	}

	SyncReflectionProbeState();
}

void ReflectionProbeObject::Init()
{
	ObjectBase::Init();

	if (reflectionProbe_)
	{
		reflectionProbe_->Init();
	}
}

void ReflectionProbeObject::PostInit()
{
	if (reflectionProbe_)
	{
		reflectionProbe_->PostInit();
	}

	ObjectBase::PostInit();
}

void ReflectionProbeObject::SetIsActive(bool isActive)
{
	ObjectBase::SetIsActive(isActive);

	if (reflectionProbe_)
	{
		reflectionProbe_->SetIsActive(isActive);
	}
}

void ReflectionProbeObject::SetSize(const Vector3& size)
{
	const Vector3 sanitizedSize = Vector3::Max(size, Vector3(SMALLER_EPSILON));
	if (size_ == sanitizedSize)
	{
		return;
	}

	size_ = sanitizedSize;

	SyncReflectionProbeState();
}

void ReflectionProbeObject::SetCaptureDistance(float captureDistance)
{
	const float sanitizedCaptureDistance = std::isfinite(captureDistance) ? GoknarMath::Max(captureDistance, SMALLER_EPSILON) : 1000.f;
	if (captureDistance_ == sanitizedCaptureDistance)
	{
		return;
	}

	captureDistance_ = sanitizedCaptureDistance;

	if (reflectionProbe_)
	{
		reflectionProbe_->SetCaptureDistance(captureDistance_);
	}
}

void ReflectionProbeObject::UpdateWorldTransformationMatrix()
{
	ObjectBase::UpdateWorldTransformationMatrix();
	SyncReflectionProbeState();
}

Vector3 ReflectionProbeObject::GetScaledSize() const
{
	const Vector3 worldScale = GetWorldScaling();
	const Vector3 absoluteWorldScale(
		GoknarMath::Abs(worldScale.x),
		GoknarMath::Abs(worldScale.y),
		GoknarMath::Abs(worldScale.z));

	return Vector3::Max(size_ * absoluteWorldScale, Vector3(SMALLER_EPSILON));
}

void ReflectionProbeObject::SyncReflectionProbeState()
{
	if (!reflectionProbe_)
	{
		return;
	}

	reflectionProbe_->SetPosition(GetWorldTransformationMatrix().GetTranslation());
	reflectionProbe_->SetSize(GetScaledSize());
	reflectionProbe_->SetCaptureDistance(captureDistance_);
	reflectionProbe_->SetIsActive(GetIsActive());
}
