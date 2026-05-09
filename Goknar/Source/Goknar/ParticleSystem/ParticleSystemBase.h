#ifndef __PARTICLE_SYSTEM_BASE_H__
#define __PARTICLE_SYSTEM_BASE_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Renderer/Types.h"
#include "Goknar/TimeDependentObject.h"

#include <cstdint>
#include <string>
#include <vector>

class Camera;
class ComputeShader;
class IMaterialBase;
class Shader;

template <typename T>
struct GOKNAR_API GPUParticleValueRange
{
	GPUParticleValueRange() = default;
	explicit GPUParticleValueRange(const T& value) :
		minValue(value),
		maxValue(value)
	{
	}

	GPUParticleValueRange(const T& minValue, const T& maxValue) :
		minValue(minValue),
		maxValue(maxValue)
	{
	}

	T minValue{};
	T maxValue{};
};

struct alignas(8) GOKNAR_API GPUParticleLifetimeState
{
	float remainingLifetime{ 0.f };
	float initialLifetime{ 0.f };
};

struct alignas(8) GOKNAR_API GPUParticleSizeState
{
	float lifetimeStartSize{ 1.f };
	float lifetimeEndSize{ 0.f };
};

struct alignas(16) GOKNAR_API GPUParticleCounterState
{
	std::uint32_t aliveCount{ 0u };
	std::uint32_t spawnedCountThisFrame{ 0u };
	std::uint32_t totalSpawnCount{ 0u };
	std::uint32_t reserved{ 0u };
};

struct alignas(16) GOKNAR_API GPUParticleDrawArraysIndirectCommand
{
	std::uint32_t vertexCountPerInstance{ 6u };
	std::uint32_t instanceCount{ 0u };
	std::uint32_t firstVertex{ 0u };
	std::uint32_t baseInstance{ 0u };
};

struct GOKNAR_API GPUParticleDrawElementsIndirectCommand
{
	std::uint32_t indexCountPerInstance{ 0u };
	std::uint32_t instanceCount{ 0u };
	std::uint32_t firstIndex{ 0u };
	std::int32_t baseVertex{ 0 };
	std::uint32_t baseInstance{ 0u };
};

struct GOKNAR_API GPUParticleFloatCurve
{
	float startValue{ 1.f };
	float endValue{ 1.f };
};

struct GOKNAR_API GPUParticleColorCurve
{
	Vector4 startValue{ Vector4(1.f) };
	Vector4 endValue{ Vector4(1.f) };
};

struct GOKNAR_API GPUParticleSpawnDesc
{
	GPUParticleValueRange<float> lifetime{ 5.f, 5.f };
	GPUParticleValueRange<Vector3> initialVelocity{ Vector3(0.f, 0.f, 2.f), Vector3(0.f, 0.f, 2.f) };
	GPUParticleValueRange<Vector3> initialRotation{ Vector3::ZeroVector, Vector3::ZeroVector };
	GPUParticleValueRange<Vector3> angularVelocity{ Vector3::ZeroVector, Vector3::ZeroVector };
	GPUParticleValueRange<Vector3> acceleration{ Vector3::ZeroVector, Vector3::ZeroVector };
	float velocityLimit{ 0.f };
	GPUParticleFloatCurve sizeByLifetime{ 1.f, 0.f };
	Vector2 sizeBySpeedRange{ 0.f, 10.f };
	GPUParticleFloatCurve sizeBySpeed{ 1.f, 1.f };
	GPUParticleColorCurve colorByLifetime{ Vector4(1.f), Vector4(1.f, 1.f, 1.f, 0.f) };
	Vector2 colorBySpeedRange{ 0.f, 10.f };
	GPUParticleColorCurve colorBySpeed{ Vector4(1.f), Vector4(1.f) };
	Vector3 spawnBoxExtents{ 0.5f, 0.5f, 0.5f };
	float spawnInterval{ 0.02f };
	std::uint32_t spawnCountPerInterval{ 1u };
	bool looping{ true };
};

struct GOKNAR_API GPUParticleSystemDesc
{
	std::uint32_t maxParticleCount{ 65536u };
	Vector3 gravity{ 0.f, 0.f, -9.81f };
	float particleSize{ 0.1f };
	GPUParticleSpawnDesc spawnDesc{};
};

class GOKNAR_API ParticleSystemBase : public TimeDependentObject
{
public:
	explicit ParticleSystemBase(const GPUParticleSystemDesc& desc = GPUParticleSystemDesc());
	~ParticleSystemBase() override;

	void PreInit();
	void Init();
	void PostInit();

	void Tick(float deltaTime) override;
	virtual void Render(const Camera* activeCamera) const = 0;

	void SetInitialParticleData(
		const std::vector<Vector4>& positions,
		const std::vector<Vector4>& velocities,
		const std::vector<Vector4>& colors,
		const std::vector<GPUParticleLifetimeState>& lifetimes);

	void QueueBurstSpawn(std::uint32_t spawnCount);
	void ClearAllParticles(bool resetSpawnSequence = true);

	std::uint32_t GetMaxParticleCount() const
	{
		return maxParticleCount_;
	}

	void SetGravity(const Vector3& gravity)
	{
		gravity_ = gravity;
	}

	const Vector3& GetGravity() const
	{
		return gravity_;
	}

	void SetParticleSize(float particleSize)
	{
		particleSize_ = particleSize;
	}

	float GetParticleSize() const
	{
		return particleSize_;
	}

	void SetSpawnDesc(const GPUParticleSpawnDesc& spawnDesc);
	const GPUParticleSpawnDesc& GetSpawnDesc() const
	{
		return spawnDesc_;
	}

	void SetRenderTransformMatrix(const Matrix& renderTransformMatrix);

	const Matrix& GetRenderTransformMatrix() const
	{
		return renderTransformMatrix_;
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

protected:
	static constexpr GEuint kPositionBufferBindingIndex = 0;
	static constexpr GEuint kVelocityBufferBindingIndex = 1;
	static constexpr GEuint kColorBufferBindingIndex = 2;
	static constexpr GEuint kLifetimeBufferBindingIndex = 3;
	static constexpr GEuint kAliveIndexBufferBindingIndex = 4;
	static constexpr GEuint kCounterBufferBindingIndex = 5;
	static constexpr GEuint kDrawIndirectBufferBindingIndex = 6;
	static constexpr GEuint kEndColorBufferBindingIndex = 7;
	static constexpr GEuint kSizeBufferBindingIndex = 8;
	static constexpr GEuint kRotationBufferBindingIndex = 9;
	static constexpr GEuint kAccelerationBufferBindingIndex = 10;
	static constexpr GEuint kAngularVelocityBufferBindingIndex = 11;

	static constexpr GEuint kComputeLocalSizeX = 256;
	static constexpr GEuint kFinalizeDrawLocalSizeX = 64;

	void BindSimulationBuffers() const;
	void BindRenderBuffers() const;
	void BindFinalizeBuffers() const;
	void ResetSimulationState() const;
	void ApplyParticleStateToShader(Shader* shader) const;
	void ApplyMaterialStateToShader(Shader* shader, const IMaterialBase* material) const;

	ComputeShader* GetFinalizeDrawArraysComputeShader() const
	{
		return finalizeDrawArraysComputeShader_;
	}

	ComputeShader* GetFinalizeDrawElementsComputeShader() const
	{
		return finalizeDrawElementsComputeShader_;
	}

	GEuint GetDrawIndirectBufferId() const
	{
		return drawIndirectBufferId_;
	}

	GEuint GetDummyVertexArrayObjectId() const
	{
		return dummyVertexArrayObjectId_;
	}

	static std::string ResolveShaderPath(const std::string& relativeShaderPath);

	virtual void CreateRenderResources() = 0;
	virtual void DestroyRenderResources() = 0;
	virtual void OnInit() = 0;
	virtual void OnPostInit() = 0;
	virtual void RecreateDrawIndirectBuffer() = 0;
	virtual void DispatchFinalizePass() const = 0;

private:
	std::uint32_t ConsumeRequestedSpawnCount(float deltaTime);
	void ClearParticleDataBuffers(bool resetSpawnSequence);
	void CreateBuffers();
	void DestroyBuffers();
	void CreateComputeShaders();
	void DestroyComputeShaders();
	void DispatchResetPass() const;

	ComputeShader* resetComputeShader_{ nullptr };
	ComputeShader* updateComputeShader_{ nullptr };
	ComputeShader* finalizeDrawArraysComputeShader_{ nullptr };
	ComputeShader* finalizeDrawElementsComputeShader_{ nullptr };

	GEuint particlePositionBufferId_{ 0 };
	GEuint particleVelocityBufferId_{ 0 };
	GEuint particleColorBufferId_{ 0 };
	GEuint particleLifetimeBufferId_{ 0 };
	GEuint aliveIndexBufferId_{ 0 };
	GEuint particleCounterBufferId_{ 0 };
	GEuint drawIndirectBufferId_{ 0 };
	GEuint particleEndColorBufferId_{ 0 };
	GEuint particleSizeBufferId_{ 0 };
	GEuint particleRotationBufferId_{ 0 };
	GEuint particleAccelerationBufferId_{ 0 };
	GEuint particleAngularVelocityBufferId_{ 0 };
	GEuint dummyVertexArrayObjectId_{ 0 };

	std::uint32_t maxParticleCount_{ 0u };
	Vector3 gravity_{ 0.f, -9.81f, 0.f };
	float particleSize_{ 0.1f };
	GPUParticleSpawnDesc spawnDesc_{};
	Matrix renderTransformMatrix_{ Matrix::IdentityMatrix };
	Matrix previousEmitterTransformMatrix_{ Matrix::IdentityMatrix };
	float spawnTimerAccumulator_{ 0.f };
	std::uint32_t queuedBurstSpawnCount_{ 0u };
	std::uint32_t activeParticleSlotCount_{ 0u };
	bool hasEmitterTransformHistory_{ false };
	bool isInitialized_{ false };
};

static_assert(sizeof(GPUParticleLifetimeState) == sizeof(float) * 2, "GPUParticleLifetimeState must match GLSL vec2 layout.");
static_assert(sizeof(GPUParticleSizeState) == sizeof(float) * 2, "GPUParticleSizeState must match GLSL vec2 layout.");
static_assert(sizeof(GPUParticleCounterState) == sizeof(std::uint32_t) * 4, "GPUParticleCounterState must match GLSL std430 alignment.");
static_assert(sizeof(GPUParticleDrawArraysIndirectCommand) == sizeof(std::uint32_t) * 4, "Indirect draw command layout must match OpenGL.");
static_assert(sizeof(GPUParticleDrawElementsIndirectCommand) == sizeof(std::uint32_t) * 4 + sizeof(std::int32_t), "Element indirect draw command layout must match OpenGL.");

#endif
