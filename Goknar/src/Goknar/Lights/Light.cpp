//#include "pch.h"

#include "Light.h"

#include "Goknar/Camera.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

Light::Light() :
	id_(0),
	position_(Vector3::ZeroVector),
	color_(Vector3::ZeroVector),
	intensity_(0.f),
	name_(""),
	shadowWidth_(1024),
	shadowHeight_(1024),
	shadowMapDepthFBO_(-1),
	isShadowEnabled_(true),
	mobility_(LightMobility::Static),
	shadowMapRenderCamera_(nullptr)
{
	shadowMapTexture_ = new Texture();
	shadowMapTexture_->SetWidth(shadowWidth_);
	shadowMapTexture_->SetHeight(shadowHeight_);
	shadowMapTexture_->SetTextureTarget(TextureTarget::TEXTURE_2D);
	shadowMapTexture_->SetTextureMinFilter(TextureMinFilter::NEAREST);
	shadowMapTexture_->SetTextureMagFilter(TextureMagFilter::NEAREST);
	shadowMapTexture_->SetTextureWrappingS(TextureWrapping::REPEAT);
	shadowMapTexture_->SetTextureWrappingT(TextureWrapping::REPEAT);
	shadowMapTexture_->SetTextureFormat(TextureFormat::DEPTH);
	shadowMapTexture_->SetTextureType(TextureType::FLOAT);
	shadowMapTexture_->SetTextureDataType(TextureDataType::DYNAMIC);
}

Light::~Light()
{
	delete shadowMapTexture_;
	delete shadowMapRenderCamera_;

	glDeleteFramebuffers(1, &shadowMapDepthFBO_);
}

void Light::SetShaderUniforms(const Shader* shader) const
{
	std::string positionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION;
	shader->SetVector3(positionName.c_str(), position_);

	std::string intensityName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
	shader->SetVector3(intensityName.c_str(), color_ * intensity_);
}

void Light::Init()
{
	glGenFramebuffers(1, &shadowMapDepthFBO_);
	shadowMapTexture_->Init();
	shadowMapTexture_->Bind();
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapDepthFBO_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, (int)shadowMapTexture_->GetTextureTarget(), shadowMapTexture_->GetRendererTextureId(), 0);
	
	switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
	case GL_FRAMEBUFFER_UNDEFINED:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_UNDEFINED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_UNSUPPORTED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
		break;
	case GL_FRAMEBUFFER_COMPLETE:
		GOKNAR_CORE_INFO("GL_FRAMEBUFFER_COMPLETE");
		break;
	default:
		break;
	}

	shadowMapTexture_->Unbind();
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
