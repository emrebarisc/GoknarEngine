#include "pch.h"

#include "Contents/Image.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Renderer/Texture.h"

Image::Image() :
	Content(),
	width_(0),
	height_(0),
	channels_(0),
	textureUsage_(TextureUsage::None),
	buffer_(nullptr),
	generatedTexture_(nullptr)
{

}

Image::Image(const std::string& path) :
	Content(path),
	width_(0),
	height_(0),
	channels_(0),
	textureUsage_(TextureUsage::None),
	buffer_(nullptr),
	generatedTexture_(nullptr)
{
}

Image::Image(const std::string& path, int width, int height, int channels, unsigned char* buffer) :
	Content(path),
	width_(width),
	height_(height),
	channels_(channels),
	textureUsage_(TextureUsage::None),
	buffer_(buffer),
	generatedTexture_(nullptr)
{
}

void Image::PreInit()
{
	// TODO
	// For now a texture is generated for every image
	// Will be changed to generating texture atlases
	generatedTexture_ = new Texture(this);
	
	if (!name_.empty())
	{
		generatedTexture_->SetName(name_);
	}

	generatedTexture_->PreInit();
	engine->GetApplication()->GetMainScene()->AddTexture(generatedTexture_);
}

void Image::Init()
{
	generatedTexture_->Init();
}

void Image::PostInit()
{
	generatedTexture_->PostInit();
}
