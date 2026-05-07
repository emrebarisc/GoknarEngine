#ifndef __REFLECTIONPROBEOBJECT_H__
#define __REFLECTIONPROBEOBJECT_H__

#include "Goknar/ObjectBase.h"

class ReflectionProbe;

class GOKNAR_API ReflectionProbeObject : public ObjectBase
{
public:
	ReflectionProbeObject(const ObjectInitializer& objectInitializer = ObjectInitializer());
	~ReflectionProbeObject() override;

	void PreInit() override;
	void Init() override;
	void PostInit() override;

	void SetIsActive(bool isActive) override;

	void SetSize(const Vector3& size);

	const Vector3& GetSize() const
	{
		return size_;
	}

	ReflectionProbe* GetReflectionProbe() const
	{
		return reflectionProbe_;
	}

protected:
	void UpdateWorldTransformationMatrix() override;

private:
	void SyncReflectionProbeState();

	ReflectionProbe* reflectionProbe_{ nullptr };
	Vector3 size_{ Vector3(10.f) };
};

#endif
