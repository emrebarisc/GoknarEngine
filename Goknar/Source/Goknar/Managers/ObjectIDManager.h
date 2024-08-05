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

	int GetAndIncreaseObjectBaseGUID()
	{
		return objectBaseGUID_++;
	}
	
	int GetAndIncreaseComponentID()
	{
		return componentGUID_++;
	}

	int GetAndIncreasePointLightGUID()
	{
		return pointLightGUID_++;
	}

	int GetAndIncreaseDirectionalLightGUID()
	{
		return directionalLightGUID_++;
	}

	int GetAndIncreaseSpotLightGUID()
	{
		return spotLightGUID_++;
	}

	int GetAndIncreaseMaterialGUID()
	{
		return materialGUID_++;
	}

	int GetAndIncreaseTextureGUID()
	{
		return textureGUID_++;
	}

	int GetAndIncreaseFrameBufferGUID()
	{
		return framebufferGUID_++;
	}

	int GetAndIncreaseRenderBufferGUID()
	{
		return renderbufferGUID_++;
	}

private:
	ObjectIDManager() :
		objectBaseGUID_(0),
		componentGUID_(0),
		pointLightGUID_(0),
		directionalLightGUID_(0),
		spotLightGUID_(0),
		materialGUID_(0),
		textureGUID_(0),
		framebufferGUID_(0),
		renderbufferGUID_(0)
	{}

	static ObjectIDManager* instance_;

	int objectBaseGUID_;
	int componentGUID_;
	int pointLightGUID_;
	int directionalLightGUID_;
	int spotLightGUID_;
	int materialGUID_;
	int textureGUID_;
	int framebufferGUID_;
	int renderbufferGUID_;
};

#endif