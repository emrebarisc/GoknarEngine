#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Types.h"

#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Delegates/Delegate.h"

#include <unordered_map>
#include <vector>

class DynamicMesh;
class InstancedStaticMesh;
class StaticMesh;
class SkeletalMesh;
class LightManager;
class ParticleSystemBase;
class GPUFoliageSystem;

class Texture;
class FrameBuffer;
class RenderBuffer;
class ReflectionProbe;
class Shader;
class BloomPostProcessingEffect;
class GammaCorrectionPostProcessingEffect;
class TemporalAntiAliasingPostProcessingEffect;
class ScreenSpaceReflectionPostProcessingEffect;

class DynamicMeshInstance;
class InstancedStaticMeshInstance;
class StaticMeshInstance;
class SkeletalMeshInstance;

class PostProcessingEffect;

class RenderTarget;

class DynamicMeshUnit;
class SkeletalMeshUnit;

template<class T>
class MeshContainer;

enum class GOKNAR_API RenderPassType : unsigned int
{
	None = 0b00000000,
	Forward = 0b00000001,
	Shadow = 0b00000010,

	// Needed for rendering point lights with geometry shaders
	PointLightShadow = 0b00000100,
	CubemapCapture = 0b00001000,
	GeometryBuffer = 0b00010000,
	Deferred = 0b00100000
};

class GOKNAR_API GeometryBufferData
{
public:
	GeometryBufferData();
	virtual ~GeometryBufferData();

	void Init();
	void Bind();
	void Unbind();

	void GenerateBuffers();
	void BindGBufferDepth(FrameBuffer* drawFrameBuffer);

	void OnViewportSizeChanged(int width, int height);

	FrameBuffer* geometryFrameBuffer{ nullptr };

	RenderBuffer* depthRenderbuffer{ nullptr };

	Texture* worldPositionTexture{ nullptr };
	Texture* worldNormalTexture{ nullptr };
	Texture* diffuseTexture{ nullptr };
	Texture* ambientOcclusionMetallicRoughnessTexture{ nullptr };
	Texture* emissiveColorTexture{ nullptr };

	int bufferWidth{ 1920 };
	int bufferHeight{ 1080 };
private:
};

class GOKNAR_API DeferredRenderingData
{
public:
	DeferredRenderingData();
	~DeferredRenderingData();

	void PreInit();
	void Init();
	void BindGeometryBuffer();
	void UnbindGeometryBuffer();

	void SetShaderTextureUniforms();

	void BindGBufferDepth(FrameBuffer* drawFrameBuffer);

	void BeginSceneRender();
	void EndSceneRender();
	void BlitSceneTo(FrameBuffer* drawFrameBuffer) const;

	Texture* GetSceneTexture() const
	{
		return sceneTexture;
	}

	FrameBuffer* GetSceneFrameBuffer() const
	{
		return sceneFrameBuffer;
	}

	void Render();

	void OnViewportSizeChanged(int width, int height);
	void BindGeometryBufferTextures(Shader* shader);

	GeometryBufferData* geometryBufferData{ nullptr };
	StaticMesh* deferredRenderingMesh{ nullptr };
	Shader* deferredRenderingMeshShader{ nullptr };
	Texture* sceneTexture{ nullptr };
	FrameBuffer* sceneFrameBuffer{ nullptr };
	RenderBuffer* sceneDepthRenderbuffer{ nullptr };

private:
	void GenerateSceneBuffers();
	void DestroySceneBuffers();
};

class GOKNAR_API Renderer
{
	friend DeferredRenderingData;

public:
	Renderer();
	~Renderer();

	void SetStaticBufferData();
	void SetSkeletalBufferData();
	void SetDynamicBufferData();
	void SetBufferData();

	void PreInit();
	void Init();
	void PostInit();

	void RenderCurrentFrame();
	void Render(RenderPassType renderPassType);

	void AddInstancedStaticMeshToRenderer(InstancedStaticMesh* object);
	void AddStaticMeshToRenderer(StaticMesh* object);
	void AddStaticMeshInstance(StaticMeshInstance* object);
	void RemoveStaticMeshInstance(StaticMeshInstance* object);
	void AddInstancedStaticMeshInstance(InstancedStaticMeshInstance* object);
	void RemoveInstancedStaticMeshInstance(InstancedStaticMeshInstance* object);

	void AddSkeletalMeshToRenderer(SkeletalMesh* object);
	void AddSkeletalMeshInstance(SkeletalMeshInstance* object);
	void RemoveSkeletalMeshInstance(SkeletalMeshInstance* object);

	void AddDynamicMeshToRenderer(DynamicMesh* object);
	void AddDynamicMeshInstance(DynamicMeshInstance* object);
	void RemoveDynamicMeshInstance(DynamicMeshInstance* object);
	void AddGPUFoliageSystem(GPUFoliageSystem* foliageSystem);
	void RemoveGPUFoliageSystem(GPUFoliageSystem* foliageSystem);
	void AddParticleSystem(ParticleSystemBase* particleSystem);
	void RemoveParticleSystem(ParticleSystemBase* particleSystem);

	void UpdateDynamicMeshVertex(const DynamicMeshUnit* object, int vertexIndex, const VertexData& newVertexData);
	void RefreshInstancedStaticMeshTransformations(const InstancedStaticMesh* object);
	void UpdateInstancedStaticMeshTransformation(const InstancedStaticMesh* object, int transformationIndex, const Matrix& newTransformationMatrix);

	void PrepareSkeletalMeshInstancesForTheCurrentFrame();
	void PrepareSkeletalMeshInstancesForTheNextFrame();

	LightManager* GetLightManager()
	{
		return lightManager_;
	}

	void BindShadowTextures(Shader* shader);
	void BindGeometryBufferTextures(Shader* shader);
	void SetLightUniforms(Shader* shader);
	void SetReflectionProbeUniforms(Shader* shader) const;
	void SetCubemapRenderPassShaderUniforms(const Shader* shader) const;
	void CaptureReflectionProbes();
	void BeginReflectionProbeCapture(const ReflectionProbe* reflectionProbe)
	{
		currentReflectionProbeCapture_ = reflectionProbe;
	}

	void EndReflectionProbeCapture()
	{
		currentReflectionProbeCapture_ = nullptr;
	}

	void SetMainRenderType(RenderPassType type)
	{
		mainRenderType_ = type;
	}

	RenderPassType GetMainRenderType() const
	{
		return mainRenderType_;
	}

	inline int GetMainRenderTypeInt() const
	{
		return (int)mainRenderType_;
	}

	DeferredRenderingData* GetDeferredRenderingData()
	{
		return deferredRenderingData_;
	}

	void AddPostProcessingEffect(PostProcessingEffect* postProcessingEffect);
	void RemovePostProcessingEffect(PostProcessingEffect* postProcessingEffect);

	void AddRenderTarget(const RenderTarget* renderTarget);
	void RemoveRenderTarget(const RenderTarget* renderTarget);

	void RenderStaticMesh(StaticMesh* staticMesh);
	void BindStaticMeshBuffers();

	void SetDrawOnWindow(bool drawOnWindow)
	{
		drawOnWindow_ = drawOnWindow;
	}

	bool GetDrawOnWindow() const
	{
		return drawOnWindow_;
	}

	const ReflectionProbe* GetClosestReflectionProbe(const Vector3& worldPosition) const;

	GammaCorrectionPostProcessingEffect* GetGammaCorrectionPostProcessingEffect() const
	{
		return gammaCorrectionPostProcessingEffect_;
	}

	int drawCallCount{ 0 };
	bool countDrawCalls{ false };

private:
	struct StaticMeshRenderData
	{
		StaticMeshInstance* meshInstance{ nullptr };
		MeshContainer<StaticMesh>* meshUnit{ nullptr };
		int subMeshIndex{ 0 };
	};

	struct InstancedStaticMeshRenderData
	{
		InstancedStaticMeshInstance* meshInstance{ nullptr };
		MeshContainer<InstancedStaticMesh>* meshUnit{ nullptr };
		int subMeshIndex{ 0 };
	};

	struct SkeletalMeshRenderData
	{
		SkeletalMeshInstance* meshInstance{ nullptr };
		MeshContainer<SkeletalMesh>* meshUnit{ nullptr };
		int subMeshIndex{ 0 };
	};

	struct DynamicMeshRenderData
	{
		DynamicMeshInstance* meshInstance{ nullptr };
		MeshContainer<DynamicMesh>* meshUnit{ nullptr };
		GEint subMeshIndex{ 0 };
	};

	struct MeshBufferData
	{
		GEuint vertexArrayId{ 0 };
		GEuint vertexBufferId{ 0 };
		GEuint indexBufferId{ 0 };

		GEuint vertexSize{ 0 };
		GEuint faceSize{ 0 };

		GEuint baseVertex{ 0 };
		GEuint vertexStartingIndex{ 0 };

		GEint vertexOffset{ 0 };
		GEint faceOffset{ 0 };

		GEint meshCount{ 0 };
	};

	void BindStaticVAO();
	bool BindInstancedStaticMesh(InstancedStaticMesh* instancedStaticMesh);
	void BindSkeletalVAO();
	void BindDynamicVAO();
	void SetAttribPointers();
	void SetAttribPointersForInstancedStaticMesh();
	void SetAttribPointersForSkeletalMesh();
	void ApplyPostProcessing(DeferredRenderingData* deferredRenderingData, FrameBuffer* destinationFrameBuffer);

	void SortOpaqueInstances();
	void SortTransparentInstances();

	std::unordered_map<const InstancedStaticMesh*, GEuint> instancedStaticMeshTransformationBufferIdMap_;
	std::unordered_map<const InstancedStaticMesh*, GEuint> instancedStaticMeshVertexArrayIdMap_;

	std::vector<MeshUnit*> staticMeshUnits_;
	std::vector<InstancedStaticMesh*> instancedStaticMeshes_;
	std::vector<SkeletalMeshUnit*> skeletalMeshUnits_;
	std::vector<DynamicMeshUnit*> dynamicMeshUnits_;

	std::vector<StaticMeshRenderData> opaqueStaticMeshRenderData_;
	std::vector<StaticMeshRenderData> transparentStaticMeshRenderData_;
	std::vector<InstancedStaticMeshRenderData> opaqueInstancedStaticMeshRenderData_;
	std::vector<InstancedStaticMeshRenderData> transparentInstancedStaticMeshRenderData_;

	std::vector<SkeletalMeshRenderData> opaqueSkeletalMeshRenderData_;
	std::vector<SkeletalMeshRenderData> transparentSkeletalMeshRenderData_;

	std::vector<DynamicMeshRenderData> opaqueDynamicMeshRenderData_;
	std::vector<DynamicMeshRenderData> transparentDynamicMeshRenderData_;
	std::vector<GPUFoliageSystem*> gpuFoliageSystems_;
	std::vector<ParticleSystemBase*> particleSystems_;

	MeshBufferData staticMeshBufferData_;
	MeshBufferData skeletalMeshBufferData_;
	MeshBufferData dynamicMeshBufferData_;

	LightManager* lightManager_{ nullptr };

	DeferredRenderingData* deferredRenderingData_{ nullptr };

	std::vector<PostProcessingEffect*> postProcessingEffects_;
	std::vector<const RenderTarget*> renderTargets_;
	TemporalAntiAliasingPostProcessingEffect* temporalAntiAliasingPostProcessingEffect_{ nullptr };
	BloomPostProcessingEffect* bloomPostProcessingEffect_{ nullptr };
	GammaCorrectionPostProcessingEffect* gammaCorrectionPostProcessingEffect_{ nullptr };
	ScreenSpaceReflectionPostProcessingEffect* screenSpaceReflectionPostProcessingEffect_{ nullptr };
	Delegate<void(int, int)> deferredWindowSizeChangedDelegate_{};

	const RenderTarget* currentRenderTarget_{ nullptr };
	const ReflectionProbe* currentReflectionProbeCapture_{ nullptr };

	RenderPassType mainRenderType_{ RenderPassType::Deferred };

	unsigned char removeStaticDataFromMemoryAfterTransferingToGPU_ : 1;
	unsigned char drawOnWindow_ : 1;
	unsigned char countDrawCallsInner_ : 1;
};

#endif
