#ifndef __BILLBOARD_PARTICLE_SYSTEM_H__
#define __BILLBOARD_PARTICLE_SYSTEM_H__

#include "ParticleSystemBase.h"

class Image;
class IMaterialBase;
class Shader;

class GOKNAR_API BillboardParticleSystem : public ParticleSystemBase
{
public:
	explicit BillboardParticleSystem(const GPUParticleSystemDesc& desc = GPUParticleSystemDesc());
	~BillboardParticleSystem() override;

	void Render(const Camera* activeCamera) const override;

	void SetParticleTexture(const Image* particleTexture);
	const Image* GetParticleTexture() const
	{
		return particleTexture_;
	}

	void SetParticleMaterial(const IMaterialBase* particleMaterial);
	const IMaterialBase* GetParticleMaterial() const
	{
		return particleMaterial_;
	}

protected:
	void CreateRenderResources() override;
	void DestroyRenderResources() override;
	void OnInit() override;
	void OnPostInit() override;
	void RecreateDrawIndirectBuffer() override;
	void DispatchFinalizePass() const override;

private:
	void RecreateRenderShader();

	Shader* renderShader_{ nullptr };
	const Image* particleTexture_{ nullptr };
	const IMaterialBase* particleMaterial_{ nullptr };
};

#endif
