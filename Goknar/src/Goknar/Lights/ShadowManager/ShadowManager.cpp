//#include "pch.h"

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

ShadowManager::ShadowManager() :
	depthBufferMaterial_(nullptr),
	depthBufferShader_(nullptr)
{
	ShaderBuilder* shaderBuilder = ShaderBuilder::GetInstance();
	if (shaderBuilder->GetIsInstatiated())
	{
		depthBufferMaterial_ = new Material();
		depthBufferShader_ = new Shader();
		depthBufferShader_->SetShaderType(ShaderType::SelfContained);
		depthBufferShader_->SetVertexShaderScript(shaderBuilder->GetDefaultSceneVertexShader());
		//depthBufferShader_->SetFragmentShaderScript(shaderBuilder->GetDefaultSceneFragmentShader());
//		depthBufferShader_->SetFragmentShaderScript("#version " + shaderBuilder->GetShaderVersion() + R"(
//layout(location = 0) out vec3 fragmentColor;
//in vec3 fragmentPosition;
//void main()
//{
//	fragmentColor = vec3(gl_FragCoord.z);
//	//fragmentColor = vec3(0.f, /*gl_FragDepth*/fragmentPosition.z / 10.f, 0.f);
//}
//)");

		depthBufferShader_->SetFragmentShaderScript("#version " + shaderBuilder->GetShaderVersion() + R"(
float near = 0.1; 
float far  = 100.0; 

layout(location = 0) out vec4 fragmentColor;  

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    fragmentColor = vec4(vec3(depth), 1.0);
}
)");
		depthBufferMaterial_->SetShader(depthBufferShader_);
		depthBufferMaterial_->SetShadingModel(MaterialShadingModel::TwoSided);
		depthBufferMaterial_->Init();
	}
	else
	{
		GOKNAR_CORE_ERROR("ShadowManager is created before ShaderBuilder is instatiated!");
	}
}

ShadowManager::~ShadowManager()
{
	delete depthBufferMaterial_;
	delete depthBufferShader_;
}

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

		cameraManager->SetActiveCamera(directionalLight->GetShadowMapRenderCamera());
		Texture* shadowMapTexture = directionalLight->GetShadowMapTexture();
		glViewport(0, 0, shadowMapTexture->GetWidth(), shadowMapTexture->GetHeight());
		glBindFramebuffer(GL_FRAMEBUFFER, directionalLight->GetShadowMapFBO());
		renderer->Render();
		//shadowMapTexture->ReadFromFramebuffer(directionalLight->GetShadowMapFBO());
		//shadowMapTexture->Save(std::string("./") + directionalLight->GetName() + "FrameBufferTexture.png");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	cameraManager->SetActiveCamera(mainCamera);
	engine->GetRenderer()->SetIsRenderingOnlyDepth(false);
}
