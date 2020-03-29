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

	void SetMin(const Vector2& min)
	{
		min_ = min;
		width_ = abs(max_.x - min_.x);
		height_ = abs(max_.y - min_.y);
	}

	float GetMinX() const
	{
		return min_.x;
	}

	void SetMinX(float minX)
	{
		min_.x = minX;
	}

	float GetMinY() const
	{
		return min_.y;
	}

	void SetMinY(float minY)
	{
		min_.y = minY;
	}

	const Vector2& GetMax() const
	{
		return max_;
	}

	void SetMax(const Vector2& max)
	{
		max_ = max;
		width_ = abs(max_.x - min_.x);
		height_ = abs(max_.y - min_.y);
	}
	float GetMaxX() const
	{
		return max_.x;
	}

	void SetMaxX(float maxX)
	{
		max_.x = maxX;
	}

	float GetMaxY() const
	{
		return max_.y;
	}

	void SetMaxY(float maxY)
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