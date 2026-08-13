#include "pch.h"

#include "ParticleSystemBase.h"

#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Renderer/ComputeShader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderTypes.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MINIMUM_LIFETIME = 0.0001f;
	constexpr float MINIMUM_SPAWN_INTERVAL = 0.0001f;

	IGraphicsAPI* GraphicsAPI()
	{
		return engine->GetGraphicsAPI();
	}

	GPUParticleValueRange<float> SanitizeFloatRange(const GPUParticleValueRange<float>& range, float minimumValue)
	{
		GPUParticleValueRange<float> sanitizedRange(range);
		sanitizedRange.minValue = (std::max)(minimumValue, sanitizedRange.minValue);
		sanitizedRange.maxValue = (std::max)(minimumValue, sanitizedRange.maxValue);
		if (sanitizedRange.maxValue < sanitizedRange.minValue)
		{
			std::swap(sanitizedRange.minValue, sanitizedRange.maxValue);
		}

		return sanitizedRange;
	}

	GPUParticleValueRange<Vector3> SanitizeVector3Range(const GPUParticleValueRange<Vector3>& range)
	{
		GPUParticleValueRange<Vector3> sanitizedRange;
		sanitizedRange.minValue = Vector3::Min(range.minValue, range.maxValue);
		sanitizedRange.maxValue = Vector3::Max(range.minValue, range.maxValue);
		return sanitizedRange;
	}

	Vector2 SanitizeRangeVector(const Vector2& range)
	{
		return Vector2((std::min)(range.x, range.y), (std::max)(range.x, range.y));
	}

	GPUParticleFloatCurve SanitizeFloatCurve(const GPUParticleFloatCurve& curve, float minimumValue)
	{
		GPUParticleFloatCurve sanitizedCurve(curve);
		sanitizedCurve.startValue = (std::max)(minimumValue, sanitizedCurve.startValue);
		sanitizedCurve.endValue = (std::max)(minimumValue, sanitizedCurve.endValue);
		return sanitizedCurve;
	}

	Vector4 SanitizeParticleColor(const Vector4& color)
	{
		Vector4 sanitizedColor(color);
		sanitizedColor.w = GoknarMath::Clamp(sanitizedColor.w, 0.f, 1.f);
		return sanitizedColor;
	}

	GPUParticleColorCurve SanitizeColorCurve(const GPUParticleColorCurve& curve)
	{
		GPUParticleColorCurve sanitizedCurve(curve);
		sanitizedCurve.startValue = SanitizeParticleColor(sanitizedCurve.startValue);
		sanitizedCurve.endValue = SanitizeParticleColor(sanitizedCurve.endValue);
		return sanitizedCurve;
	}

	Vector3 SanitizeEmissiveColor(const Vector3& color)
	{
		return Vector3(
			(std::max)(0.f, color.x),
			(std::max)(0.f, color.y),
			(std::max)(0.f, color.z));
	}

	GPUParticleVector3Curve SanitizeEmissiveColorCurve(const GPUParticleVector3Curve& curve)
	{
		GPUParticleVector3Curve sanitizedCurve(curve);
		sanitizedCurve.startValue = SanitizeEmissiveColor(sanitizedCurve.startValue);
		sanitizedCurve.endValue = SanitizeEmissiveColor(sanitizedCurve.endValue);
		return sanitizedCurve;
	}

	template <typename T>
	bool AreRangesEqual(const GPUParticleValueRange<T>& firstRange, const GPUParticleValueRange<T>& secondRange)
	{
		return firstRange.minValue == secondRange.minValue && firstRange.maxValue == secondRange.maxValue;
	}

	bool AreFloatCurvesEqual(const GPUParticleFloatCurve& firstCurve, const GPUParticleFloatCurve& secondCurve)
	{
		return firstCurve.startValue == secondCurve.startValue && firstCurve.endValue == secondCurve.endValue;
	}

	bool AreColorCurvesEqual(const GPUParticleColorCurve& firstCurve, const GPUParticleColorCurve& secondCurve)
	{
		return firstCurve.startValue == secondCurve.startValue && firstCurve.endValue == secondCurve.endValue;
	}

	bool AreVector3CurvesEqual(const GPUParticleVector3Curve& firstCurve, const GPUParticleVector3Curve& secondCurve)
	{
		return firstCurve.startValue == secondCurve.startValue && firstCurve.endValue == secondCurve.endValue;
	}

	bool AreSpawnDescsEqual(const GPUParticleSpawnDesc& firstDesc, const GPUParticleSpawnDesc& secondDesc)
	{
		return
			AreRangesEqual(firstDesc.lifetime, secondDesc.lifetime) &&
			firstDesc.infiniteLifetime == secondDesc.infiniteLifetime &&
			AreRangesEqual(firstDesc.initialVelocity, secondDesc.initialVelocity) &&
			AreRangesEqual(firstDesc.initialRotation, secondDesc.initialRotation) &&
			AreRangesEqual(firstDesc.angularVelocity, secondDesc.angularVelocity) &&
			AreRangesEqual(firstDesc.acceleration, secondDesc.acceleration) &&
			firstDesc.velocityLimit == secondDesc.velocityLimit &&
			AreRangesEqual(firstDesc.initialSize, secondDesc.initialSize) &&
			AreFloatCurvesEqual(firstDesc.sizeByLifetime, secondDesc.sizeByLifetime) &&
			firstDesc.sizeBySpeedRange == secondDesc.sizeBySpeedRange &&
			AreFloatCurvesEqual(firstDesc.sizeBySpeed, secondDesc.sizeBySpeed) &&
			AreColorCurvesEqual(firstDesc.colorByLifetime, secondDesc.colorByLifetime) &&
			firstDesc.colorBySpeedRange == secondDesc.colorBySpeedRange &&
			AreColorCurvesEqual(firstDesc.colorBySpeed, secondDesc.colorBySpeed) &&
			AreVector3CurvesEqual(firstDesc.emissiveColorByLifetime, secondDesc.emissiveColorByLifetime) &&
			firstDesc.spawnBoxExtents == secondDesc.spawnBoxExtents &&
			firstDesc.spawnInterval == secondDesc.spawnInterval &&
			firstDesc.spawnCountPerInterval == secondDesc.spawnCountPerInterval &&
			firstDesc.looping == secondDesc.looping;
	}

	bool IsAliveLifetime(const GPUParticleLifetimeState& lifetime)
	{
		return 0.f < lifetime.remainingLifetime || lifetime.initialLifetime < 0.f;
	}

	GPUParticleSpawnDesc SanitizeSpawnDesc(const GPUParticleSpawnDesc& spawnDesc)
	{
		GPUParticleSpawnDesc sanitizedDesc(spawnDesc);
		sanitizedDesc.lifetime = SanitizeFloatRange(sanitizedDesc.lifetime, MINIMUM_LIFETIME);
		sanitizedDesc.initialVelocity = SanitizeVector3Range(sanitizedDesc.initialVelocity);
		sanitizedDesc.initialRotation = SanitizeVector3Range(sanitizedDesc.initialRotation);
		sanitizedDesc.angularVelocity = SanitizeVector3Range(sanitizedDesc.angularVelocity);
		sanitizedDesc.acceleration = SanitizeVector3Range(sanitizedDesc.acceleration);
		sanitizedDesc.velocityLimit = (std::max)(0.f, sanitizedDesc.velocityLimit);
		sanitizedDesc.initialSize = SanitizeFloatRange(sanitizedDesc.initialSize, 0.f);
		sanitizedDesc.sizeByLifetime = SanitizeFloatCurve(sanitizedDesc.sizeByLifetime, 0.f);
		sanitizedDesc.sizeBySpeedRange = SanitizeRangeVector(sanitizedDesc.sizeBySpeedRange);
		sanitizedDesc.sizeBySpeed = SanitizeFloatCurve(sanitizedDesc.sizeBySpeed, 0.f);
		sanitizedDesc.colorByLifetime = SanitizeColorCurve(sanitizedDesc.colorByLifetime);
		sanitizedDesc.colorBySpeedRange = SanitizeRangeVector(sanitizedDesc.colorBySpeedRange);
		sanitizedDesc.colorBySpeed = SanitizeColorCurve(sanitizedDesc.colorBySpeed);
		sanitizedDesc.emissiveColorByLifetime = SanitizeEmissiveColorCurve(sanitizedDesc.emissiveColorByLifetime);
		sanitizedDesc.spawnBoxExtents.x = (std::max)(0.f, sanitizedDesc.spawnBoxExtents.x);
		sanitizedDesc.spawnBoxExtents.y = (std::max)(0.f, sanitizedDesc.spawnBoxExtents.y);
		sanitizedDesc.spawnBoxExtents.z = (std::max)(0.f, sanitizedDesc.spawnBoxExtents.z);
		sanitizedDesc.spawnCountPerInterval = (std::max)(1u, sanitizedDesc.spawnCountPerInterval);
		if (sanitizedDesc.infiniteLifetime)
		{
			sanitizedDesc.spawnInterval = 0.f;
			sanitizedDesc.looping = false;
		}
		else
		{
			sanitizedDesc.spawnInterval = (std::max)(MINIMUM_SPAWN_INTERVAL, sanitizedDesc.spawnInterval);
		}
		return sanitizedDesc;
	}
}

ParticleSystemBase::ParticleSystemBase(const GPUParticleSystemDesc& desc) :
	maxParticleCount_(desc.maxParticleCount),
	gravity_(desc.gravity),
	particleSize_(desc.particleSize),
	spawnDesc_(SanitizeSpawnDesc(desc.spawnDesc))
{
	GOKNAR_CORE_ASSERT(0u < maxParticleCount_, "ParticleSystemBase requires at least one particle slot.");

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->AddParticleSystem(this);
	}
}

ParticleSystemBase::~ParticleSystemBase()
{
	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->RemoveParticleSystem(this);
	}

	DestroyComputeShaders();
	DestroyBuffers();
}

void ParticleSystemBase::PreInit()
{
	CreateBuffers();
	CreateComputeShaders();
	CreateRenderResources();

	isInitialized_ = true;
}

void ParticleSystemBase::Init()
{
	OnInit();
	RecreateDrawIndirectBuffer();
	ResetSimulationState();
}

void ParticleSystemBase::PostInit()
{
	if (resetComputeShader_)
	{
		resetComputeShader_->PostInit();
	}

	if (updateComputeShader_)
	{
		updateComputeShader_->PostInit();
	}

	if (finalizeDrawArraysComputeShader_)
	{
		finalizeDrawArraysComputeShader_->PostInit();
	}

	if (finalizeDrawElementsComputeShader_)
	{
		finalizeDrawElementsComputeShader_->PostInit();
	}

	OnPostInit();
}

void ParticleSystemBase::Tick(float deltaTime)
{
	if (!isInitialized_ || !GetIsActive() || !updateComputeShader_ || drawIndirectBufferId_ == 0)
	{
		return;
	}

	if (!hasEmitterTransformHistory_)
	{
		previousEmitterTransformMatrix_ = renderTransformMatrix_;
		hasEmitterTransformHistory_ = true;
	}

	const std::uint32_t requestedSpawnCount = ConsumeRequestedSpawnCount(deltaTime);
	const std::uint64_t requestedSlotCount = static_cast<std::uint64_t>(activeParticleSlotCount_) + requestedSpawnCount;
	const std::uint32_t dispatchParticleCount = static_cast<std::uint32_t>(
		(std::min)(requestedSlotCount, static_cast<std::uint64_t>(maxParticleCount_)));

	if (dispatchParticleCount == 0u)
	{
		return;
	}

	activeParticleSlotCount_ = (std::max)(activeParticleSlotCount_, dispatchParticleCount);

	DispatchResetPass();
	GraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderStorage | GraphicsMemoryBarrier::Command);

	BindSimulationBuffers();

	updateComputeShader_->Use();
	updateComputeShader_->SetInt("particleCount", static_cast<int>(dispatchParticleCount));
	updateComputeShader_->SetFloat("deltaTime", deltaTime);
	updateComputeShader_->SetVector3("gravity", gravity_);
	updateComputeShader_->SetInt("requestedSpawnCount", static_cast<int>(requestedSpawnCount));
	updateComputeShader_->SetVector2("lifetimeRange", Vector2(spawnDesc_.lifetime.minValue, spawnDesc_.lifetime.maxValue));
	updateComputeShader_->SetVector3("initialVelocityMin", spawnDesc_.initialVelocity.minValue);
	updateComputeShader_->SetVector3("initialVelocityMax", spawnDesc_.initialVelocity.maxValue);
	updateComputeShader_->SetVector3("initialRotationMin", spawnDesc_.initialRotation.minValue);
	updateComputeShader_->SetVector3("initialRotationMax", spawnDesc_.initialRotation.maxValue);
	updateComputeShader_->SetVector3("angularVelocityMin", spawnDesc_.angularVelocity.minValue);
	updateComputeShader_->SetVector3("angularVelocityMax", spawnDesc_.angularVelocity.maxValue);
	updateComputeShader_->SetVector3("accelerationMin", spawnDesc_.acceleration.minValue);
	updateComputeShader_->SetVector3("accelerationMax", spawnDesc_.acceleration.maxValue);
	updateComputeShader_->SetFloat("velocityLimit", spawnDesc_.velocityLimit);
	updateComputeShader_->SetBool("infiniteLifetime", spawnDesc_.infiniteLifetime);
	updateComputeShader_->SetVector2("initialSizeRange", Vector2(spawnDesc_.initialSize.minValue, spawnDesc_.initialSize.maxValue));
	updateComputeShader_->SetVector2("sizeByLifetime", Vector2(spawnDesc_.sizeByLifetime.startValue, spawnDesc_.sizeByLifetime.endValue));
	updateComputeShader_->SetVector3("spawnBoxExtents", spawnDesc_.spawnBoxExtents);
	updateComputeShader_->SetVector4("colorByLifetimeStart", spawnDesc_.colorByLifetime.startValue);
	updateComputeShader_->SetVector4("colorByLifetimeEnd", spawnDesc_.colorByLifetime.endValue);
	updateComputeShader_->SetMatrix("previousEmitterMatrix", previousEmitterTransformMatrix_);
	updateComputeShader_->SetMatrix("currentEmitterMatrix", renderTransformMatrix_);

	const GEuint groupCountX = (dispatchParticleCount + COMPUTE_LOCAL_SIZE_X - 1u) / COMPUTE_LOCAL_SIZE_X;
	updateComputeShader_->Dispatch(groupCountX, 1u, 1u);

	GraphicsAPI()->MemoryBarrier(static_cast<GraphicsMemoryBarrierFlags>(GraphicsMemoryBarrier::ShaderStorage));

	DispatchFinalizePass();

	GraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderStorage | GraphicsMemoryBarrier::Command);

	previousEmitterTransformMatrix_ = renderTransformMatrix_;
}

void ParticleSystemBase::SetInitialParticleData(
	const std::vector<Vector4>& positions,
	const std::vector<Vector4>& velocities,
	const std::vector<Vector4>& colors,
	const std::vector<GPUParticleLifetimeState>& lifetimes)
{
	GOKNAR_CORE_ASSERT(isInitialized_, "ParticleSystemBase must be pre-initialized before uploading particle data.");
	GOKNAR_CORE_ASSERT(positions.size() == maxParticleCount_, "Position array size must match maxParticleCount.");
	GOKNAR_CORE_ASSERT(velocities.size() == maxParticleCount_, "Velocity array size must match maxParticleCount.");
	GOKNAR_CORE_ASSERT(colors.size() == maxParticleCount_, "Color array size must match maxParticleCount.");
	GOKNAR_CORE_ASSERT(lifetimes.size() == maxParticleCount_, "Lifetime array size must match maxParticleCount.");

	std::vector<Vector4> endColors(maxParticleCount_, Vector4(1.f, 1.f, 1.f, 0.f));
	std::vector<GPUParticleSizeState> sizes(maxParticleCount_, GPUParticleSizeState{ 1.f, 1.f });
	std::vector<Vector4> zeroVector4Data(maxParticleCount_, Vector4(0.f));

	for (std::uint32_t particleIndex = 0u; particleIndex < maxParticleCount_; ++particleIndex)
	{
		endColors[particleIndex] = Vector4(colors[particleIndex].x, colors[particleIndex].y, colors[particleIndex].z, 0.f);
	}

	GraphicsAPI()->NamedBufferSubData(particlePositionBufferId_, 0, static_cast<GEsizeiptr>(positions.size() * sizeof(Vector4)), positions.data());
	GraphicsAPI()->NamedBufferSubData(particleVelocityBufferId_, 0, static_cast<GEsizeiptr>(velocities.size() * sizeof(Vector4)), velocities.data());
	GraphicsAPI()->NamedBufferSubData(particleColorBufferId_, 0, static_cast<GEsizeiptr>(colors.size() * sizeof(Vector4)), colors.data());
	GraphicsAPI()->NamedBufferSubData(particleLifetimeBufferId_, 0, static_cast<GEsizeiptr>(lifetimes.size() * sizeof(GPUParticleLifetimeState)), lifetimes.data());
	GraphicsAPI()->NamedBufferSubData(particleEndColorBufferId_, 0, static_cast<GEsizeiptr>(endColors.size() * sizeof(Vector4)), endColors.data());
	GraphicsAPI()->NamedBufferSubData(particleSizeBufferId_, 0, static_cast<GEsizeiptr>(sizes.size() * sizeof(GPUParticleSizeState)), sizes.data());
	GraphicsAPI()->NamedBufferSubData(particleRotationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleAccelerationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleAngularVelocityBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());

	activeParticleSlotCount_ = 0u;
	for (std::uint32_t particleIndex = 0u; particleIndex < maxParticleCount_; ++particleIndex)
	{
		if (IsAliveLifetime(lifetimes[particleIndex]))
		{
			activeParticleSlotCount_ = particleIndex + 1u;
		}
	}

	hasSpawnedInfiniteLifetimeInitialBatch_ = true;
	Tick(0.f);
}

void ParticleSystemBase::QueueBurstSpawn(std::uint32_t spawnCount)
{
	const std::uint64_t requestedBurstSpawnCount =
		static_cast<std::uint64_t>(queuedBurstSpawnCount_) + spawnCount;
	queuedBurstSpawnCount_ = static_cast<std::uint32_t>(
		(std::min)(requestedBurstSpawnCount, static_cast<std::uint64_t>(maxParticleCount_)));
}

void ParticleSystemBase::ClearAllParticles(bool resetSpawnSequence)
{
	if (!isInitialized_)
	{
		return;
	}

	spawnTimerAccumulator_ = 0.f;
	queuedBurstSpawnCount_ = 0u;
	activeParticleSlotCount_ = 0u;
	hasEmitterTransformHistory_ = false;
	previousEmitterTransformMatrix_ = renderTransformMatrix_;
	// Clearing particles should allow persistent emitters to seed again;
	// resetSpawnSequence only controls the random sequence counter.
	hasSpawnedInfiniteLifetimeInitialBatch_ = false;

	ClearParticleDataBuffers(resetSpawnSequence);
	ResetSimulationState();
}

void ParticleSystemBase::SetSpawnDesc(const GPUParticleSpawnDesc& spawnDesc)
{
	const GPUParticleSpawnDesc sanitizedSpawnDesc = SanitizeSpawnDesc(spawnDesc);
	const bool wasInfiniteLifetime = spawnDesc_.infiniteLifetime;
	if (AreSpawnDescsEqual(spawnDesc_, sanitizedSpawnDesc))
	{
		return;
	}

	spawnDesc_ = sanitizedSpawnDesc;
	spawnTimerAccumulator_ = 0.f;
	hasSpawnedInfiniteLifetimeInitialBatch_ = false;
	if ((wasInfiniteLifetime || spawnDesc_.infiniteLifetime) && isInitialized_)
	{
		ClearAllParticles(false);
	}
}

void ParticleSystemBase::SetRenderTransformMatrix(const Matrix& renderTransformMatrix)
{
	renderTransformMatrix_ = renderTransformMatrix;

	if (!hasEmitterTransformHistory_)
	{
		previousEmitterTransformMatrix_ = renderTransformMatrix_;
	}
}

std::string ParticleSystemBase::ResolveShaderPath(const std::string& relativeShaderPath)
{
#if defined(GOKNAR_BUILD_DEBUG)
	const std::string projectShaderPath = ContentDir + relativeShaderPath;
	if (DataEncryption::FileExists(projectShaderPath))
	{
		return projectShaderPath;
	}

	return EngineContentDir + relativeShaderPath;
#else
	return ContentDir + relativeShaderPath;
#endif
}

void ParticleSystemBase::BindSimulationBuffers() const
{
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, POSITION_BUFFER_BINDING_INDEX, particlePositionBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, VELOCITY_BUFFER_BINDING_INDEX, particleVelocityBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, COLOR_BUFFER_BINDING_INDEX, particleColorBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, LIFETIME_BUFFER_BINDING_INDEX, particleLifetimeBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, ALIVE_INDEX_BUFFER_BINDING_INDEX, aliveIndexBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, COUNTER_BUFFER_BINDING_INDEX, particleCounterBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, END_COLOR_BUFFER_BINDING_INDEX, particleEndColorBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, SIZE_BUFFER_BINDING_INDEX, particleSizeBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, ROTATION_BUFFER_BINDING_INDEX, particleRotationBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, ACCELERATION_BUFFER_BINDING_INDEX, particleAccelerationBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, ANGULAR_VELOCITY_BUFFER_BINDING_INDEX, particleAngularVelocityBufferId_);
}

void ParticleSystemBase::BindRenderBuffers() const
{
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, POSITION_BUFFER_BINDING_INDEX, particlePositionBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, VELOCITY_BUFFER_BINDING_INDEX, particleVelocityBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, COLOR_BUFFER_BINDING_INDEX, particleColorBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, LIFETIME_BUFFER_BINDING_INDEX, particleLifetimeBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, ALIVE_INDEX_BUFFER_BINDING_INDEX, aliveIndexBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, END_COLOR_BUFFER_BINDING_INDEX, particleEndColorBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, SIZE_BUFFER_BINDING_INDEX, particleSizeBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, ROTATION_BUFFER_BINDING_INDEX, particleRotationBufferId_);
}

void ParticleSystemBase::BindFinalizeBuffers() const
{
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, COUNTER_BUFFER_BINDING_INDEX, particleCounterBufferId_);
	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, DRAW_INDIRECT_BUFFER_BINDING_INDEX, drawIndirectBufferId_);
}

void ParticleSystemBase::ResetSimulationState() const
{
	if (!isInitialized_ || drawIndirectBufferId_ == 0)
	{
		return;
	}

	DispatchResetPass();
	GraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderStorage | GraphicsMemoryBarrier::Command);

	DispatchFinalizePass();
	GraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderStorage | GraphicsMemoryBarrier::Command);
}

void ParticleSystemBase::ApplyParticleStateToShader(Shader* shader) const
{
	if (!shader)
	{
		return;
	}

	shader->SetFloat(SHADER_VARIABLE_NAMES::PARTICLE::PARTICLE_SIZE, particleSize_);
	shader->SetVector2("particleSizeBySpeedRange", spawnDesc_.sizeBySpeedRange);
	shader->SetVector2("particleSizeBySpeedValues", Vector2(spawnDesc_.sizeBySpeed.startValue, spawnDesc_.sizeBySpeed.endValue));
	shader->SetVector2("particleColorBySpeedRange", spawnDesc_.colorBySpeedRange);
	shader->SetVector4("particleColorBySpeedStart", spawnDesc_.colorBySpeed.startValue);
	shader->SetVector4("particleColorBySpeedEnd", spawnDesc_.colorBySpeed.endValue);
	shader->SetVector3("particleEmissiveColorStart", spawnDesc_.emissiveColorByLifetime.startValue);
	shader->SetVector3("particleEmissiveColorEnd", spawnDesc_.emissiveColorByLifetime.endValue);
}

void ParticleSystemBase::ApplyMaterialStateToShader(Shader* shader, const IMaterialBase* material) const
{
	if (!shader || !engine || !engine->GetRenderer())
	{
		return;
	}

	shader->SetVector4(SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR, material ? material->GetBaseColor() : Vector4(1.f));
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION, material ? material->GetAmbientOcclusion() : 1.f);
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::METALLIC, material ? material->GetMetallic() : 0.f);
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::ROUGHNESS, material ? material->GetRoughness() : 0.5f);
	shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR, material ? material->GetEmissiveColor() : Vector3::ZeroVector);
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY, material ? material->GetTranslucency() : 0.f);

	engine->GetRenderer()->SetLightUniforms(shader);
	if (material && material->GetUsesReflectionProbe())
	{
		engine->GetRenderer()->SetReflectionProbeUniforms(shader);
	}
	else
	{
		shader->SetBool(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::HAS_REFLECTION_PROBE, false);
		shader->SetInt(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP, 0);
	}
}

std::uint32_t ParticleSystemBase::ConsumeRequestedSpawnCount(float deltaTime)
{
	std::uint64_t requestedSpawnCount = queuedBurstSpawnCount_;
	queuedBurstSpawnCount_ = 0u;

	if (spawnDesc_.infiniteLifetime)
	{
		if (!hasSpawnedInfiniteLifetimeInitialBatch_ && 0u < spawnDesc_.spawnCountPerInterval)
		{
			requestedSpawnCount += spawnDesc_.spawnCountPerInterval;
			hasSpawnedInfiniteLifetimeInitialBatch_ = true;
		}
	}
	else if (spawnDesc_.looping && 0u < spawnDesc_.spawnCountPerInterval)
	{
		spawnTimerAccumulator_ += (std::max)(0.f, deltaTime);
		if (spawnDesc_.spawnInterval <= 0.f)
		{
			requestedSpawnCount += spawnDesc_.spawnCountPerInterval;
		}
		else
		{
			const std::uint32_t intervalCount = static_cast<std::uint32_t>(std::floor(spawnTimerAccumulator_ / spawnDesc_.spawnInterval));
			if (0u < intervalCount)
			{
				spawnTimerAccumulator_ -= static_cast<float>(intervalCount) * spawnDesc_.spawnInterval;
				requestedSpawnCount += static_cast<std::uint64_t>(intervalCount) * spawnDesc_.spawnCountPerInterval;
			}
		}
	}

	return static_cast<std::uint32_t>((std::min)(requestedSpawnCount, static_cast<std::uint64_t>(maxParticleCount_)));
}

void ParticleSystemBase::ClearParticleDataBuffers(bool resetSpawnSequence)
{
	const std::vector<Vector4> zeroVector4Data(maxParticleCount_, Vector4(0.f));
	const std::vector<GPUParticleLifetimeState> zeroLifetimeData(maxParticleCount_, GPUParticleLifetimeState{});
	const std::vector<GPUParticleSizeState> zeroSizeData(maxParticleCount_, GPUParticleSizeState{});
	const std::vector<std::uint32_t> zeroIndexData(maxParticleCount_, 0u);

	GraphicsAPI()->NamedBufferSubData(particlePositionBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleVelocityBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleColorBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleLifetimeBufferId_, 0, static_cast<GEsizeiptr>(zeroLifetimeData.size() * sizeof(GPUParticleLifetimeState)), zeroLifetimeData.data());
	GraphicsAPI()->NamedBufferSubData(aliveIndexBufferId_, 0, static_cast<GEsizeiptr>(zeroIndexData.size() * sizeof(std::uint32_t)), zeroIndexData.data());
	GraphicsAPI()->NamedBufferSubData(particleEndColorBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleSizeBufferId_, 0, static_cast<GEsizeiptr>(zeroSizeData.size() * sizeof(GPUParticleSizeState)), zeroSizeData.data());
	GraphicsAPI()->NamedBufferSubData(particleRotationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleAccelerationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	GraphicsAPI()->NamedBufferSubData(particleAngularVelocityBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());

	if (resetSpawnSequence)
	{
		const GPUParticleCounterState zeroCounterState{};
		GraphicsAPI()->NamedBufferSubData(particleCounterBufferId_, 0, sizeof(GPUParticleCounterState), &zeroCounterState);
	}
}

void ParticleSystemBase::CreateBuffers()
{
	const std::vector<Vector4> zeroVector4Data(maxParticleCount_, Vector4(0.f));
	const std::vector<GPUParticleLifetimeState> zeroLifetimeData(maxParticleCount_, GPUParticleLifetimeState{});
	const std::vector<GPUParticleSizeState> zeroSizeData(maxParticleCount_, GPUParticleSizeState{});
	const std::vector<std::uint32_t> zeroIndexData(maxParticleCount_, 0u);
	const GPUParticleCounterState zeroCounterState{};

	auto createStorageBuffer = [](GEuint& bufferId, GEsizeiptr size, const void* data)
	{
		bufferId = GraphicsAPI()->CreateBuffer();
		GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, bufferId);
		GraphicsAPI()->BufferData(GraphicsBufferTarget::ShaderStorageBuffer, size, data, GraphicsBufferUsage::DynamicDraw);
	};

	createStorageBuffer(particlePositionBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data());
	createStorageBuffer(particleVelocityBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data());
	createStorageBuffer(particleColorBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data());
	createStorageBuffer(particleLifetimeBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(GPUParticleLifetimeState)), zeroLifetimeData.data());
	createStorageBuffer(aliveIndexBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(std::uint32_t)), zeroIndexData.data());
	createStorageBuffer(particleCounterBufferId_, sizeof(GPUParticleCounterState), &zeroCounterState);
	createStorageBuffer(drawIndirectBufferId_, sizeof(GPUParticleDrawArraysIndirectCommand), nullptr);
	createStorageBuffer(particleEndColorBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data());
	createStorageBuffer(particleSizeBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(GPUParticleSizeState)), zeroSizeData.data());
	createStorageBuffer(particleRotationBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data());
	createStorageBuffer(particleAccelerationBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data());
	createStorageBuffer(particleAngularVelocityBufferId_, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data());

	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0);

	dummyVertexArrayObjectId_ = GraphicsAPI()->CreateVertexArray();
}

void ParticleSystemBase::DestroyBuffers()
{
	if (dummyVertexArrayObjectId_ != 0)
	{
		GraphicsAPI()->DeleteVertexArray(dummyVertexArrayObjectId_);
		dummyVertexArrayObjectId_ = 0;
	}

	if (particlePositionBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particlePositionBufferId_);
		particlePositionBufferId_ = 0;
	}

	if (particleVelocityBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleVelocityBufferId_);
		particleVelocityBufferId_ = 0;
	}

	if (particleColorBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleColorBufferId_);
		particleColorBufferId_ = 0;
	}

	if (particleLifetimeBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleLifetimeBufferId_);
		particleLifetimeBufferId_ = 0;
	}

	if (aliveIndexBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(aliveIndexBufferId_);
		aliveIndexBufferId_ = 0;
	}

	if (particleCounterBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleCounterBufferId_);
		particleCounterBufferId_ = 0;
	}

	if (drawIndirectBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(drawIndirectBufferId_);
		drawIndirectBufferId_ = 0;
	}

	if (particleEndColorBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleEndColorBufferId_);
		particleEndColorBufferId_ = 0;
	}

	if (particleSizeBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleSizeBufferId_);
		particleSizeBufferId_ = 0;
	}

	if (particleRotationBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleRotationBufferId_);
		particleRotationBufferId_ = 0;
	}

	if (particleAccelerationBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleAccelerationBufferId_);
		particleAccelerationBufferId_ = 0;
	}

	if (particleAngularVelocityBufferId_ != 0)
	{
		GraphicsAPI()->DeleteBuffer(particleAngularVelocityBufferId_);
		particleAngularVelocityBufferId_ = 0;
	}
}

void ParticleSystemBase::CreateComputeShaders()
{
	resetComputeShader_ = new ComputeShader();
	resetComputeShader_->SetComputeShaderPathAbsolute(ResolveShaderPath("Shaders/ParticleSystem/GPUDrivenParticleSystemReset.comp"));
	resetComputeShader_->PreInit();
	resetComputeShader_->Init();

	updateComputeShader_ = new ComputeShader();
	updateComputeShader_->SetComputeShaderPathAbsolute(ResolveShaderPath("Shaders/ParticleSystem/GPUDrivenParticleSystemUpdate.comp"));
	updateComputeShader_->PreInit();
	updateComputeShader_->Init();

	finalizeDrawArraysComputeShader_ = new ComputeShader();
	finalizeDrawArraysComputeShader_->SetComputeShaderPathAbsolute(ResolveShaderPath("Shaders/ParticleSystem/GPUDrivenParticleSystemFinalizeDrawArrays.comp"));
	finalizeDrawArraysComputeShader_->PreInit();
	finalizeDrawArraysComputeShader_->Init();

	finalizeDrawElementsComputeShader_ = new ComputeShader();
	finalizeDrawElementsComputeShader_->SetComputeShaderPathAbsolute(ResolveShaderPath("Shaders/ParticleSystem/GPUDrivenParticleSystemFinalizeDrawElements.comp"));
	finalizeDrawElementsComputeShader_->PreInit();
	finalizeDrawElementsComputeShader_->Init();
}

void ParticleSystemBase::DestroyComputeShaders()
{
	delete resetComputeShader_;
	resetComputeShader_ = nullptr;

	delete updateComputeShader_;
	updateComputeShader_ = nullptr;

	delete finalizeDrawArraysComputeShader_;
	finalizeDrawArraysComputeShader_ = nullptr;

	delete finalizeDrawElementsComputeShader_;
	finalizeDrawElementsComputeShader_ = nullptr;
}

void ParticleSystemBase::DispatchResetPass() const
{
	if (!resetComputeShader_)
	{
		return;
	}

	BindSimulationBuffers();
	resetComputeShader_->Dispatch(1u, 1u, 1u);
}
