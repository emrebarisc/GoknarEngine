#ifndef __OBJECTIDMANAGER_H__
#define __OBJECTIDMANAGER_H__

class ObjectIDManager
{
public:
	static ObjectIDManager* GetInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new ObjectIDManager();
		}

		return instance_;
	}

	~ObjectIDManager()
	{
	}

	int GetAndIncreasePointLightID()
	{
		return pointLightID_++;
	}

	int GetAndIncreaseDirectionalLightID()
	{
		return directionalLightID_++;
	}

	int GetAndIncreaseSpotLightID()
	{
		return spotLightID_++;
	}

	int GetAndIncreaseMaterialID()
	{
		return materialID_++;
	}

	int GetAndIncreaseTextureID()
	{
		return textureID_++;
	}

	int GetAndIncreaseFramebufferID()
	{
		return framebufferID_++;
	}

private:
	ObjectIDManager() :
		pointLightID_(0),
		directionalLightID_(0),
		spotLightID_(0),
		materialID_(0),
		textureID_(0),
		framebufferID_(0)
	{}

	static ObjectIDManager* instance_;

	int pointLightID_;
	int directionalLightID_;
	int spotLightID_;
	int materialID_;
	int textureID_;
	int framebufferID_;
};

#endif