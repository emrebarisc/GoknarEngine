#include "pch.h"

#include "SceneParser.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/ObjectBase.h"

#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Components/DynamicMeshComponent.h"
#include "Goknar/Components/InstancedStaticMeshComponent.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Components/SkeletalMeshComponent.h"

#include "Goknar/Factories/DynamicObjectFactory.h"

#include "Goknar/Helpers/AssetParser.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/IO/ModelLoader.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"

#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/ResourceManager.h"

#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialInstance.h"
#include "Goknar/Materials/MaterialSerializer.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/SkeletalMesh.h"

#include "Goknar/Debug/DebugDrawer.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Texture.h"

#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/Components/BoxCollisionComponent.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"
#include "Goknar/Physics/Components/SphereCollisionComponent.h"
#include "Goknar/Physics/Components/MovingTriangleMeshCollisionComponent.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

#include "tinyxml2.h"

namespace
{
	std::unordered_map<const InstancedStaticMeshComponent*, std::vector<std::string>> instancedStaticMeshComponentMaterialPathMap;
	std::unordered_map<const StaticMeshComponent*, std::vector<std::string>> staticMeshComponentMaterialPathMap;
	std::unordered_map<const InstancedStaticMesh*, std::string> instancedStaticMeshSourcePathMap;
	std::unordered_map<std::string, Material*> sharedMaterialPathMap;
	size_t instancedStaticMeshIdentifier = 0;

	std::vector<std::string> NormalizeMaterialPaths(const std::vector<std::string>& materialPaths);

	std::string SerializeVector3(const Vector3& vector)
	{
		return std::to_string(vector.x) + " " + std::to_string(vector.y) + " " + std::to_string(vector.z);
	}

	bool IsIdentityTranslation(const Vector3& translation)
	{
		return translation.Equals(Vector3::ZeroVector);
	}

	bool IsIdentityRotation(const Quaternion& rotation)
	{
		return rotation.Equals(Quaternion::Identity);
	}

	bool IsIdentityScaling(const Vector3& scaling)
	{
		return scaling.Equals(Vector3(1.f));
	}

	void WriteVectorElement(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, const char* elementName, const Vector3& value)
	{
		if (!parentElement)
		{
			return;
		}

		tinyxml2::XMLElement* vectorElement = xmlDocument.NewElement(elementName);
		vectorElement->SetText(SerializeVector3(value).c_str());
		parentElement->InsertEndChild(vectorElement);
	}

	void WriteTransformElementsIfNeeded(
		tinyxml2::XMLDocument& xmlDocument,
		tinyxml2::XMLElement* parentElement,
		const Vector3& translation,
		const Quaternion& rotation,
		const Vector3& scaling,
		const char* translationElementName,
		const char* rotationElementName,
		const char* scalingElementName)
	{
		if (!IsIdentityTranslation(translation))
		{
			WriteVectorElement(xmlDocument, parentElement, translationElementName, translation);
		}

		if (!IsIdentityRotation(rotation))
		{
			WriteVectorElement(xmlDocument, parentElement, rotationElementName, rotation.ToEulerDegrees());
		}

		if (!IsIdentityScaling(scaling))
		{
			WriteVectorElement(xmlDocument, parentElement, scalingElementName, scaling);
		}
	}

	template <typename MeshComponentType, typename MeshInstanceType, typename MeshType>
	void ApplyMeshComponentMaterialPaths(
		MeshComponentType* meshComponent,
		const std::vector<std::string>& materialPaths,
		std::unordered_map<const MeshComponentType*, std::vector<std::string>>& materialPathMap)
	{
		if (!meshComponent)
		{
			return;
		}

		const std::vector<std::string> normalizedMaterialPaths = NormalizeMaterialPaths(materialPaths);
		if (normalizedMaterialPaths.empty())
		{
			materialPathMap.erase(meshComponent);
		}

		MeshInstanceType* meshInstance = meshComponent->GetMeshInstance();
		MeshType* mesh = meshInstance ? meshInstance->GetMesh() : nullptr;
		if (!meshInstance || !mesh)
		{
			if (!normalizedMaterialPaths.empty())
			{
				materialPathMap[meshComponent] = normalizedMaterialPaths;
			}

			return;
		}

		const auto& subMeshes = mesh->GetSubMeshes();
		for (size_t subMeshIndex = 0; subMeshIndex < subMeshes.size(); ++subMeshIndex)
		{
			MaterialInstance* materialInstance = nullptr;

			if (subMeshIndex < normalizedMaterialPaths.size() && !normalizedMaterialPaths[subMeshIndex].empty())
			{
				Material* material = SceneParser::GetOrCreateSharedMaterial(normalizedMaterialPaths[subMeshIndex]);
				if (material)
				{
					materialInstance = MaterialInstance::Create(material);
				}
			}

			meshInstance->SetMaterial(static_cast<int>(subMeshIndex), materialInstance);
		}

		if (!normalizedMaterialPaths.empty())
		{
			materialPathMap[meshComponent] = normalizedMaterialPaths;
		}
	}

	template <typename MeshComponentType>
	std::vector<std::string> GetMeshComponentMaterialPaths(
		const MeshComponentType* meshComponent,
		const std::unordered_map<const MeshComponentType*, std::vector<std::string>>& materialPathMap)
	{
		auto materialPathIterator = materialPathMap.find(meshComponent);
		if (materialPathIterator == materialPathMap.end())
		{
			return {};
		}

		return materialPathIterator->second;
	}

	template <typename MeshComponentType, typename MeshInstanceType, typename MeshType>
	void ClearMeshComponentMaterialPath(
		const MeshComponentType* meshComponent,
		std::unordered_map<const MeshComponentType*, std::vector<std::string>>& materialPathMap)
	{
		materialPathMap.erase(meshComponent);

		if (!meshComponent)
		{
			return;
		}

		MeshInstanceType* meshInstance = meshComponent->GetMeshInstance();
		MeshType* mesh = meshInstance ? meshInstance->GetMesh() : nullptr;
		if (!meshInstance || !mesh)
		{
			return;
		}

		const auto& subMeshes = mesh->GetSubMeshes();
		for (size_t subMeshIndex = 0; subMeshIndex < subMeshes.size(); ++subMeshIndex)
		{
			meshInstance->SetMaterial(static_cast<int>(subMeshIndex), nullptr);
		}
	}

	void ApplyInstancedStaticMeshComponentMaterialPaths(InstancedStaticMeshComponent* instancedStaticMeshComponent, const std::vector<std::string>& materialPaths)
	{
		ApplyMeshComponentMaterialPaths<InstancedStaticMeshComponent, InstancedStaticMeshInstance, InstancedStaticMesh>(
			instancedStaticMeshComponent,
			materialPaths,
			instancedStaticMeshComponentMaterialPathMap);
	}

	std::vector<std::string> GetInstancedStaticMeshComponentMaterialPaths(const InstancedStaticMeshComponent* instancedStaticMeshComponent)
	{
		return GetMeshComponentMaterialPaths(instancedStaticMeshComponent, instancedStaticMeshComponentMaterialPathMap);
	}

	void ClearInstancedStaticMeshComponentMaterialPath(const InstancedStaticMeshComponent* instancedStaticMeshComponent)
	{
		ClearMeshComponentMaterialPath<InstancedStaticMeshComponent, InstancedStaticMeshInstance, InstancedStaticMesh>(
			instancedStaticMeshComponent,
			instancedStaticMeshComponentMaterialPathMap);
	}

	void ApplyMaterialPathsToInstancedStaticMesh(InstancedStaticMesh* instancedStaticMesh, const std::vector<std::string>& materialPaths)
	{
		if (!instancedStaticMesh)
		{
			return;
		}

		const auto& subMeshes = instancedStaticMesh->GetSubMeshes();
		for (size_t subMeshIndex = 0; subMeshIndex < subMeshes.size() && subMeshIndex < materialPaths.size(); ++subMeshIndex)
		{
			const std::string relativeMaterialPath = ContentPathUtils::ToContentRelativePath(materialPaths[subMeshIndex]);
			if (relativeMaterialPath.empty())
			{
				continue;
			}

			Material* material = subMeshes[subMeshIndex] ? subMeshes[subMeshIndex]->GetMaterial() : nullptr;
			if (material)
			{
				MaterialSerializer::Deserialize(relativeMaterialPath, material);
			}
		}
	}

	InstancedStaticMesh* CreateInstancedStaticMeshFromPath(const std::string& meshPath)
	{
		const std::string relativeMeshPath = ContentPathUtils::ToContentRelativePath(meshPath);
		if (relativeMeshPath.empty())
		{
			return nullptr;
		}

		StaticMesh* sourceMesh = engine->GetResourceManager()->GetContent<StaticMesh>(relativeMeshPath);
		if (!sourceMesh)
		{
			return nullptr;
		}

		InstancedStaticMesh* instancedStaticMesh = InstancedStaticMesh::CreateFromStaticMesh(
			sourceMesh,
			sourceMesh->GetPath() + "::InstancedStaticMesh_" + std::to_string(instancedStaticMeshIdentifier++));
		if (!instancedStaticMesh)
		{
			return nullptr;
		}

		ApplyMaterialPathsToInstancedStaticMesh(instancedStaticMesh, AssetParser::GetMeshMaterialPaths(relativeMeshPath));
		instancedStaticMeshSourcePathMap[instancedStaticMesh] = relativeMeshPath;
		return instancedStaticMesh;
	}

	void TrimTrailingEmptyMaterialPaths(std::vector<std::string>& materialPaths)
	{
		while (!materialPaths.empty() && materialPaths.back().empty())
		{
			materialPaths.pop_back();
		}
	}

	std::vector<std::string> NormalizeMaterialPaths(const std::vector<std::string>& materialPaths)
	{
		std::vector<std::string> normalizedMaterialPaths;
		normalizedMaterialPaths.reserve(materialPaths.size());

		for (const std::string& materialPath : materialPaths)
		{
			normalizedMaterialPaths.push_back(ContentPathUtils::ToContentRelativePath(materialPath));
		}

		TrimTrailingEmptyMaterialPaths(normalizedMaterialPaths);
		return normalizedMaterialPaths;
	}

	std::vector<std::string> ReadMaterialPaths(const tinyxml2::XMLElement* parentElement)
	{
		std::vector<std::string> materialPaths;
		if (!parentElement)
		{
			return materialPaths;
		}

		const tinyxml2::XMLElement* materialPathsElement = parentElement->FirstChildElement("MaterialPaths");
		if (materialPathsElement)
		{
			for (const tinyxml2::XMLElement* materialPathElement = materialPathsElement->FirstChildElement("MaterialPath");
				materialPathElement != nullptr;
				materialPathElement = materialPathElement->NextSiblingElement("MaterialPath"))
			{
				const char* materialPathText = materialPathElement->GetText();
				materialPaths.push_back(materialPathText ? ContentPathUtils::ToContentRelativePath(materialPathText) : "");
			}

			TrimTrailingEmptyMaterialPaths(materialPaths);
			return materialPaths;
		}

		const tinyxml2::XMLElement* materialPathElement = parentElement->FirstChildElement("MaterialPath");
		if (materialPathElement && materialPathElement->GetText())
		{
			materialPaths.push_back(ContentPathUtils::ToContentRelativePath(materialPathElement->GetText()));
		}

		return materialPaths;
	}

	void WriteMaterialPaths(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, const std::vector<std::string>& materialPaths)
	{
		if (!parentElement || materialPaths.empty())
		{
			return;
		}

		if (materialPaths.size() == 1)
		{
			tinyxml2::XMLElement* materialPathElement = xmlDocument.NewElement("MaterialPath");
			materialPathElement->SetText(materialPaths[0].c_str());
			parentElement->InsertEndChild(materialPathElement);
			return;
		}

		tinyxml2::XMLElement* materialPathsElement = xmlDocument.NewElement("MaterialPaths");
		for (const std::string& materialPath : materialPaths)
		{
			tinyxml2::XMLElement* materialPathElement = xmlDocument.NewElement("MaterialPath");
			if (!materialPath.empty())
			{
				materialPathElement->SetText(materialPath.c_str());
			}

			materialPathsElement->InsertEndChild(materialPathElement);
		}

		parentElement->InsertEndChild(materialPathsElement);
	}
}

Material* SceneParser::GetOrCreateSharedMaterial(const std::string& materialPath)
{
	const std::string relativeMaterialPath = ContentPathUtils::ToContentRelativePath(materialPath);
	if (relativeMaterialPath.empty())
	{
		return nullptr;
	}

	auto sharedMaterialIterator = sharedMaterialPathMap.find(relativeMaterialPath);
	if (sharedMaterialIterator != sharedMaterialPathMap.end())
	{
		return sharedMaterialIterator->second;
	}

	Material* material = new Material();
	MaterialSerializer::Deserialize(relativeMaterialPath, material);
	sharedMaterialPathMap[relativeMaterialPath] = material;
	return material;
}

void SceneParser::ApplyStaticMeshComponentMaterialPaths(StaticMeshComponent* staticMeshComponent, const std::vector<std::string>& materialPaths)
{
	ApplyMeshComponentMaterialPaths<StaticMeshComponent, StaticMeshInstance, StaticMesh>(
		staticMeshComponent,
		materialPaths,
		staticMeshComponentMaterialPathMap);
}

std::vector<std::string> SceneParser::GetStaticMeshComponentMaterialPaths(const StaticMeshComponent* staticMeshComponent)
{
	return GetMeshComponentMaterialPaths(staticMeshComponent, staticMeshComponentMaterialPathMap);
}

void SceneParser::ApplyStaticMeshComponentMaterialPath(StaticMeshComponent* staticMeshComponent, const std::string& materialPath)
{
	if (materialPath.empty())
	{
		ApplyStaticMeshComponentMaterialPaths(staticMeshComponent, {});
		return;
	}

	ApplyStaticMeshComponentMaterialPaths(staticMeshComponent, { materialPath });
}

std::string SceneParser::GetStaticMeshComponentMaterialPath(const StaticMeshComponent* staticMeshComponent)
{
	const std::vector<std::string> materialPaths = GetStaticMeshComponentMaterialPaths(staticMeshComponent);
	return materialPaths.empty() ? "" : materialPaths[0];
}

void SceneParser::ClearStaticMeshComponentMaterialPath(const StaticMeshComponent* staticMeshComponent)
{
	ClearMeshComponentMaterialPath<StaticMeshComponent, StaticMeshInstance, StaticMesh>(
		staticMeshComponent,
		staticMeshComponentMaterialPathMap);
}

void SceneParser::ClearCaches()
{
	instancedStaticMeshComponentMaterialPathMap.clear();
	instancedStaticMeshSourcePathMap.clear();
	staticMeshComponentMaterialPathMap.clear();
	sharedMaterialPathMap.clear();
}

void SceneParser::Parse(Scene* scene, const std::string& filePath)
{
	tinyxml2::XMLDocument xmlFile;
	std::stringstream stream;

	tinyxml2::XMLError res;

	ResourceManager* resourceManager = engine->GetResourceManager();

	try
	{
		res = xmlFile.LoadFile(filePath.c_str());
		if (res)
		{
			throw std::runtime_error("Error: Scene XML file could not be loaded at " + filePath + ".");
		}
	}
	catch (std::exception & exception)
	{
		std::cerr << exception.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	auto root = xmlFile.FirstChild();
	if (!root)
	{
		throw std::runtime_error("Error: Root could not be found.");
	}

	tinyxml2::XMLElement* element = root->FirstChildElement("BackgroundColor");
	if (element)
	{
		stream << element->GetText() << std::endl;
	}
	else
	{
		stream << "0 0 0" << std::endl;
	}

	Colorf backgroundColor;
	stream >> backgroundColor.r >> backgroundColor.g >> backgroundColor.b;

	scene->SetBackgroundColor(backgroundColor / 255.f);

	stream.clear();

	//Get Cameras
	element = root->FirstChildElement("Cameras");
	if (element)
	{
		element = element->FirstChildElement("Camera");
		Camera* camera;
		while (element)
		{
			camera = new Camera();
			auto child = element->FirstChildElement("Position");
			stream << child->GetText() << std::endl;
			float x, y, z;
			stream >> x >> y >> z;
			camera->SetPosition(Vector3(x, y, z));

			child = element->FirstChildElement("NearDistance");
			if (child)
			{
				stream << child->GetText() << std::endl;
				float nearDistance;
				stream >> nearDistance;
				camera->SetNearDistance(nearDistance);
			}

			child = element->FirstChildElement("FarDistance");
			if (child)
			{
				stream << child->GetText() << std::endl;
				float farDistance;
				stream >> farDistance;
				camera->SetFarDistance(farDistance);
			}

			child = element->FirstChildElement("Projection");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string projection;
				stream >> projection;
				camera->SetProjection(projection == "Orthographic" ? CameraProjection::Orthographic : CameraProjection::Perspective);
			}

			const char* cameraType = element->Attribute("type");
			if (cameraType && std::string(cameraType) == "simple")
			{
				child = element->FirstChildElement("GazePoint");
				if (child)
				{
					Vector3 gazePoint;
					stream << child->GetText() << std::endl;
					stream >> gazePoint.x >> gazePoint.y >> gazePoint.z;

					camera->SetForwardVector((gazePoint - camera->GetPosition()).GetNormalized());
				}
				else
				{
					child = element->FirstChildElement("Gaze");
					if (child)
					{
						stream << child->GetText() << std::endl;
						float x, y, z;
						stream >> x >> y >> z;
						camera->SetForwardVector(Vector3(x, y, z));
					}
				}

				float fovY;
				child = element->FirstChildElement("FovY");
				stream << child->GetText() << std::endl;
				stream >> fovY;

				float resolutionProportion = (float)camera->GetImageWidth() / camera->GetImageHeight();

				float halfOfFovY = fovY * 0.5f;
				float top, bottom, left, right;
				top = camera->GetNearDistance() * tan(DEGREE_TO_RADIAN(halfOfFovY));
				bottom = -top;
				left = bottom * resolutionProportion;
				right = top * resolutionProportion;

				camera->SetNearPlane(Vector4(left, right, bottom, top));
			}
			else
			{
				child = element->FirstChildElement("Gaze");
				stream << child->GetText() << std::endl;
				Vector3 forward;
				stream >> forward.x >> forward.y >> forward.z;
				camera->SetForwardVector(forward);

				child = element->FirstChildElement("NearPlane");
				stream << child->GetText() << std::endl;
				Vector4 nearPlane;
				stream >> nearPlane.x >> nearPlane.y >> nearPlane.z >> nearPlane.w;
				camera->SetNearPlane(nearPlane);
			}

			child = element->FirstChildElement("Up");
			stream << child->GetText() << std::endl;
			Vector3 up;
			stream >> up.x >> up.y >> up.z;
			camera->SetUpVector(up);

			element = element->NextSiblingElement("Camera");
			stream.clear();
		}
	}

	//Get Ambient Light
	element = root->FirstChildElement("Lights");
	tinyxml2::XMLElement* child;

	if (element)
	{
		child = element->FirstChildElement("AmbientLight");
		if (child)
		{
			stream << child->GetText() << std::endl;
			Vector3 ambientLight;
			stream >> ambientLight.x >> ambientLight.y >> ambientLight.z;

			scene->SetAmbientLight(ambientLight);
		}
	}

	//Get Lights
	element = root->FirstChildElement("Lights");
	if (element)
	{
		element = element->FirstChildElement("PointLight");
		PointLight* pointLight;
		while (element)
		{
			pointLight = new PointLight();

			child = element->FirstChildElement("Position");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Color");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Intensity");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Radius");
			stream << child->GetText() << std::endl;

			bool isCastingShadowElementFound = false;
			child = element->FirstChildElement("IsCastingShadow");
			if (child)
			{
				isCastingShadowElementFound = true;
				stream << child->GetText() << std::endl;
			}

			bool shadowIntensityElementFound = false;
			child = element->FirstChildElement("ShadowIntensity");
			if (child)
			{
				shadowIntensityElementFound = true;
				stream << child->GetText() << std::endl;
			}

			bool shadowMapResolutionElementFound = false;
			child = element->FirstChildElement("ShadowMapResolution");
			if (child)
			{
				shadowMapResolutionElementFound = true;
				stream << child->GetText() << std::endl;
			}

			Vector3 position;
			stream >> position.x >> position.y >> position.z;
			pointLight->SetPosition(position);

			Vector3 color;
			stream >> color.x >> color.y >> color.z;
			pointLight->SetColor(color);

			float intensity;
			stream >> intensity;
			pointLight->SetIntensity(intensity);

			float radius;
			stream >> radius;
			pointLight->SetRadius(radius);

			if (isCastingShadowElementFound)
			{
				bool isCastingShadow = false;
				stream >> isCastingShadow;
				pointLight->SetIsShadowEnabled(isCastingShadow);
			}

			if (shadowIntensityElementFound)
			{
				float shadowIntensity = 0.f;
				stream >> shadowIntensity;
				pointLight->SetShadowIntensity(shadowIntensity);
			}

			if (shadowMapResolutionElementFound)
			{
				int x = 1024;
				int y = 1024;
				stream >> x >> y;
				pointLight->SetShadowWidth(x);
				pointLight->SetShadowHeight(y);
			}

			element = element->NextSiblingElement("PointLight");

			stream.clear();
		}
	}

	//Get Directional Lights
	element = root->FirstChildElement("Lights");
	if (element)
	{
		element = element->FirstChildElement("DirectionalLight");
		DirectionalLight* directionalLight;
		while (element)
		{
			directionalLight = new DirectionalLight();

			child = element->FirstChildElement("Direction");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Color");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Intensity");
			stream << child->GetText() << std::endl;

			bool isCastingShadowElementFound = false;
			child = element->FirstChildElement("IsCastingShadow");
			if (child)
			{
				isCastingShadowElementFound = true;
				stream << child->GetText() << std::endl;
			}

			bool shadowIntensityElementFound = false;
			child = element->FirstChildElement("ShadowIntensity");
			if (child)
			{
				shadowIntensityElementFound = true;
				stream << child->GetText() << std::endl;
			}

			bool shadowMapResolutionElementFound = false;
			child = element->FirstChildElement("ShadowMapResolution");
			if (child)
			{
				shadowMapResolutionElementFound = true;
				stream << child->GetText() << std::endl;
			}

			Vector3 direction;
			stream >> direction.x >> direction.y >> direction.z;
			direction.Normalize();
			directionalLight->SetDirection(direction);

			Vector3 color;
			stream >> color.x >> color.y >> color.z;
			directionalLight->SetColor(color);

			float intensity;
			stream >> intensity;
			directionalLight->SetIntensity(intensity);

			if (isCastingShadowElementFound)
			{
				bool isCastingShadow = false;
				stream >> isCastingShadow;
				directionalLight->SetIsShadowEnabled(isCastingShadow);
			}

			if (shadowIntensityElementFound)
			{
				float shadowIntensity = 0.f;
				stream >> shadowIntensity;
				directionalLight->SetShadowIntensity(shadowIntensity);
			}

			if (shadowMapResolutionElementFound)
			{
				int x = 1024;
				int y = 1024;
				stream >> x >> y;
				directionalLight->SetShadowWidth(x);
				directionalLight->SetShadowHeight(y);
			}

			element = element->NextSiblingElement("DirectionalLight");

			stream.clear();
		}
	}

	//Get Spot Lights
	element = root->FirstChildElement("Lights");
	if (element)
	{
		element = element->FirstChildElement("SpotLight");
		SpotLight* spotLight;
		while (element)
		{
			spotLight = new SpotLight();

			child = element->FirstChildElement("Position");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Direction");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Color");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("Intensity");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("CoverageAngle");
			stream << child->GetText() << std::endl;

			child = element->FirstChildElement("FalloffAngle");
			stream << child->GetText() << std::endl;

			bool isCastingShadowElementFound = false;
			child = element->FirstChildElement("IsCastingShadow");
			if (child)
			{
				isCastingShadowElementFound = true;
				stream << child->GetText() << std::endl;
			}

			bool shadowIntensityElementFound = false;
			child = element->FirstChildElement("ShadowIntensity");
			if (child)
			{
				shadowIntensityElementFound = true;
				stream << child->GetText() << std::endl;
			}

			bool shadowMapResolutionElementFound = false;
			child = element->FirstChildElement("ShadowMapResolution");
			if (child)
			{
				shadowMapResolutionElementFound = true;
				stream << child->GetText() << std::endl;
			}

			Vector3 position;
			stream >> position.x >> position.y >> position.z;
			spotLight->SetPosition(position);

			Vector3 direction;
			stream >> direction.x >> direction.y >> direction.z;
			direction.Normalize();
			spotLight->SetDirection(direction);

			Vector3 color;
			stream >> color.x >> color.y >> color.z;
			spotLight->SetColor(color);

			float intensity;
			stream >> intensity;
			spotLight->SetIntensity(intensity);

			float coverageAngle;
			stream >> coverageAngle;
			spotLight->SetCoverageAngle(coverageAngle);

			float falloffAngle;
			stream >> falloffAngle;
			spotLight->SetFalloffAngle(falloffAngle);

			if (isCastingShadowElementFound)
			{
				bool isCastingShadow = false;
				stream >> isCastingShadow;
				spotLight->SetIsShadowEnabled(isCastingShadow);
			}

			if (shadowIntensityElementFound)
			{
				float shadowIntensity = 0.f;
				stream >> shadowIntensity;
				spotLight->SetShadowIntensity(shadowIntensity);
			}

			if (shadowMapResolutionElementFound)
			{
				int x = 1024;
				int y = 1024;
				stream >> x >> y;
				spotLight->SetShadowWidth(x);
				spotLight->SetShadowHeight(y);
			}

			element = element->NextSiblingElement("SpotLight");
		}
		stream.clear();
	}

	//Get Textures
	element = root->FirstChildElement("Textures");
	if (element)
	{
		element = element->FirstChildElement("Texture");
		Texture* texture;
		while (element)
		{
			texture = new Texture();

			child = element->FirstChildElement("Path");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string textureImagePath;
				stream >> textureImagePath;
				texture->SetTextureImagePath(textureImagePath);
			}
			stream.clear();

			child = element->FirstChildElement("Wrapping");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string textureWrapping;
				stream >> textureWrapping;

				texture->SetTextureWrappingS(textureWrapping == "Repeat" ? TextureWrapping::REPEAT :
											textureWrapping == "MirroredRepeat" ? TextureWrapping::MIRRORED_REPEAT :
											textureWrapping == "ClampToEdge" ? TextureWrapping::CLAMP_TO_EDGE :
											textureWrapping == "ClampToBorder" ? TextureWrapping::CLAMP_TO_BORDER :
											texture->GetTextureWrappingS());
			}
			stream.clear();

			child = element->FirstChildElement("MinFilter");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string textureMinFilter;
				stream >> textureMinFilter;

				texture->SetTextureMinFilter(textureMinFilter == "Nearest" ? TextureMinFilter::NEAREST :
											textureMinFilter == "Linear" ? TextureMinFilter::LINEAR :
											textureMinFilter == "NearestMipMapNearest" ? TextureMinFilter::NEAREST_MIPMAP_NEAREST :
											textureMinFilter == "LinearMipMapNearest" ? TextureMinFilter::LINEAR_MIPMAP_NEAREST :
											textureMinFilter == "NearestMipMapLinear" ? TextureMinFilter::NEAREST_MIPMAP_LINEAR :
											textureMinFilter == "LinearMipMapLinear" ? TextureMinFilter::LINEAR_MIPMAP_LINEAR :
											texture->GetTextureMinFilter());
			}
			stream.clear();

			child = element->FirstChildElement("MagFilter");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string textureMagFilter;
				stream >> textureMagFilter;

				texture->SetTextureMagFilter(textureMagFilter == "Nearest" ? TextureMagFilter::NEAREST :
											 textureMagFilter == "Linear" ? TextureMagFilter::LINEAR :
											 texture->GetTextureMagFilter());
			}
			stream.clear();

			child = element->FirstChildElement("Usage");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string usage;
				stream >> usage;

				texture->SetTextureUsage(	usage == "Diffuse" ? TextureUsage::Diffuse :
											usage == "Normal" ? TextureUsage::Normal :
											usage == "AmbientOcclusion" ? TextureUsage::AmbientOcclusion :
											usage == "Height" ? TextureUsage::Height :
											usage == "Metallic" ? TextureUsage::Metallic :
											usage == "Roughness" ? TextureUsage::Roughness :
											usage == "Specular" ? TextureUsage::Specular :
											TextureUsage::None);
			}
			stream.clear();

			child = element->FirstChildElement("Name");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string textureName;
				stream >> textureName;
				texture->SetName(textureName.c_str());
			}

			scene->AddTexture(texture);
			element = element->NextSiblingElement("Texture");
		}
		stream.clear();
	}

	//Get Materials
	element = root->FirstChildElement("Materials");
	if (element)
	{
		element = element->FirstChildElement("Material");
		Material* material;
		while (element)
		{
			material = new Material();

			child = element->FirstChildElement("BlendModel");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string blendModel;
				stream >> blendModel;
				material->SetBlendModel(blendModel == "Masked" ? MaterialBlendModel::Masked :
					blendModel == "Transparent" ? MaterialBlendModel::Transparent :
					MaterialBlendModel::Opaque);
			}

			child = element->FirstChildElement("ShadingModel");
			if (child)
			{
				stream << child->GetText() << std::endl;
				std::string shadingModel;
				stream >> shadingModel;
				material->SetShadingModel(shadingModel == "Default" ? MaterialShadingModel::Default :
										  shadingModel == "TwoSided" ? MaterialShadingModel::TwoSided :
										  material->GetShadingModel());
			}

			child = element->FirstChildElement("Texture");
			while (child)
			{
				int textureId = std::stoi(child->Attribute("id"));
				material->GetShader(RenderPassType::Forward)->AddTexture(scene->GetTexture(textureId));
				child = child->NextSiblingElement("Texture");
			}

			child = element->FirstChildElement("AmbientReflectance");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 ambientReflectance;
				stream >> ambientReflectance.x >> ambientReflectance.y >> ambientReflectance.z;
				material->SetAmbientReflectance(ambientReflectance);
			}

			child = element->FirstChildElement("DiffuseReflectance");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 diffuseReflectance;
				stream >> diffuseReflectance.x >> diffuseReflectance.y >> diffuseReflectance.z;
				material->SetBaseColor(diffuseReflectance);
			}

			child = element->FirstChildElement("SpecularReflectance");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 specularReflectance;
				stream >> specularReflectance.x >> specularReflectance.y >> specularReflectance.z;
				material->SetSpecularReflectance(specularReflectance);
			}

			child = element->FirstChildElement("PhongExponent");
			if (child)
			{
				stream << child->GetText() << std::endl;
			}
			else
			{
				stream << "1" << std::endl;
			}
			float phongExponent;
			stream >> phongExponent;
			if (!std::isfinite(phongExponent) || phongExponent < 1.f)
			{
				phongExponent = 1.f;
			}
			material->SetPhongExponent(phongExponent);

			element = element->NextSiblingElement("Material");
		}
		stream.clear();
	}

	element = root->FirstChildElement("Objects");
	if (element)
	{
		DynamicObjectFactory* dynamicObjectFactory = DynamicObjectFactory::GetInstance();
		const auto& factoryObjects = dynamicObjectFactory->GetObjectMap();

		for (const auto& factoryObject : factoryObjects)
		{
			const std::string& objectFactoryName = factoryObject.first;
			tinyxml2::XMLElement* objectElement = element->FirstChildElement(objectFactoryName.c_str());
			while (objectElement)
			{
				ObjectBase* object = factoryObject.second();
				object->SetName(objectFactoryName);

				if (RigidBody* rigidBody = dynamic_cast<RigidBody*>(object))
				{
					ParseRigidBody(rigidBody, objectElement);
				}

				ParseObjectBase(object, objectElement);

				objectElement = objectElement->NextSiblingElement(objectFactoryName.c_str());
			}
			stream.clear();
		}
	}
}

void SceneParser::SaveScene(Scene* scene, const std::string& filePath)
{
	tinyxml2::XMLDocument sceneXML;
	tinyxml2::XMLNode* rootElement = sceneXML.NewElement("Scene");

	sceneXML.InsertFirstChild(rootElement);

	tinyxml2::XMLElement* subElement = sceneXML.NewElement("Lights");
	GetXMLElement_DirectionalLights(sceneXML, subElement, scene);
	GetXMLElement_PointLights(sceneXML, subElement, scene);
	GetXMLElement_SpotLights(sceneXML, subElement, scene);
	rootElement->InsertEndChild(subElement);

	subElement = sceneXML.NewElement("Objects");
	GetXMLElement_Objects(sceneXML, subElement);
	rootElement->InsertEndChild(subElement);

	sceneXML.SaveFile(filePath.c_str());
}

void SceneParser::ParseComponentValues(Component* component, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("PivotPoint");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 pivotPoint;
		stream >> pivotPoint.x >> pivotPoint.y >> pivotPoint.z;
		component->SetPivotPoint(pivotPoint);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("RelativePosition");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 relativePosition;
		stream >> relativePosition.x >> relativePosition.y >> relativePosition.z;
		component->SetRelativePosition(relativePosition);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("EulerRelativeRotation");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 relativeRotation;
		stream >> relativeRotation.x >> relativeRotation.y >> relativeRotation.z;
		component->SetRelativeRotation(Quaternion::FromEulerDegrees(relativeRotation));
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("RelativeScaling");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 relativeScaling;
		stream >> relativeScaling.x >> relativeScaling.y >> relativeScaling.z;
		component->SetRelativeScaling(relativeScaling);
	}
	stream.clear();
}

void SceneParser::ParseStaticMeshComponentValues(StaticMeshComponent* staticMeshComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("MeshPath");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		std::string meshPath;
		stream >> meshPath;
		StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>(ContentPathUtils::ToContentRelativePath(meshPath));
		if (staticMesh)
		{
			staticMeshComponent->SetMesh(staticMesh);
		}
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("RenderMask");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		std::string renderMaskString;
		stream >> renderMaskString;
		int renderMask = std::stoi(renderMaskString);
		staticMeshComponent->GetMeshInstance()->SetRenderMask(renderMask);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("MaterialPath");
	if (componentElement->FirstChildElement("MaterialPaths") || dataElement)
	{
		ApplyStaticMeshComponentMaterialPaths(staticMeshComponent, ReadMaterialPaths(componentElement));
	}
	else
	{
		ClearStaticMeshComponentMaterialPath(staticMeshComponent);
	}

	stream.clear();
}

void SceneParser::ParseInstancedStaticMeshComponentValues(InstancedStaticMeshComponent* instancedStaticMeshComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("MeshPath");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		std::string meshPath;
		stream >> meshPath;
		InstancedStaticMesh* instancedStaticMesh = CreateInstancedStaticMeshFromPath(meshPath);
		if (instancedStaticMesh)
		{
			instancedStaticMeshComponent->SetMesh(instancedStaticMesh);
		}
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("RenderMask");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		std::string renderMaskString;
		stream >> renderMaskString;
		int renderMask = std::stoi(renderMaskString);
		instancedStaticMeshComponent->GetMeshInstance()->SetRenderMask(renderMask);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("MaterialPath");
	if (componentElement->FirstChildElement("MaterialPaths") || dataElement)
	{
		ApplyInstancedStaticMeshComponentMaterialPaths(instancedStaticMeshComponent, ReadMaterialPaths(componentElement));
	}
	else
	{
		ClearInstancedStaticMeshComponentMaterialPath(instancedStaticMeshComponent);
	}

	InstancedStaticMeshInstance* meshInstance = instancedStaticMeshComponent->GetMeshInstance();
	InstancedStaticMesh* instancedStaticMesh = meshInstance ? meshInstance->GetMesh() : nullptr;
	tinyxml2::XMLElement* instanceTransformationsElement = componentElement->FirstChildElement("InstanceTransformations");
	if (instancedStaticMesh && instanceTransformationsElement)
	{
		std::vector<Matrix> instanceTransformations;
		for (tinyxml2::XMLElement* instanceTransformationElement = instanceTransformationsElement->FirstChildElement("InstanceTransformation");
			instanceTransformationElement != nullptr;
			instanceTransformationElement = instanceTransformationElement->NextSiblingElement("InstanceTransformation"))
		{
			Vector3 translation = Vector3::ZeroVector;
			Quaternion rotation = Quaternion::Identity;
			Vector3 scaling = Vector3(1.f);

			tinyxml2::XMLElement* transformationValueElement = instanceTransformationElement->FirstChildElement("Translation");
			if (transformationValueElement && transformationValueElement->GetText())
			{
				stream << transformationValueElement->GetText() << std::endl;
				stream >> translation.x >> translation.y >> translation.z;
				stream.clear();
			}

			transformationValueElement = instanceTransformationElement->FirstChildElement("EulerRotation");
			if (transformationValueElement && transformationValueElement->GetText())
			{
				Vector3 eulerRotation = Vector3::ZeroVector;
				stream << transformationValueElement->GetText() << std::endl;
				stream >> eulerRotation.x >> eulerRotation.y >> eulerRotation.z;
				rotation = Quaternion::FromEulerDegrees(eulerRotation);
				stream.clear();
			}

			transformationValueElement = instanceTransformationElement->FirstChildElement("Scaling");
			if (transformationValueElement && transformationValueElement->GetText())
			{
				stream << transformationValueElement->GetText() << std::endl;
				stream >> scaling.x >> scaling.y >> scaling.z;
				stream.clear();
			}

			instanceTransformations.push_back(Matrix::GetTransformationMatrix(rotation, translation, scaling));
		}

		instancedStaticMesh->SetInstanceTransformations(instanceTransformations);
		instancedStaticMesh->UpdateAllTransforms();
	}
}

void SceneParser::ParseBoxCollisionComponentValues(BoxCollisionComponent* boxCollisionComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("HalfSize");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 halfSize;
		stream >> halfSize.x >> halfSize.y >> halfSize.z;
		boxCollisionComponent->SetHalfSize(halfSize);
	}
	stream.clear();
}

void SceneParser::ParseCapsuleCollisionComponentValues(CapsuleCollisionComponent* capsuleCollisionComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("Radius");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		float radius;
		stream >> radius;
		capsuleCollisionComponent->SetRadius(radius);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("Height");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		float height;
		stream >> height;
		capsuleCollisionComponent->SetHeight(height);
	}
	stream.clear();
}

void SceneParser::ParseSphereCollisionComponentValues(SphereCollisionComponent* sphereCollisionComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("Radius");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		float radius;
		stream >> radius;
		sphereCollisionComponent->SetRadius(radius);
	}
	stream.clear();
}

void SceneParser::ParseMovingTriangleMeshCollisionComponentValues(MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("Mesh");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		std::string meshPath;
		stream >> meshPath;

		StaticMesh* relativeMesh = engine->GetResourceManager()->GetContent<StaticMesh>(ContentPathUtils::ToContentRelativePath(meshPath));
		if (relativeMesh)
		{
			movingTriangleMeshCollisionComponent->SetMesh(relativeMesh);
		}
	}
	stream.clear();
}

void SceneParser::ParseNonMovingTriangleMeshCollisionComponentValues(NonMovingTriangleMeshCollisionComponent* nonMovingTriangleMeshCollisionComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("Mesh");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		std::string meshPath;
		stream >> meshPath;

		StaticMesh* relativeMesh = engine->GetResourceManager()->GetContent<StaticMesh>(ContentPathUtils::ToContentRelativePath(meshPath));
		if(relativeMesh)
		{
			nonMovingTriangleMeshCollisionComponent->SetMesh(relativeMesh);
		}
	}
	stream.clear();
}

void SceneParser::ParseObjectBase(ObjectBase* object, tinyxml2::XMLElement* objectElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* child = objectElement->FirstChildElement("Name");

	if (child)
	{
		stream << child->GetText() << std::endl;
		std::string name;
		stream >> name;
		object->SetName(name);
	}
	stream.clear();

	child = objectElement->FirstChildElement("WorldPosition");
	if (child)
	{
		stream << child->GetText() << std::endl;
		Vector3 worldPosition;
		stream >> worldPosition.x >> worldPosition.y >> worldPosition.z;
		object->SetWorldPosition(worldPosition);
	}
	stream.clear();

	child = objectElement->FirstChildElement("EulerWorldRotation");
	if (child)
	{
		stream << child->GetText() << std::endl;
		Vector3 worldRotation;
		stream >> worldRotation.x >> worldRotation.y >> worldRotation.z;
		object->SetWorldRotation(Quaternion::FromEulerDegrees(worldRotation));
	}
	stream.clear();

	child = objectElement->FirstChildElement("WorldScaling");
	if (child)
	{
		stream << child->GetText() << std::endl;
		Vector3 worldScaling;
		stream >> worldScaling.x >> worldScaling.y >> worldScaling.z;
		object->SetWorldScaling(worldScaling);
	}
	stream.clear();

	child = objectElement->FirstChildElement("Components");
	if (child)
	{
		tinyxml2::XMLElement* componentElement = child->FirstChildElement("StaticMeshComponent");
		while (componentElement)
		{
			StaticMeshComponent* staticMeshComponent = object->AddSubComponent<StaticMeshComponent>();
			ParseStaticMeshComponentValues(staticMeshComponent, componentElement);

			ParseComponentValues(staticMeshComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("StaticMeshComponent");
		}

		componentElement = child->FirstChildElement("InstancedStaticMeshComponent");
		while (componentElement)
		{
			InstancedStaticMeshComponent* instancedStaticMeshComponent = object->AddSubComponent<InstancedStaticMeshComponent>();
			ParseInstancedStaticMeshComponentValues(instancedStaticMeshComponent, componentElement);

			ParseComponentValues(instancedStaticMeshComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("InstancedStaticMeshComponent");
		}
	}
}

void SceneParser::ParseRigidBody(RigidBody* rigidBody, tinyxml2::XMLElement* objectElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* child = objectElement->FirstChildElement("Mass");

	if (child)
	{
		stream << child->GetText() << std::endl;
		float mass;
		stream >> mass;
		rigidBody->SetMass(mass);
	}
	stream.clear();

	child = objectElement->FirstChildElement("CollisionGroup");
	if (child)
	{
		stream << child->GetText() << std::endl;
		int collisionGroupInt;
		stream >> collisionGroupInt;
		rigidBody->SetCollisionGroup((CollisionGroup)collisionGroupInt);
	}
	stream.clear();

	child = objectElement->FirstChildElement("CollisionMask");
	if (child)
	{
		stream << child->GetText() << std::endl;
		int collisionMaskInt;
		stream >> collisionMaskInt;
		rigidBody->SetCollisionMask((CollisionMask)collisionMaskInt);
	}
	stream.clear();

	child = objectElement->FirstChildElement("Components");
	if (child)
	{
		tinyxml2::XMLElement* componentElement = child->FirstChildElement("BoxCollisionComponent");
		while (componentElement)
		{
			BoxCollisionComponent* boxCollisionComponent = rigidBody->AddSubComponent<BoxCollisionComponent>();
			ParseBoxCollisionComponentValues(boxCollisionComponent, componentElement);

			ParseComponentValues(boxCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("BoxCollisionComponent");
		}

		componentElement = child->FirstChildElement("SphereCollisionComponent");
		while (componentElement)
		{
			SphereCollisionComponent* sphereCollisionComponent = rigidBody->AddSubComponent<SphereCollisionComponent>();
			ParseSphereCollisionComponentValues(sphereCollisionComponent, componentElement);

			ParseComponentValues(sphereCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("SphereCollisionComponent");
		}

		componentElement = child->FirstChildElement("CapsuleCollisionComponent");
		while (componentElement)
		{
			CapsuleCollisionComponent* capsuleCollisionComponent = rigidBody->AddSubComponent<CapsuleCollisionComponent>();
			ParseCapsuleCollisionComponentValues(capsuleCollisionComponent, componentElement);

			ParseComponentValues(capsuleCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("CapsuleCollisionComponent");
		}

		componentElement = child->FirstChildElement("MovingTriangleMeshCollisionComponent");
		while (componentElement)
		{
			MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent = rigidBody->AddSubComponent<MovingTriangleMeshCollisionComponent>();
			ParseMovingTriangleMeshCollisionComponentValues(movingTriangleMeshCollisionComponent, componentElement);

			ParseComponentValues(movingTriangleMeshCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("MovingTriangleMeshCollisionComponent");
		}

		componentElement = child->FirstChildElement("NonMovingTriangleMeshCollisionComponent");
		while (componentElement)
		{
			NonMovingTriangleMeshCollisionComponent* nonMovingTriangleMeshCollisionComponent = rigidBody->AddSubComponent<NonMovingTriangleMeshCollisionComponent>();
			ParseNonMovingTriangleMeshCollisionComponentValues(nonMovingTriangleMeshCollisionComponent, componentElement);

			ParseComponentValues(nonMovingTriangleMeshCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("NonMovingTriangleMeshCollisionComponent");
		}
	}
}

void SceneParser::GetXMLElement_DirectionalLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene)
{
	for (auto directionalLight : scene->GetDirectionalLights())
	{
		if (directionalLight->GetName().find("__Editor__") != std::string::npos)
		{
			continue;
		}

		tinyxml2::XMLElement* directionalLightElement = xmlDocument.NewElement("DirectionalLight");

		tinyxml2::XMLElement* directionalLightDirectionElement = xmlDocument.NewElement("Direction");
		directionalLightDirectionElement->SetText(Serialize(directionalLight->GetDirection()).c_str());
		directionalLightElement->InsertEndChild(directionalLightDirectionElement);

		tinyxml2::XMLElement* directionalLightColorElement = xmlDocument.NewElement("Color");
		directionalLightColorElement->SetText(Serialize(directionalLight->GetColor()).c_str());
		directionalLightElement->InsertEndChild(directionalLightColorElement);

		tinyxml2::XMLElement* directionalLightIntensityElement = xmlDocument.NewElement("Intensity");
		directionalLightIntensityElement->SetText(directionalLight->GetIntensity());
		directionalLightElement->InsertEndChild(directionalLightIntensityElement);

		tinyxml2::XMLElement* directionalLightIsCastingShadowElement = xmlDocument.NewElement("IsCastingShadow");
		directionalLightIsCastingShadowElement->SetText(directionalLight->GetIsShadowEnabled() ? "1" : "0");
		directionalLightElement->InsertEndChild(directionalLightIsCastingShadowElement);

		tinyxml2::XMLElement* directionalLightShadowIntensityElement = xmlDocument.NewElement("ShadowIntensity");
		directionalLightShadowIntensityElement->SetText(directionalLight->GetShadowIntensity());
		directionalLightElement->InsertEndChild(directionalLightShadowIntensityElement);

		tinyxml2::XMLElement* directionalLightShadowMapResolutionElement = xmlDocument.NewElement("ShadowMapResolution");
		directionalLightShadowMapResolutionElement->SetText((std::to_string(directionalLight->GetShadowWidth()) + " " + std::to_string(directionalLight->GetShadowHeight())).c_str());
		directionalLightElement->InsertEndChild(directionalLightShadowMapResolutionElement);

		parentElement->InsertEndChild(directionalLightElement);
	}
}

void SceneParser::GetXMLElement_SpotLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene)
{
	for (auto spotLight : scene->GetSpotLights())
	{
		if (spotLight->GetName().find("__Editor__") != std::string::npos)
		{
			continue;
		}

		tinyxml2::XMLElement* spotLightElement = xmlDocument.NewElement("SpotLight");

		tinyxml2::XMLElement* spotLightPositionElement = xmlDocument.NewElement("Position");
		spotLightPositionElement->SetText(Serialize(spotLight->GetPosition()).c_str());
		spotLightElement->InsertEndChild(spotLightPositionElement);

		tinyxml2::XMLElement* spotLightDirectionElement = xmlDocument.NewElement("Direction");
		spotLightDirectionElement->SetText(Serialize(spotLight->GetDirection()).c_str());
		spotLightElement->InsertEndChild(spotLightDirectionElement);

		tinyxml2::XMLElement* spotLightColorElement = xmlDocument.NewElement("Color");
		spotLightColorElement->SetText(Serialize(spotLight->GetColor()).c_str());
		spotLightElement->InsertEndChild(spotLightColorElement);

		tinyxml2::XMLElement* spotLightIntensityElement = xmlDocument.NewElement("Intensity");
		spotLightIntensityElement->SetText(spotLight->GetIntensity());
		spotLightElement->InsertEndChild(spotLightIntensityElement);

		tinyxml2::XMLElement* spotLightCoverageAngleElement = xmlDocument.NewElement("CoverageAngle");
		spotLightCoverageAngleElement->SetText(spotLight->GetCoverageAngle());
		spotLightElement->InsertEndChild(spotLightCoverageAngleElement);

		tinyxml2::XMLElement* spotLightFalloffAngleElement = xmlDocument.NewElement("FalloffAngle");
		spotLightFalloffAngleElement->SetText(spotLight->GetFalloffAngle());
		spotLightElement->InsertEndChild(spotLightFalloffAngleElement);

		tinyxml2::XMLElement* spotLightRadiusElement = xmlDocument.NewElement("Radius");
		spotLightRadiusElement->SetText(spotLight->GetRadius());
		spotLightElement->InsertEndChild(spotLightRadiusElement);

		tinyxml2::XMLElement* spotLightIsCastingShadowElement = xmlDocument.NewElement("IsCastingShadow");
		spotLightIsCastingShadowElement->SetText(spotLight->GetIsShadowEnabled() ? "1" : "0");
		spotLightElement->InsertEndChild(spotLightIsCastingShadowElement);

		tinyxml2::XMLElement* spotLightShadowIntensityElement = xmlDocument.NewElement("ShadowIntensity");
		spotLightShadowIntensityElement->SetText(spotLight->GetShadowIntensity());
		spotLightElement->InsertEndChild(spotLightShadowIntensityElement);

		tinyxml2::XMLElement* spotLightShadowMapResolutionElement = xmlDocument.NewElement("ShadowMapResolution");
		spotLightShadowMapResolutionElement->SetText((std::to_string(spotLight->GetShadowWidth()) + " " + std::to_string(spotLight->GetShadowHeight())).c_str());
		spotLightElement->InsertEndChild(spotLightShadowMapResolutionElement);

		parentElement->InsertEndChild(spotLightElement);
	}
}

void SceneParser::GetXMLElement_PointLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene)
{
	for (auto pointLight : scene->GetPointLights())
	{
		if (pointLight->GetName().find("__Editor__") != std::string::npos)
		{
			continue;
		}

		tinyxml2::XMLElement* pointLightElement = xmlDocument.NewElement("PointLight");

		tinyxml2::XMLElement* pointLightPositionElement = xmlDocument.NewElement("Position");
		pointLightPositionElement->SetText(Serialize(pointLight->GetPosition()).c_str());
		pointLightElement->InsertEndChild(pointLightPositionElement);

		tinyxml2::XMLElement* pointLightColorElement = xmlDocument.NewElement("Color");
		pointLightColorElement->SetText(Serialize(pointLight->GetColor()).c_str());
		pointLightElement->InsertEndChild(pointLightColorElement);

		tinyxml2::XMLElement* pointLightIntensityElement = xmlDocument.NewElement("Intensity");
		pointLightIntensityElement->SetText(pointLight->GetIntensity());
		pointLightElement->InsertEndChild(pointLightIntensityElement);

		tinyxml2::XMLElement* pointLightRadiusElement = xmlDocument.NewElement("Radius");
		pointLightRadiusElement->SetText(pointLight->GetRadius());
		pointLightElement->InsertEndChild(pointLightRadiusElement);

		tinyxml2::XMLElement* pointLightIsCastingShadowElement = xmlDocument.NewElement("IsCastingShadow");
		pointLightIsCastingShadowElement->SetText(pointLight->GetIsShadowEnabled() ? "1" : "0");
		pointLightElement->InsertEndChild(pointLightIsCastingShadowElement);

		tinyxml2::XMLElement* pointLightShadowIntensityElement = xmlDocument.NewElement("ShadowIntensity");
		pointLightShadowIntensityElement->SetText(pointLight->GetShadowIntensity());
		pointLightElement->InsertEndChild(pointLightShadowIntensityElement);

		tinyxml2::XMLElement* pointLightShadowMapResolutionElement = xmlDocument.NewElement("ShadowMapResolution");
		pointLightShadowMapResolutionElement->SetText((std::to_string(pointLight->GetShadowWidth()) + " " + std::to_string(pointLight->GetShadowHeight())).c_str());
		pointLightElement->InsertEndChild(pointLightShadowMapResolutionElement);

		parentElement->InsertEndChild(pointLightElement);
	}
}

void SceneParser::GetXMLElement_Objects(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const std::vector<ObjectBase*>& registeredObjects = engine->GetRegisteredObjects();
	for (ObjectBase* object : registeredObjects)
	{
		if (object->GetName().find("__Editor__") != std::string::npos)
		{
			continue;
		}

		if (dynamic_cast<DebugObject*>(object))
		{
			continue;
		}

		RigidBody* rigidBody = dynamic_cast<RigidBody*>(object);

		std::string objectTypeString = rigidBody ? "RigidBody" : "ObjectBase";

		tinyxml2::XMLElement* objectElement = xmlDocument.NewElement(objectTypeString.c_str());

		tinyxml2::XMLElement* objectNameElement = xmlDocument.NewElement("Name");
		objectNameElement->SetText(object->GetNameWithoutId().c_str());
		objectElement->InsertEndChild(objectNameElement);

		WriteTransformElementsIfNeeded(
			xmlDocument,
			objectElement,
			object->GetWorldPosition(),
			object->GetWorldRotation(),
			object->GetWorldScaling(),
			"WorldPosition",
			"EulerWorldRotation",
			"WorldScaling");

		if (rigidBody)
		{
			tinyxml2::XMLElement* rigidBodyMassElement = xmlDocument.NewElement("Mass");
			rigidBodyMassElement->SetText(rigidBody->GetMass());
			objectElement->InsertEndChild(rigidBodyMassElement);
			
			tinyxml2::XMLElement* rigidBodyCollisionGroupElement = xmlDocument.NewElement("CollisionGroup");
			rigidBodyCollisionGroupElement->SetText((int)rigidBody->GetCollisionGroup());
			objectElement->InsertEndChild(rigidBodyCollisionGroupElement);
			
			tinyxml2::XMLElement* rigidBodyCollisionMaskElement = xmlDocument.NewElement("CollisionMask");
			rigidBodyCollisionMaskElement->SetText((int)rigidBody->GetCollisionMask());
			objectElement->InsertEndChild(rigidBodyCollisionMaskElement);
		}

		tinyxml2::XMLElement* componentsElement = xmlDocument.NewElement("Components");
		GetXMLElement_Components(object, xmlDocument, componentsElement);
		objectElement->InsertEndChild(componentsElement);
		
		parentElement->InsertEndChild(objectElement);
	}
}

void SceneParser::GetXMLElement_Components(const ObjectBase* const objectBase, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const std::vector<Component*>& components = objectBase->GetComponents();
	for (Component* component : components)
	{
		if (component == nullptr)
		{
			continue;
		}

		tinyxml2::XMLElement* componentElement;

		if (StaticMeshComponent* staticMeshComponent = dynamic_cast<StaticMeshComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("StaticMeshComponent");
			GetXMLElement_StaticMeshComponent(staticMeshComponent, xmlDocument, componentElement);
		}
		else if (InstancedStaticMeshComponent* instancedStaticMeshComponent = dynamic_cast<InstancedStaticMeshComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("InstancedStaticMeshComponent");
			GetXMLElement_InstancedStaticMeshComponent(instancedStaticMeshComponent, xmlDocument, componentElement);
		}
		else if (BoxCollisionComponent* boxCollisionComponent = dynamic_cast<BoxCollisionComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("BoxCollisionComponent");
			GetXMLElement_BoxCollisionComponent(boxCollisionComponent, xmlDocument, componentElement);
		}
		else if (CapsuleCollisionComponent* capsuleCollisionComponent = dynamic_cast<CapsuleCollisionComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("CapsuleCollisionComponent");
			GetXMLElement_CapsuleCollisionComponent(capsuleCollisionComponent, xmlDocument, componentElement);
		}
		else if (SphereCollisionComponent* sphereCollisionComponent = dynamic_cast<SphereCollisionComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("SphereCollisionComponent");
			GetXMLElement_SphereCollisionComponent(sphereCollisionComponent, xmlDocument, componentElement);
		}
		else if (MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent = dynamic_cast<MovingTriangleMeshCollisionComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("MovingTriangleMeshCollisionComponent");
			GetXMLElement_MovingTriangleMeshCollisionComponent(movingTriangleMeshCollisionComponent, xmlDocument, componentElement);
		}
		else if (NonMovingTriangleMeshCollisionComponent* nonMovingTriangleMeshCollisionComponent = dynamic_cast<NonMovingTriangleMeshCollisionComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("NonMovingTriangleMeshCollisionComponent");
			GetXMLElement_NonMovingTriangleMeshCollisionComponent(nonMovingTriangleMeshCollisionComponent, xmlDocument, componentElement);
		}
		else
		{
			componentElement = xmlDocument.NewElement("Component");
		}

		WriteTransformElementsIfNeeded(
			xmlDocument,
			componentElement,
			component->GetRelativePosition(),
			component->GetRelativeRotation(),
			component->GetRelativeScaling(),
			"RelativePosition",
			"EulerRelativeRotation",
			"RelativeScaling");

		parentElement->InsertEndChild(componentElement);
	}
}

void SceneParser::GetXMLElement_StaticMeshComponent(const StaticMeshComponent* const staticMeshComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	tinyxml2::XMLElement* staticMeshComponentMeshPathElement = xmlDocument.NewElement("MeshPath"); 
	const std::string meshPath = ContentPathUtils::ToContentRelativePath(staticMeshComponent->GetMeshInstance()->GetMesh()->GetPath());
	staticMeshComponentMeshPathElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(staticMeshComponentMeshPathElement);

	tinyxml2::XMLElement* staticMeshInstanceRenderMaskElement = xmlDocument.NewElement("RenderMask");
	staticMeshInstanceRenderMaskElement->SetText(staticMeshComponent->GetMeshInstance()->GetRenderMask());
	parentElement->InsertEndChild(staticMeshInstanceRenderMaskElement);

	WriteMaterialPaths(xmlDocument, parentElement, GetStaticMeshComponentMaterialPaths(staticMeshComponent));
}

void SceneParser::GetXMLElement_InstancedStaticMeshComponent(const InstancedStaticMeshComponent* const instancedStaticMeshComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const InstancedStaticMeshInstance* meshInstance = instancedStaticMeshComponent->GetMeshInstance();
	const InstancedStaticMesh* instancedStaticMesh = meshInstance ? meshInstance->GetMesh() : nullptr;
	if (!meshInstance || !instancedStaticMesh)
	{
		return;
	}

	tinyxml2::XMLElement* instancedStaticMeshComponentMeshPathElement = xmlDocument.NewElement("MeshPath");
	auto sourcePathIterator = instancedStaticMeshSourcePathMap.find(instancedStaticMesh);
	const std::string meshPath = !instancedStaticMesh->GetSourceMeshPath().empty() ?
		ContentPathUtils::ToContentRelativePath(instancedStaticMesh->GetSourceMeshPath()) :
		sourcePathIterator != instancedStaticMeshSourcePathMap.end() ?
			sourcePathIterator->second :
			ContentPathUtils::ToContentRelativePath(instancedStaticMesh->GetPath());
	instancedStaticMeshComponentMeshPathElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(instancedStaticMeshComponentMeshPathElement);

	tinyxml2::XMLElement* instancedStaticMeshInstanceRenderMaskElement = xmlDocument.NewElement("RenderMask");
	instancedStaticMeshInstanceRenderMaskElement->SetText(meshInstance->GetRenderMask());
	parentElement->InsertEndChild(instancedStaticMeshInstanceRenderMaskElement);

	WriteMaterialPaths(xmlDocument, parentElement, GetInstancedStaticMeshComponentMaterialPaths(instancedStaticMeshComponent));

	if (instancedStaticMesh->GetInstanceCount() == 0)
	{
		return;
	}

	tinyxml2::XMLElement* instanceTransformationsElement = xmlDocument.NewElement("InstanceTransformations");
	for (size_t instanceIndex = 0; instanceIndex < instancedStaticMesh->GetInstanceCount(); ++instanceIndex)
	{
		const Matrix& instanceTransformationMatrix = instancedStaticMesh->GetInstanceTransformationAt(instanceIndex);
		Vector3 translation = Vector3::ZeroVector;
		Vector3 scaling = Vector3(1.f);
		Quaternion rotation = Quaternion::Identity;
		instanceTransformationMatrix.Decompose(translation, scaling, rotation);

		tinyxml2::XMLElement* instanceTransformationElement = xmlDocument.NewElement("InstanceTransformation");
		WriteTransformElementsIfNeeded(
			xmlDocument,
			instanceTransformationElement,
			translation,
			rotation,
			scaling,
			"Translation",
			"EulerRotation",
			"Scaling");
		instanceTransformationsElement->InsertEndChild(instanceTransformationElement);
	}

	parentElement->InsertEndChild(instanceTransformationsElement);
}

void SceneParser::GetXMLElement_BoxCollisionComponent(const BoxCollisionComponent* const boxCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	tinyxml2::XMLElement* boxCollisionComponentHalfSizeElement = xmlDocument.NewElement("HalfSize");
	boxCollisionComponentHalfSizeElement->SetText(Serialize(boxCollisionComponent->GetHalfSize()).c_str());
	parentElement->InsertEndChild(boxCollisionComponentHalfSizeElement);
}

void SceneParser::GetXMLElement_CapsuleCollisionComponent(const CapsuleCollisionComponent* const capsuleCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	tinyxml2::XMLElement* capsuleCollisionComponentRadiusElement = xmlDocument.NewElement("Radius");
	capsuleCollisionComponentRadiusElement->SetText(capsuleCollisionComponent->GetRadius());
	parentElement->InsertEndChild(capsuleCollisionComponentRadiusElement);

	tinyxml2::XMLElement* capsuleCollisionComponentHeightElement = xmlDocument.NewElement("Height");
	capsuleCollisionComponentHeightElement->SetText(capsuleCollisionComponent->GetHeight());
	parentElement->InsertEndChild(capsuleCollisionComponentHeightElement);
}

void SceneParser::GetXMLElement_SphereCollisionComponent(const SphereCollisionComponent* const sphereCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	tinyxml2::XMLElement* sphereCollisionComponentRadiusElement = xmlDocument.NewElement("Radius");
	sphereCollisionComponentRadiusElement->SetText(sphereCollisionComponent->GetRadius());
	parentElement->InsertEndChild(sphereCollisionComponentRadiusElement);
}

void SceneParser::GetXMLElement_MovingTriangleMeshCollisionComponent(const MovingTriangleMeshCollisionComponent* const movingTriangleMeshCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	tinyxml2::XMLElement* movingTriangleMeshCollisionComponentElement = xmlDocument.NewElement("Mesh");
	const std::string meshPath = ContentPathUtils::ToContentRelativePath(movingTriangleMeshCollisionComponent->GetMesh()->GetPath());
	movingTriangleMeshCollisionComponentElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(movingTriangleMeshCollisionComponentElement);
}

void SceneParser::GetXMLElement_NonMovingTriangleMeshCollisionComponent(const NonMovingTriangleMeshCollisionComponent* const nonMovingTriangleMeshCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	tinyxml2::XMLElement* nonMovingTriangleMeshCollisionComponentElement = xmlDocument.NewElement("Mesh");
	const std::string meshPath = ContentPathUtils::ToContentRelativePath(nonMovingTriangleMeshCollisionComponent->GetMesh()->GetPath());
	nonMovingTriangleMeshCollisionComponentElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(nonMovingTriangleMeshCollisionComponentElement);
}

std::string SceneParser::Serialize(const Vector3& vector)
{
	return SerializeVector3(vector);
}
