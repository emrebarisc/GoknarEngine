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

	int GetAndIncreasePointLightID()
	{
		return pointLightID++;
	}

	int GetAndIncreaseDirectionalLightID()
	{
		return directionalLightID++;
	}

	int GetAndIncreaseSpotLightID()
	{
		return spotLightID++;
	}

	int GetAndIncreaseMaterialID()
	{
		return materialID++;
	}

	int GetAndIncreaseTextureID()
	{
		return textureID++;
	}

private:
	ObjectIDManager() :
		pointLightID(0),
		directionalLightID(0),
		spotLightID(0),
		materialID(0),
		textureID(0)
	{}

	static ObjectIDManager* instance_;

	int pointLightID;
	int directionalLightID;
	int spotLightID;
	int materialID;
	int textureID;
};

#endif