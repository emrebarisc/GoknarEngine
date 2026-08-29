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

#include "Goknar/Graphics/IGraphicsAPI.h"

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
#include "Goknar/Model/Mesh.h"

#include "Goknar/ParticleSystem/ParticleSystemBase.h"

#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "Goknar/IO/IOManager.h"
#include "Goknar/Profiling/ProfileMacros.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/GPUFoliageSystem.h"
#include "Goknar/Renderer/ReflectionProbe.h"
#include "Goknar/Renderer/RenderTarget.h"
#include "Goknar/Renderer/PostProcessing/PostProcessing.h"
#include "Goknar/Renderer/PostProcessing/BloomPostProcessingEffect.h"
#include "Goknar/Renderer/PostProcessing/GammaCorrectionPostProcessingEffect.h"
#include "Goknar/Renderer/PostProcessing/ScreenSpaceReflectionPostProcessingEffect.h"
#include "Goknar/Renderer/PostProcessing/TemporalAntiAliasingPostProcessingEffect.h"

#include <algorithm>
#include <cfloat>
#include <climits>
#include <unordered_set>
#include <typeinfo>

#define VERTEX_COLOR_LOCATION 0
#define VERTEX_POSITION_LOCATION 1
#define VERTEX_NORMAL_LOCATION 2
#define VERTEX_UV_LOCATION 3
#define VERTEX_TANGENT_LOCATION 4
#define BONE_ID_LOCATION 5
#define BONE_WEIGHT_LOCATION 6
#define INSTANCE_TRANSFORMATION_ROW_0_LOCATION 5
#define INSTANCE_TRANSFORMATION_ROW_1_LOCATION 6
#define INSTANCE_TRANSFORMATION_ROW_2_LOCATION 7
#define INSTANCE_TRANSFORMATION_ROW_3_LOCATION 8

namespace
{
	IGraphicsAPI* GraphicsAPI()
	{
		return engine->GetGraphicsAPI();
	}

	void BlitFrameBufferColor(const FrameBuffer* readFrameBuffer, FrameBuffer* drawFrameBuffer, int width, int height)
	{
		if (!readFrameBuffer || width <= 0 || height <= 0)
		{
			return;
		}

		readFrameBuffer->Bind(FrameBufferBindTarget::READ_FRAMEBUFFER);
		GraphicsAPI()->ReadBuffer(FrameBufferAttachment::COLOR_ATTACHMENT0);

		if (drawFrameBuffer)
		{
			drawFrameBuffer->Bind(FrameBufferBindTarget::DRAW_FRAMEBUFFER);
		}
		else
		{
			GraphicsAPI()->BindFrameBuffer(FrameBufferBindTarget::DRAW_FRAMEBUFFER, 0);
		}

		GraphicsAPI()->BlitFrameBuffer(
			0, 0, width, height,
			0, 0, width, height,
			static_cast<GraphicsClearBufferFlags>(GraphicsClearBuffer::Color), GraphicsBlitFilter::Nearest);

		GraphicsAPI()->BindFrameBuffer(FrameBufferBindTarget::FRAMEBUFFER, 0);
	}

	const char* GetRenderPassProfileName(RenderPassType renderPassType)
	{
		switch (renderPassType)
		{
		case RenderPassType::Forward:
			return "Render Pass: Forward";
		case RenderPassType::Shadow:
			return "Render Pass: Shadow";
		case RenderPassType::PointLightShadow:
			return "Render Pass: Point Light Shadow";
		case RenderPassType::CubemapCapture:
			return "Render Pass: Cubemap Capture";
		case RenderPassType::GeometryBuffer:
			return "Render Pass: Geometry Buffer";
		case RenderPassType::Deferred:
			return "Render Pass: Deferred";
		case RenderPassType::None:
		default:
			return "Render Pass: Unknown";
		}
	}
}

Renderer::Renderer() :
	lightManager_(nullptr),
	removeStaticDataFromMemoryAfterTransferingToGPU_(false),
	drawOnWindow_(true)
{
}

Renderer::~Renderer()
{
	if (!deferredWindowSizeChangedDelegate_.isNull())
	{
		engine->GetWindowManager()->RemoveWindowSizeCallback(deferredWindowSizeChangedDelegate_);
	}

	delete temporalAntiAliasingPostProcessingEffect_;
	delete bloomPostProcessingEffect_;
	delete gammaCorrectionPostProcessingEffect_;
	delete screenSpaceReflectionPostProcessingEffect_;
	delete lightManager_;
	delete deferredRenderingData_;

	GraphicsAPI()->DeleteVertexArray(staticMeshBufferData_.vertexArrayId);
	for (const auto& [instancedStaticMesh, vertexArrayId] : instancedStaticMeshVertexArrayIdMap_)
	{
		if (vertexArrayId != 0)
		{
			GraphicsAPI()->DeleteVertexArray(vertexArrayId);
		}
	}
	instancedStaticMeshVertexArrayIdMap_.clear();
	GraphicsAPI()->DeleteVertexArray(skeletalMeshBufferData_.vertexArrayId);
	GraphicsAPI()->DeleteVertexArray(dynamicMeshBufferData_.vertexArrayId);

	GraphicsAPI()->DeleteBuffer(staticMeshBufferData_.vertexBufferId);
	for (const auto& [instancedStaticMesh, transformationBufferId] : instancedStaticMeshTransformationBufferIdMap_)
	{
		if (transformationBufferId != 0)
		{
			GraphicsAPI()->DeleteBuffer(transformationBufferId);
		}
	}
	instancedStaticMeshTransformationBufferIdMap_.clear();

	GraphicsAPI()->DeleteBuffer(skeletalMeshBufferData_.vertexBufferId);
	GraphicsAPI()->DeleteBuffer(dynamicMeshBufferData_.vertexBufferId);
	GraphicsAPI()->DeleteBuffer(staticMeshBufferData_.indexBufferId);
	GraphicsAPI()->DeleteBuffer(skeletalMeshBufferData_.indexBufferId);
	GraphicsAPI()->DeleteBuffer(dynamicMeshBufferData_.indexBufferId);

	EXIT_ON_GRAPHICS_API_ERROR("Renderer::~Renderer");
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
		deferredWindowSizeChangedDelegate_ = Delegate<void(int, int)>::Create<DeferredRenderingData, &DeferredRenderingData::OnViewportSizeChanged>(deferredRenderingData_);
		engine->GetWindowManager()->AddWindowSizeCallback(deferredWindowSizeChangedDelegate_);

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

		//if (!screenSpaceReflectionPostProcessingEffect_)
		//{
		//	screenSpaceReflectionPostProcessingEffect_ = new ScreenSpaceReflectionPostProcessingEffect();
		//	screenSpaceReflectionPostProcessingEffect_->PreInit();
		//	screenSpaceReflectionPostProcessingEffect_->Init();
		//	screenSpaceReflectionPostProcessingEffect_->PostInit();
		//	AddPostProcessingEffect(screenSpaceReflectionPostProcessingEffect_);
		//}

		if (!gammaCorrectionPostProcessingEffect_)
		{
			gammaCorrectionPostProcessingEffect_ = new GammaCorrectionPostProcessingEffect();
			gammaCorrectionPostProcessingEffect_->PreInit();
			gammaCorrectionPostProcessingEffect_->Init();
			gammaCorrectionPostProcessingEffect_->PostInit();
			AddPostProcessingEffect(gammaCorrectionPostProcessingEffect_);
		}
	}

	GraphicsAPI()->SetFrontFace(GraphicsFrontFace::CounterClockwise);
	GraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::DepthTest, true);
	GraphicsAPI()->SetDepthFunction(GraphicsDepthFunction::Lequal);

	for (MeshGeometry* subMesh : staticMeshGeometries_)
	{
		staticMeshBufferData_.vertexSize += (unsigned int)subMesh->GetVerticesPointer()->size();
		staticMeshBufferData_.faceSize += (unsigned int)subMesh->GetFacesPointer()->size();
	}

	for (SkeletalMeshGeometry* subMesh : skeletalMeshGeometries_)
	{
		skeletalMeshBufferData_.vertexSize += (unsigned int)subMesh->GetVerticesPointer()->size();
		skeletalMeshBufferData_.faceSize += (unsigned int)subMesh->GetFacesPointer()->size();
	}

	for (DynamicMeshGeometry* subMesh : dynamicMeshGeometries_)
	{
		dynamicMeshBufferData_.vertexSize += (unsigned int)subMesh->GetVerticesPointer()->size();
		dynamicMeshBufferData_.faceSize += (unsigned int)subMesh->GetFacesPointer()->size();
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
	staticMeshBufferData_.vertexArrayId = GraphicsAPI()->CreateVertexArray();
	GraphicsAPI()->BindVertexArray(staticMeshBufferData_.vertexArrayId);

	/*
		Vertex buffer
	*/
	unsigned long long sizeOfVertexData = sizeof(VertexData);

	staticMeshBufferData_.vertexBufferId = GraphicsAPI()->CreateBuffer();
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ArrayBuffer, staticMeshBufferData_.vertexBufferId);
	GraphicsAPI()->BufferData(GraphicsBufferTarget::ArrayBuffer, staticMeshBufferData_.vertexSize * sizeOfVertexData, nullptr, GraphicsBufferUsage::StaticDraw);

	/*
		Index buffer
	*/
	staticMeshBufferData_.indexBufferId = GraphicsAPI()->CreateBuffer();
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ElementArrayBuffer, staticMeshBufferData_.indexBufferId);
	GraphicsAPI()->BufferData(GraphicsBufferTarget::ElementArrayBuffer, staticMeshBufferData_.faceSize * sizeof(Face), nullptr, GraphicsBufferUsage::StaticDraw);

	/*
		Buffer Sub-Data
	*/
	for (MeshGeometry* subMesh : staticMeshGeometries_)
	{
		subMesh->SetBaseVertex(staticMeshBufferData_.baseVertex);
		subMesh->SetVertexStartingIndex(staticMeshBufferData_.vertexStartingIndex);

		const VertexArray* vertexArrayPtr = subMesh->GetVerticesPointer();
		int vertexSizeInBytes = (int)vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		GraphicsAPI()->BufferSubData(GraphicsBufferTarget::ArrayBuffer, staticMeshBufferData_.vertexOffset, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = subMesh->GetFacesPointer();
		int faceSizeInBytes = (int)faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		GraphicsAPI()->BufferSubData(GraphicsBufferTarget::ElementArrayBuffer, staticMeshBufferData_.faceOffset, faceSizeInBytes, &faceArrayPtr->at(0));

		staticMeshBufferData_.vertexOffset += vertexSizeInBytes;
		staticMeshBufferData_.faceOffset += faceSizeInBytes;

		staticMeshBufferData_.baseVertex += subMesh->GetVertexCount();
		staticMeshBufferData_.vertexStartingIndex += subMesh->GetFaceCount() * 3 * (int)sizeof(Face::vertexIndices[0]);

		if (removeStaticDataFromMemoryAfterTransferingToGPU_)
		{
			subMesh->ClearDataFromMemory();
		}
	}
	SetAttribPointers();
	GraphicsAPI()->BindVertexArray(0);
}

void Renderer::SetSkeletalBufferData()
{
	skeletalMeshBufferData_.vertexArrayId = GraphicsAPI()->CreateVertexArray();
	GraphicsAPI()->BindVertexArray(skeletalMeshBufferData_.vertexArrayId);

	/*
		Vertex buffer
	*/
	unsigned long long int sizeOfSkeletalMeshVertexData = sizeof(VertexData) + sizeof(VertexBoneData);

	skeletalMeshBufferData_.vertexBufferId = GraphicsAPI()->CreateBuffer();
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ArrayBuffer, skeletalMeshBufferData_.vertexBufferId);
	GraphicsAPI()->BufferData(GraphicsBufferTarget::ArrayBuffer, skeletalMeshBufferData_.vertexSize * sizeOfSkeletalMeshVertexData, nullptr, GraphicsBufferUsage::StaticDraw);

	/*
		Index buffer
	*/
	skeletalMeshBufferData_.indexBufferId = GraphicsAPI()->CreateBuffer();
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ElementArrayBuffer, skeletalMeshBufferData_.indexBufferId);
	GraphicsAPI()->BufferData(GraphicsBufferTarget::ElementArrayBuffer, skeletalMeshBufferData_.faceSize * sizeof(Face), nullptr, GraphicsBufferUsage::StaticDraw);

	/*
		Buffer Sub-Data
	*/
	for (SkeletalMeshGeometry* subMesh : skeletalMeshGeometries_)
	{
		subMesh->SetBaseVertex(skeletalMeshBufferData_.baseVertex);
		subMesh->SetVertexStartingIndex(skeletalMeshBufferData_.vertexStartingIndex);

		const VertexArray* vertexArrayPtr = subMesh->GetVerticesPointer();

		unsigned int vertexArrayPtrSize = vertexArrayPtr->size();
		if (vertexArrayPtrSize == 0)
		{
			continue;
		}

		GEintptr vertexSizeInBytes = sizeof(vertexArrayPtr->at(0));

		const VertexBoneDataArray* vertexBoneDataArray = subMesh->GetVertexBoneDataArray();
		int vertexBoneDataArraySizeInBytes = sizeof(vertexBoneDataArray->at(0));
		for (unsigned int i = 0; i < vertexArrayPtrSize; ++i)
		{
			GraphicsAPI()->BufferSubData(GraphicsBufferTarget::ArrayBuffer, skeletalMeshBufferData_.vertexOffset, vertexSizeInBytes, &vertexArrayPtr->at(i));
			skeletalMeshBufferData_.vertexOffset += vertexSizeInBytes;

			GraphicsAPI()->BufferSubData(GraphicsBufferTarget::ArrayBuffer, skeletalMeshBufferData_.vertexOffset, vertexBoneDataArraySizeInBytes, &vertexBoneDataArray->at(i));
			skeletalMeshBufferData_.vertexOffset += vertexBoneDataArraySizeInBytes;
		}

		const FaceArray* faceArrayPtr = subMesh->GetFacesPointer();
		int faceSizeInBytes = (int)faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		GraphicsAPI()->BufferSubData(GraphicsBufferTarget::ElementArrayBuffer, skeletalMeshBufferData_.faceOffset, faceSizeInBytes, &faceArrayPtr->at(0));
		skeletalMeshBufferData_.faceOffset += faceSizeInBytes;

		skeletalMeshBufferData_.baseVertex += subMesh->GetVertexCount();
		skeletalMeshBufferData_.vertexStartingIndex += subMesh->GetFaceCount() * 3 * (int)sizeof(Face::vertexIndices[0]);

		if (removeStaticDataFromMemoryAfterTransferingToGPU_)
		{
			subMesh->ClearDataFromMemory();
		}
	}
	SetAttribPointersForSkeletalMesh();
	GraphicsAPI()->BindVertexArray(0);
}

void Renderer::SetDynamicBufferData()
{
	dynamicMeshBufferData_.vertexArrayId = GraphicsAPI()->CreateVertexArray();
	GraphicsAPI()->BindVertexArray(dynamicMeshBufferData_.vertexArrayId);

	/*
		Vertex buffer
	*/
	unsigned long long sizeOfVertexData = sizeof(VertexData);

	dynamicMeshBufferData_.vertexBufferId = GraphicsAPI()->CreateBuffer();
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ArrayBuffer, dynamicMeshBufferData_.vertexBufferId);
	GraphicsAPI()->BufferData(GraphicsBufferTarget::ArrayBuffer, dynamicMeshBufferData_.vertexSize * sizeOfVertexData, nullptr, GraphicsBufferUsage::DynamicDraw);

	/*
		Index buffer
	*/
	dynamicMeshBufferData_.indexBufferId = GraphicsAPI()->CreateBuffer();
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ElementArrayBuffer, dynamicMeshBufferData_.indexBufferId);
	GraphicsAPI()->BufferData(GraphicsBufferTarget::ElementArrayBuffer, dynamicMeshBufferData_.faceSize * sizeof(Face), nullptr, GraphicsBufferUsage::DynamicDraw);

	/*
		Buffer Sub-Data
	*/
	for (DynamicMeshGeometry* subMesh : dynamicMeshGeometries_)
	{
		subMesh->SetBaseVertex(dynamicMeshBufferData_.baseVertex);
		subMesh->SetVertexStartingIndex(dynamicMeshBufferData_.vertexStartingIndex);
		subMesh->SetRendererVertexOffset(dynamicMeshBufferData_.vertexOffset);

		const VertexArray* vertexArrayPtr = subMesh->GetVerticesPointer();
		int vertexSizeInBytes = (int)vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		GraphicsAPI()->BufferSubData(GraphicsBufferTarget::ArrayBuffer, dynamicMeshBufferData_.vertexOffset, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = subMesh->GetFacesPointer();
		int faceSizeInBytes = (int)faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		GraphicsAPI()->BufferSubData(GraphicsBufferTarget::ElementArrayBuffer, dynamicMeshBufferData_.faceOffset, faceSizeInBytes, &faceArrayPtr->at(0));

		dynamicMeshBufferData_.vertexOffset += vertexSizeInBytes;
		dynamicMeshBufferData_.faceOffset += faceSizeInBytes;

		dynamicMeshBufferData_.baseVertex += subMesh->GetVertexCount();
		dynamicMeshBufferData_.vertexStartingIndex += subMesh->GetFaceCount() * 3 * (int)sizeof(Face::vertexIndices[0]);
	}

	SetAttribPointers();
	GraphicsAPI()->BindVertexArray(0);
}

void Renderer::SetBufferData()
{
	if (0 < staticMeshBufferData_.meshCount) SetStaticBufferData();
	if (0 < skeletalMeshBufferData_.meshCount) SetSkeletalBufferData();
	if (0 < dynamicMeshBufferData_.meshCount) SetDynamicBufferData();
}

void Renderer::RenderCurrentFrame()
{
	GOKNAR_PROFILE_FUNCTION();

	countDrawCallsInner_ = false;

	PrepareSkeletalMeshInstancesForTheCurrentFrame();
	CaptureReflectionProbes();

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

			if (GetMainRenderTypeInt() & (int)RenderPassType::Forward)
			{
				renderTargetFrameBuffer->Bind();
				Render(RenderPassType::Forward);
				renderTargetFrameBuffer->Unbind();
			}
			else if (GetMainRenderTypeInt() & (int)RenderPassType::Deferred)
			{
				deferredRenderingData_ = renderTarget->GetDeferredRenderingData();

				PreparePostProcessingFrame(deferredRenderingData_);
				Render(RenderPassType::GeometryBuffer);
				Render(RenderPassType::Deferred);
				
				if (renderTarget->GetRequirePostProcessingEffects())
				{
					ApplyPostProcessing(deferredRenderingData_, renderTargetFrameBuffer);
				}
				else
				{
					const int width = deferredRenderingData_->geometryBufferData->bufferWidth;
					const int height = deferredRenderingData_->geometryBufferData->bufferHeight;
					Texture* finalTexture = deferredRenderingData_->GetSceneTexture();
					FrameBuffer* finalFrameBuffer = deferredRenderingData_->GetSceneFrameBuffer();
					BlitFrameBufferColor(finalFrameBuffer, renderTargetFrameBuffer, width, height);
				}

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
		if (GetMainRenderTypeInt() & (int)RenderPassType::Forward)
		{
			countDrawCallsInner_ = countDrawCalls;
			Render(RenderPassType::Forward);
		}
		else if (GetMainRenderTypeInt() & (int)RenderPassType::Deferred)
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
		GraphicsAPI()->ClearColor(0.f, 0.f, 0.f, 1.f);
		GraphicsAPI()->Clear(GraphicsClearBuffer::Color | GraphicsClearBuffer::Depth);
	}

	PrepareSkeletalMeshInstancesForTheNextFrame();
}

void Renderer::ApplyPostProcessing(DeferredRenderingData* deferredRenderingData, FrameBuffer* destinationFrameBuffer)
{
	GOKNAR_PROFILE_SCOPE("Apply Post Processing");

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
		if (!postProcessingEffect || !postProcessingEffect->GetIsActive())
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
	GOKNAR_PROFILE_SCOPE(GetRenderPassProfileName(renderPassType));

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();

	if (!activeCamera)
	{
		return;
	}

	switch (renderPassType)
	{
	case RenderPassType::Forward:
	{
		GraphicsAPI()->SetDepthMask(true);
		const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
		GraphicsAPI()->ClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
		GraphicsAPI()->Clear(GraphicsClearBuffer::Color | GraphicsClearBuffer::Depth);
		break;
	}
	case RenderPassType::GeometryBuffer:
	{
		GOKNAR_CORE_CHECK(deferredRenderingData_ != nullptr, "Main rendering is not set to deferred rendering but deferred rendering is called.");

		deferredRenderingData_->BindGeometryBuffer();
		const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
		GraphicsAPI()->ClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
		GraphicsAPI()->Clear(GraphicsClearBuffer::Color | GraphicsClearBuffer::Depth);
		GraphicsAPI()->SetDepthMask(true);
		break;
	}
	case RenderPassType::Deferred:
	{
		GOKNAR_CORE_CHECK(deferredRenderingData_ != nullptr, "Main rendering is not set to deferred rendering but deferred rendering is called.");
		deferredRenderingData_->BeginSceneRender();
		const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
		GraphicsAPI()->ClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
		GraphicsAPI()->Clear(GraphicsClearBuffer::Color | GraphicsClearBuffer::Depth);
		deferredRenderingData_->Render();
		break;
	}
	case RenderPassType::CubemapCapture:
	{
		GraphicsAPI()->SetDepthMask(true);
		const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
		GraphicsAPI()->ClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
		GraphicsAPI()->Clear(GraphicsClearBuffer::Color | GraphicsClearBuffer::Depth);
		break;
	}
	case RenderPassType::Shadow:
	case RenderPassType::PointLightShadow:
	{
		GraphicsAPI()->Clear(static_cast<GraphicsClearBufferFlags>(GraphicsClearBuffer::Depth));
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
	const bool skipFrustumCulling =
		renderPassType == RenderPassType::PointLightShadow ||
		renderPassType == RenderPassType::CubemapCapture;
	const RenderPassType meshUnitRenderPassType =
		renderPassType == RenderPassType::Deferred ?
		RenderPassType::Forward :
		renderPassType;

	SortOpaqueInstances();

	auto RenderStaticMesh = [&](const StaticMeshRenderData& renderData)
		{
			StaticMeshInstance* staticMeshInstance = renderData.meshInstance;
			StaticMesh* meshContainer = renderData.mesh;

			const Box& meshContainerAABB = meshContainer->GetAABB();
			const Matrix& componentToWorldTransformationMatrix = staticMeshInstance->GetParentComponent()->GetComponentToWorldTransformationMatrix();

			float meshContainerWindowCoverage = activeCamera->GetAABBFrameCoverage(meshContainerAABB, componentToWorldTransformationMatrix);

			const int forcedLODIndex = staticMeshInstance->GetForcedLODIndex();
			size_t LODIndex = forcedLODIndex < 0 ? meshContainer->GetLODIndex(meshContainerWindowCoverage) : (size_t)forcedLODIndex;
			const StaticMeshLOD* LODMesh = meshContainer->GetLOD((int)LODIndex);
			if (!LODMesh)
			{
				return;
			}

			int subMeshIndex = renderData.subMeshIndex;
			if (subMeshIndex < 0 || (int)LODMesh->GetSubMeshes().size() <= subMeshIndex)
			{
				return;
			}

			const MeshGeometry* subMesh = LODMesh->GetMesh(subMeshIndex);

			if (!skipFrustumCulling &&
				!activeCamera->IsAABBVisible(meshContainerAABB, componentToWorldTransformationMatrix)) return;

			if (countDrawCallsInner_) ++drawCallCount;

			staticMeshInstance->PreRender(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);
			staticMeshInstance->Render(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);

			int facePointCount = subMesh->GetFaceCount() * 3;
			GraphicsAPI()->DrawElementsBaseVertex(GraphicsPrimitive::Triangles, facePointCount, GraphicsDataType::UnsignedInt, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
		};

	auto RenderInstancedStaticMesh = [&](const InstancedStaticMeshRenderData& renderData)
		{
			InstancedStaticMeshInstance* instancedStaticMeshInstance = renderData.meshInstance;
			InstancedStaticMesh* meshContainer = renderData.mesh;
			const Box& meshContainerAABB = meshContainer->GetAABB();
			const Matrix& componentToWorldTransformationMatrix = instancedStaticMeshInstance->GetParentComponent()->GetComponentToWorldTransformationMatrix();
			float meshContainerWindowCoverage = activeCamera->GetAABBFrameCoverage(meshContainerAABB, componentToWorldTransformationMatrix);
			const int forcedLODIndex = instancedStaticMeshInstance->GetForcedLODIndex();
			size_t LODIndex = forcedLODIndex < 0 ? meshContainer->GetLODIndex(meshContainerWindowCoverage) : (size_t)forcedLODIndex;
			InstancedStaticMeshLOD* instancedStaticMesh = meshContainer->GetLOD((int)LODIndex);
			if (!instancedStaticMesh)
			{
				return;
			}

			const int subMeshIndex = renderData.subMeshIndex;
			if (subMeshIndex < 0 || (int)instancedStaticMesh->GetSubMeshes().size() <= subMeshIndex)
			{
				return;
			}

			const int instanceCount = (int)instancedStaticMesh->GetInstanceCount();
			if (instanceCount <= 0)
			{
				return;
			}

			const MeshGeometry* subMesh = instancedStaticMesh->GetMesh(subMeshIndex);

			if (!skipFrustumCulling &&
				!activeCamera->IsAABBVisible(instancedStaticMesh->GetSubMeshInstanceAABB((size_t)subMeshIndex), componentToWorldTransformationMatrix)) return;

			if (!BindInstancedStaticMesh(instancedStaticMesh))
			{
				return;
			}

			if (countDrawCallsInner_) ++drawCallCount;

			instancedStaticMeshInstance->PreRender(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);
			instancedStaticMeshInstance->Render(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);

			int facePointCount = subMesh->GetFaceCount() * 3;
			GraphicsAPI()->DrawElementsInstancedBaseVertex(
				GraphicsPrimitive::Triangles,
				facePointCount,
				GraphicsDataType::UnsignedInt,
				(void*)(unsigned long long)subMesh->GetVertexStartingIndex(),
				instanceCount,
				subMesh->GetBaseVertex());
		};

	auto RenderSkeletalMesh = [&](const SkeletalMeshRenderData& renderData)
		{
			SkeletalMeshInstance* skeletalMeshInstance = renderData.meshInstance;
			SkeletalMesh* meshContainer = renderData.mesh;
			const Box& meshContainerAABB = meshContainer->GetAABB();
			const Matrix& componentToWorldTransformationMatrix = skeletalMeshInstance->GetParentComponent()->GetComponentToWorldTransformationMatrix();
			float meshContainerWindowCoverage = activeCamera->GetAABBFrameCoverage(meshContainerAABB, componentToWorldTransformationMatrix);
			const int forcedLODIndex = skeletalMeshInstance->GetForcedLODIndex();
			size_t LODIndex = forcedLODIndex < 0 ? meshContainer->GetLODIndex(meshContainerWindowCoverage) : (size_t)forcedLODIndex;
			const SkeletalMeshLOD* LODMesh = meshContainer->GetLOD((int)LODIndex);
			if (!LODMesh)
			{
				return;
			}

			const int subMeshIndex = renderData.subMeshIndex;
			if (subMeshIndex < 0 || (int)LODMesh->GetSubMeshes().size() <= subMeshIndex)
			{
				return;
			}

			const SkeletalMeshGeometry* subMesh = LODMesh->GetMesh(subMeshIndex);

			if (!skipFrustumCulling &&
				!activeCamera->IsAABBVisible(subMesh->GetAABB(), componentToWorldTransformationMatrix)) return;

			if (countDrawCallsInner_) ++drawCallCount;

			skeletalMeshInstance->PreRender(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);
			skeletalMeshInstance->Render(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);

			int facePointCount = subMesh->GetFaceCount() * 3;
			GraphicsAPI()->DrawElementsBaseVertex(GraphicsPrimitive::Triangles, facePointCount, GraphicsDataType::UnsignedInt, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
		};

	auto RenderDynamicMesh = [&](const DynamicMeshRenderData& renderData)
		{
			DynamicMeshInstance* dynamicMeshInstance = renderData.meshInstance;
			DynamicMesh* meshContainer = renderData.mesh;
			const Box& meshContainerAABB = meshContainer->GetAABB();
			const Matrix& componentToWorldTransformationMatrix = dynamicMeshInstance->GetParentComponent()->GetComponentToWorldTransformationMatrix();
			float meshContainerWindowCoverage = activeCamera->GetAABBFrameCoverage(meshContainerAABB, componentToWorldTransformationMatrix);
			const int forcedLODIndex = dynamicMeshInstance->GetForcedLODIndex();
			size_t LODIndex = forcedLODIndex < 0 ? meshContainer->GetLODIndex(meshContainerWindowCoverage) : (size_t)forcedLODIndex;
			const DynamicMeshLOD* LODMesh = meshContainer->GetLOD((int)LODIndex);
			if (!LODMesh)
			{
				return;
			}

			const int subMeshIndex = renderData.subMeshIndex;
			if (subMeshIndex < 0 || (int)LODMesh->GetSubMeshes().size() <= subMeshIndex)
			{
				return;
			}

			const DynamicMeshGeometry* subMesh = LODMesh->GetMesh(subMeshIndex);

			if (!skipFrustumCulling &&
				!activeCamera->IsAABBVisible(subMesh->GetAABB(), componentToWorldTransformationMatrix)) return;

			if (countDrawCallsInner_) ++drawCallCount;

			dynamicMeshInstance->PreRender(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);
			dynamicMeshInstance->Render(meshUnitRenderPassType, subMeshIndex, (int)LODIndex);

			int facePointCount = subMesh->GetFaceCount() * 3;
			GraphicsAPI()->DrawElementsBaseVertex(GraphicsPrimitive::Triangles, facePointCount, GraphicsDataType::UnsignedInt, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
		};

	if (renderPassType != RenderPassType::Deferred)
	{
		// Static MeshGeometry Instances
		{
			if (0 < staticMeshBufferData_.meshCount)
			{
				BindStaticVAO();

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

		// Skeletal MeshGeometry Instances
		{
			if (0 < skeletalMeshBufferData_.meshCount)
			{
				BindSkeletalVAO();

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

		// Dynamic MeshGeometry Instances
		{
			if (0 < dynamicMeshBufferData_.meshCount)
			{
				BindDynamicVAO();

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

		for (GPUFoliageSystem* gpuFoliageSystem : gpuFoliageSystems_)
		{
			if (!gpuFoliageSystem)
			{
				continue;
			}

			const int foliageDrawCount = gpuFoliageSystem->Render(activeCamera, renderPassType, skipFrustumCulling);
			if (countDrawCallsInner_)
			{
				drawCallCount += foliageDrawCount;
			}
		}
	}
	else
	{
		deferredRenderingData_->BindGBufferDepth(deferredRenderingData_->GetSceneFrameBuffer());
	}

	if (renderPassType == RenderPassType::Forward ||
		renderPassType == RenderPassType::Deferred ||
		renderPassType == RenderPassType::CubemapCapture)
	{
		GraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::Blend, false);
		GraphicsAPI()->SetDepthMask(true);
		for (ParticleSystemBase* particleSystem : particleSystems_)
		{
			if (!particleSystem || !particleSystem->GetIsActive())
			{
				continue;
			}

			const std::uint32_t particleDrawCount = particleSystem->Render(activeCamera, ParticleRenderStage::Opaque);
			if (countDrawCallsInner_)
			{
				drawCallCount += particleDrawCount;
			}
		}

		SortTransparentInstances();

		GraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::Blend, true);
		GraphicsAPI()->SetDepthMask(false);

		BindStaticVAO();

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

		BindSkeletalVAO();
		for (const SkeletalMeshRenderData& transparentSkeletalMeshRenderData : transparentSkeletalMeshRenderData_)
		{
			SkeletalMeshInstance* transparentSkeletalMeshInstance = transparentSkeletalMeshRenderData.meshInstance;
			if (!transparentSkeletalMeshInstance->GetIsRendered()) continue;
			if (!(activeCamera->GetRenderMask() & transparentSkeletalMeshInstance->GetRenderMask())) continue;

			RenderSkeletalMesh(transparentSkeletalMeshRenderData);
		}

		BindDynamicVAO();
		for (const DynamicMeshRenderData& transparentDynamicMeshRenderData : transparentDynamicMeshRenderData_)
		{
			DynamicMeshInstance* transparentDynamicMeshInstance = transparentDynamicMeshRenderData.meshInstance;
			if (!transparentDynamicMeshInstance->GetIsRendered()) continue;
			if (!(activeCamera->GetRenderMask() & transparentDynamicMeshInstance->GetRenderMask())) continue;

			RenderDynamicMesh(transparentDynamicMeshRenderData);
		}

		for (ParticleSystemBase* particleSystem : particleSystems_)
		{
			if (!particleSystem || !particleSystem->GetIsActive())
			{
				continue;
			}

			const std::uint32_t particleDrawCount = particleSystem->Render(activeCamera, ParticleRenderStage::Transparent);
			if (countDrawCallsInner_)
			{
				drawCallCount += particleDrawCount;
			}
		}
		GraphicsAPI()->SetDepthMask(true);
		GraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::Blend, false);

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

void Renderer::AddStaticMeshToRenderer(StaticMeshLOD* staticMesh)
{
	for (MeshGeometry* subMesh : staticMesh->GetSubMeshes())
	{
		staticMeshGeometries_.push_back(subMesh);
		staticMeshBufferData_.meshCount++;
	}
}

void Renderer::AddInstancedStaticMeshToRenderer(InstancedStaticMeshLOD* instancedStaticMesh)
{
	instancedStaticMeshes_.push_back(instancedStaticMesh);
	AddStaticMeshToRenderer(instancedStaticMesh);
}

void Renderer::AddStaticMeshInstance(StaticMeshInstance* meshInstance)
{
	StaticMesh* staticMeshContainer = meshInstance->GetMesh();
	if (!staticMeshContainer || staticMeshContainer->GetLODCount() == 0)
	{
		return;
	}

	const StaticMeshLOD* LOD0Mesh = staticMeshContainer->GetLOD(0);
	if (!LOD0Mesh)
	{
		return;
	}

	const std::vector<MeshGeometry*>& subMeshes = LOD0Mesh->GetSubMeshes();
	size_t subMeshCount = subMeshes.size();
	for (int subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex)
	{
		StaticMeshRenderData renderData{ meshInstance, staticMeshContainer, subMeshIndex };
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
	InstancedStaticMesh* instancedStaticMeshContainer = instancedStaticMeshInstance->GetMesh();
	if (!instancedStaticMeshContainer || instancedStaticMeshContainer->GetLODCount() == 0)
	{
		return;
	}

	const InstancedStaticMeshLOD* LOD0Mesh = instancedStaticMeshContainer->GetLOD(0);
	if (!LOD0Mesh)
	{
		return;
	}

	const std::vector<MeshGeometry*>& subMeshes = LOD0Mesh->GetSubMeshes();
	size_t subMeshCount = subMeshes.size();
	for (int subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex)
	{
		InstancedStaticMeshRenderData renderData{ instancedStaticMeshInstance, instancedStaticMeshContainer, subMeshIndex };
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

void Renderer::AddSkeletalMeshToRenderer(SkeletalMeshLOD* skeletalMesh)
{
	for (SkeletalMeshGeometry* subMesh : skeletalMesh->GetSubMeshes())
	{
		skeletalMeshGeometries_.push_back(subMesh);
		skeletalMeshBufferData_.meshCount++;
	}
}

void Renderer::AddSkeletalMeshInstance(SkeletalMeshInstance* skeletalMeshInstance)
{
	SkeletalMesh* skeletalMeshContainer = skeletalMeshInstance->GetMesh();
	if (!skeletalMeshContainer || skeletalMeshContainer->GetLODCount() == 0)
	{
		return;
	}

	const SkeletalMeshLOD* LOD0Mesh = skeletalMeshContainer->GetLOD(0);
	if (!LOD0Mesh)
	{
		return;
	}

	const std::vector<SkeletalMeshGeometry*>& subMeshes = LOD0Mesh->GetSubMeshes();
	size_t subMeshCount = subMeshes.size();
	for (int subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex)
	{
		SkeletalMeshRenderData renderData{ skeletalMeshInstance, skeletalMeshContainer, subMeshIndex };
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

void Renderer::AddDynamicMeshToRenderer(DynamicMeshLOD* dynamicMesh)
{
	for (DynamicMeshGeometry* subMesh : dynamicMesh->GetSubMeshes())
	{
		dynamicMeshGeometries_.push_back(subMesh);
	}
	dynamicMeshBufferData_.meshCount++;
}

void Renderer::AddDynamicMeshInstance(DynamicMeshInstance* dynamicMeshInstance)
{
	DynamicMesh* dynamicMeshContainer = dynamicMeshInstance->GetMesh();
	if (!dynamicMeshContainer || dynamicMeshContainer->GetLODCount() == 0)
	{
		return;
	}

	const DynamicMeshLOD* LOD0Mesh = dynamicMeshContainer->GetLOD(0);
	if (!LOD0Mesh)
	{
		return;
	}

	const std::vector<DynamicMeshGeometry*>& subMeshes = LOD0Mesh->GetSubMeshes();
	size_t subMeshCount = subMeshes.size();
	for (int subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex)
	{
		DynamicMeshRenderData renderData{ dynamicMeshInstance, dynamicMeshContainer, subMeshIndex };
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

void Renderer::AddGPUFoliageSystem(GPUFoliageSystem* foliageSystem)
{
	if (!foliageSystem)
	{
		return;
	}

	if (std::find(gpuFoliageSystems_.begin(), gpuFoliageSystems_.end(), foliageSystem) != gpuFoliageSystems_.end())
	{
		return;
	}

	gpuFoliageSystems_.push_back(foliageSystem);
}

void Renderer::RemoveGPUFoliageSystem(GPUFoliageSystem* foliageSystem)
{
	gpuFoliageSystems_.erase(
		std::remove(gpuFoliageSystems_.begin(), gpuFoliageSystems_.end(), foliageSystem),
		gpuFoliageSystems_.end());
}

void Renderer::AddParticleSystem(ParticleSystemBase* particleSystem)
{
	if (!particleSystem)
	{
		return;
	}

	if (std::find(particleSystems_.begin(), particleSystems_.end(), particleSystem) != particleSystems_.end())
	{
		return;
	}

	particleSystems_.push_back(particleSystem);
}

void Renderer::RemoveParticleSystem(ParticleSystemBase* particleSystem)
{
	particleSystems_.erase(
		std::remove(particleSystems_.begin(), particleSystems_.end(), particleSystem),
		particleSystems_.end());
}

void Renderer::UpdateDynamicMeshVertex(const DynamicMeshGeometry* object, int vertexIndex, const VertexData& newVertexData)
{
	int sizeOfVertexData = sizeof(VertexData);
	GraphicsAPI()->NamedBufferSubData(dynamicMeshBufferData_.vertexBufferId, object->GetRendererVertexOffset() + vertexIndex * sizeOfVertexData, sizeOfVertexData, &newVertexData);
}

void Renderer::RefreshInstancedStaticMeshTransformations(const InstancedStaticMeshLOD* instancedStaticMesh)
{
	if (!instancedStaticMesh || staticMeshBufferData_.vertexBufferId == 0)
	{
		return;
	}

	GEuint& transformationBufferId = instancedStaticMeshTransformationBufferIdMap_[instancedStaticMesh];
	if (transformationBufferId == 0)
	{
		transformationBufferId = GraphicsAPI()->CreateBuffer();
	}

	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ArrayBuffer, transformationBufferId);

	const std::vector<Matrix>& instanceTransformationMatrices = instancedStaticMesh->GetInstanceTransformationMatrices();
	const void* bufferData = instanceTransformationMatrices.empty() ? nullptr : instanceTransformationMatrices.data();
	GraphicsAPI()->BufferData(
		GraphicsBufferTarget::ArrayBuffer,
		(GEsizeiptr)(instanceTransformationMatrices.size() * sizeof(Matrix)),
		bufferData,
		GraphicsBufferUsage::DynamicDraw);

	const_cast<InstancedStaticMeshLOD*>(instancedStaticMesh)->ClearPendingFullTransformUpload();
}

void Renderer::UpdateInstancedStaticMeshTransformation(const InstancedStaticMeshLOD* instancedStaticMesh, int transformationIndex, const Matrix& newTransformationMatrix)
{
	if (!instancedStaticMesh || staticMeshBufferData_.vertexBufferId == 0 || transformationIndex < 0)
	{
		return;
	}

	auto transformationBufferIterator = instancedStaticMeshTransformationBufferIdMap_.find(instancedStaticMesh);
	if (transformationBufferIterator == instancedStaticMeshTransformationBufferIdMap_.end())
	{
		return;
	}

	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ArrayBuffer, transformationBufferIterator->second);
	GraphicsAPI()->BufferSubData(
		GraphicsBufferTarget::ArrayBuffer,
		(GEintptr)(transformationIndex * sizeof(Matrix)),
		sizeof(Matrix),
		&newTransformationMatrix);
}

void Renderer::PrepareSkeletalMeshInstancesForTheCurrentFrame()
{
	GOKNAR_PROFILE_FUNCTION();

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
	GOKNAR_PROFILE_FUNCTION();

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
	directionalLightTextureIndices.reserve((std::min)(directionalLights.size(), static_cast<size_t>(MAX_DIRECTIONAL_LIGHT_COUNT)));
	for (DirectionalLight* directionalLight : directionalLights)
	{
		if (!(directionalLightTextureIndices.size() < MAX_DIRECTIONAL_LIGHT_COUNT))
		{
			break;
		}

		if (!directionalLight || !directionalLight->GetIsActive())
		{
			continue;
		}

		int textureIndex = 0;
		if (directionalLight->GetIsShadowEnabled())
		{
			Texture* shadowTexture = directionalLight->GetShadowMapTexture();
			if (shadowTexture)
			{
				textureIndex = shadowTexture->GetRendererTextureId();
				shadowTexture->Bind(shader);
			}
		}

		directionalLightTextureIndices.push_back(textureIndex);
	}
	shader->SetIntVector(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_ARRAY_NAME, directionalLightTextureIndices);

	std::vector<int> pointLightTextureIndices;
	const std::vector<PointLight*>& pointLights = scene->GetPointLights();
	pointLightTextureIndices.reserve((std::min)(pointLights.size(), static_cast<size_t>(MAX_POINT_LIGHT_COUNT)));
	for (PointLight* pointLight : pointLights)
	{
		if (!(pointLightTextureIndices.size() < MAX_POINT_LIGHT_COUNT))
		{
			break;
		}

		if (!pointLight || !pointLight->GetIsActive())
		{
			continue;
		}

		int textureIndex = 0;
		if (pointLight->GetIsShadowEnabled())
		{
			Texture* shadowTexture = pointLight->GetShadowMapTexture();
			if (shadowTexture)
			{
				textureIndex = shadowTexture->GetRendererTextureId();
				shadowTexture->Bind(shader);
			}
		}

		pointLightTextureIndices.push_back(textureIndex);
	}
	shader->SetIntVector(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_ARRAY_NAME, pointLightTextureIndices);

	std::vector<int> spotLightTextureIndices;
	const std::vector<SpotLight*>& spotLights = scene->GetSpotLights();
	spotLightTextureIndices.reserve((std::min)(spotLights.size(), static_cast<size_t>(MAX_SPOT_LIGHT_COUNT)));
	for (SpotLight* spotLight : spotLights)
	{
		if (!(spotLightTextureIndices.size() < MAX_SPOT_LIGHT_COUNT))
		{
			break;
		}

		if (!spotLight || !spotLight->GetIsActive())
		{
			continue;
		}

		int textureIndex = 0;
		if (spotLight->GetIsShadowEnabled())
		{
			Texture* shadowTexture = spotLight->GetShadowMapTexture();
			if (shadowTexture)
			{
				textureIndex = shadowTexture->GetRendererTextureId();
				shadowTexture->Bind(shader);
			}
		}

		spotLightTextureIndices.push_back(textureIndex);
	}
	shader->SetIntVector(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_ARRAY_NAME, spotLightTextureIndices);
}

void Renderer::CaptureReflectionProbes()
{
	Scene* scene = engine->GetApplication()->GetMainScene();
	if (!scene)
	{
		return;
	}

	for (ReflectionProbe* reflectionProbe : scene->GetReflectionProbes())
	{
		if (!reflectionProbe || !reflectionProbe->GetIsActive())
		{
			continue;
		}

		if (!reflectionProbe->GetCaptureEveryFrame() && !reflectionProbe->GetNeedsCapture())
		{
			continue;
		}

		reflectionProbe->Capture();
	}
}

void Renderer::AddPostProcessingEffect(PostProcessingEffect* postProcessingEffect)
{
	postProcessingEffects_.push_back(postProcessingEffect);
}

void Renderer::RemovePostProcessingEffect(PostProcessingEffect* postProcessingEffect)
{
	std::vector<PostProcessingEffect*>::const_iterator postProcessingEffectIterator = postProcessingEffects_.cbegin();
	while (postProcessingEffectIterator != postProcessingEffects_.cend())
	{
		if (postProcessingEffect == *postProcessingEffectIterator)
		{
			postProcessingEffects_.erase(postProcessingEffectIterator);
			break;
		}

		++postProcessingEffectIterator;
	}
}
void Renderer::AddRenderTarget(const RenderTarget* renderTarget)
{
	if (!renderTarget)
	{
		return;
	}

	if (std::find(renderTargets_.begin(), renderTargets_.end(), renderTarget) == renderTargets_.end())
	{
		renderTargets_.push_back(renderTarget);
	}
}

void Renderer::RemoveRenderTarget(const RenderTarget* renderTarget)
{
	renderTargets_.erase(
		std::remove(renderTargets_.begin(), renderTargets_.end(), renderTarget),
		renderTargets_.end()
	);
}

void Renderer::BindGeometryBufferTextures(Shader* shader)
{
	if (deferredRenderingData_)
	{
		shader->Use();
		deferredRenderingData_->BindGeometryBufferTextures(shader);
	}
}

const ReflectionProbe* Renderer::GetClosestReflectionProbe(const Vector3& worldPosition) const
{
	Scene* scene = engine->GetApplication()->GetMainScene();
	if (!scene)
	{
		return nullptr;
	}

	const ReflectionProbe* closestReflectionProbe = nullptr;
	const ReflectionProbe* closestContainingReflectionProbe = nullptr;
	float closestDistanceSquared = FLT_MAX;
	float closestContainingDistanceSquared = FLT_MAX;

	for (const ReflectionProbe* reflectionProbe : scene->GetReflectionProbes())
	{
		if (!reflectionProbe || !reflectionProbe->GetIsActive() || !reflectionProbe->GetIsInitialized() || !reflectionProbe->GetCubemapTexture())
		{
			continue;
		}

		const float distanceSquared = (reflectionProbe->GetPosition() - worldPosition).SquareLength();
		if (reflectionProbe->ContainsWorldPosition(worldPosition) && distanceSquared < closestContainingDistanceSquared)
		{
			closestContainingDistanceSquared = distanceSquared;
			closestContainingReflectionProbe = reflectionProbe;
		}

		if (distanceSquared < closestDistanceSquared)
		{
			closestDistanceSquared = distanceSquared;
			closestReflectionProbe = reflectionProbe;
		}
	}

	return closestContainingReflectionProbe ? closestContainingReflectionProbe : closestReflectionProbe;
}

void Renderer::SetReflectionProbeUniforms(Shader* shader) const
{
	if (!shader)
	{
		return;
	}

	shader->Use();

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	const ReflectionProbe* reflectionProbe =
		activeCamera ?
		GetClosestReflectionProbe(activeCamera->GetPosition()) :
		nullptr;

	if (!reflectionProbe)
	{
		shader->SetBool(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::HAS_REFLECTION_PROBE, false);
		shader->SetInt(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP, 0);
		shader->SetVector3(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::POSITION, Vector3::ZeroVector);
		shader->SetVector3(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MIN, Vector3::ZeroVector);
		shader->SetVector3(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MAX, Vector3::ZeroVector);
		return;
	}

	Texture* cubemapTexture = reflectionProbe->GetCubemapTexture();
	const Vector3 probePosition = reflectionProbe->GetPosition();
	const Vector3 probeHalfSize = reflectionProbe->GetSize() * 0.5f;

	cubemapTexture->BindToTextureUnit(cubemapTexture->GetRendererTextureId());
	shader->SetBool(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::HAS_REFLECTION_PROBE, true);
	shader->SetInt(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP, cubemapTexture->GetRendererTextureId());
	shader->SetVector3(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::POSITION, probePosition);
	shader->SetVector3(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MIN, probePosition - probeHalfSize);
	shader->SetVector3(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MAX, probePosition + probeHalfSize);
}

void Renderer::SetCubemapRenderPassShaderUniforms(const Shader* shader) const
{
	if (currentReflectionProbeCapture_)
	{
		currentReflectionProbeCapture_->SetRenderPassShaderUniforms(shader);
	}
}

void Renderer::SetLightUniforms(Shader* shader)
{
	if (lightManager_)
	{
		lightManager_->BindLightUniforms(shader);
	}
}

void Renderer::RenderStaticMesh(StaticMeshLOD* staticMesh)
{
	BindStaticVAO();

	for (MeshGeometry* subMesh : staticMesh->GetSubMeshes())
	{
		int facePointCount = subMesh->GetFaceCount() * 3;
		GraphicsAPI()->DrawElementsBaseVertex(GraphicsPrimitive::Triangles, facePointCount, GraphicsDataType::UnsignedInt, (void*)(unsigned long long)subMesh->GetVertexStartingIndex(), subMesh->GetBaseVertex());
	}
}

void Renderer::BindStaticMeshBuffers()
{
	BindStaticVAO();
}

void Renderer::BindStaticVAO()
{
	GraphicsAPI()->BindVertexArray(staticMeshBufferData_.vertexArrayId);
}

bool Renderer::BindInstancedStaticMesh(InstancedStaticMeshLOD* instancedStaticMesh)
{
	if (!instancedStaticMesh || staticMeshBufferData_.vertexArrayId == 0)
	{
		return false;
	}

	if (instancedStaticMesh->HasPendingFullTransformUpload())
	{
		RefreshInstancedStaticMeshTransformations(instancedStaticMesh);
	}

	auto bufferIterator = instancedStaticMeshTransformationBufferIdMap_.find(instancedStaticMesh);
	if (bufferIterator == instancedStaticMeshTransformationBufferIdMap_.end())
	{
		RefreshInstancedStaticMeshTransformations(instancedStaticMesh);
		bufferIterator = instancedStaticMeshTransformationBufferIdMap_.find(instancedStaticMesh);
		if (bufferIterator == instancedStaticMeshTransformationBufferIdMap_.end())
		{
			return false;
		}
	}

	const GEuint transformationBufferId = bufferIterator->second;
	if (transformationBufferId == 0)
	{
		return false;
	}

	GEuint& vertexArrayId = instancedStaticMeshVertexArrayIdMap_[instancedStaticMesh];
	if (vertexArrayId == 0)
	{
		vertexArrayId = GraphicsAPI()->CreateVertexArray();
		GraphicsAPI()->BindVertexArray(vertexArrayId);

		GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ArrayBuffer, staticMeshBufferData_.vertexBufferId);
		GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ElementArrayBuffer, staticMeshBufferData_.indexBufferId);
		SetAttribPointers();

		GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ArrayBuffer, transformationBufferId);
		SetAttribPointersForInstancedStaticMesh();

		GraphicsAPI()->BindVertexArray(0);
	}

	GraphicsAPI()->BindVertexArray(vertexArrayId);
	return true;
}

void Renderer::BindSkeletalVAO()
{
	GraphicsAPI()->BindVertexArray(skeletalMeshBufferData_.vertexArrayId);
}

void Renderer::BindDynamicVAO()
{
	GraphicsAPI()->BindVertexArray(dynamicMeshBufferData_.vertexArrayId);
}

void Renderer::SetAttribPointers()
{
	GEsizei sizeOfVertexData = (GEsizei)sizeof(VertexData);
	// Vertex color
	long long offset = 0;
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_COLOR_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_COLOR_LOCATION, 4, GraphicsDataType::Float, false, sizeOfVertexData, (void*)offset);

	// Vertex position
	offset += sizeof(VertexData::color);
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_POSITION_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_POSITION_LOCATION, 3, GraphicsDataType::Float, false, sizeOfVertexData, (void*)offset);

	// Vertex normal
	offset += sizeof(VertexData::position);
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_NORMAL_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_NORMAL_LOCATION, 3, GraphicsDataType::Float, false, sizeOfVertexData, (void*)offset);

	// Vertex UV
	offset += sizeof(VertexData::normal);
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_UV_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_UV_LOCATION, 2, GraphicsDataType::Float, false, sizeOfVertexData, (void*)offset);

	// Vertex tangent
	offset += sizeof(VertexData::uv);
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_TANGENT_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_TANGENT_LOCATION, 4, GraphicsDataType::Float, false, sizeOfVertexData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(VERTEX_TANGENT_LOCATION, 0);

	GraphicsAPI()->DisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_0_LOCATION);
	GraphicsAPI()->DisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_1_LOCATION);
	GraphicsAPI()->DisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_2_LOCATION);
	GraphicsAPI()->DisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_3_LOCATION);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_0_LOCATION, 0);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_1_LOCATION, 0);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_2_LOCATION, 0);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_3_LOCATION, 0);
}

void Renderer::SetAttribPointersForInstancedStaticMesh()
{
	GEsizei sizeOfInstanceTransformationData = (GEsizei)sizeof(Matrix);
	long long offset = 0;

	GraphicsAPI()->EnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_0_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_0_LOCATION, 4, GraphicsDataType::Float, false, sizeOfInstanceTransformationData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_0_LOCATION, 1);
	offset += sizeof(Vector4);

	GraphicsAPI()->EnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_1_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_1_LOCATION, 4, GraphicsDataType::Float, false, sizeOfInstanceTransformationData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_1_LOCATION, 1);
	offset += sizeof(Vector4);

	GraphicsAPI()->EnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_2_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_2_LOCATION, 4, GraphicsDataType::Float, false, sizeOfInstanceTransformationData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_2_LOCATION, 1);
	offset += sizeof(Vector4);

	GraphicsAPI()->EnableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_3_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(INSTANCE_TRANSFORMATION_ROW_3_LOCATION, 4, GraphicsDataType::Float, false, sizeOfInstanceTransformationData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(INSTANCE_TRANSFORMATION_ROW_3_LOCATION, 1);
}

void Renderer::SetAttribPointersForSkeletalMesh()
{
	GEsizei sizeOfSkeletalMeshVertexData = (GEsizei)(sizeof(VertexData) + sizeof(VertexBoneData));

	long long offset = 0;
	// Vertex color
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_COLOR_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_COLOR_LOCATION, 4, GraphicsDataType::Float, false, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::color);

	// Vertex position
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_POSITION_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_POSITION_LOCATION, 3, GraphicsDataType::Float, false, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::position);

	// Vertex normal
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_NORMAL_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_NORMAL_LOCATION, 3, GraphicsDataType::Float, false, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::normal);

	// Vertex UV
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_UV_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_UV_LOCATION, 2, GraphicsDataType::Float, false, sizeOfSkeletalMeshVertexData, (void*)offset);
	offset += sizeof(VertexData::uv);

	// Vertex tangent
	GraphicsAPI()->EnableVertexAttribArray(VERTEX_TANGENT_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(VERTEX_TANGENT_LOCATION, 4, GraphicsDataType::Float, false, sizeOfSkeletalMeshVertexData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(VERTEX_TANGENT_LOCATION, 0);
	offset += sizeof(VertexData::tangent);

	// Bone ID
	GraphicsAPI()->EnableVertexAttribArray(BONE_ID_LOCATION);
	GraphicsAPI()->SetVertexAttribIPointer(BONE_ID_LOCATION, MAX_BONE_SIZE_PER_VERTEX, GraphicsDataType::UnsignedInt, sizeOfSkeletalMeshVertexData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(BONE_ID_LOCATION, 0);
	offset += sizeof(VertexBoneData::boneIDs);

	// Bone Weight
	GraphicsAPI()->EnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	GraphicsAPI()->SetVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_BONE_SIZE_PER_VERTEX, GraphicsDataType::Float, false, sizeOfSkeletalMeshVertexData, (void*)offset);
	GraphicsAPI()->SetVertexAttribDivisor(BONE_WEIGHT_LOCATION, 0);

	GraphicsAPI()->DisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_2_LOCATION);
	GraphicsAPI()->DisableVertexAttribArray(INSTANCE_TRANSFORMATION_ROW_3_LOCATION);
}

void Renderer::SortOpaqueInstances()
{
	auto getMaterialAtlasKey = [](const IMaterialBase* material) -> unsigned long long
	{
		if (!material)
		{
			return ULLONG_MAX;
		}

		const Shader* shader = material->GetShader(RenderPassType::Forward);
		const std::vector<const Texture*>* textures = shader ? shader->GetTextures() : nullptr;
		if (!textures || textures->empty())
		{
			return ULLONG_MAX - 1;
		}

		unsigned long long key = ULLONG_MAX - 2;
		for (const Texture* texture : *textures)
		{
			if (!texture)
			{
				continue;
			}

			const unsigned long long textureKey = texture->GetUsesAtlasTexture() ?
				(static_cast<unsigned long long>(texture->GetTextureAtlasCategory()) << 56) | static_cast<unsigned long long>((std::max)(texture->GetTextureAtlasIndex(), 0)) :
				(1ull << 63) | static_cast<unsigned long long>(texture->GetRendererTextureId());
			key = (std::min)(key, textureKey);
		}

		return key;
	};

	auto sortByAtlas = [&getMaterialAtlasKey](auto& renderDataList)
	{
		std::stable_sort(
			renderDataList.begin(),
			renderDataList.end(),
			[&getMaterialAtlasKey](const auto& left, const auto& right)
			{
				return getMaterialAtlasKey(left.meshInstance->GetMaterial(left.subMeshIndex)) <
					getMaterialAtlasKey(right.meshInstance->GetMaterial(right.subMeshIndex));
			});
	};

	sortByAtlas(opaqueStaticMeshRenderData_);
	sortByAtlas(opaqueInstancedStaticMeshRenderData_);
	sortByAtlas(opaqueSkeletalMeshRenderData_);
	sortByAtlas(opaqueDynamicMeshRenderData_);
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
	delete ambientOcclusionMetallicRoughnessTexture;
	delete emissiveColorTexture;

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
	worldNormalTexture->SetTextureFormat(TextureFormat::RGB);
	worldNormalTexture->SetTextureInternalFormat(TextureInternalFormat::RGB16F);
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
	diffuseTexture->SetTextureFormat(TextureFormat::RGBA);
	diffuseTexture->SetTextureInternalFormat(TextureInternalFormat::RGBA);
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

	ambientOcclusionMetallicRoughnessTexture = new Texture();
	ambientOcclusionMetallicRoughnessTexture->SetName(SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS);
	ambientOcclusionMetallicRoughnessTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	ambientOcclusionMetallicRoughnessTexture->SetTextureFormat(TextureFormat::RGBA);
	ambientOcclusionMetallicRoughnessTexture->SetTextureInternalFormat(TextureInternalFormat::RGBA);
	ambientOcclusionMetallicRoughnessTexture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	ambientOcclusionMetallicRoughnessTexture->SetTextureMagFilter(TextureMagFilter::NEAREST);
	ambientOcclusionMetallicRoughnessTexture->SetWidth(bufferWidth);
	ambientOcclusionMetallicRoughnessTexture->SetHeight(bufferHeight);
	ambientOcclusionMetallicRoughnessTexture->SetGenerateMipmap(false);
	ambientOcclusionMetallicRoughnessTexture->SetTextureType(TextureType::UNSIGNED_BYTE);
	ambientOcclusionMetallicRoughnessTexture->PreInit();
	ambientOcclusionMetallicRoughnessTexture->Init();
	ambientOcclusionMetallicRoughnessTexture->PostInit();
	geometryFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT3, ambientOcclusionMetallicRoughnessTexture);

	emissiveColorTexture = new Texture();
	emissiveColorTexture->SetName(SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR);
	emissiveColorTexture->SetTextureDataType(TextureDataType::DYNAMIC);
	emissiveColorTexture->SetTextureFormat(TextureFormat::RGB);
	emissiveColorTexture->SetTextureInternalFormat(TextureInternalFormat::RGB16F);
	emissiveColorTexture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	emissiveColorTexture->SetTextureMagFilter(TextureMagFilter::NEAREST);
	emissiveColorTexture->SetWidth(bufferWidth);
	emissiveColorTexture->SetHeight(bufferHeight);
	emissiveColorTexture->SetGenerateMipmap(false);
	emissiveColorTexture->SetTextureType(TextureType::FLOAT);
	emissiveColorTexture->PreInit();
	emissiveColorTexture->Init();
	emissiveColorTexture->PostInit();
	geometryFrameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT4, emissiveColorTexture);

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

	EXIT_ON_GRAPHICS_API_ERROR("GeometryBufferData::GenerateBuffers");
}

void GeometryBufferData::OnViewportSizeChanged(int width, int height)
{
	delete worldPositionTexture;
	delete worldNormalTexture;
	delete diffuseTexture;
	delete ambientOcclusionMetallicRoughnessTexture;
	delete emissiveColorTexture;

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
		GraphicsAPI()->BindFrameBuffer(FrameBufferBindTarget::DRAW_FRAMEBUFFER, 0);
	}

	GraphicsAPI()->BlitFrameBuffer(
		0, 0, bufferWidth, bufferHeight,
		0, 0, bufferWidth, bufferHeight,
		static_cast<GraphicsClearBufferFlags>(GraphicsClearBuffer::Depth), GraphicsBlitFilter::Nearest);
}

DeferredRenderingData::DeferredRenderingData()
{
	geometryBufferData = new GeometryBufferData();

	MeshGeometry* deferredRenderingMeshGeometry = new MeshGeometry();
	deferredRenderingMeshGeometry->AddVertex(Vector3{ -1.f, -1.f, 0.f });
	deferredRenderingMeshGeometry->AddVertex(Vector3{ 3.f, -1.f, 0.f });
	deferredRenderingMeshGeometry->AddVertex(Vector3{ -1.f, 3.f, 0.f });
	deferredRenderingMeshGeometry->AddFace(Face{ 0, 1, 2 });

	deferredRenderingMesh = new StaticMeshLOD();
	deferredRenderingMesh->AddMesh(deferredRenderingMeshGeometry);
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
	deferredRenderingMeshShader->SetInt(
		SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS,
		geometryBufferData->ambientOcclusionMetallicRoughnessTexture->GetRendererTextureId());
	deferredRenderingMeshShader->SetInt(SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR, geometryBufferData->emissiveColorTexture->GetRendererTextureId());
}

void DeferredRenderingData::Render()
{
	engine->GetRenderer()->BindStaticMeshBuffers();
	engine->GetRenderer()->BindShadowTextures(deferredRenderingMeshShader);
	BindGeometryBufferTextures(deferredRenderingMeshShader);
	SetShaderTextureUniforms();
	engine->GetRenderer()->SetLightUniforms(deferredRenderingMeshShader);
	engine->GetRenderer()->SetReflectionProbeUniforms(deferredRenderingMeshShader);

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	if (activeCamera)
	{
		deferredRenderingMeshShader->SetVector3(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION, activeCamera->GetPosition());
	}

	deferredRenderingMeshShader->SetFloat(SHADER_VARIABLE_NAMES::TIMING::DELTA_TIME, engine->GetDeltaTime());
	deferredRenderingMeshShader->SetFloat(SHADER_VARIABLE_NAMES::TIMING::ELAPSED_TIME, engine->GetElapsedTime());

	MeshGeometry* deferredRenderingMeshGeometry = deferredRenderingMesh->GetSubMeshes()[0];
	int facePointCount = deferredRenderingMeshGeometry->GetFaceCount() * 3;
	GraphicsAPI()->DrawElementsBaseVertex(GraphicsPrimitive::Triangles, facePointCount, GraphicsDataType::UnsignedInt, (void*)(unsigned long long)deferredRenderingMeshGeometry->GetVertexStartingIndex(), deferredRenderingMeshGeometry->GetBaseVertex());
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
	geometryBufferData->ambientOcclusionMetallicRoughnessTexture->Bind(shader);
	geometryBufferData->emissiveColorTexture->Bind(shader);
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
