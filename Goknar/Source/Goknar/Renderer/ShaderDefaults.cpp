#include "pch.h"

#include "ShaderDefaults.h"
#include "ShaderTypes.h"

namespace SHADER_DEFAULTS
{
	namespace VERTEX_SHADER
	{
		std::string VERTEX_NORMAL = std::string("normalize(") + SHADER_VARIABLE_NAMES::VERTEX::NORMAL + "* transpose(inverse(mat3(" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX + "))))";
		std::string UV = std::string("vec2(") + SHADER_VARIABLE_NAMES::VERTEX::UV +".x, 1.f - " + SHADER_VARIABLE_NAMES::VERTEX::UV + ".y)";
	}

	namespace FRAGMENT_SHADER
	{

	}
}