#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Types.h"

#include "Goknar/Model/MeshUnit.h"

#include "glad/glad.h"

#include <vector>

class DynamicMesh;
class StaticMesh;
class SkeletalMesh;
class LightManager;

class Texture;
class FrameBuffer;
class Shader;

class DynamicMeshInstance;
class StaticMeshInstance;
class SkeletalMeshInstance;

class PostProcessingEffect;

class RenderTarget;

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
	void BindGBufferDepth();

	void OnViewportSizeChanged(int width, int height);

	FrameBuffer* geometryFrameBuffer{ nullptr };

	Texture* worldPositionTexture{ nullptr };
	Texture* worldNormalTexture{ nullptr };
	Texture* diffuseTexture{ nullptr };
	Texture* specularTexture{ nullptr };
	Texture* emmisiveColorTexture{ nullptr };

	unsigned int depthRenderbuffer{ 0 };

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

	void BindGBufferDepth();

	void Render();

	void OnViewportSizeChanged(int width, int height);
	void BindGeometryBufferTextures(Shader* shader);

	GeometryBufferData* geometryBufferData{ nullptr };
	StaticMesh* deferredRenderingMesh{ nullptr };
	Shader* deferredRenderingMeshShader{ nullptr };

private:
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

	void AddStaticMeshToRenderer(StaticMesh* object);
	void AddStaticMeshInstance(StaticMeshInstance* object);
	void RemoveStaticMeshInstance(StaticMeshInstance* object);

	void AddSkeletalMeshToRenderer(SkeletalMesh* object);
	void AddSkeletalMeshInstance(SkeletalMeshInstance* object);
	void RemoveSkeletalMeshInstance(SkeletalMeshInstance* object);

	void AddDynamicMeshToRenderer(DynamicMesh* object);
	void AddDynamicMeshInstance(DynamicMeshInstance* object);
	void RemoveDynamicMeshInstance(DynamicMeshInstance* object);

	void UpdateDynamicMeshVertex(const DynamicMesh* object, int vertexIndex, const VertexData& newVertexData);

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

	void AddPostProcessingEffect(const PostProcessingEffect* postProcessingEffect)
	{
		postProcessingEffects_.push_back(postProcessingEffect);
	}

	void RemovePostProcessingEffect(const PostProcessingEffect* postProcessingEffect)
	{
		std::vector<const PostProcessingEffect*>::const_iterator postProcessingEffectIterator = postProcessingEffects_.cbegin();
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


private:
	void BindStaticVBO();
	void BindSkeletalVBO();
	void BindDynamicVBO();
	void SetAttribPointers();
	void SetAttribPointersForSkeletalMesh();

	void SortTransparentInstances();

	std::vector<StaticMesh*> staticMeshes_;
	std::vector<SkeletalMesh*> skeletalMeshes_;
	std::vector<DynamicMesh*> dynamicMeshes_;

	std::vector<StaticMeshInstance*> opaqueStaticMeshInstances_;
	//TODO: Is it really necessary to hold masked objects as a seperate array?
	std::vector<StaticMeshInstance*> maskedStaticMeshInstances_;
	std::vector<StaticMeshInstance*> transparentStaticMeshInstances_;

	std::vector<SkeletalMeshInstance*> opaqueSkeletalMeshInstances_;
	//TODO: Is it really necessary to hold masked objects as a seperate array?
	std::vector<SkeletalMeshInstance*> maskedSkeletalMeshInstances_;
	std::vector<SkeletalMeshInstance*> transparentSkeletalMeshInstances_;

	std::vector<DynamicMeshInstance*> opaqueDynamicMeshInstances_;
	//TODO: Is it really necessary to hold masked objects as a seperate array?
	std::vector<DynamicMeshInstance*> maskedDynamicMeshInstances_;
	std::vector<DynamicMeshInstance*> transparentDynamicMeshInstances_;

	LightManager* lightManager_{ nullptr };

	DeferredRenderingData* deferredRenderingData_{ nullptr };

	std::vector<const PostProcessingEffect*> postProcessingEffects_;
	std::vector<const RenderTarget*> renderTargets_;

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

	GEuint skeletalVertexBufferId_;
	GEuint skeletalIndexBufferId_;

	GEuint dynamicVertexBufferId_;
	GEuint dynamicIndexBufferId_;

	RenderPassType mainRenderType_{ RenderPassType::Deferred };

	unsigned char removeStaticDataFromMemoryAfterTransferingToGPU_ : 1;
	unsigned char drawOnWindow_ : 1;
};

#endif
