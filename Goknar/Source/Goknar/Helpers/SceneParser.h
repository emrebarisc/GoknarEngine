#ifndef __SCENEPARSER_H__
#define __SCENEPARSER_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

#include "TinyXML/include/tinyxml2.h"

class BoxCollisionComponent;
class Component;
class CapsuleCollisionComponent;
class RigidBody;
class Scene;
class SphereCollisionComponent;
class StaticMeshComponent;
class ObjectBase;

class GOKNAR_API SceneParser
{
public:
	static void Parse(Scene* scene, const std::string& filePath);
	static void SaveScene(Scene* scene, const std::string& filePath);

private:
	static void ParseComponentValues(Component* component, tinyxml2::XMLElement* componentElement);
	static void ParseStaticMeshComponentValues(StaticMeshComponent* staticMeshComponent, tinyxml2::XMLElement* componentElement);
	static void ParseBoxCollisionComponentValues(BoxCollisionComponent* boxCollisionComponent, tinyxml2::XMLElement* componentElement);
	static void ParseCapsuleCollisionComponentValues(CapsuleCollisionComponent* capsuleCollisionComponent, tinyxml2::XMLElement* componentElement);
	static void ParseSphereCollisionComponentValues(SphereCollisionComponent* sphereCollisionComponent, tinyxml2::XMLElement* componentElement);

	static void ParseObjectBase(ObjectBase* object, tinyxml2::XMLElement* objectElement);
	static void ParseRigidBody(RigidBody* rigidBody, tinyxml2::XMLElement* objectElement);

	static void GetXMLElement_Meshes(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);

	static void GetXMLElement_DirectionalLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene);
	static void GetXMLElement_SpotLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene);
	static void GetXMLElement_PointLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene);
	static void GetXMLElement_Objects(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void GetXMLElement_Components(const ObjectBase* const objectBase, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void GetXMLElement_StaticMeshComponent(const StaticMeshComponent* const staticMeshComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void GetXMLElement_BoxCollisionComponent(const BoxCollisionComponent* const boxCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void GetXMLElement_CapsuleCollisionComponent(const CapsuleCollisionComponent* const capsuleCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void GetXMLElement_SphereCollisionComponent(const SphereCollisionComponent* const sphereCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);

	static std::string Serialize(const Vector3& vector);
};

#endif