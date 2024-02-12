#ifndef __BOX_H__
#define __BOX_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

class GOKNAR_API Box
{
public:
	Box();
	Box(const Vector3& min, const Vector3& max);

	inline void ExtendWRTPoint(const Vector3& point, bool recalculateSize = true)
	{
		if (max_.x < point.x)
		{
			SetMaxX(point.x, recalculateSize);
		}
		if (max_.y < point.y)
		{
			SetMaxY(point.y, recalculateSize);
		}
		if (max_.z < point.z)
		{
			SetMaxZ(point.z, recalculateSize);
		}

		if (point.x < min_.x)
		{
			SetMinX(point.x, recalculateSize);
		}
		if (point.y < min_.y)
		{
			SetMinY(point.y, recalculateSize);
		}
		if (point.z < min_.z)
		{
			SetMinZ(point.z, recalculateSize);
		}
	}

	const Vector3& GetMin() const
	{
		return min_;
	}

	inline void SetMin(const Vector3& min)
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

	inline const Vector3& GetMax() const
	{
		return max_;
	}

	inline void SetMax(const Vector3& max)
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

	const Vector3& GetSize() const
	{
		return size_;
	}

	float GetWidth() const
	{
		return size_.x;
	}

	float GetDepth() const
	{
		return size_.y;
	}

	float GetHeight() const
	{
		return size_.z;
	}

	void CalculateSize()
	{
		size_ = max_ - min_;
	}
protected:

private:
	Vector3 min_{ Vector3::ZeroVector };
	Vector3 max_{ Vector3::ZeroVector };
	Vector3 size_{ Vector3::ZeroVector };
};

#endif