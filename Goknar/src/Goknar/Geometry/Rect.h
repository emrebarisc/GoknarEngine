#ifndef __RECT_H__
#define __RECT_H__

#include "Goknar/Core.h"
#include "Goknar/Math.h"

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
		width_ = abs(max_.x - min_.x);
		height_ = abs(max_.y - min_.y);
	}

	inline float GetMinX() const
	{
		return min_.x;
	}

	inline void SetMinX(float minX)
	{
		min_.x = minX;
	}

	inline float GetMinY() const
	{
		return min_.y;
	}

	inline void SetMinY(float minY)
	{
		min_.y = minY;
	}

	inline const Vector2& GetMax() const
	{
		return max_;
	}

	inline void SetMax(const Vector2& max)
	{
		max_ = max;
		width_ = abs(max_.x - min_.x);
		height_ = abs(max_.y - min_.y);
	}

	inline float GetMaxX() const
	{
		return max_.x;
	}

	inline void SetMaxX(float maxX)
	{
		max_.x = maxX;
	}

	inline float GetMaxY() const
	{
		return max_.y;
	}

	inline void SetMaxY(float maxY)
	{
		max_.y = maxY;
	}

protected:

private:
	Vector2 min_;
	Vector2 max_;

	float width_;
	float height_;
};

#endif