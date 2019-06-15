#ifndef __SCENEPARSER_H__
#define __SCENEPARSER_H__

#include "../Core.h"

class Scene;

class GOKNAR_API SceneParser
{
public:
	static void Parse(Scene* scene, char* filePath);
};

#endif