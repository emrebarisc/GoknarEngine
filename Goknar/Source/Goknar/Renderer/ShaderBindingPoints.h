#ifndef __SHADERBINDINGPOINTS_H__
#define __SHADERBINDINGPOINTS_H__

#include "Goknar/Renderer/Types.h"

namespace ShaderBindingPoints
{
	namespace UniformBuffer
	{
		inline constexpr GEuint FRAME_DATA = 0u;
		inline constexpr GEuint DIRECTIONAL_LIGHT_SHADOW_MATRICES = 3u;
		inline constexpr GEuint SPOT_LIGHT_SHADOW_MATRICES = 4u;
	}

	namespace ShaderStorage
	{
		namespace Particle
		{
			inline constexpr GEuint POSITION = 0u;
			inline constexpr GEuint VELOCITY = 1u;
			inline constexpr GEuint COLOR = 2u;
			inline constexpr GEuint LIFETIME = 3u;
			inline constexpr GEuint ALIVE_INDEX = 4u;
			inline constexpr GEuint COUNTER = 5u;
			inline constexpr GEuint DRAW_INDIRECT = 6u;
			inline constexpr GEuint END_COLOR = 7u;
			inline constexpr GEuint SIZE = 8u;
			inline constexpr GEuint ROTATION = 9u;
			inline constexpr GEuint ACCELERATION = 10u;
			inline constexpr GEuint ANGULAR_VELOCITY = 11u;
		}

		inline constexpr GEuint INSTANCE_DATA = 12u;
		inline constexpr GEuint LIGHT_DATA = 13u;
		inline constexpr GEuint BONE_DATA = 14u;
		inline constexpr GEuint OBJECT_DATA = 15u;
		inline constexpr GEuint MATERIAL_DATA = 16u;
		// TODO: Wire BONE_DATA, OBJECT_DATA, and MATERIAL_DATA once the renderer has stable draw-time indices for them.
	}
}

#endif
