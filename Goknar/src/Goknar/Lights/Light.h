#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Goknar/Math.h"

enum LightMobility : uint8_t
{
	Static = 0,
	Movable
};

class Light
{
public:
    Light() : position_(Vector3::ZeroVector), color_(Vector3::ZeroVector), intensity_(0.f), mobility_(LightMobility::Static)
    {
        
    }

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

protected:
	LightMobility mobility_;

private:
    Vector3 position_;
	Vector3 color_;
    float intensity_;
};

#endif