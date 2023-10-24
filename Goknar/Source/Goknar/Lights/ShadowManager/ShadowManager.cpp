#include "pch.h"

#include "ShadowManager.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Material.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"

#include "Goknar/Lights/DirectionalLight.h"

#include "Goknar/IO/IOManager.h"
#include "Goknar/Renderer/Texture.h"

void ShadowManager::RenderShadowMaps()
{
	Renderer* renderer = engine->GetRenderer();
	renderer->SetIsRenderingOnlyDepth(true);

	Scene* mainScene = engine->GetApplication()->GetMainScene();

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();

	// Only draw the depth buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	const std::vector<DirectionalLight*> staticDirectionalLights = mainScene->GetStaticDirectionalLights();
	size_t staticDirectionalLightCount = staticDirectionalLights.size();
	for (size_t i = 0; i < staticDirectionalLightCount; i++)
	{
		DirectionalLight* directionalLight = staticDirectionalLights[i];
		if (!directionalLight->GetIsShadowEnabled())
		{
			continue;
		}

		Camera* shadowMapRenderCamera = directionalLight->GetShadowMapRenderCamera();
		Vector3 mainCameraForwardVector = mainCamera->GetForwardVector();
		Vector3 mainCameraPosition = mainCamera->GetPosition();
		Vector3 position{ mainCameraPosition.x, mainCameraPosition.y, 0.f};
		//shadowMapRenderCamera->SetPosition(position + 10.f * mainCameraForwardVector - 25.f * shadowMapRenderCamera->GetForwardVector());
		shadowMapRenderCamera->SetPosition(Vector3{20.f, 0.f, 0.f} - shadowMapRenderCamera->GetForwardVector() * 25.f);

		cameraManager->SetActiveCamera(shadowMapRenderCamera);
		Texture* shadowMapTexture = directionalLight->GetShadowMapTexture();
		glViewport(0, 0, shadowMapTexture->GetWidth(), shadowMapTexture->GetHeight());
		glBindFramebuffer(GL_FRAMEBUFFER, directionalLight->GetShadowMapFBO());
		renderer->Render();

		// For outputing only!
		//shadowMapTexture->ReadFromFramebuffer(directionalLight->GetShadowMapFBO());
		//shadowMapTexture->Save(CONTENT_DIR + directionalLight->GetName() + "FrameBufferTexture.png");
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	cameraManager->SetActiveCamera(mainCamera);
	engine->GetRenderer()->SetIsRenderingOnlyDepth(false);
}
