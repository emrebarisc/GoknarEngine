#include "pch.h"

#include "Renderer.h"

#include "Texture.h"
#include "Framebuffer.h"
#include "RenderBuffer.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Materials/MaterialInstance.h"

#include "Goknar/Delegates/Delegate.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Lights/LightManager/LightManager.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Model/InstancedStaticMeshInstance.h"
#include "Goknar/Model/StaticMeshInstance.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "Goknar/IO/IOManager.h"

#include "Goknar/Renderer/BloomPostProcessingEffect.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/PostProcessing.h"
#include "Goknar/Renderer/RenderTarget.h"
#include "Goknar/Renderer/TemporalAntiAliasingPostProcessingEffect.h"

#include <unordered_set>

#define VERTEX_COLOR_LOCATION 0
#define VERTEX_POSITION_LOCATION 1
#define VERTEX_NORMAL_LOCATION 2
#define VERTEX_UV_LOCATION 3
#define BONE_ID_LOCATION 4
#define BONE_WEIGHT_LOCATION 5
#define INSTANCE_TRANSFORMATION_ROW_0_LOCATION 4
#define INSTANCE_TRANSFORMATION_ROW_1_LOCATION 5
#define INSTANCE_TRANSFORMATION_ROW_2_LOCATION 6
#define INSTANCE_TRANSFORMATION_ROW_3_LOCATION 7

namespace
{
	void BlitFrameBufferColor(const FrameBuffer* readFrameBuffer, FrameBuffer* drawFrameBuffer, int width, int height)
	{
		if (!readFrameBuffer || width <= 0 || height <= 0)
		{
			return;
		}

		readFrameBuffer->Bind(FrameBufferBindTarget::READ_FRAMEBUFFER);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		if (drawFrameBuffer)
		{
			drawFrameBuffer->Bind(FrameBufferBindTarget::DRAW_FRAMEBUFFER);
		}
		else
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		glBlitFramebuffer(
			0, 0, width, height,
			0, 0, width, height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

Renderer::Renderer() :
	staticVertexBufferId_(0),
	staticIndexBufferId_(0),
	skeletalVertexBufferId_(0),
	skeletalIndexBufferId_(0),
	dynamicVertexBufferId_(0),
	dynamicIndexBufferId_(0),
	totalStaticMeshVertexSize_(0),
	totalStaticMeshFaceSize_(0),
	totalSkeletalMeshVertexSize_(0),
	totalSkeletalMeshFaceSize_(0),
	totalDynamicMeshVertexSize_(0),
	totalDynamicMeshFaceSize_(0),
	totalStaticMeshCount_(0),
	totalSkeletalMeshCount_(0),
	totalDynamicMeshCount_(0),
	lightManager_(nullptr),
	removeStaticDataFromMemoryAfterTransferingToGPU_(false),
	drawOnWindow_(true)
{
}

Renderer::~Renderer()
{
	delete temporalAntiAliasingPostProcessingEffect_;
	delete bloomPostProcessingEffect_;
	delete lightManager_;
	delete deferredRenderingData_;

	EXIT_ON_GL_ERROR("Renderer::~Renderer");

	glDeleteBuffers(1, &staticVertexBufferId_);
	for (const auto& [instancedStaticMesh, transformationBufferId] : instancedStaticMeshTransformationBufferIdMap_)
	{
		if (transformationBufferId != 0)
		{
			glDeleteBuffers(1, &transformationBufferId);
		}
	}
	instancedStaticMeshTransformationBufferIdMap_.clear();

	glDeleteBuffers(1, &skeletalVertexBufferId_);
	glDeleteBuffers(1, &dynamicVertexBufferId_);
	glDeleteBuffers(1, &staticIndexBufferId_);
	glDeleteBuffers(1, &skeletalIndexBufferId_);
	glDeleteBuffers(1, &dynamicIndexBufferId_);
}

void Renderer::PreInit()
{
	lightManager_ = new LightManager();
	lightManager_->PreInit();

	if (mainRenderType_ == RenderPassType::Deferred)
	{
		deferredRenderingData_ = new DeferredRenderingData();
		deferredRenderingData_->PreInit();
		deferredRenderingData_->Init();
		engine->GetWindowManager()->AddWindowSizeCallback(Delegate<void(int, int)>::Create<DeferredRenderingData, &DeferredRenderingData::OnViewportSizeChanged>(deferredRenderingData_));

		if (!temporalAntiAliasingPostProcessingEffect_)
		{
			temporalAntiAliasingPostProcessingEffect_ = new TemporalAntiAliasingPostProcessingEffect();
			temporalAntiAliasingPostProcessingEffect_->PreInit();
			temporalAntiAliasingPostProcessingEffect_->Init();
			temporalAntiAliasingPostProcessingEffect_->PostInit();
			AddPostProcessingEffect(temporalAntiAliasingPostProcessingEffect_);
		}

		if (!bloomPostProcessingEffect_)
		{
			bloomPostProcessingEffect_ = new BloomPostProcessingEffect();
			bloomPostProcessingEffect_->PreInit();
			bloomPostProcessingEffect_->Init();
			bloomPostProcessingEffect_->PostInit();
			AddPostProcessingEffect(bloomPostProcessingEffect_);
		}
	}

	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	for (MeshUnit* subMesh : staticMeshUnits_)
	{
		totalStaticMeshVertexSize_ += (unsigned int)subMesh->GetVerticesPointer()->size();
		totalStaticMeshFaceSize_ += (unsigned int)subMesh->GetFacesPointer()->size();
	}

	for (SkeletalMeshUnit* subMesh : skeletalMeshUnits_)
	{
		totalSkeletalMeshVertexSize_ += (unsigned int)subMesh->GetVerticesPointer()->size();
		totalSkeletalMeshFaceSize_ += (unsigned int)subMesh->GetFacesPointer()->size();
	}

	for (DynamicMeshUnit* subMesh : dynamicMeshUnits_)
	{
		totalDynamicMeshVertexSize_ += (unsigned int)subMesh->GetVerticesPointer()->size();
		totalDynamicMeshFaceSize_ += (unsigned int)subMesh->GetFacesPointer()->size();
	}

	SetBufferData();
}

void Renderer::Init()
{
	lightManager_->Init();
}

void Renderer::PostInit()
{
	lightManager_->PostInit();
}

void Renderer::SetStaticBufferData()
{
	/*
		Vertex buffer
	*/
	unsigned long long sizeOfVertexData = sizeof(VertexData);

	glGenBuffers(1, &staticVertexBufferId_);
	glBindBuffer(GL_ARRAY_BUFFER, staticVertexBufferId_);
	glBufferData(GL_ARRAY_BUFFER, totalStaticMeshVertexSize_ * sizeOfVertexData, nullptr, GL_STATIC_DRAW);

	/*
		Index buffer
	*/
	glGenBuffers(1, &staticIndexBufferId_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticIndexBufferId_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalStaticMeshFaceSize_ * sizeof(Face), nullptr, GL_STATIC_DRAW);

	/*
		Buffer Sub-Data
	*/
	unsigned int baseVertex = 0;
	unsigned int vertexStartingIndex = 0;

	int vertexOffset = 0;
	int faceOffset = 0;
	for (MeshUnit* subMesh : staticMeshUnits_)
	{
		subMesh->SetBaseVertex(baseVertex);
		subMesh->SetVertexStartingIndex(vertexStartingIndex);

		const VertexArray* vertexArrayPtr = subMesh->GetVerticesPointer();
		int vertexSizeInBytes = (int)vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		glBufferSubData(GL_ARRAY_BUFFER, vertexOffset, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = subMesh->GetFacesPointer();
		int faceSizeInBytes = (int)faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceOffset, faceSizeInBytes, &faceArrayPtr->at(0));

		vertexOffset += vertexSizeInBytes;
		faceOffset += faceSizeInBytes;

		baseVertex += subMesh->GetVertexCount();
		vertexStartingIndex += subMesh->GetFaceCount() * 3 * (int)sizeof(Face::vertexIndices[0]);

		if (removeStaticDataFromMemoryAfterTransferingToGPU_)
		{
			subMesh->ClearDataFromMemory();
		}
	}
	SetAttribPointers();
}

void Renderer::SetSkeletalBufferData()
{
	/*
		Vertex buffer
	*/
	unsigned long long int sizeOfSkeletalMeshVertexData = sizeof(VertexData) + sizeof(VertexBoneData);

	glGenBuffers(1, &skeletalVertexBufferId_);
	glBindBuffer(GL_ARRAY_BUFFER, skeletalVertexBufferId_);
	glBufferData(GL_ARRAY_BUFFER, totalSkeletalMeshVertexSize_ * sizeOfSkeletalMeshVertexData, nullptr, GL_STATIC_DRAW);

	/*
		Index buffer
	*/
	glGenBuffers(1, &skeletalIndexBufferId_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skeletalIndexBufferId_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalSkeletalMeshFaceSize_ * sizeof(Face), nullptr, GL_STATIC_DRAW);

	/*
		Buffer Sub-Data
	*/
	unsigned int baseVertex = 0;
	unsigned int vertexStartingIndex = 0;

	int vertexOffset = 0;
	int faceOffset = 0;
	for (SkeletalMeshUnit* subMesh : skeletalMeshUnits_)
	{
		subMesh->SetBaseVertex(baseVertex);
		subMesh->SetVertexStartingIndex(vertexStartingIndex);

		const VertexArray* vertexArrayPtr = subMesh->GetVerticesPointer();

		unsigned int vertexArrayPtrSize = vertexArrayPtr->size();
		if (vertexArrayPtrSize == 0)
		{
			continue;
		}

		GLintptr vertexSizeInBytes = sizeof(vertexArrayPtr->at(0));

		const VertexBoneDataArray* vertexBoneDataArray = subMesh->GetVertexBoneDataArray();
		int vertexBoneDataArraySizeInBytes = sizeof(vertexBoneDataArray->at(0));
		for (unsigned int i = 0; i < vertexArrayPtrSize; ++i)
		{
			glBufferSubData(GL_ARRAY_BUFFER, vertexOffset, vertexSizeInBytes, &vertexArrayPtr->at(i));
			vertexOffset += vertexSizeInBytes;

			glBufferSubData(GL_ARRAY_BUFFER, vertexOffset, vertexBoneDataArraySizeInBytes, &vertexBoneDataArray->at(i));
			vertexOffset += vertexBoneDataArraySizeInBytes;
		}

		const FaceArray* faceArrayPtr = subMesh->GetFacesPointer();
		int faceSizeInBytes = (int)faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceOffset, faceSizeInBytes, &faceArrayPtr->at(0));
		faceOffset += faceSizeInBytes;

		baseVertex += subMesh->GetVertexCount();
		vertexStartingIndex += subMesh->GetFaceCount() * 3 * (int)sizeof(Face::vertexIndices[0]);

		if (removeStaticDataFromMemoryAfterTransferingToGPU_)
		{
			subMesh->ClearDataFromMemory();
		}
	}
	SetAttribPointersForSkeletalMesh();
}

void Renderer::SetDynamicBufferData()
{
	/*
		Vertex buffer
	*/
	unsigned long long sizeOfVertexData = sizeof(VertexData);

	glGenBuffers(1, &dynamicVertexBufferId_);
	glBindBuffer(GL_ARRAY_BUFFER, dynamicVertexBufferId_);
	glBufferData(GL_ARRAY_BUFFER, totalDynamicMeshVertexSize_ * sizeOfVertexData, nullptr, GL_DYNAMIC_DRAW);

	/*
		Index buffer
	*/
	glGenBuffers(1, &dynamicIndexBufferId_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynamicIndexBufferId_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalDynamicMeshFaceSize_ * sizeof(Face), nullptr, GL_DYNAMIC_DRAW);

	/*
		Buffer Sub-Data
	*/
	unsigned int baseVertex = 0;
	unsigned int vertexStartingIndex = 0;

	int vertexOffset = 0;
	int faceOffset = 0;
	for (DynamicMeshUnit* subMesh : dynamicMeshUnits_)
	{
		subMesh->SetBaseVertex(baseVertex);
		subMesh->SetVertexStartingIndex(vertexStartingIndex);
		subMesh->SetRendererVertexOffset(vertexOffset);

		const VertexArray* vertexArrayPtr = subMesh->GetVerticesPointer();
		int vertexSizeInBytes = (int)vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		glBufferSubData(GL_ARRAY_BUFFER, vertexOffset, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = subMesh->GetFacesPointer();
		int faceSizeInBytes = (int)faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceOffset, faceSizeInBytes, &faceArrayPtr->at(0));

		vertexOffset += vertexSizeInBytes;
		faceOffset += faceSizeInBytes;

		baseVertex += subMesh->GetVertexCount();
		vertexStartingIndex += subMesh->GetFaceCount() * 3 * (int)sizeof(Face::vertexIndices[0]);
	}

	SetAttribPointers();
}

void Renderer::SetBufferData()
{
	if (0 < totalStaticMeshCount_) SetStaticBufferData();
	if (0 < totalSkeletalMeshCount_) SetSkeletalBufferData();
	if (0 < totalDynamicMeshCount_) SetDynamicBufferData();
}

void Renderer::RenderCurrentFrame()
{
	countDrawCallsInner_ = false;

	PrepareSkeletalMeshInstancesForTheCurrentFrame();

	DeferredRenderingData* mainDeferredRenderingData = deferredRenderingData_;

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* activeCamera = cameraManager->GetActiveCamera();
	auto PreparePostProcessingFrame = [&](DeferredRenderingData* deferredRenderingData)
		{
			for (PostProcessingEffect* postProcessingEffect : postProcessingEffects_)
			{
				if (postProcessingEffect)
				{
					postProcessingEffect->PrepareFrame(deferredRenderingData, cameraManager->GetActiveCamera());
				}
			}
		};
	std::vector<const RenderTarget*>::const_iterator renderTargetIterator = renderTargets_.cbegin();
	while (renderTargetIterator != renderTargets_.cend())
	{
		const RenderTarget* renderTarget = *renderTargetIterator;

		currentRenderTarget_ = renderTarget;

		if (renderTarget->GetIsActive())
		{
			cameraManager->SetActiveCamera(renderTarget->GetCamera());

			if (renderTarget->GetRerenderShadowMaps())
			{
				GetLightManager()->RenderShadowMaps();
			}

			FrameBuffer* renderTargetFrameBuffer = renderTarget->GetFrameBuffer();

			if (GetMainRenderType() == RenderPassType::Forward)
			{
				renderTargetFrameBuffer->Bind();
				Render(RenderPassType::Forward);
				renderTargetFrameBuffer->Unbind();
			}
			else if (GetMainRenderType() == RenderPassType::Deferred)
			{
				deferredRenderingData_ = renderTarget->GetDeferredRenderingData();

				PreparePostProcessingFrame(deferredRenderingData_);
				Render(RenderPassType::GeometryBuffer);
				Render(RenderPassType::Deferred);
				ApplyPostProcessing(deferredRenderingData_, renderTargetFrameBuffer);

				if (cameraManager->GetActiveCamera())
				{
					cameraManager->GetActiveCamera()->SetTemporalJitter(Vector2::ZeroVector);
				}
			}

		}

		renderTargetIterator++;
	}

	currentRenderTarget_ = nullptr;

	cameraManager->SetActiveCamera(activeCamera);
	deferredRenderingData_ = mainDeferredRenderingData;

	if (drawOnWindow_)
	{
		GetLightManager()->RenderShadowMaps();

		drawCallCount = 0;
		if (GetMainRenderType() == RenderPassType::Forward)
		{
			countDrawCallsInner_ = countDrawCalls;
			Render(RenderPassType::Forward);
		}
		else if (GetMainRenderType() == RenderPassType::Deferred)
		{
			countDrawCallsInner_ = countDrawCalls;
			PreparePostProcessingFrame(deferredRenderingData_);
			Render(RenderPassType::GeometryBuffer);
			countDrawCallsInner_ = false;
			Render(RenderPassType::Deferred);
			ApplyPostProcessing(deferredRenderingData_, nullptr);

			if (cameraManager->GetActiveCamera())
			{
				cameraManager->GetActiveCamera()->SetTemporalJitter(Vector2::ZeroVector);
			}
		}
	}
	else
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	PrepareSkeletalMeshInstancesForTheNextFrame();
}

void Renderer::ApplyPostProcessing(DeferredRenderingData* deferredRenderingData, FrameBuffer* destinationFrameBuffer)
{
	if (!deferredRenderingData)
	{
		return;
	}

	const int width = deferredRenderingData->geometryBufferData->bufferWidth;
	const int height = deferredRenderingData->geometryBufferData->bufferHeight;

	Texture* finalTexture = deferredRenderingData->GetSceneTexture();
	FrameBuffer* finalFrameBuffer = deferredRenderingData->GetSceneFrameBuffer();

	for (PostProcessingEffect* postProcessingEffect : postProcessingEffects_)
	{
		if (!postProcessingEffect || !postProcessingEffect->GetIsEnabled())
		{
			continue;
		}

		Texture* effectOutputTexture = postProcessingEffect->Render(deferredRenderingData, finalTexture, width, height);
		if (effectOutputTexture != nullptr && effectOutputTexture != finalTexture)
		{
			finalTexture = effectOutputTexture;
			finalFrameBuffer = postProcessingEffect->GetOutputFrameBuffer();
		}
	}

	BlitFrameBufferColor(finalFrameBuffer, destinationFrameBuffer, width, height);
}

void Renderer::Render(RenderPassType renderPassType)
{
	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();

	if (!activeCamera)
	{
		return;
	}

	switch (renderPassType)
	{
	case RenderPassType::Forward:
	{
		glDepthMask(GL_TRUE);
		const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
		glClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		break;
	}
	case RenderPassType::GeometryBuffer:
	{
		GOKNAR_CORE_CHECK(deferredRenderingData_ != nullptr, "Main rendering is not set to deferred rendering but deferred rendering is called.");

		deferredRenderingData_->BindGeometryBuffer();
		const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
		glClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthMask(GL_TRUE);
		break;
	}
	case RenderPassType::Deferred:
	{
		GOKNAR_CORE_CHECK(deferredRenderingData_ != nullptr, "Main rendering is not set to deferred rendering but deferred rendering is called.");
		deferredRenderingData_->BeginSceneRender();
		const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
		glClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		deferredRenderingData_->Render();
		break;
	}
	case RenderPassType::Shadow:
	case RenderPassType::PointLightShadow:
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
	}
	case RenderPassType::None:
	default:
	{
		GOKNAR_CORE_ASSERT(false, "Render function called without a correct pass type!");
		return;
	}
	}

	bool isShadowRender =
		renderPassType == RenderPassType::Shadow ||
		renderPassType == RenderPassType::PointLightShadow;

	auto RenderStaticMesh = [&](const StaticMeshRenderData& renderData)
		{
			StaticMeshInstance* staticMeshInstance = renderData.meshInstance;
			MeshUnit* subMesh = renderData.meshUnit;
			const int subMeshIndex = renderData.subMeshIndex;

			if (!activeCamera->IsAABBVisible(subMesh->GetAABB(), staticMeshInstance->GetParentComponent()->GetComponentToWorldTransformationMatrix())) return;

			if (countDrawCallsInner_) ++drawCallCount;

			staticMeshInstance->PreRender(subMeshIndex, renderPassType);
			staticMeshInstance->Render(subMeshIndex, renderPassType);

			int facePointCount = subMesh->GetFaceCount() * 3;
			glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
		};

	auto RenderInstancedStaticMesh = [&](const InstancedStaticMeshRenderData& renderData)
		{
			InstancedStaticMeshInstance* instancedStaticMeshInstance = renderData.meshInstance;
			InstancedStaticMesh* instancedStaticMesh = instancedStaticMeshInstance->GetMesh();
			if (!instancedStaticMesh)
			{
				return;
			}

			const int instanceCount = (int)instancedStaticMesh->GetInstanceCount();
			if (instanceCount <= 0)
			{
				return;
			}

			if (!BindInstancedStaticMesh(instancedStaticMesh))
			{
				return;
			}

			MeshUnit* subMesh = renderData.meshUnit;
			const int subMeshIndex = renderData.subMeshIndex;

			if (countDrawCallsInner_) ++drawCallCount;

			instancedStaticMeshInstance->PreRender(subMeshIndex, renderPassType);
			instancedStaticMeshInstance->Render(subMeshIndex, renderPassType);

			int facePointCount = subMesh->GetFaceCount() * 3;
			glDrawElementsInstancedBaseVertex(
				GL_TRIANGLES,
				facePointCount,
				GL_UNSIGNED_INT,
				(void*)(unsigned long long)subMesh->GetVertexStartingIndex(),
				instanceCount,
				subMesh->GetBaseVertex());
		};

	auto RenderSkeletalMesh = [&](const SkeletalMeshRenderData& renderData)
		{
			SkeletalMeshInstance* skeletalMeshInstance = renderData.meshInstance;
			SkeletalMeshUnit* subMesh = renderData.meshUnit;
			const int subMeshIndex = renderData.subMeshIndex;

			if (!activeCamera->IsAABBVisible(subMesh->GetAABB(), skeletalMeshInstance->GetParentComponent()->GetComponentToWorldTransformationMatrix())) return;

			if (countDrawCallsInner_) ++drawCallCount;

			skeletalMeshInstance->PreRender(subMeshIndex, renderPassType);
			skeletalMeshInstance->Render(subMeshIndex, renderPassType);

			int facePointCount = subMesh->GetFaceCount() * 3;
			glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
		};

	auto RenderDynamicMesh = [&](const DynamicMeshRenderData& renderData)
		{
			DynamicMeshInstance* dynamicMeshInstance = renderData.meshInstance;
			DynamicMeshUnit* subMesh = renderData.meshUnit;
			const int subMeshIndex = renderData.subMeshIndex;

			if (!activeCamera->IsAABBVisible(subMesh->GetAABB(), dynamicMeshInstance->GetParentComponent()->GetComponentToWorldTransformationMatrix())) return;

			if (countDrawCallsInner_) ++drawCallCount;

			dynamicMeshInstance->PreRender(subMeshIndex, renderPassType);
			dynamicMeshInstance->Render(subMeshIndex, renderPassType);

			int facePointCount = subMesh->GetFaceCount() * 3;
			glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
		};

	if (renderPassType != RenderPassType::Deferred)
	{
		// Static MeshUnit Instances
		{
			if (0 < totalStaticMeshCount_)
			{
				BindStaticVBO();

				for (const StaticMeshRenderData& opaqueStaticMeshRenderData : opaqueStaticMeshRenderData_)
				{
					StaticMeshInstance* opaqueStaticMeshInstance = opaqueStaticMeshRenderData.meshInstance;
					if (!opaqueStaticMeshInstance->GetIsRendered()) continue;
					if (isShadowRender && !opaqueStaticMeshInstance->GetIsCastingShadow()) continue;
					if (!(activeCamera->GetRenderMask() & opaqueStaticMeshInstance->GetRenderMask())) continue;

					RenderStaticMesh(opaqueStaticMeshRenderData);
				}

				for (const InstancedStaticMeshRenderData& opaqueInstancedStaticMeshRenderData : opaqueInstancedStaticMeshRenderData_)
				{
					InstancedStaticMeshInstance* opaqueInstancedStaticMeshInstance = opaqueInstancedStaticMeshRenderData.meshInstance;
					if (!opaqueInstancedStaticMeshInstance->GetIsRendered()) continue;
					if (isShadowRender && !opaqueInstancedStaticMeshInstance->GetIsCastingShadow()) continue;
					if (!(activeCamera->GetRenderMask() & opaqueInstancedStaticMeshInstance->GetRenderMask())) continue;

					RenderInstancedStaticMesh(opaqueInstancedStaticMeshRenderData);
				}
			}
		}

		// Skeletal MeshUnit Instances
		{
			if (0 < totalSkeletalMeshCount_)
			{
				BindSkeletalVBO();

				for (const SkeletalMeshRenderData& opaqueSkeletalMeshRenderData : opaqueSkeletalMeshRenderData_)
				{
					SkeletalMeshInstance* opaqueSkeletalMeshInstance = opaqueSkeletalMeshRenderData.meshInstance;
					if (!opaqueSkeletalMeshInstance->GetIsRendered()) continue;
					if (isShadowRender && !opaqueSkeletalMeshInstance->GetIsCastingShadow()) continue;
					if (!(activeCamera->GetRenderMask() & opaqueSkeletalMeshInstance->GetRenderMask())) continue;

					RenderSkeletalMesh(opaqueSkeletalMeshRenderData);
				}
			}
		}

		// Dynamic MeshUnit Instances
		{
			if (0 < totalDynamicMeshCount_)
			{
				BindDynamicVBO();

				for (const DynamicMeshRenderData& opaqueDynamicMeshRenderData : opaqueDynamicMeshRenderData_)
				{
					DynamicMeshInstance* opaqueDynamicMeshInstance = opaqueDynamicMeshRenderData.meshInstance;
					if (!opaqueDynamicMeshInstance->GetIsRendered()) continue;
					if (isShadowRender && !opaqueDynamicMeshInstance->GetIsCastingShadow()) continue;
					if (!(activeCamera->GetRenderMask() & opaqueDynamicMeshInstance->GetRenderMask())) continue;

					RenderDynamicMesh(opaqueDynamicMeshRenderData);
				}
			}
		}
	}
	else
	{
		deferredRenderingData_->BindGBufferDepth(deferredRenderingData_->GetSceneFrameBuffer());
	}

	if (renderPassType == RenderPassType::Forward ||
		renderPassType == RenderPassType::Deferred)
	{
		SortTransparentInstances();

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);

		BindStaticVBO();

		for (const StaticMeshRenderData& transparentStaticMeshRenderData : transparentStaticMeshRenderData_)
		{
			StaticMeshInstance* transparentStaticMeshInstance = transparentStaticMeshRenderData.meshInstance;
			if (!transparentStaticMeshInstance->GetIsRendered()) continue;
			if (!(activeCamera->GetRenderMask() & transparentStaticMeshInstance->GetRenderMask())) continue;

			RenderStaticMesh(transparentStaticMeshRenderData);
		}

		for (const InstancedStaticMeshRenderData& transparentInstancedStaticMeshRenderData : transparentInstancedStaticMeshRenderData_)
		{
			InstancedStaticMeshInstance* transparentInstancedStaticMeshInstance = transparentInstancedStaticMeshRenderData.meshInstance;
			if (!transparentInstancedStaticMeshInstance->GetIsRendered()) continue;
			if (!(activeCamera->GetRenderMask() & transparentInstancedStaticMeshInstance->GetRenderMask())) continue;

			RenderInstancedStaticMesh(transparentInstancedStaticMeshRenderData);
		}

		BindSkeletalVBO();
		for (const SkeletalMeshRenderData& transparentSkeletalMeshRenderData : transparentSkeletalMeshRenderData_)
		{
			SkeletalMeshInstance* transparentSkeletalMeshInstance = transparentSkeletalMeshRenderData.meshInstance;
			if (!transparentSkeletalMeshInstance->GetIsRendered()) continue;
			if (!(activeCamera->GetRenderMask() & transparentSkeletalMeshInstance->GetRenderMask())) continue;

			RenderSkeletalMesh(transparentSkeletalMeshRenderData);
		}

		BindDynamicVBO();
		for (const DynamicMeshRenderData& transparentDynamicMeshRenderData : transparentDynamicMeshRenderData_)
		{
			DynamicMeshInstance* transparentDynamicMeshInstance = transparentDynamicMeshRenderData.meshInstance;
			if (!transparentDynamicMeshInstance->GetIsRendered()) continue;
			if (!(activeCamera->GetRenderMask() & transparentDynamicMeshInstance->GetRenderMask())) continue;

			RenderDynamicMesh(transparentDynamicMeshRenderData);
		}
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		// After finishing transparent object rendering
		// Use deferred rendering shader again if doing deferred rendering
		// Otherwise it causes crash on setting g-buffer debug on
		// And wishing to visualize g-buffers on deferred rendering mesh
		if (renderPassType == RenderPassType::Deferred)
		{
			deferredRenderingData_->deferredRenderingMeshShader->Use();
		}
	}

	if (renderPassType == RenderPassType::Deferred)
	{
		deferredRenderingData_->EndSceneRender();
	}

	if (renderPassType == RenderPassType::GeometryBuffer)
	{
		deferredRenderingData_->UnbindGeometryBuffer();
		return;
	}
}

void Renderer::AddStaticMeshToRenderer(StaticMesh* staticMesh)
{
	for (MeshUnit* subMesh : staticMesh->GetSubMeshes())
	{
		staticMeshUnits_.push_back(subMesh);
	}
	totalStaticMeshCount_++;
}

void Renderer::AddInstancedStaticMeshToRenderer(InstancedStaticMesh* instancedStaticMesh)
{
	instancedStaticMeshes_.push_back(instancedStaticMesh);
	AddStaticMeshToRenderer(instancedStaticMesh);
}

void Renderer::AddStaticMeshInstance(StaticMeshInstance* meshInstance)
{
	const std::vector<MeshUnit*>& subMeshes = meshInstance->GetMesh()->GetSubMeshes();
	for (int subMeshIndex = 0; subMeshIndex < (int)subMeshes.size(); ++subMeshIndex)
	{
		StaticMeshRenderData renderData{ meshInstance, subMeshes[subMeshIndex], subMeshIndex };
		const IMaterialBase* material = meshInstance->GetMaterial(subMeshIndex);
		const MaterialBlendModel materialBlendModel = material ? material->GetBlendModel() : MaterialBlendModel::Opaque;

		switch (materialBlendModel)
		{
		case MaterialBlendModel::Opaque:
		case MaterialBlendModel::Masked:
			opaqueStaticMeshRenderData_.push_back(renderData);
			break;
		case MaterialBlendModel::Transparent:
			transparentStaticMeshRenderData_.push_back(renderData);
			break;
		default:
			break;
		}
	}
}

void Renderer::RemoveStaticMeshInstance(StaticMeshInstance* staticMeshInstance)
{
	auto removeRenderData = [staticMeshInstance](std::vector<StaticMeshRenderData>& renderDataList)
		{
			renderDataList.erase(
				std::remove_if(
					renderDataList.begin(),
					renderDataList.end(),
					[staticMeshInstance](const StaticMeshRenderData& renderData)
					{
						return renderData.meshInstance == staticMeshInstance;
					}),
				renderDataList.end());
		};

	removeRenderData(opaqueStaticMeshRenderData_);
	removeRenderData(transparentStaticMeshRenderData_);
}

void Renderer::AddInstancedStaticMeshInstance(InstancedStaticMeshInstance* instancedStaticMeshInstance)
{
	const std::vector<MeshUnit*>& subMeshes = instancedStaticMeshInstance->GetMesh()->GetSubMeshes();
	for (int subMeshIndex = 0; subMeshIndex < (int)subMeshes.size(); ++subMeshIndex)
	{
		InstancedStaticMeshRenderData renderData{ instancedStaticMeshInstance, subMeshes[subMeshIndex], subMeshIndex };
		const IMaterialBase* material = instancedStaticMeshInstance->GetMaterial(subMeshIndex);
		const MaterialBlendModel materialBlendModel = material ? material->GetBlendModel() : MaterialBlendModel::Opaque;

		switch (materialBlendModel)
		{
		case MaterialBlendModel::Opaque:
		case MaterialBlendModel::Masked:
			opaqueInstancedStaticMeshRenderData_.push_back(renderData);
			break;
		case MaterialBlendModel::Transparent:
			transparentInstancedStaticMeshRenderData_.push_back(renderData);
			break;
		default:
			break;
		}
	}
}

void Renderer::RemoveInstancedStaticMeshInstance(InstancedStaticMeshInstance* instancedStaticMeshInstance)
{
	auto removeRenderData = [instancedStaticMeshInstance](std::vector<InstancedStaticMeshRenderData>& renderDataList)
		{
			renderDataList.erase(
				std::remove_if(
					renderDataList.begin(),
					renderDataList.end(),
					[instancedStaticMeshInstance](const InstancedStaticMeshRenderData& renderData)
					{
						return renderData.meshInstance == instancedStaticMeshInstance;
					}),
				renderDataList.end());
		};

	removeRenderData(opaqueInstancedStaticMeshRenderData_);
	removeRenderData(transparentInstancedStaticMeshRenderData_);
}

void Renderer::AddSkeletalMeshToRenderer(SkeletalMesh* skeletalMesh)
{
	for (SkeletalMeshUnit* subMesh : skeletalMesh->GetSubMeshes())
	{
		skeletalMeshUnits_.push_back(subMesh);
	}
	totalSkeletalMeshCount_++;
}

void Renderer::AddSkeletalMeshInstance(SkeletalMeshInstance* skeletalMeshInstance)
{
	const std::vector<SkeletalMeshUnit*>& subMeshes = skeletalMeshInstance->GetMesh()->GetSubMeshes();
	for (int subMeshIndex = 0; subMeshIndex < (int)subMeshes.size(); ++subMeshIndex)
	{
		SkeletalMeshRenderData renderData{ skeletalMeshInstance, subMeshes[subMeshIndex], subMeshIndex };
		const IMaterialBase* material = skeletalMeshInstance->GetMaterial(subMeshIndex);
		const MaterialBlendModel materialBlendModel = material ? material->GetBlendModel() : MaterialBlendModel::Opaque;

		switch (materialBlendModel)
		{
		case MaterialBlendModel::Opaque:
		case MaterialBlendModel::Masked:
			opaqueSkeletalMeshRenderData_.push_back(renderData);
			break;
		case MaterialBlendModel::Transparent:
			transparentSkeletalMeshRenderData_.push_back(renderData);
			break;
		default:
			break;
		}
	}
}

void Renderer::RemoveSkeletalMeshInstance(SkeletalMeshInstance* skeletalMeshInstance)
{
	auto removeRenderData = [skeletalMeshInstance](std::vector<SkeletalMeshRenderData>& renderDataList)
		{
			renderDataList.erase(
				std::remove_if(
					renderDataList.begin(),
					renderDataList.end(),
					[skeletalMeshInstance](const SkeletalMeshRenderData& renderData)
					{
						return renderData.meshInstance == skeletalMeshInstance;
					}),
				renderDataList.end());
		};

	removeRenderData(opaqueSkeletalMeshRenderData_);
	removeRenderData(transparentSkeletalMeshRenderData_);
}

void Renderer::AddDynamicMeshToRenderer(DynamicMesh* dynamicMesh)
{
	for (DynamicMeshUnit* subMesh : dynamicMesh->GetSubMeshes())
	{
		dynamicMeshUnits_.push_back(subMesh);
	}
	totalDynamicMeshCount_++;
}

void Renderer::AddDynamicMeshInstance(DynamicMeshInstance* dynamicMeshInstance)
{
	const std::vector<DynamicMeshUnit*>& subMeshes = dynamicMeshInstance->GetMesh()->GetSubMeshes();
	for (int subMeshIndex = 0; subMeshIndex < (int)subMeshes.size(); ++subMeshIndex)
	{
		DynamicMeshRenderData renderData{ dynamicMeshInstance, subMeshes[subMeshIndex], subMeshIndex };
		const IMaterialBase* material = dynamicMeshInstance->GetMaterial(subMeshIndex);
		const MaterialBlendModel materialBlendModel = material ? material->GetBlendModel() : MaterialBlendModel::Opaque;

		switch (materialBlendModel)
		{
		case MaterialBlendModel::Masked:
		case MaterialBlendModel::Opaque:
			opaqueDynamicMeshRenderData_.push_back(renderData);
			break;
		case MaterialBlendModel::Transparent:
			transparentDynamicMeshRenderData_.push_back(renderData);
			break;
		default:
			break;
		}
	}
}

void Renderer::RemoveDynamicMeshInstance(DynamicMeshInstance* dynamicMeshInstance)
{
	auto removeRenderData = [dynamicMeshInstance](std::vector<DynamicMeshRenderData>& renderDataList)
		{
			renderDataList.erase(
				std::remove_if(
					renderDataList.begin(),
					renderDataList.end(),
					[dynamicMeshInstance](const DynamicMeshRenderData& renderData)
					{
						return renderData.meshInstance == dynamicMeshInstance;
					}),
				renderDataList.end());
		};

	removeRenderData(opaqueDynamicMeshRenderData_);
	removeRenderData(transparentDynamicMeshRenderData_);
}

void Renderer::UpdateDynamicMeshVertex(const DynamicMeshUnit* object, int vertexIndex, const VertexData& newVertexData)
{
	int sizeOfVertexData = sizeof(VertexData);
	glNamedBufferSubData(dynamicVertexBufferId_, object->GetRendererVertexOffset() + vertexIndex * sizeOfVertexData, sizeOfVertexData, &newVertexData);
}

void Renderer::RefreshInstancedStaticMeshTransformations(const InstancedStaticMesh* instancedStaticMesh)
{
	if (!instancedStaticMesh || staticVertexBufferId_ == 0)
	{
		return;
	}

	GEuint& transformationBufferId = instancedStaticMeshTransformationBufferIdMap_[instancedStaticMesh];
	if (transformationBufferId == 0)
	{
		glGenBuffers(1, &transformationBufferId);
	}

	glBindBuffer(GL_ARRAY_BUFFER, transformationBufferId);

	const std::vector<Matrix>& instanceTransformationMatrices = instancedStaticMesh->GetInstanceTransformationMatrices();
	const void* bufferData = instanceTransformationMatrices.empty() ? nullptr : instanceTransformationMatrices.data();
	glBufferData(
		GL_ARRAY_BUFFER,
		(GLsizeiptr)(instanceTransformationMatrices.size() * sizeof(Matrix)),
		bufferData,
		GL_DYNAMIC_DRAW);

	const_cast<InstancedStaticMesh*>(instancedStaticMesh)->ClearPendingFullTransformUpload();
}

void Renderer::UpdateInstancedStaticMeshTransformation(const InstancedStaticMesh* instancedStaticMesh, int transformationIndex, const Matrix& newTransformationMatrix)
{
	if (!instancedStaticMesh || staticVertexBufferId_ == 0 || transformationIndex < 0)
	{
		return;
	}

	auto transformationBufferIterator = instancedStaticMeshTransformationBufferIdMap_.find(instancedStaticMesh);
	if (transformationBufferIterator == instancedStaticMeshTransformationBufferIdMap_.end())
	{
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, transformationBufferIterator->second);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		(GLintptr)(transformationIndex * sizeof(Matrix)),
		sizeof(Matrix),
		&newTransformationMatrix);
}

void Renderer::PrepareSkeletalMeshInstancesForTheCurrentFrame()
{
	std::unordered_set<SkeletalMeshInstance*> preparedSkeletalMeshInstances;
	auto prepareRenderData = [&preparedSkeletalMeshInstances](const SkeletalMeshRenderData& renderData)
	{
		SkeletalMeshInstance* skeletalMeshInstance = renderData.meshInstance;
		if (preparedSkeletalMeshInstances.insert(skeletalMeshInstance).second)
		{
			skeletalMeshInstance->PrepareForTheCurrentFrame();
		}
	};

	for (const SkeletalMeshRenderData& renderData : opaqueSkeletalMeshRenderData_)
	{
		prepareRenderData(renderData);
	}

	for (const SkeletalMeshRenderData& renderData : transparentSkeletalMeshRenderData_)
	{
		prepareRenderData(renderData);
	}
}


void Renderer::PrepareSkeletalMeshInstancesForTheNextFrame()
{
	std::unordered_set<SkeletalMeshInstance*> preparedSkeletalMeshInstances;
	auto prepareRenderData = [&preparedSkeletalMeshInstances](const SkeletalMeshRenderData& renderData)
	{
		SkeletalMeshInstance* skeletalMeshInstance = renderData.meshInstance;
		if (preparedSkeletalMeshInstances.insert(skeletalMeshInstance).second)
		{
			skeletalMeshInstance->PrepareForTheNextFrame();
		}
	};

	for (const SkeletalMeshRenderData& renderData : opaqueSkeletalMeshRenderData_)
	{
		prepareRenderData(renderData);
	}

	for (const SkeletalMeshRenderData& renderData : transparentSkeletalMeshRenderData_)
	{
		prepareRenderData(renderData);
	}
}

void Renderer::BindShadowTextures(Shader* shader)
{
	Scene* scene = engine->GetApplication()->GetMainScene();

	shader->Use();

	std::vector<int> directionalLightTextureIndices;
	const std::vector<DirectionalLight*>& directionalLights = scene->GetDirectionalLights();
	size_t directionalLightCount = directionalLights.size();
	for (size_t i = 0; i < directionalLightCount; i++)
	{
		DirectionalLight* directionalLight = directionalLights[i];
		if (directionalLight->GetIsShadowEnabled())
		{
			Texture* shadowTexture = directionalLight->GetShadowMapTexture();
			directionalLightTextureIndices.push_back(shadowTexture->GetRendererTextureId());
			shadowTexture->Bind(shader);
		}
	}
	shader->SetIntVector(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_ARRAY_NAME, directionalLightTextureIndices);

	std::vector<int> pointLightTextureIndices;
	const std::vector<PointLight*>& pointLights = scene->GetPointLights();
	size_t pointLightCount = pointLights.size();
	for (size_t i = 0; i < pointLightCount; i++)
	{
		PointLight* pointLight = pointLights[i];
		if (pointLight->GetIsShadowEnabled())
		{
			Texture* shadowTexture = pointLight->GetShadowMapTexture();
			pointLightTextureIndices.push_back(shadowTexture->GetRendererTextureId());
			shadowTexture->Bind(shader);
		}
	}
	shader->SetIntVector(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_ARRAY_NAME, pointLightTextureIndices);

	std::vector<int> spotLightTextureIndices;
	const std::vector<SpotLight*>& spotLights = scene->GetSpotLights();
	size_t spotLightCount = spotLights.size();
	for (size_t i = 0; i < spotLightCount; i++)
	{
		SpotLight* spotLight = spotLights[i];
		if (spotLight->GetIsShadowEnabled())
		{
			Texture* shadowTexture = spotLight->GetShadowMapTexture();
			spotLightTextureIndices.push_back(shadowTexture->GetRendererTextureId());
			shadowTexture->Bind(shader);
		}
	}
	shader->SetIntVector(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_ARRAY_NAME, spotLightTextureIndices);
}

void Renderer::BindGeometryBufferTextures(Shader* shader)
{
	if (deferredRenderingData_)
	{
		shader->Use();
		deferredRenderingData_->BindGeometryBufferTextures(shader);
	}
}

void Renderer::SetLightUniforms(Shader* shader)
{
	Scene* scene = engine->GetApplication()->GetMainScene();

	const std::vector<DirectionalLight*>& directionalLights = scene->GetDirectionalLights();
	size_t directionalLightCount = directionalLights.size();
	for (size_t i = 0; i < directionalLightCount; i++)
	{
		directionalLights[i]->SetShaderUniforms(shader);
	}

	const std::vector<PointLight*>& pointLights = scene->GetPointLights();
	size_t pointLightCount = pointLights.size();
	for (size_t i = 0; i < pointLightCount; i++)
	{
		pointLights[i]->SetShaderUniforms(shader);
	}

	const std::vector<SpotLight*>& spotLights = scene->GetSpotLights();
	size_t spotLightCount = spotLights.size();
	for (size_t i = 0; i < spotLightCount; i++)
	{
		spotLights[i]->SetShaderUniforms(shader);
	}
}

void Renderer::RenderStaticMesh(StaticMesh* staticMesh)
{
	BindStaticVBO();

	for (MeshUnit* subMesh : staticMesh->GetSubMeshes())
	{
		int facePointCount = subMesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
	}
}

void Renderer::BindStaticVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, staticVertexBufferId_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticIndexBufferId_);
	SetAttribPointers();
}

bool Renderer::BindInstancedStaticMesh(InstancedStaticMesh* instancedStaticMesh)
{
	BindStaticVBO();

	if (instancedStaticMesh && instancedStaticMesh->HasPendingFullTransformUpload())
	{
		RefreshInstancedStaticMeshTransformations(instancedStaticMesh);
	}

	auto bufferIterator = instancedStaticMeshTransformationBufferIdMap_.find(instancedStaticMesh);
	if (bufferIterator == instancedStaticMeshTransformationBufferIdMap_.end())
	{
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, bufferIterator->second);
	SetAttribPointersForInstancedStaticMesh();
	return true;
}

void Renderer::BindSkeletalVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, skeletalVertexBufferId_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skeletalIndexBufferId_);
	SetAttribPointersForSkeletalMesh();
}

void Renderer::BindDynamicVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, dynamicVertexBufferId_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynamicIndexBufferId_);
	SetAttribPointers();
}

void Renderer::SetAttribPointers()
{
	GEsizei sizeOfVertexData = (GEsizei)sizeof(VertexData);
	// Vertex color
	long long offset = 0;
	glEnableVertexAttribArray(VERTEX_COLOR_LOCATION);
	glVertexAttribPointer(VERTEX_COLOR_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);

	// Vertex position
	offset += sizeof(VertexData::color);
	glEnableVertexAttribArray(VERTEX_POSITION_LOCATION);
	glVertexAttribPointer(VERTEX_POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);

	// Vertex normal
	offset += sizeof(VertexData::position);
	glEnableVertexAttribArray(VERTEX_NORMAL_LOCATION);
	glVertexAttribPointer(VERTEX_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);

	// Vertex UV
	offset += sizeof(VertexData::normal);
	glEnableVertexAttribArray(VERTEX_UV_LOCATION);
	glVertexAttribPointer(VERTEX_UV_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);

	glDisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_0_LOCATION);
	glDisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_1_LOCATION);
	glDisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_2_LOCATION);
	glDisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_3_LOCATION);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_0_LOCATION, 0);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_1_LOCATION, 0);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_2_LOCATION, 0);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_3_LOCATION, 0);
}

void Renderer::SetAttribPointersForInstancedStaticMesh()
{
	GEsizei sizeOfInstanceTransformationData = (GEsizei)sizeof(Matrix);
	long long offset = 0;

	glEnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_0_LOCATION);
	glVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_0_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeOfInstanceTransformationData, (void*)offset);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_0_LOCATION, 1);
	offset += sizeof(Vector4);

	glEnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_1_LOCATION);
	glVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_1_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeOfInstanceTransformationData, (void*)offset);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_1_LOCATION, 1);
	offset += sizeof(Vector4);

	glEnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_2_LOCATION);
	glVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_2_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeOfInstanceTransformationData, (void*)offset);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_2_LOCATION, 1);
	offset += sizeof(Vector4);

	glEnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_3_LOCATION);
	glVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_3_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeOfInstanceTransformationData, (void*)offset);
	glVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_3_LOCATION, 1);
}

void Renderer::SetAttribPointersForSkeletalMesh()
{
	GEsizei sizeOfSkeletalMeshVertexData = (GEsizei)(sizeof(VertexData) + sizeof(VertexBoneData));

	long long offset = 0;
	// Vertex color
	glEnableVertexAttribArray(VERTEX_COLOR_LOCATION);
	glVertexAttribPointer(VERTEX_COLOR_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::color);

	// Vertex position
	glEnableVertexAttribArray(VERTEX_POSITION_LOCATION);
	glVertexAttribPointer(VERTEX_POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::position);

	// Vertex normal
	glEnableVertexAttribArray(VERTEX_NORMAL_LOCATION);
	glVertexAttribPointer(VERTEX_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::normal);

	// Vertex UV
	glEnableVertexAttribArray(VERTEX_UV_LOCATION);
	glVertexAttribPointer(VERTEX_UV_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::uv);

	// Bone ID
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, MAX_BONE_SIZE_PER_VERTEX, GL_UNSIGNED_INT, sizeOfSkeletalMeshVertexData, (void*)offset);
	glVertexAttribDivisor(BONE_ID_LOCATION, 0);
	offset += sizeof(VertexBoneData::boneIDs);

	// Bone Weight
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_BONE_SIZE_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeOfSkeletalMeshVertexData, (void*)offset);
	glVertexAttribDivisor(BONE_WEIGHT_LOCATION, 0);

	glDisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_2_LOCATION);
	glDisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_3_LOCATION);
}

void Renderer::SortTransparentInstances()
{
	struct
	{
		Vector3 cameraPosition = engine->GetCameraManager()->GetActiveCamera()->GetPosition();
		bool operator()(const StaticMeshRenderData& a, const StaticMeshRenderData& b) const
		{
			return  (cameraPosition - a.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength() >
				(cameraPosition - b.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength();
		}
		bool operator()(const SkeletalMeshRenderData& a, const SkeletalMeshRenderData& b) const
		{
			return  (cameraPosition - a.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength() >
				(cameraPosition - b.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength();
		}
		bool operator()(const DynamicMeshRenderData& a, const DynamicMeshRenderData& b) const
		{
			return  (cameraPosition - a.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength() >
				(cameraPosition - b.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength();
		}
		bool operator()(const InstancedStaticMeshRenderData& a, const InstancedStaticMeshRenderData& b) const
		{
			return  (cameraPosition - a.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength() >
				(cameraPosition - b.meshInstance->GetParentComponent()->GetWorldPosition()).SquareLength();
		}
	} cameraDistanceSorter;

	std::sort(
		transparentStaticMeshRenderData_.begin(),
		transparentStaticMeshRenderData_.end(),
		cameraDistanceSorter);

	std::sort(
		transparentInstancedStaticMeshRenderData_.begin(),
		transparentInstancedStaticMeshRenderData_.end(),
		cameraDistanceSorter);

	std::sort(
		transparentSkeletalMeshRenderData_.begin(),
		transparentSkeletalMeshRenderData_.end(),
		cameraDistanceSorter);

	std::sort(
		transparentDynamicMeshRenderData_.begin(),
		transparentDynamicMeshRenderData_.end(),
		cameraDistanceSorter);
}

GeometryBufferData::GeometryBufferData() :
	bufferWidth(engine->GetWindowManager()->GetWindowSize().x),
	bufferHeight(engine->GetWindowManager()->GetWindowSize().y)
{

}

GeometryBufferData::~GeometryBufferData()
{
	delete worldPositionTexture;
	delete worldNormalTexture;
	delete diffuseTexture;
	delete specularTexture;
	delete emisiveColorTexture;

	delete depthRenderbuffer;
	delete geometryFrameBuffer;
}

void GeometryBufferData::Init()
{
	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();

	bufferWidth = windowSize.x;
	bufferHeight = windowSize.y;

	GenerateBuffers();
}

void GeometryBufferData::Bind()
{
	geometryFrameBuffer->Bind();
}

void GeometryBufferData::Unbind()
{
	geometryFrameBuffer->Unbind();
}

void GeometryBufferData::GenerateBuffers()
{
	geometryFrameBuffer = new FrameBuffer();

	worldPositionTexture = new Texture();
	worldPositionTexture->SetName(SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION);
	worldPositionTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	worldPositionTexture->SetTextureFormat(TextureFormat::RGB);
	worldPositionTexture->SetTextureInternalFormat(TextureInternalFormat::RGB32F);
	worldPositionTexture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	worldPositionTexture->SetTextureMagFilter(TextureMagFilter::NEAREST);
	worldPositionTexture->SetWidth(bufferWidth);
	worldPositionTexture->SetHeight(bufferHeight);
	worldPositionTexture->SetGenerateMipmap(false);
	worldPositionTexture->PreInit();
	worldPositionTexture->Init();
	worldPositionTexture->PostInit();
	geometryFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT0, worldPositionTexture);

	worldNormalTexture = new Texture();
	worldNormalTexture->SetName(SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL);
	worldNormalTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	worldNormalTexture->SetTextureFormat(TextureFormat::RGBA);
	worldNormalTexture->SetTextureInternalFormat(TextureInternalFormat::RGBA16F);
	worldNormalTexture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	worldNormalTexture->SetTextureMagFilter(TextureMagFilter::NEAREST);
	worldNormalTexture->SetWidth(bufferWidth);
	worldNormalTexture->SetHeight(bufferHeight);
	worldNormalTexture->SetGenerateMipmap(false);
	worldNormalTexture->PreInit();
	worldNormalTexture->Init();
	worldNormalTexture->PostInit();
	geometryFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT1, worldNormalTexture);

	diffuseTexture = new Texture();
	diffuseTexture->SetName(SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE);
	diffuseTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	diffuseTexture->SetTextureFormat(TextureFormat::RGB);
	diffuseTexture->SetTextureInternalFormat(TextureInternalFormat::RGB);
	diffuseTexture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	diffuseTexture->SetTextureMagFilter(TextureMagFilter::NEAREST);
	diffuseTexture->SetWidth(bufferWidth);
	diffuseTexture->SetHeight(bufferHeight);
	diffuseTexture->SetGenerateMipmap(false);
	diffuseTexture->SetTextureType(TextureType::UNSIGNED_BYTE);
	diffuseTexture->PreInit();
	diffuseTexture->Init();
	diffuseTexture->PostInit();
	geometryFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT2, diffuseTexture);

	specularTexture = new Texture();
	specularTexture->SetName(SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR_PHONG);
	specularTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	specularTexture->SetTextureFormat(TextureFormat::RGBA);
	specularTexture->SetTextureInternalFormat(TextureInternalFormat::RGBA);
	specularTexture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	specularTexture->SetTextureMagFilter(TextureMagFilter::NEAREST);
	specularTexture->SetWidth(bufferWidth);
	specularTexture->SetHeight(bufferHeight);
	specularTexture->SetGenerateMipmap(false);
	specularTexture->SetTextureType(TextureType::UNSIGNED_BYTE);
	specularTexture->PreInit();
	specularTexture->Init();
	specularTexture->PostInit();
	geometryFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT3, specularTexture);

	emisiveColorTexture = new Texture();
	emisiveColorTexture->SetName(SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR);
	emisiveColorTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	emisiveColorTexture->SetTextureFormat(TextureFormat::RGB);
	emisiveColorTexture->SetTextureInternalFormat(TextureInternalFormat::RGB16F);
	emisiveColorTexture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	emisiveColorTexture->SetTextureMagFilter(TextureMagFilter::NEAREST);
	emisiveColorTexture->SetWidth(bufferWidth);
	emisiveColorTexture->SetHeight(bufferHeight);
	emisiveColorTexture->SetGenerateMipmap(false);
	emisiveColorTexture->SetTextureType(TextureType::FLOAT);
	emisiveColorTexture->PreInit();
	emisiveColorTexture->Init();
	emisiveColorTexture->PostInit();
	geometryFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT4, emisiveColorTexture);

	geometryFrameBuffer->PreInit();
	geometryFrameBuffer->Init();
	geometryFrameBuffer->PostInit();
	geometryFrameBuffer->Bind();
	geometryFrameBuffer->Attach();

	geometryFrameBuffer->DrawBuffers();

	depthRenderbuffer = new RenderBuffer();
	depthRenderbuffer->SetWidth(bufferWidth);
	depthRenderbuffer->SetHeight(bufferHeight);
	depthRenderbuffer->SetRenderBufferAttachment(RenderBufferAttachment::DEPTH_ATTACHMENT);
	depthRenderbuffer->SetRenderBufferBindTarget(RenderBufferBindTarget::RENDERBUFFER);
	depthRenderbuffer->SetRenderBufferInternalType(RenderBufferInternalType::DEPTH);

	depthRenderbuffer->PreInit();
	depthRenderbuffer->Init();
	depthRenderbuffer->PostInit();

	depthRenderbuffer->BindToFrameBuffer();

	geometryFrameBuffer->Unbind();

	EXIT_ON_GL_ERROR("GeometryBufferData::GenerateBuffers");
}

void GeometryBufferData::OnViewportSizeChanged(int width, int height)
{
	delete worldPositionTexture;
	delete worldNormalTexture;
	delete diffuseTexture;
	delete specularTexture;
	delete emisiveColorTexture;

	delete geometryFrameBuffer;

	delete depthRenderbuffer;

	bufferWidth = width;
	bufferHeight = height;
	GenerateBuffers();
}

void GeometryBufferData::BindGBufferDepth(FrameBuffer* drawFrameBuffer)
{
	geometryFrameBuffer->Bind(FrameBufferBindTarget::READ_FRAMEBUFFER);

	if (drawFrameBuffer)
	{
		drawFrameBuffer->Bind(FrameBufferBindTarget::DRAW_FRAMEBUFFER);
	}
	else
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	glBlitFramebuffer(
		0, 0, bufferWidth, bufferHeight,
		0, 0, bufferWidth, bufferHeight,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

DeferredRenderingData::DeferredRenderingData()
{
	geometryBufferData = new GeometryBufferData();

	MeshUnit* deferredRenderingMeshUnit = new MeshUnit();
	deferredRenderingMeshUnit->AddVertex(Vector3{ -1.f, -1.f, 0.f });
	deferredRenderingMeshUnit->AddVertex(Vector3{ 3.f, -1.f, 0.f });
	deferredRenderingMeshUnit->AddVertex(Vector3{ -1.f, 3.f, 0.f });
	deferredRenderingMeshUnit->AddFace(Face{ 0, 1, 2 });

	deferredRenderingMesh = new StaticMesh();
	deferredRenderingMesh->AddMesh(deferredRenderingMeshUnit);
	deferredRenderingMesh->PreInit();

	deferredRenderingMeshShader = new Shader();
}

DeferredRenderingData::~DeferredRenderingData()
{
	delete geometryBufferData;
	delete deferredRenderingMesh;
	delete deferredRenderingMeshShader;
	DestroySceneBuffers();
}

void DeferredRenderingData::PreInit()
{
}

void DeferredRenderingData::Init()
{
	deferredRenderingMeshShader->SetVertexShaderScript(ShaderBuilder::GetInstance()->DeferredRenderPass_GetVertexShaderScript());
	deferredRenderingMeshShader->SetFragmentShaderScript(ShaderBuilder::GetInstance()->DeferredRenderPass_GetFragmentShaderScript());

	deferredRenderingMeshShader->PreInit();
	deferredRenderingMeshShader->Init();
	deferredRenderingMeshShader->PostInit();

	engine->GetRenderer()->BindShadowTextures(deferredRenderingMeshShader);

	geometryBufferData->Init();
	GenerateSceneBuffers();

	SetShaderTextureUniforms();
}

void DeferredRenderingData::BindGeometryBuffer()
{
	geometryBufferData->Bind();
}

void DeferredRenderingData::UnbindGeometryBuffer()
{
	geometryBufferData->Unbind();
}

void DeferredRenderingData::BeginSceneRender()
{
	sceneFrameBuffer->Bind();
}

void DeferredRenderingData::EndSceneRender()
{
	sceneFrameBuffer->Unbind();
}

void DeferredRenderingData::SetShaderTextureUniforms()
{
	deferredRenderingMeshShader->Use();
	deferredRenderingMeshShader->SetInt(SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION, geometryBufferData->worldPositionTexture->GetRendererTextureId());
	deferredRenderingMeshShader->SetInt(SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL, geometryBufferData->worldNormalTexture->GetRendererTextureId());
	deferredRenderingMeshShader->SetInt(SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE, geometryBufferData->diffuseTexture->GetRendererTextureId());
	deferredRenderingMeshShader->SetInt(SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR_PHONG, geometryBufferData->specularTexture->GetRendererTextureId());
	deferredRenderingMeshShader->SetInt(SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR, geometryBufferData->emisiveColorTexture->GetRendererTextureId());
}

void DeferredRenderingData::Render()
{
	engine->GetRenderer()->BindStaticVBO();

	engine->GetRenderer()->BindShadowTextures(deferredRenderingMeshShader);
	BindGeometryBufferTextures(deferredRenderingMeshShader);
	SetShaderTextureUniforms();
	engine->GetRenderer()->SetLightUniforms(deferredRenderingMeshShader);

	MeshUnit* deferredRenderingMeshUnit = deferredRenderingMesh->GetSubMeshes()[0];
	int facePointCount = deferredRenderingMeshUnit->GetFaceCount() * 3;
	glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)(unsigned long long)deferredRenderingMeshUnit->GetVertexStartingIndex(), deferredRenderingMeshUnit->GetBaseVertex());
}

void DeferredRenderingData::OnViewportSizeChanged(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		return;
	}

	if (geometryBufferData->bufferWidth == width && geometryBufferData->bufferHeight == height)
	{
		return;
	}

	geometryBufferData->OnViewportSizeChanged(width, height);
	DestroySceneBuffers();
	GenerateSceneBuffers();
	SetShaderTextureUniforms();

	const std::vector<std::unique_ptr<Material>> &materials = engine->GetResourceManager()->GetMaterials();

	decltype(materials.begin()) materialIteration = materials.begin();
	for (; materialIteration < materials.end(); ++materialIteration)
	{
		Shader* shader = (*materialIteration)->GetShader(RenderPassType::GeometryBuffer);
		if (shader)
		{
			BindGeometryBufferTextures(shader);
		}
	}
}

void DeferredRenderingData::BindGeometryBufferTextures(Shader* shader)
{
	shader->Use();

	geometryBufferData->worldPositionTexture->Bind(shader);
	geometryBufferData->worldNormalTexture->Bind(shader);
	geometryBufferData->diffuseTexture->Bind(shader);
	geometryBufferData->specularTexture->Bind(shader);
	geometryBufferData->emisiveColorTexture->Bind(shader);
}

void DeferredRenderingData::BindGBufferDepth(FrameBuffer* drawFrameBuffer)
{
	geometryBufferData->BindGBufferDepth(drawFrameBuffer);
}

void DeferredRenderingData::BlitSceneTo(FrameBuffer* drawFrameBuffer) const
{
	BlitFrameBufferColor(sceneFrameBuffer, drawFrameBuffer, geometryBufferData->bufferWidth, geometryBufferData->bufferHeight);
}

void DeferredRenderingData::GenerateSceneBuffers()
{
	sceneFrameBuffer = new FrameBuffer();

	sceneTexture = new Texture();
	sceneTexture->SetName("sceneInputTexture");
	sceneTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	sceneTexture->SetTextureFormat(TextureFormat::RGBA);
	sceneTexture->SetTextureInternalFormat(TextureInternalFormat::RGBA16F);
	sceneTexture->SetTextureMinFilter(TextureMinFilter::LINEAR);
	sceneTexture->SetTextureMagFilter(TextureMagFilter::LINEAR);
	sceneTexture->SetWidth(geometryBufferData->bufferWidth);
	sceneTexture->SetHeight(geometryBufferData->bufferHeight);
	sceneTexture->SetGenerateMipmap(false);
	sceneTexture->SetTextureType(TextureType::FLOAT);
	sceneTexture->PreInit();
	sceneTexture->Init();
	sceneTexture->PostInit();

	sceneFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT0, sceneTexture);
	sceneFrameBuffer->PreInit();
	sceneFrameBuffer->Init();
	sceneFrameBuffer->PostInit();
	sceneFrameBuffer->Bind();
	sceneFrameBuffer->Attach();
	sceneFrameBuffer->DrawBuffers();

	sceneDepthRenderbuffer = new RenderBuffer();
	sceneDepthRenderbuffer->SetWidth(geometryBufferData->bufferWidth);
	sceneDepthRenderbuffer->SetHeight(geometryBufferData->bufferHeight);
	sceneDepthRenderbuffer->SetRenderBufferAttachment(RenderBufferAttachment::DEPTH_ATTACHMENT);
	sceneDepthRenderbuffer->SetRenderBufferBindTarget(RenderBufferBindTarget::RENDERBUFFER);
	sceneDepthRenderbuffer->SetRenderBufferInternalType(RenderBufferInternalType::DEPTH);
	sceneDepthRenderbuffer->PreInit();
	sceneDepthRenderbuffer->Init();
	sceneDepthRenderbuffer->PostInit();
	sceneDepthRenderbuffer->BindToFrameBuffer();

	sceneFrameBuffer->Unbind();
}

void DeferredRenderingData::DestroySceneBuffers()
{
	delete sceneTexture;
	sceneTexture = nullptr;

	delete sceneFrameBuffer;
	sceneFrameBuffer = nullptr;

	delete sceneDepthRenderbuffer;
	sceneDepthRenderbuffer = nullptr;
}
