#ifndef __RECT_H__
#define __RECT_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

class GOKNAR_API Rect
{
public:
	Rect();
	Rect(const Vector2& min, const Vector2& max);

	const Vector2& GetMin() const
	{
		return min_;
	}

	inline void SetMin(const Vector2& min)
	{
		min_ = min;
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

	inline const Vector2& GetMax() const
	{
		return max_;
	}

	inline void SetMax(const Vector2& max)
	{
		max_ = max;
		CalculateSize();
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

	void CalculateSize()
	{
		width_ = max_.x - min_.x;
		height_ = max_.y - min_.y;
	}

protected:

private:

	Vector2 min_;
	Vector2 max_;

	float width_;
	float height_;
};

#endif