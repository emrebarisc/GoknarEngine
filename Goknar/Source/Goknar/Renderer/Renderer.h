#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Types.h"

#include "Goknar/Model/MeshUnit.h"

#include "glad/glad.h"

#include <unordered_map>
#include <vector>

class DynamicMesh;
class InstancedStaticMesh;
class StaticMesh;
class SkeletalMesh;
class LightManager;

class Texture;
class FrameBuffer;
class RenderBuffer;
class Shader;
class BloomPostProcessingEffect;
class TemporalAntiAliasingPostProcessingEffect;

class DynamicMeshInstance;
class InstancedStaticMeshInstance;
class StaticMeshInstance;
class SkeletalMeshInstance;

class PostProcessingEffect;

class RenderTarget;

class DynamicMeshUnit;
class SkeletalMeshUnit;

enum class GOKNAR_API RenderPassType : unsigned int
{
	None = 0b00000000,
	Forward = 0b00000001,
	Shadow = 0b00000010,

	// Needed for rendering point lights with geometry shaders
	PointLightShadow,
	GeometryBuffer = 0b00000100,
	Deferred = 0b00001000
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
	Texture* emisiveColorTexture{ nullptr };

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

	void SetMainRenderType(RenderPassType type)
	{
		mainRenderType_ = type;
	}

	RenderPassType GetMainRenderType() const
	{
		return mainRenderType_;
	}

	DeferredRenderingData* GetDeferredRenderingData()
	{
		return deferredRenderingData_;
	}

	void AddPostProcessingEffect(PostProcessingEffect* postProcessingEffect)
	{
		postProcessingEffects_.push_back(postProcessingEffect);
	}

	void RemovePostProcessingEffect(PostProcessingEffect* postProcessingEffect)
	{
		std::vector<PostProcessingEffect*>::const_iterator postProcessingEffectIterator = postProcessingEffects_.cbegin();
		while (postProcessingEffectIterator != postProcessingEffects_.cend())
		{
			if (postProcessingEffect == *postProcessingEffectIterator)
			{
				postProcessingEffects_.erase(postProcessingEffectIterator);
				break;
			}
		}
	}

	void AddRenderTarget(const RenderTarget* renderTarget)
	{
		renderTargets_.push_back(renderTarget);
	}

	void RemoveRenderTarget(const RenderTarget* renderTarget)
	{
		std::vector<const RenderTarget*>::const_iterator renderTargetIterator = renderTargets_.cbegin();
		while (renderTargetIterator != renderTargets_.cend())
		{
			if (renderTarget == *renderTargetIterator)
			{
				renderTargets_.erase(renderTargetIterator);
				break;
			}
		}
	}

	void RenderStaticMesh(StaticMesh* staticMesh);

	void SetDrawOnWindow(bool drawOnWindow)
	{
		drawOnWindow_ = drawOnWindow;
	}

	bool GetDrawOnWindow() const
	{
		return drawOnWindow_;
	}

	int drawCallCount{ 0 };
	bool countDrawCalls{ false };

private:
	struct StaticMeshRenderData
	{
		StaticMeshInstance* meshInstance{ nullptr };
		MeshUnit* meshUnit{ nullptr };
		int subMeshIndex{ 0 };
	};

	struct InstancedStaticMeshRenderData
	{
		InstancedStaticMeshInstance* meshInstance{ nullptr };
		MeshUnit* meshUnit{ nullptr };
		int subMeshIndex{ 0 };
	};

	struct SkeletalMeshRenderData
	{
		SkeletalMeshInstance* meshInstance{ nullptr };
		SkeletalMeshUnit* meshUnit{ nullptr };
		int subMeshIndex{ 0 };
	};

	struct DynamicMeshRenderData
	{
		DynamicMeshInstance* meshInstance{ nullptr };
		DynamicMeshUnit* meshUnit{ nullptr };
		int subMeshIndex{ 0 };
	};

	void BindStaticVBO();
	bool BindInstancedStaticMesh(InstancedStaticMesh* instancedStaticMesh);
	void BindSkeletalVBO();
	void BindDynamicVBO();
	void SetAttribPointers();
	void SetAttribPointersForInstancedStaticMesh();
	void SetAttribPointersForSkeletalMesh();
	void ApplyPostProcessing(DeferredRenderingData* deferredRenderingData, FrameBuffer* destinationFrameBuffer);

	void SortTransparentInstances();

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

	LightManager* lightManager_{ nullptr };

	DeferredRenderingData* deferredRenderingData_{ nullptr };

	std::vector<PostProcessingEffect*> postProcessingEffects_;
	std::vector<const RenderTarget*> renderTargets_;
	TemporalAntiAliasingPostProcessingEffect* temporalAntiAliasingPostProcessingEffect_{ nullptr };
	BloomPostProcessingEffect* bloomPostProcessingEffect_{ nullptr };

	const RenderTarget* currentRenderTarget_{ nullptr };

	unsigned int totalStaticMeshVertexSize_;
	unsigned int totalStaticMeshFaceSize_;

	unsigned int totalSkeletalMeshVertexSize_;
	unsigned int totalSkeletalMeshFaceSize_;

	unsigned int totalDynamicMeshVertexSize_;
	unsigned int totalDynamicMeshFaceSize_;

	int totalStaticMeshCount_;
	int totalSkeletalMeshCount_;
	int totalDynamicMeshCount_;

	GEuint staticVertexBufferId_;
	GEuint staticIndexBufferId_;
	std::unordered_map<const InstancedStaticMesh*, GEuint> instancedStaticMeshTransformationBufferIdMap_;

	GEuint skeletalVertexBufferId_;
	GEuint skeletalIndexBufferId_;

	GEuint dynamicVertexBufferId_;
	GEuint dynamicIndexBufferId_;

	RenderPassType mainRenderType_{ RenderPassType::Deferred };

	unsigned char removeStaticDataFromMemoryAfterTransferingToGPU_ : 1;
	unsigned char drawOnWindow_ : 1;
	unsigned char countDrawCallsInner_ : 1;
};

#endif
