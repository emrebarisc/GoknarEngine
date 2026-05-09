#include "pch.h"

#include "ParticleSystemBase.h"

#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Renderer/ComputeShader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderTypes.h"

#include <algorithm>
#include <cmath>

#include <glad/glad.h>

namespace
{
	constexpr float kMinimumLifetime = 0.0001f;
	constexpr float kMinimumSpawnInterval = 0.0001f;

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

	GPUParticleSpawnDesc SanitizeSpawnDesc(const GPUParticleSpawnDesc& spawnDesc)
	{
		GPUParticleSpawnDesc sanitizedDesc(spawnDesc);
		sanitizedDesc.lifetime = SanitizeFloatRange(sanitizedDesc.lifetime, kMinimumLifetime);
		sanitizedDesc.initialVelocity = SanitizeVector3Range(sanitizedDesc.initialVelocity);
		sanitizedDesc.initialRotation = SanitizeVector3Range(sanitizedDesc.initialRotation);
		sanitizedDesc.angularVelocity = SanitizeVector3Range(sanitizedDesc.angularVelocity);
		sanitizedDesc.acceleration = SanitizeVector3Range(sanitizedDesc.acceleration);
		sanitizedDesc.velocityLimit = (std::max)(0.f, sanitizedDesc.velocityLimit);
		sanitizedDesc.sizeByLifetime = SanitizeFloatCurve(sanitizedDesc.sizeByLifetime, 0.f);
		sanitizedDesc.sizeBySpeedRange = SanitizeRangeVector(sanitizedDesc.sizeBySpeedRange);
		sanitizedDesc.sizeBySpeed = SanitizeFloatCurve(sanitizedDesc.sizeBySpeed, 0.f);
		sanitizedDesc.colorByLifetime = SanitizeColorCurve(sanitizedDesc.colorByLifetime);
		sanitizedDesc.colorBySpeedRange = SanitizeRangeVector(sanitizedDesc.colorBySpeedRange);
		sanitizedDesc.colorBySpeed = SanitizeColorCurve(sanitizedDesc.colorBySpeed);
		sanitizedDesc.spawnBoxExtents.x = (std::max)(0.f, sanitizedDesc.spawnBoxExtents.x);
		sanitizedDesc.spawnBoxExtents.y = (std::max)(0.f, sanitizedDesc.spawnBoxExtents.y);
		sanitizedDesc.spawnBoxExtents.z = (std::max)(0.f, sanitizedDesc.spawnBoxExtents.z);
		sanitizedDesc.spawnInterval = (std::max)(kMinimumSpawnInterval, sanitizedDesc.spawnInterval);
		sanitizedDesc.spawnCountPerInterval = (std::max)(1u, sanitizedDesc.spawnCountPerInterval);
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
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

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
	updateComputeShader_->SetVector2("sizeByLifetime", Vector2(spawnDesc_.sizeByLifetime.startValue, spawnDesc_.sizeByLifetime.endValue));
	updateComputeShader_->SetVector3("spawnBoxExtents", spawnDesc_.spawnBoxExtents);
	updateComputeShader_->SetVector4("colorByLifetimeStart", spawnDesc_.colorByLifetime.startValue);
	updateComputeShader_->SetVector4("colorByLifetimeEnd", spawnDesc_.colorByLifetime.endValue);
	updateComputeShader_->SetMatrix("previousEmitterMatrix", previousEmitterTransformMatrix_);
	updateComputeShader_->SetMatrix("currentEmitterMatrix", renderTransformMatrix_);

	const GEuint groupCountX = (dispatchParticleCount + kComputeLocalSizeX - 1u) / kComputeLocalSizeX;
	updateComputeShader_->Dispatch(groupCountX, 1u, 1u);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	DispatchFinalizePass();

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

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

	glNamedBufferSubData(particlePositionBufferId_, 0, static_cast<GEsizeiptr>(positions.size() * sizeof(Vector4)), positions.data());
	glNamedBufferSubData(particleVelocityBufferId_, 0, static_cast<GEsizeiptr>(velocities.size() * sizeof(Vector4)), velocities.data());
	glNamedBufferSubData(particleColorBufferId_, 0, static_cast<GEsizeiptr>(colors.size() * sizeof(Vector4)), colors.data());
	glNamedBufferSubData(particleLifetimeBufferId_, 0, static_cast<GEsizeiptr>(lifetimes.size() * sizeof(GPUParticleLifetimeState)), lifetimes.data());
	glNamedBufferSubData(particleEndColorBufferId_, 0, static_cast<GEsizeiptr>(endColors.size() * sizeof(Vector4)), endColors.data());
	glNamedBufferSubData(particleSizeBufferId_, 0, static_cast<GEsizeiptr>(sizes.size() * sizeof(GPUParticleSizeState)), sizes.data());
	glNamedBufferSubData(particleRotationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleAccelerationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleAngularVelocityBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());

	activeParticleSlotCount_ = 0u;
	for (std::uint32_t particleIndex = 0u; particleIndex < maxParticleCount_; ++particleIndex)
	{
		if (0.f < lifetimes[particleIndex].remainingLifetime)
		{
			activeParticleSlotCount_ = particleIndex + 1u;
		}
	}

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

	ClearParticleDataBuffers(resetSpawnSequence);
	ResetSimulationState();
}

void ParticleSystemBase::SetSpawnDesc(const GPUParticleSpawnDesc& spawnDesc)
{
	spawnDesc_ = SanitizeSpawnDesc(spawnDesc);
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kPositionBufferBindingIndex, particlePositionBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kVelocityBufferBindingIndex, particleVelocityBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kColorBufferBindingIndex, particleColorBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kLifetimeBufferBindingIndex, particleLifetimeBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kAliveIndexBufferBindingIndex, aliveIndexBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kCounterBufferBindingIndex, particleCounterBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kEndColorBufferBindingIndex, particleEndColorBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kSizeBufferBindingIndex, particleSizeBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kRotationBufferBindingIndex, particleRotationBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kAccelerationBufferBindingIndex, particleAccelerationBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kAngularVelocityBufferBindingIndex, particleAngularVelocityBufferId_);
}

void ParticleSystemBase::BindRenderBuffers() const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kPositionBufferBindingIndex, particlePositionBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kVelocityBufferBindingIndex, particleVelocityBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kColorBufferBindingIndex, particleColorBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kLifetimeBufferBindingIndex, particleLifetimeBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kAliveIndexBufferBindingIndex, aliveIndexBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kEndColorBufferBindingIndex, particleEndColorBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kSizeBufferBindingIndex, particleSizeBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kRotationBufferBindingIndex, particleRotationBufferId_);
}

void ParticleSystemBase::BindFinalizeBuffers() const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kCounterBufferBindingIndex, particleCounterBufferId_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kDrawIndirectBufferBindingIndex, drawIndirectBufferId_);
}

void ParticleSystemBase::ResetSimulationState() const
{
	if (!isInitialized_ || drawIndirectBufferId_ == 0)
	{
		return;
	}

	DispatchResetPass();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

	DispatchFinalizePass();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
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
	shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR, material ? material->GetEmisiveColor() : Vector3::ZeroVector);
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

	if (spawnDesc_.looping && 0u < spawnDesc_.spawnCountPerInterval)
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

	glNamedBufferSubData(particlePositionBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleVelocityBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleColorBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleLifetimeBufferId_, 0, static_cast<GEsizeiptr>(zeroLifetimeData.size() * sizeof(GPUParticleLifetimeState)), zeroLifetimeData.data());
	glNamedBufferSubData(aliveIndexBufferId_, 0, static_cast<GEsizeiptr>(zeroIndexData.size() * sizeof(std::uint32_t)), zeroIndexData.data());
	glNamedBufferSubData(particleEndColorBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleSizeBufferId_, 0, static_cast<GEsizeiptr>(zeroSizeData.size() * sizeof(GPUParticleSizeState)), zeroSizeData.data());
	glNamedBufferSubData(particleRotationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleAccelerationBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());
	glNamedBufferSubData(particleAngularVelocityBufferId_, 0, static_cast<GEsizeiptr>(zeroVector4Data.size() * sizeof(Vector4)), zeroVector4Data.data());

	if (resetSpawnSequence)
	{
		const GPUParticleCounterState zeroCounterState{};
		glNamedBufferSubData(particleCounterBufferId_, 0, sizeof(GPUParticleCounterState), &zeroCounterState);
	}
}

void ParticleSystemBase::CreateBuffers()
{
	const std::vector<Vector4> zeroVector4Data(maxParticleCount_, Vector4(0.f));
	const std::vector<GPUParticleLifetimeState> zeroLifetimeData(maxParticleCount_, GPUParticleLifetimeState{});
	const std::vector<GPUParticleSizeState> zeroSizeData(maxParticleCount_, GPUParticleSizeState{});
	const std::vector<std::uint32_t> zeroIndexData(maxParticleCount_, 0u);
	const GPUParticleCounterState zeroCounterState{};

	glGenBuffers(1, &particlePositionBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePositionBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleVelocityBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleVelocityBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleColorBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleColorBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleLifetimeBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleLifetimeBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(GPUParticleLifetimeState)), zeroLifetimeData.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &aliveIndexBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, aliveIndexBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(std::uint32_t)), zeroIndexData.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleCounterBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleCounterBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUParticleCounterState), &zeroCounterState, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &drawIndirectBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawIndirectBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUParticleDrawArraysIndirectCommand), nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleEndColorBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleEndColorBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleSizeBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSizeBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(GPUParticleSizeState)), zeroSizeData.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleRotationBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleRotationBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleAccelerationBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleAccelerationBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &particleAngularVelocityBufferId_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleAngularVelocityBufferId_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GEsizeiptr>(maxParticleCount_ * sizeof(Vector4)), zeroVector4Data.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenVertexArrays(1, &dummyVertexArrayObjectId_);
}

void ParticleSystemBase::DestroyBuffers()
{
	if (dummyVertexArrayObjectId_ != 0)
	{
		glDeleteVertexArrays(1, &dummyVertexArrayObjectId_);
		dummyVertexArrayObjectId_ = 0;
	}

	if (particlePositionBufferId_ != 0)
	{
		glDeleteBuffers(1, &particlePositionBufferId_);
		particlePositionBufferId_ = 0;
	}

	if (particleVelocityBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleVelocityBufferId_);
		particleVelocityBufferId_ = 0;
	}

	if (particleColorBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleColorBufferId_);
		particleColorBufferId_ = 0;
	}

	if (particleLifetimeBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleLifetimeBufferId_);
		particleLifetimeBufferId_ = 0;
	}

	if (aliveIndexBufferId_ != 0)
	{
		glDeleteBuffers(1, &aliveIndexBufferId_);
		aliveIndexBufferId_ = 0;
	}

	if (particleCounterBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleCounterBufferId_);
		particleCounterBufferId_ = 0;
	}

	if (drawIndirectBufferId_ != 0)
	{
		glDeleteBuffers(1, &drawIndirectBufferId_);
		drawIndirectBufferId_ = 0;
	}

	if (particleEndColorBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleEndColorBufferId_);
		particleEndColorBufferId_ = 0;
	}

	if (particleSizeBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleSizeBufferId_);
		particleSizeBufferId_ = 0;
	}

	if (particleRotationBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleRotationBufferId_);
		particleRotationBufferId_ = 0;
	}

	if (particleAccelerationBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleAccelerationBufferId_);
		particleAccelerationBufferId_ = 0;
	}

	if (particleAngularVelocityBufferId_ != 0)
	{
		glDeleteBuffers(1, &particleAngularVelocityBufferId_);
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
