#ifndef __RENDERUNIFORMS_H__
#define __RENDERUNIFORMS_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Renderer/ShaderBindingPoints.h"
#include "Goknar/Renderer/Types.h"

inline constexpr GEuint FRAME_UNIFORM_BIND_INDEX = ShaderBindingPoints::UniformBuffer::FRAME_DATA;

struct GOKNAR_API FrameUniforms
{
	Matrix viewMatrix{ Matrix::IdentityMatrix };
	Matrix projectionMatrix{ Matrix::IdentityMatrix };
	Matrix viewProjectionMatrix{ Matrix::IdentityMatrix };
	Vector4 viewPositionAndDeltaTime{ Vector4::ZeroVector };
	Vector4 elapsedTimeAndPadding{ Vector4::ZeroVector };
};

static_assert(sizeof(FrameUniforms) == sizeof(Matrix) * 3 + sizeof(Vector4) * 2, "FrameUniforms must match the GLSL std140 block layout.");

#endif
