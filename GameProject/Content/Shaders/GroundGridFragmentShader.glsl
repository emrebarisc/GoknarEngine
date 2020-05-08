#version 440 core

#define PI 3.141592653589793238462643383279

uniform vec3 viewPosition;

in vec3 fragmentPosition;

out vec4 fragmentColor;

vec4 oneGridColor = vec4(0.25f, 0.25f, 0.25f, 1.f);
vec4 oneTenthGridColor = vec4(0.125f, 0.125f, 0.125f, 1.f);
vec4 axisColor = vec4(0.588235294f, 0.196078431f, 0.196078431f, 1.f);

float maxGridDistance = 250.f;

void main()
{
	float absoluteDifferenceX = abs(fragmentPosition.x - round(fragmentPosition.x));
	float absoluteDifferenceY = abs(fragmentPosition.y - round(fragmentPosition.y));

	float oneThirdValueX = absoluteDifferenceX / 0.2f;
	float oneThirdValueXRound = round(oneThirdValueX);

	float oneThirdValueY = absoluteDifferenceY / 0.2f;
	float oneThirdValueYRound = round(oneThirdValueY);


	if (abs(fragmentPosition.x) < 0.01f ||
		abs(fragmentPosition.y) < 0.01f)
	{
		fragmentColor = axisColor;
	}
	else if(absoluteDifferenceX < 0.005f || absoluteDifferenceY < 0.005f)
	{
		//vec3 wo = viewPosition - fragmentPosition;
		fragmentColor = oneGridColor;
	}
	else if (abs(oneThirdValueX - oneThirdValueXRound) < 0.01f || 
			 abs(oneThirdValueY - oneThirdValueYRound) < 0.01f)
	{
		fragmentColor = oneTenthGridColor;
	}
	else
	{
		discard;
	}

	//float cosValueX = (cos(fragmentPosition.x * 2 * PI) + 1) * 0.5f;
	//float cosValueY = (cos(fragmentPosition.y * 2 * PI) + 1) * 0.5f;

	//float oneThirdValueX = cosValueX / 0.2f;
	//float oneThirdValueXRound = round(oneThirdValueX);

	//float oneThirdValueY = cosValueY / 0.2f;
	//float oneThirdValueYRound = round(oneThirdValueY);

	//if (abs(fragmentPosition.x) < 0.01f ||
	//	abs(fragmentPosition.y) < 0.01f)
	//{
	//	fragmentColor = axisColor;
	//}
	//if (0.995f < cosValueX || 0.995f < cosValueY)
	//{
	//	fragmentColor = oneGridColor;
	//}
	//if (abs(oneThirdValueX - oneThirdValueXRound) < 0.05f || 
	//	abs(oneThirdValueY - oneThirdValueYRound) < 0.05f)
	//{
	//	fragmentColor = oneTenthGridColor;
	//}
	//else
	//{
	//	discard;
	//}
}