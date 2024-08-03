#ifndef __SCENEPARSER_H__
#define __SCENEPARSER_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

#include "TinyXML/include/tinyxml2.h"

class Component;
class ObjectBase;
class Scene;
class StaticMeshComponent;

class GOKNAR_API SceneParser
{
public:
	static void Parse(Scene* scene, const std::string& filePath);
	static void SaveScene(Scene* scene, const std::string& filePath);

private:
	static void ParseComponentValues(Component* component, tinyxml2::XMLElement* componentElement);

	static void GetXMLElement_DirectionalLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene);
	static void GetXMLElement_SpotLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene);
	static void GetXMLElement_PointLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene);
	static void GetXMLElement_Objects(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void GetXMLElement_Components(const ObjectBase* const objectBase, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void GetXMLElement_StaticMeshComponent(const StaticMeshComponent* const staticMeshComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);

	static std::string Serialize(const Vector3& vector);
};

#endif