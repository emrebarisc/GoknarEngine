#ifndef __DEBUGDRAWER_H__
#define __DEBUGDRAWER_H__

#include "Core.h"

#include "Color.h"
#include "Math/GoknarMath.h"

class GOKNAR_API DebugDrawer
{
public:
	DebugDrawer();
	~DebugDrawer();

	void DrawLine(const Vector3& start, const Vector3& end, const Colorf& color, float thickness, float time);

protected:

private:

};

#endif
