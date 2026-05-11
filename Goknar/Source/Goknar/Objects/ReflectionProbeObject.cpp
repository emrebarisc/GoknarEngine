#include "pch.h"

#include "Goknar/Objects/ReflectionProbeObject.h"

#include "Goknar/Renderer/ReflectionProbe.h"

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

	if (reflectionProbe_)
	{
		reflectionProbe_->SetSize(size_);
	}
}

void ReflectionProbeObject::UpdateWorldTransformationMatrix()
{
	ObjectBase::UpdateWorldTransformationMatrix();
	SyncReflectionProbeState();
}

void ReflectionProbeObject::SyncReflectionProbeState()
{
	if (!reflectionProbe_)
	{
		return;
	}

	reflectionProbe_->SetPosition(GetWorldTransformationMatrix().GetTranslation());
	reflectionProbe_->SetSize(size_);
	reflectionProbe_->SetIsActive(GetIsActive());
}
