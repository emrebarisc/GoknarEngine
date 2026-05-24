#ifndef __RECT_H__
#define __RECT_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

class GOKNAR_API Rect
{
public:
	Rect();
	Rect(const Vector3& min, const Vector3& max);

	const Vector3& GetMin() const
	{
		return min_;
	}

	inline void SetMin(const Vector3& min)
	{
		min_ = min;
		CalculateSize();
	}

	inline const Vector3& GetMax() const
	{
		return max_;
	}

	inline void SetMax(const Vector3& max)
	{
		max_ = max;
		CalculateSize();
	}

	inline float GetMinX() const
	{
		return min_.x;
	}

	inline void SetMinX(float minX, bool recalculateSize = true)
	{
		min_.x = minX;
		if(recalculateSize)
		{
			CalculateSize();
		}
	}

	inline float GetMinY() const
	{
		return min_.y;
	}

	inline void SetMinY(float minY, bool recalculateSize = true)
	{
		min_.y = minY;
		if(recalculateSize)
		{
			CalculateSize();
		}
	}

	inline float GetMinZ() const
	{
		return min_.z;
	}

	inline void SetMinZ(float minZ, bool recalculateSize = true)
	{
		min_.z = minZ;
		if(recalculateSize)
		{
			CalculateSize();
		}
	}

	inline float GetMaxX() const
	{
		return max_.x;
	}

	inline void SetMaxX(float maxX, bool recalculateSize = true)
	{
		max_.x = maxX;
		if(recalculateSize)
		{
			CalculateSize();
		}
	}

	inline float GetMaxY() const
	{
		return max_.y;
	}

	inline void SetMaxY(float maxY, bool recalculateSize = true)
	{
		max_.y = maxY;
		if(recalculateSize)
		{
			CalculateSize();
		}
	}

	inline float GetMaxZ() const
	{
		return max_.z;
	}

	inline void SetMaxZ(float maxZ, bool recalculateSize = true)
	{
		max_.z = maxZ;
		if(recalculateSize)
		{
			CalculateSize();
		}
	}

	void CalculateSize()
	{
		depth_ = max_.x - min_.x;
		width_ = max_.y - min_.y;
		height_ = max_.z - min_.z;
	}

protected:

private:
	Vector3 min_;
	Vector3 max_;

	float width_;
	float height_;
	float depth_;
};

#endif