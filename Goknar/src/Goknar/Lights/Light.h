#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Goknar/Math.h"

class Shader;

enum LightMobility : uint8_t
{
	Static = 0,
	Movable
};

class GOKNAR_API Light
{
public:
	Light() :
		id_(0),
		position_(Vector3::ZeroVector), 
		color_(Vector3::ZeroVector), 
		intensity_(0.f), 
		name_(""),
		mobility_(LightMobility::Static)
	{}

	virtual void SetShaderUniforms(const Shader* shader) const;

    virtual ~Light()
    {
        
    }

	void SetPosition(const Vector3& position)
	{
		position_ = position;
	}

	const Vector3& GetPosition() const
    {
        return position_;
    }

	void SetColor(const Vector3& color)
	{
		color_ = color;
	}

	const Vector3& GetColor() const
	{
		return color_;
	}

	void SetIntensity(float intensity)
	{
		intensity_ = intensity;
	}

	float GetIntensity() const
	{
		return intensity_;
	}

	LightMobility GetLightMobility() const
	{
		return mobility_;
	}

	void SetLightMobility(LightMobility mobility)
	{
		mobility_ = mobility;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}
	
	int GetID() const
	{
		return id_;
	}

	void SetID(int id)
	{
		id_ = id;
	}

protected:

	Vector3 position_;
	Vector3 color_;

	std::string name_;

	int id_;

	float intensity_;

private:

	LightMobility mobility_;
};

#endif