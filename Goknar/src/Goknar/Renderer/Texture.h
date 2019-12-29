#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Goknar/Managers/ObjectIDManager.h"
#include "Types.h"

class Shader;

class Texture
{
public:
	Texture() :
		imagePath_(""),
		buffer_(nullptr),
		width_(0),
		height_(0),
		channels_(0),
		rendererTextureId_(-1)
	{
		objectId_ = ObjectIDManager::GetInstance()->GetAndIncreaseTextureID();
		name_ = std::string("texture" + std::to_string(objectId_));
	}

	~Texture() 
	{
		delete[] buffer_;
	}

	void SetTextureImagePath(const std::string& imagePath)
	{
		imagePath_ = imagePath;
	}

	GEuint GetTextureObjectId() const
	{
		return objectId_;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}

	bool LoadTextureImage();

	void Init();
	void Bind(Shader* shader) const;

protected:

private:
	std::string name_;
	std::string imagePath_;
	unsigned char* buffer_;
	GEuint rendererTextureId_;

	int objectId_;
	int width_;
	int height_;
	int channels_;
};

#endif