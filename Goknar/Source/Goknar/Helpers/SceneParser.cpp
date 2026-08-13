#include "pch.h"

#include "SceneParser.h"

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Scene.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Objects/ReflectionProbeObject.h"
#include "Goknar/Objects/PlayerStart.h"

#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Components/DynamicMeshComponent.h"
#include "Goknar/Components/GPUFoliageComponent.h"
#include "Goknar/Components/InstancedStaticMeshComponent.h"
#include "Goknar/Components/ParticleSystemComponent.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Components/SkeletalMeshComponent.h"

#include "Goknar/Factories/DynamicObjectFactory.h"

#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Helpers/AssetParser.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/IO/ModelLoader.h"
#include "Goknar/Contents/Image.h"

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
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "Goknar/Debug/DebugDrawer.h"

#include "Goknar/Navigation/NavigationTreeComponent.h"
#include "Goknar/Navigation/NavigationTypes.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Texture.h"

#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/OverlappingPhysicsObject.h"
#include "Goknar/Physics/Components/BoxCollisionComponent.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"
#include "Goknar/Physics/Components/HeightMapCollisionComponent.h"
#include "Goknar/Physics/Components/SphereCollisionComponent.h"
#include "Goknar/Physics/Components/MovingTriangleMeshCollisionComponent.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

#include "tinyxml2.h"

namespace
{
	std::unordered_map<const InstancedStaticMeshComponent*, std::vector<std::string>> instancedStaticMeshComponentMaterialPathMap;
	std::unordered_map<const StaticMeshComponent*, std::vector<std::string>> staticMeshComponentMaterialPathMap;
	std::unordered_map<const SkeletalMeshComponent*, std::vector<std::string>> skeletalMeshComponentMaterialPathMap;
	std::unordered_map<const InstancedStaticMesh*, std::string> instancedStaticMeshSourcePathMap;
	std::unordered_map<std::string, Material*> sharedMaterialPathMap;
	size_t instancedStaticMeshIdentifier = 0;

	struct SceneTransform
	{
		Vector3 position{ Vector3::ZeroVector };
		Quaternion rotation{ Quaternion::Identity };
		Vector3 scaling{ Vector3(1.f) };
	};

	struct SceneParseContext
	{
		std::string scenePath;
		SceneTransform transform;
		ObjectBase* sceneRootObject{ nullptr };
		bool isReferencedScene{ false };
		bool applySceneSettings{ true };
	};

	std::vector<SceneParseContext> sceneParseContextStack;

	std::vector<std::string> NormalizeMaterialPaths(const std::vector<std::string>& materialPaths);

	TextureAtlasCategory GetTextureAtlasCategory(const Material* material)
	{
		return material && material->GetBlendModel() == MaterialBlendModel::Transparent ?
			TextureAtlasCategory::Transparent :
			TextureAtlasCategory::Opaque;
	}

	std::string SerializeVector3(const Vector3& vector)
	{
		return std::to_string(vector.x) + " " + std::to_string(vector.y) + " " + std::to_string(vector.z);
	}

	std::string SerializeVector4(const Vector4& vector)
	{
		return std::to_string(vector.x) + " " + std::to_string(vector.y) + " " + std::to_string(vector.z) + " " + std::to_string(vector.w);
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
				AssetParser::RegisterMaterialTexturesToTextureAtlas(material);
			}
		}
	}

	StaticMesh* ResolveStaticMeshFromPath(const std::string& meshPath)
	{
		const std::string relativeMeshPath = ContentPathUtils::ToContentRelativePath(meshPath);
		if (relativeMeshPath.empty())
		{
			return nullptr;
		}

		return engine->GetResourceManager()->GetContent<StaticMesh>(relativeMeshPath);
	}

	InstancedStaticMesh* CreateInstancedStaticMeshFromPath(const std::string& meshPath)
	{
		const std::string relativeMeshPath = ContentPathUtils::ToContentRelativePath(meshPath);
		StaticMesh* sourceMesh = ResolveStaticMeshFromPath(meshPath);
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

	Vector3 TransformPoint(const SceneTransform& transform, const Vector3& point)
	{
		return transform.position + transform.rotation * (point * transform.scaling);
	}

	Vector3 TransformDirection(const SceneTransform& transform, const Vector3& direction)
	{
		return transform.rotation * direction;
	}

	Quaternion TransformRotation(const SceneTransform& transform, const Quaternion& rotation)
	{
		return transform.rotation * rotation;
	}

	Vector3 TransformScaling(const SceneTransform& transform, const Vector3& scaling)
	{
		return transform.scaling * scaling;
	}

	SceneTransform ComposeTransform(const SceneTransform& parentTransform, const SceneTransform& relativeTransform)
	{
		SceneTransform composedTransform;
		composedTransform.position = TransformPoint(parentTransform, relativeTransform.position);
		composedTransform.rotation = TransformRotation(parentTransform, relativeTransform.rotation);
		composedTransform.scaling = TransformScaling(parentTransform, relativeTransform.scaling);
		return composedTransform;
	}

	const char* GetElementText(const tinyxml2::XMLElement* element)
	{
		return element ? element->GetText() : nullptr;
	}

	std::string TrimString(const std::string& value)
	{
		const size_t firstNonWhitespaceCharacterIndex = value.find_first_not_of(" \t\n\r");
		if (firstNonWhitespaceCharacterIndex == std::string::npos)
		{
			return "";
		}

		const size_t lastNonWhitespaceCharacterIndex = value.find_last_not_of(" \t\n\r");
		return value.substr(firstNonWhitespaceCharacterIndex, lastNonWhitespaceCharacterIndex - firstNonWhitespaceCharacterIndex + 1);
	}

	bool ReadVector3Element(const tinyxml2::XMLElement* parentElement, const char* elementName, Vector3& outVector)
	{
		const tinyxml2::XMLElement* vectorElement = parentElement ? parentElement->FirstChildElement(elementName) : nullptr;
		const char* text = GetElementText(vectorElement);
		if (!text)
		{
			return false;
		}

		std::stringstream stream;
		stream << text << std::endl;
		stream >> outVector.x >> outVector.y >> outVector.z;
		return true;
	}

	bool ReadStringElement(const tinyxml2::XMLElement* parentElement, const char* elementName, std::string& outValue)
	{
		const tinyxml2::XMLElement* stringElement = parentElement ? parentElement->FirstChildElement(elementName) : nullptr;
		const char* text = GetElementText(stringElement);
		if (!text)
		{
			return false;
		}

		outValue = TrimString(text);
		return true;
	}

	bool ReadNavigationTreePath(const tinyxml2::XMLElement* element, std::string& outNavigationTreePath)
	{
		if (!element)
		{
			return false;
		}

		const char* pathAttribute = element->Attribute("NavigationTreePath");
		if (!pathAttribute)
		{
			pathAttribute = element->Attribute("TreePath");
		}
		if (!pathAttribute)
		{
			pathAttribute = element->Attribute("Path");
		}
		if (pathAttribute)
		{
			outNavigationTreePath = TrimString(pathAttribute);
			return !outNavigationTreePath.empty();
		}

		return ReadStringElement(element, "NavigationTreePath", outNavigationTreePath) ||
			ReadStringElement(element, "TreePath", outNavigationTreePath) ||
			ReadStringElement(element, "Path", outNavigationTreePath);
	}

	SceneTransform ReadSceneReferenceTransform(const tinyxml2::XMLElement* sceneElement)
	{
		SceneTransform transform;
		ReadVector3Element(sceneElement, "WorldPosition", transform.position);

		Vector3 eulerRotation = Vector3::ZeroVector;
		if (ReadVector3Element(sceneElement, "WorldRotation", eulerRotation))
		{
			transform.rotation = Quaternion::FromEulerDegrees(eulerRotation);
		}

		ReadVector3Element(sceneElement, "WorldScaling", transform.scaling);

		return transform;
	}

	std::string ReadSceneReferencePath(const tinyxml2::XMLElement* sceneElement)
	{
		if (!sceneElement)
		{
			return "";
		}

		const char* pathAttribute = sceneElement->Attribute("Path");
		if (!pathAttribute)
		{
			pathAttribute = sceneElement->Attribute("path");
		}
		if (pathAttribute)
		{
			return pathAttribute;
		}

		const char* pathText = GetElementText(sceneElement->FirstChildElement("Path"));
		if (!pathText)
		{
			pathText = GetElementText(sceneElement->FirstChildElement("ScenePath"));
		}
		if (!pathText)
		{
			pathText = GetElementText(sceneElement);
		}

		return pathText ? pathText : "";
	}

	bool SceneFileExists(const std::string& contentRelativeScenePath)
	{
		return std::filesystem::exists(ContentPathUtils::ToAbsoluteContentPath(contentRelativeScenePath));
	}

	std::string ResolveSceneReferencePath(const std::string& referencePath, const std::string& currentScenePath)
	{
		const std::string normalizedReferencePath = ContentPathUtils::NormalizePath(referencePath);
		if (normalizedReferencePath.empty())
		{
			return "";
		}

		const std::string contentRelativeReferencePath = ContentPathUtils::ToContentRelativePath(normalizedReferencePath);
		if (SceneFileExists(contentRelativeReferencePath))
		{
			return contentRelativeReferencePath;
		}

		const std::filesystem::path currentSceneDirectory = std::filesystem::path(currentScenePath).parent_path();
		const std::string relativeToCurrentScenePath = ContentPathUtils::NormalizePath((currentSceneDirectory / normalizedReferencePath).generic_string());
		if (SceneFileExists(relativeToCurrentScenePath))
		{
			return relativeToCurrentScenePath;
		}

		return contentRelativeReferencePath;
	}

	bool IsSceneInCurrentParseStack(const std::string& scenePath)
	{
		for (const SceneParseContext& parseContext : sceneParseContextStack)
		{
			if (parseContext.scenePath == scenePath)
			{
				return true;
			}
		}

		return false;
	}

	bool ParseSceneReference(Scene* scene, const SceneReference& sceneReference, const SceneParseContext& currentContext, bool addReferenceToScene)
	{
		if (!scene)
		{
			return false;
		}

		const std::string referencedScenePath = ResolveSceneReferencePath(sceneReference.path, currentContext.scenePath);
		if (referencedScenePath.empty())
		{
			return false;
		}

		if (!SceneFileExists(referencedScenePath))
		{
			GOKNAR_CORE_WARN("Skipping missing scene reference %s.", referencedScenePath.c_str());
			return false;
		}

		if (IsSceneInCurrentParseStack(referencedScenePath))
		{
			GOKNAR_CORE_WARN("Skipping recursive scene reference %s.", referencedScenePath.c_str());
			return false;
		}

		SceneTransform relativeTransform;
		relativeTransform.position = sceneReference.position;
		relativeTransform.rotation = sceneReference.rotation;
		relativeTransform.scaling = sceneReference.scaling;

		SceneParseContext referencedSceneContext;
		referencedSceneContext.scenePath = referencedScenePath;
		referencedSceneContext.transform = ComposeTransform(currentContext.transform, relativeTransform);
		referencedSceneContext.isReferencedScene = true;
		referencedSceneContext.applySceneSettings = false;

		ObjectBase* sceneRootObject = new ObjectBase();
		std::string sceneRootObjectName = std::filesystem::path(referencedScenePath).stem().generic_string();
		if (sceneRootObjectName.empty())
		{
			sceneRootObjectName = std::filesystem::path(referencedScenePath).filename().generic_string();
		}
		sceneRootObject->SetName(sceneRootObjectName.empty() ? "Scene" : sceneRootObjectName);
		sceneRootObject->SetWorldPosition(referencedSceneContext.transform.position, false);
		sceneRootObject->SetWorldRotation(referencedSceneContext.transform.rotation, false);
		sceneRootObject->SetWorldScaling(referencedSceneContext.transform.scaling);
		if (currentContext.sceneRootObject)
		{
			sceneRootObject->SetParent(currentContext.sceneRootObject, SnappingRule::KeepWorldAll);
		}

		scene->AddObject(sceneRootObject, true);
		referencedSceneContext.sceneRootObject = sceneRootObject;

		SceneReference resolvedSceneReference = sceneReference;
		resolvedSceneReference.path = referencedScenePath;
		resolvedSceneReference.sceneRootObject = sceneRootObject;

		if (addReferenceToScene)
		{
			scene->AddSceneReference(resolvedSceneReference);
		}

		sceneParseContextStack.push_back(referencedSceneContext);
		SceneParser::Parse(scene, ContentPathUtils::ToAbsoluteContentPath(referencedScenePath));
		sceneParseContextStack.pop_back();

		return true;
	}

	void ParseReferencedScenes(Scene* scene, tinyxml2::XMLElement* rootElement)
	{
		const tinyxml2::XMLElement* scenesElement = rootElement ? rootElement->FirstChildElement("Scenes") : nullptr;
		if (!scenesElement || sceneParseContextStack.empty())
		{
			return;
		}

		const SceneParseContext currentContext = sceneParseContextStack.back();
		for (const tinyxml2::XMLElement* sceneElement = scenesElement->FirstChildElement("Scene");
			sceneElement != nullptr;
			sceneElement = sceneElement->NextSiblingElement("Scene"))
		{
			SceneReference sceneReference;
			sceneReference.path = ReadSceneReferencePath(sceneElement);
			const SceneTransform relativeTransform = ReadSceneReferenceTransform(sceneElement);
			sceneReference.position = relativeTransform.position;
			sceneReference.rotation = relativeTransform.rotation;
			sceneReference.scaling = relativeTransform.scaling;

			ParseSceneReference(scene, sceneReference, currentContext, !currentContext.isReferencedScene);
		}
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
	AssetParser::RegisterMaterialTexturesToTextureAtlas(material);
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

void SceneParser::ApplySkeletalMeshComponentMaterialPaths(SkeletalMeshComponent* skeletalMeshComponent, const std::vector<std::string>& materialPaths)
{
	ApplyMeshComponentMaterialPaths<SkeletalMeshComponent, SkeletalMeshInstance, SkeletalMesh>(
		skeletalMeshComponent,
		materialPaths,
		skeletalMeshComponentMaterialPathMap);
}

std::vector<std::string> SceneParser::GetSkeletalMeshComponentMaterialPaths(const SkeletalMeshComponent* skeletalMeshComponent)
{
	return GetMeshComponentMaterialPaths(skeletalMeshComponent, skeletalMeshComponentMaterialPathMap);
}

void SceneParser::ApplySkeletalMeshComponentMaterialPath(SkeletalMeshComponent* skeletalMeshComponent, const std::string& materialPath)
{
	if (materialPath.empty())
	{
		ApplySkeletalMeshComponentMaterialPaths(skeletalMeshComponent, {});
		return;
	}

	ApplySkeletalMeshComponentMaterialPaths(skeletalMeshComponent, { materialPath });
}

std::string SceneParser::GetSkeletalMeshComponentMaterialPath(const SkeletalMeshComponent* skeletalMeshComponent)
{
	const std::vector<std::string> materialPaths = GetSkeletalMeshComponentMaterialPaths(skeletalMeshComponent);
	return materialPaths.empty() ? "" : materialPaths[0];
}

void SceneParser::ClearSkeletalMeshComponentMaterialPath(const SkeletalMeshComponent* skeletalMeshComponent)
{
	ClearMeshComponentMaterialPath<SkeletalMeshComponent, SkeletalMeshInstance, SkeletalMesh>(
		skeletalMeshComponent,
		skeletalMeshComponentMaterialPathMap);
}

void SceneParser::ClearCaches()
{
	instancedStaticMeshComponentMaterialPathMap.clear();
	instancedStaticMeshSourcePathMap.clear();
	staticMeshComponentMaterialPathMap.clear();
	skeletalMeshComponentMaterialPathMap.clear();
	sharedMaterialPathMap.clear();
}

bool SceneParser::InsertSceneReference(Scene* scene, const SceneReference& sceneReference)
{
	if (!scene)
	{
		return false;
	}

	const bool pushedDefaultParseContext = sceneParseContextStack.empty();
	if (pushedDefaultParseContext)
	{
		SceneParseContext parseContext;
		parseContext.scenePath = ContentPathUtils::NormalizePath(ContentPathUtils::ToContentRelativePath(scene->GetPath()));
		sceneParseContextStack.push_back(parseContext);
	}

	const SceneParseContext currentContext = sceneParseContextStack.back();
	const bool parsedSceneReference = ParseSceneReference(scene, sceneReference, currentContext, true);
	if (parsedSceneReference)
	{
		const NavMeshSettings defaultSettings;
		scene->RebuildNavigationMesh(defaultSettings);
	}

	if (pushedDefaultParseContext)
	{
		sceneParseContextStack.pop_back();
	}

	return parsedSceneReference;
}

void SceneParser::Parse(Scene* scene, const std::string& filePath)
{
	bool pushedDefaultParseContext = false;
	if (sceneParseContextStack.empty())
	{
		SceneParseContext parseContext;
		parseContext.scenePath = ContentPathUtils::ToContentRelativePath(filePath);
		sceneParseContextStack.push_back(parseContext);
		pushedDefaultParseContext = true;
	}

	const SceneParseContext currentParseContext = sceneParseContextStack.back();

	tinyxml2::XMLDocument xmlFile;
	std::stringstream stream;

	tinyxml2::XMLError res;

	ResourceManager* resourceManager = engine->GetResourceManager();

	try
	{
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(filePath, fileContents))
		{
			res = tinyxml2::XML_ERROR_FILE_NOT_FOUND;
		}
		else
		{
			res = xmlFile.Parse(fileContents.c_str(), fileContents.size());
		}
		if (res)
		{
			throw std::runtime_error("Error: Scene XML file could not be loaded at " + filePath + ".");
		}
	}
	catch (std::exception& exception)
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
	if (currentParseContext.applySceneSettings)
	{
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
	}

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

	if (currentParseContext.applySceneSettings && element)
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
			position = TransformPoint(currentParseContext.transform, position);
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

			scene->AddPointLight(pointLight, currentParseContext.isReferencedScene);

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
			direction = TransformDirection(currentParseContext.transform, direction);
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

			scene->AddDirectionalLight(directionalLight, currentParseContext.isReferencedScene);

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
			position = TransformPoint(currentParseContext.transform, position);
			spotLight->SetPosition(position);

			Vector3 direction;
			stream >> direction.x >> direction.y >> direction.z;
			direction = TransformDirection(currentParseContext.transform, direction);
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

			scene->AddSpotLight(spotLight, currentParseContext.isReferencedScene);

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
			std::string textureImagePath;
			bool hasExplicitTextureName = false;
			// Scene <Texture> entries are material-facing textures, so they are safe
			// candidates for the generated atlas-aware shader path by default.
			// UseTextureAtlas="false" / CanUseTextureAtlas="false" remains an opt-out.
			const bool useTextureAtlas = AssetParser::ReadTextureAtlasUsage(element, true);

			child = element->FirstChildElement("Path");
			if (child)
			{
				stream << child->GetText() << std::endl;
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

				texture->SetTextureUsage(usage == "Diffuse" ? TextureUsage::Diffuse :
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
				hasExplicitTextureName = !textureName.empty();
			}
			stream.clear();

			if (useTextureAtlas && !textureImagePath.empty())
			{
				AssetParser::RegisterTextureToTextureAtlas(texture, hasExplicitTextureName, true);
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
				Texture* texture = const_cast<Texture*>(scene->GetTexture(textureId));
				if (texture && (texture->GetWaitsForTextureAtlas() || texture->GetUsesAtlasTexture()))
				{
					AssetParser::RegisterTextureToTextureAtlas(texture, false, false, GetTextureAtlasCategory(material));
				}

				material->GetShader(RenderPassType::Forward)->AddTexture(texture);
				child = child->NextSiblingElement("Texture");
			}

			material->SetAmbientOcclusion(1.f);
			child = element->FirstChildElement("AmbientOcclusionValue");
			if (child)
			{
				stream << child->GetText() << std::endl;
				float ambientOcclusion = 1.f;
				stream >> ambientOcclusion;
				material->SetAmbientOcclusion(ambientOcclusion);
			}

			child = element->FirstChildElement("DiffuseReflectance");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 diffuseReflectance;
				stream >> diffuseReflectance.x >> diffuseReflectance.y >> diffuseReflectance.z;
				material->SetBaseColor(diffuseReflectance);
			}

			material->SetMetallic(0.f);
			child = element->FirstChildElement("MetallicValue");
			if (child)
			{
				stream << child->GetText() << std::endl;
				float metallic = 0.f;
				stream >> metallic;
				material->SetMetallic(metallic);
			}

			material->SetRoughness(0.5f);
			child = element->FirstChildElement("RoughnessValue");
			if (child)
			{
				stream << child->GetText() << std::endl;
				float roughness = 0.5f;
				stream >> roughness;
				material->SetRoughness(roughness);
			}
			else
			{
				child = element->FirstChildElement("PhongExponent");
				if (child)
				{
					stream << child->GetText() << std::endl;
					float phongExponent = 1.f;
					stream >> phongExponent;
					if (!std::isfinite(phongExponent) || phongExponent < 1.f)
					{
						phongExponent = 1.f;
					}

					material->SetRoughness(std::sqrt(2.f / (phongExponent + 2.f)));
				}
			}

			element = element->NextSiblingElement("Material");
		}
		stream.clear();
	}

	ParseReferencedScenes(scene, root->ToElement());

	element = root->FirstChildElement("Objects");
	if (element)
	{
		DynamicObjectFactory* dynamicObjectFactory = DynamicObjectFactory::GetInstance();

		const auto parseObjectElement =
			[&](auto&& parseObjectElement, tinyxml2::XMLElement* objectElement, ObjectBase* parentObject) -> void
			{
				if (!objectElement)
				{
					return;
				}

				const char* objectTypeName = objectElement->Name();
				ObjectBase* object = dynamicObjectFactory->Create(objectTypeName ? objectTypeName : "");
				if (!object)
				{
					return;
				}

				object->SetName(objectTypeName ? objectTypeName : "ObjectBase");

				PhysicsObject* physicsObject = dynamic_cast<RigidBody*>(object);
				if (!physicsObject)
				{
					physicsObject = dynamic_cast<OverlappingPhysicsObject*>(object);
				}

				if (physicsObject)
				{
					ParsePhysicsObject(physicsObject, objectElement);
				}

				ParseObjectBase(object, objectElement);

				if (parentObject)
				{
					const Vector3 parentRelativePosition = object->GetWorldPosition();
					const Quaternion parentRelativeRotation = object->GetWorldRotation();
					const Vector3 parentRelativeScaling = object->GetWorldScaling();
					object->SetParent(parentObject, SnappingRule::None, false);
					object->SetWorldPosition(parentRelativePosition, false);
					object->SetWorldRotation(parentRelativeRotation, false);
					object->SetWorldScaling(parentRelativeScaling);
				}
				else
				{
					object->SetWorldPosition(TransformPoint(currentParseContext.transform, object->GetWorldPosition()), false);
					object->SetWorldRotation(TransformRotation(currentParseContext.transform, object->GetWorldRotation()), false);
					object->SetWorldScaling(TransformScaling(currentParseContext.transform, object->GetWorldScaling()));
					if (currentParseContext.sceneRootObject)
					{
						object->SetParent(currentParseContext.sceneRootObject, SnappingRule::KeepWorldAll);
					}
				}

				if (ReflectionProbeObject* reflectionProbeObject = dynamic_cast<ReflectionProbeObject*>(object))
				{
					ParseReflectionProbeObject(reflectionProbeObject, objectElement);
				}

				scene->AddObject(object, currentParseContext.isReferencedScene);

				tinyxml2::XMLElement* childrenElement = objectElement->FirstChildElement("Children");
				if (childrenElement)
				{
					for (tinyxml2::XMLElement* childObjectElement = childrenElement->FirstChildElement();
						childObjectElement;
						childObjectElement = childObjectElement->NextSiblingElement())
					{
						parseObjectElement(parseObjectElement, childObjectElement, object);
					}
				}
			};

		for (tinyxml2::XMLElement* objectElement = element->FirstChildElement();
			objectElement;
			objectElement = objectElement->NextSiblingElement())
		{
			parseObjectElement(parseObjectElement, objectElement, nullptr);
		}
	}

	if (pushedDefaultParseContext)
	{
		sceneParseContextStack.pop_back();
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

	if (!scene->GetSceneReferences().empty())
	{
		subElement = sceneXML.NewElement("Scenes");
		GetXMLElement_SceneReferences(sceneXML, subElement, scene);
		rootElement->InsertEndChild(subElement);
	}

	subElement = sceneXML.NewElement("Objects");
	GetXMLElement_Objects(sceneXML, subElement, scene);
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
		Vector3 position;
		stream >> position.x >> position.y >> position.z;
		component->SetRelativePosition(position);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("EulerRelativeRotation");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 rotation;
		stream >> rotation.x >> rotation.y >> rotation.z;
		component->SetRelativeRotation(Quaternion::FromEulerDegrees(rotation));
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("RelativeScaling");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 scaling;
		stream >> scaling.x >> scaling.y >> scaling.z;
		component->SetRelativeScaling(scaling);
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

void SceneParser::ParseSkeletalMeshComponentValues(SkeletalMeshComponent* skeletalMeshComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("MeshPath");
	if (dataElement)
	{
		stream << dataElement->GetText() << std::endl;
		std::string meshPath;
		stream >> meshPath;
		SkeletalMesh* skeletalMesh = engine->GetResourceManager()->GetContent<SkeletalMesh>(ContentPathUtils::ToContentRelativePath(meshPath));
		if (skeletalMesh)
		{
			skeletalMeshComponent->SetMesh(skeletalMesh);
		}
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("RenderMask");
	if (dataElement && skeletalMeshComponent->GetMeshInstance())
	{
		stream << dataElement->GetText() << std::endl;
		std::string renderMaskString;
		stream >> renderMaskString;
		int renderMask = std::stoi(renderMaskString);
		skeletalMeshComponent->GetMeshInstance()->SetRenderMask(renderMask);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("MaterialPath");
	if (componentElement->FirstChildElement("MaterialPaths") || dataElement)
	{
		ApplySkeletalMeshComponentMaterialPaths(skeletalMeshComponent, ReadMaterialPaths(componentElement));
	}
	else
	{
		ClearSkeletalMeshComponentMaterialPath(skeletalMeshComponent);
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

		instancedStaticMesh->SetInstanceTransformations(instanceTransformations, false);
		instancedStaticMesh->RecalculateAABB();
		instancedStaticMesh->UpdateAllTransforms();
	}
}

void SceneParser::ParseGPUFoliageComponentValues(GPUFoliageComponent* gpuFoliageComponent, tinyxml2::XMLElement* componentElement)
{
	if (!gpuFoliageComponent || !componentElement)
	{
		return;
	}

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("MeshPath");
	if (dataElement && dataElement->GetText())
	{
		std::stringstream stream(dataElement->GetText());
		std::string meshPath;
		stream >> meshPath;
		if (StaticMesh* staticMesh = ResolveStaticMeshFromPath(meshPath))
		{
			gpuFoliageComponent->SetStaticMesh(staticMesh);
		}
	}

	dataElement = componentElement->FirstChildElement("CastsShadow");
	if (dataElement)
	{
		bool castsShadow = true;
		if (dataElement->QueryBoolText(&castsShadow) == tinyxml2::XML_SUCCESS)
		{
			gpuFoliageComponent->SetCastsShadow(castsShadow);
		}
	}

	tinyxml2::XMLElement* instanceTransformationsElement = componentElement->FirstChildElement("InstanceTransformations");
	if (!instanceTransformationsElement)
	{
		return;
	}

	std::vector<GPUFoliageInstance> instances;
	for (tinyxml2::XMLElement* instanceTransformationElement = instanceTransformationsElement->FirstChildElement("InstanceTransformation");
		instanceTransformationElement != nullptr;
		instanceTransformationElement = instanceTransformationElement->NextSiblingElement("InstanceTransformation"))
	{
		Vector3 translation = Vector3::ZeroVector;
		Quaternion rotation = Quaternion::Identity;
		Vector3 scaling = Vector3(1.f);

		ReadVector3Element(instanceTransformationElement, "Translation", translation);

		Vector3 eulerRotation = Vector3::ZeroVector;
		if (ReadVector3Element(instanceTransformationElement, "EulerRotation", eulerRotation))
		{
			rotation = Quaternion::FromEulerDegrees(eulerRotation);
		}

		ReadVector3Element(instanceTransformationElement, "Scaling", scaling);

		GPUFoliageInstance instance;
		instance.transform = Matrix::GetTransformationMatrix(rotation, translation, scaling);

		tinyxml2::XMLElement* colorElement = instanceTransformationElement->FirstChildElement("Color");
		if (colorElement && colorElement->GetText())
		{
			std::stringstream stream(colorElement->GetText());
			stream >> instance.color.x >> instance.color.y >> instance.color.z >> instance.color.w;
		}

		instances.push_back(instance);
	}

	gpuFoliageComponent->SetInstances(instances);
}

void SceneParser::ParseParticleSystemComponentValues(ParticleSystemComponent* particleSystemComponent, tinyxml2::XMLElement* componentElement)
{
	std::stringstream stream;
	GPUParticleSpawnDesc spawnDesc = particleSystemComponent->GetSpawnDesc();

	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("RenderMode");

	dataElement = componentElement->FirstChildElement("MaxParticleCount");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		std::uint32_t maxParticleCount = 0u;
		stream >> maxParticleCount;
		particleSystemComponent->SetMaxParticleCount(maxParticleCount);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("Gravity");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		Vector3 gravity;
		stream >> gravity.x >> gravity.y >> gravity.z;
		particleSystemComponent->SetGravity(gravity);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("ParticleSize");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float particleSize = 0.f;
		stream >> particleSize;
		particleSystemComponent->SetParticleSize(particleSize);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("PreviewParticleCount");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		std::uint32_t previewParticleCount = 0u;
		stream >> previewParticleCount;
		particleSystemComponent->SetPreviewParticleCount(previewParticleCount);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("Looping");
	if (dataElement && dataElement->GetText())
	{
		spawnDesc.looping = std::string(dataElement->GetText()) == "1" || std::string(dataElement->GetText()) == "true";
	}

	dataElement = componentElement->FirstChildElement("InfiniteLifetime");
	if (dataElement && dataElement->GetText())
	{
		spawnDesc.infiniteLifetime = std::string(dataElement->GetText()) == "1" || std::string(dataElement->GetText()) == "true";
	}

	dataElement = componentElement->FirstChildElement("SpawnInterval");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.spawnInterval;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("SpawnCountPerInterval");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.spawnCountPerInterval;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("SpawnBoxExtents");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.spawnBoxExtents.x >> spawnDesc.spawnBoxExtents.y >> spawnDesc.spawnBoxExtents.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("LifetimeRange");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.lifetime.minValue >> spawnDesc.lifetime.maxValue;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("InitialSizeRange");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.initialSize.minValue >> spawnDesc.initialSize.maxValue;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("InitialVelocityMin");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.initialVelocity.minValue.x >> spawnDesc.initialVelocity.minValue.y >> spawnDesc.initialVelocity.minValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("InitialVelocityMax");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.initialVelocity.maxValue.x >> spawnDesc.initialVelocity.maxValue.y >> spawnDesc.initialVelocity.maxValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("InitialRotationMin");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.initialRotation.minValue.x >> spawnDesc.initialRotation.minValue.y >> spawnDesc.initialRotation.minValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("InitialRotationMax");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.initialRotation.maxValue.x >> spawnDesc.initialRotation.maxValue.y >> spawnDesc.initialRotation.maxValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("AngularVelocityMin");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.angularVelocity.minValue.x >> spawnDesc.angularVelocity.minValue.y >> spawnDesc.angularVelocity.minValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("AngularVelocityMax");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.angularVelocity.maxValue.x >> spawnDesc.angularVelocity.maxValue.y >> spawnDesc.angularVelocity.maxValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("AccelerationMin");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.acceleration.minValue.x >> spawnDesc.acceleration.minValue.y >> spawnDesc.acceleration.minValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("AccelerationMax");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.acceleration.maxValue.x >> spawnDesc.acceleration.maxValue.y >> spawnDesc.acceleration.maxValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("VelocityLimit");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.velocityLimit;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("SizeByLifetime");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.sizeByLifetime.startValue >> spawnDesc.sizeByLifetime.endValue;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("SizeBySpeedRange");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.sizeBySpeedRange.x >> spawnDesc.sizeBySpeedRange.y;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("SizeBySpeed");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.sizeBySpeed.startValue >> spawnDesc.sizeBySpeed.endValue;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("ColorByLifetimeStart");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.colorByLifetime.startValue.x >> spawnDesc.colorByLifetime.startValue.y >> spawnDesc.colorByLifetime.startValue.z >> spawnDesc.colorByLifetime.startValue.w;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("ColorByLifetimeEnd");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.colorByLifetime.endValue.x >> spawnDesc.colorByLifetime.endValue.y >> spawnDesc.colorByLifetime.endValue.z >> spawnDesc.colorByLifetime.endValue.w;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("ColorBySpeedRange");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.colorBySpeedRange.x >> spawnDesc.colorBySpeedRange.y;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("ColorBySpeedStart");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.colorBySpeed.startValue.x >> spawnDesc.colorBySpeed.startValue.y >> spawnDesc.colorBySpeed.startValue.z >> spawnDesc.colorBySpeed.startValue.w;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("ColorBySpeedEnd");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.colorBySpeed.endValue.x >> spawnDesc.colorBySpeed.endValue.y >> spawnDesc.colorBySpeed.endValue.z >> spawnDesc.colorBySpeed.endValue.w;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("EmissiveColorStart");
	if (!dataElement)
	{
		dataElement = componentElement->FirstChildElement("EmissiveColorByLifetimeStart");
	}
	if (!dataElement)
	{
		dataElement = componentElement->FirstChildElement("EmmisiveColorStart");
	}
	if (!dataElement)
	{
		dataElement = componentElement->FirstChildElement("EmmisiveColorByLifetimeStart");
	}
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.emissiveColorByLifetime.startValue.x >> spawnDesc.emissiveColorByLifetime.startValue.y >> spawnDesc.emissiveColorByLifetime.startValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("EmissiveColorEnd");
	if (!dataElement)
	{
		dataElement = componentElement->FirstChildElement("EmissiveColorByLifetimeEnd");
	}
	if (!dataElement)
	{
		dataElement = componentElement->FirstChildElement("EmmisiveColorEnd");
	}
	if (!dataElement)
	{
		dataElement = componentElement->FirstChildElement("EmmisiveColorByLifetimeEnd");
	}
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.emissiveColorByLifetime.endValue.x >> spawnDesc.emissiveColorByLifetime.endValue.y >> spawnDesc.emissiveColorByLifetime.endValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("StartSpeedRange");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.initialVelocity.minValue.z >> spawnDesc.initialVelocity.maxValue.z;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("StartSizeRange");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.sizeByLifetime.startValue;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("EndSizeRange");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.sizeByLifetime.endValue;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("StartColorMin");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.colorByLifetime.startValue.x >> spawnDesc.colorByLifetime.startValue.y >> spawnDesc.colorByLifetime.startValue.z >> spawnDesc.colorByLifetime.startValue.w;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("EndColorMin");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		stream >> spawnDesc.colorByLifetime.endValue.x >> spawnDesc.colorByLifetime.endValue.y >> spawnDesc.colorByLifetime.endValue.z >> spawnDesc.colorByLifetime.endValue.w;
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("PreviewLifetime");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float previewLifetime = 0.f;
		stream >> previewLifetime;
		spawnDesc.lifetime = GPUParticleValueRange<float>(previewLifetime, previewLifetime);
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("PreviewInitialSpeed");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float previewInitialSpeed = 0.f;
		stream >> previewInitialSpeed;
		spawnDesc.initialVelocity = GPUParticleValueRange<Vector3>(
			Vector3(0.f, 0.f, previewInitialSpeed),
			Vector3(0.f, 0.f, previewInitialSpeed));
	}
	stream.clear();

	dataElement = componentElement->FirstChildElement("PreviewSpawnRadius");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float previewSpawnRadius = 0.f;
		stream >> previewSpawnRadius;
		spawnDesc.spawnBoxExtents = Vector3(previewSpawnRadius, previewSpawnRadius, previewSpawnRadius);
	}
	stream.clear();

	particleSystemComponent->SetSpawnDesc(spawnDesc);

	dataElement = componentElement->FirstChildElement("StaticMeshPath");
	if (StaticMeshParticleSystemComponent* staticMeshParticleSystemComponent = dynamic_cast<StaticMeshParticleSystemComponent*>(particleSystemComponent))
	{
		staticMeshParticleSystemComponent->SetStaticMeshPath(dataElement && dataElement->GetText() ? dataElement->GetText() : "");
	}

	dataElement = componentElement->FirstChildElement("BillboardTexturePath");
	if (BillboardParticleSystemComponent* billboardParticleSystemComponent = dynamic_cast<BillboardParticleSystemComponent*>(particleSystemComponent))
	{
		billboardParticleSystemComponent->SetBillboardTexturePath(dataElement && dataElement->GetText() ? dataElement->GetText() : "");
	}

	dataElement = componentElement->FirstChildElement("BillboardMaterialPath");
	if (BillboardParticleSystemComponent* billboardParticleSystemComponent = dynamic_cast<BillboardParticleSystemComponent*>(particleSystemComponent))
	{
		billboardParticleSystemComponent->SetBillboardMaterialPath(dataElement && dataElement->GetText() ? dataElement->GetText() : "");
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

void SceneParser::ParseHeightMapCollisionComponentValues(HeightMapCollisionComponent* heightMapCollisionComponent, tinyxml2::XMLElement* componentElement)
{
	if (!heightMapCollisionComponent || !componentElement)
	{
		return;
	}

	std::string heightMapImagePath;
	if (ReadStringElement(componentElement, "HeightMapImage", heightMapImagePath) ||
		ReadStringElement(componentElement, "HeightMapImagePath", heightMapImagePath))
	{
		const std::string relativeHeightMapImagePath = ContentPathUtils::ToContentRelativePath(heightMapImagePath);
		if (!relativeHeightMapImagePath.empty())
		{
			Image* heightMapImage = engine->GetResourceManager()->GetContent<Image>(relativeHeightMapImagePath);
			if (heightMapImage)
			{
				heightMapCollisionComponent->SetHeightMapImage(heightMapImage);
			}
			else
			{
				GOKNAR_CORE_WARN("Height map image could not be loaded from %s.", relativeHeightMapImagePath.c_str());
			}
		}
	}

	std::stringstream stream;
	tinyxml2::XMLElement* dataElement = componentElement->FirstChildElement("HeightStickWidth");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		int heightStickWidth;
		stream >> heightStickWidth;
		heightMapCollisionComponent->SetHeightStickWidth(heightStickWidth);
	}
	stream.clear();
	stream.str("");

	dataElement = componentElement->FirstChildElement("HeightStickLength");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		int heightStickLength;
		stream >> heightStickLength;
		heightMapCollisionComponent->SetHeightStickLength(heightStickLength);
	}
	stream.clear();
	stream.str("");

	dataElement = componentElement->FirstChildElement("MinHeight");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float minHeight;
		stream >> minHeight;
		heightMapCollisionComponent->SetMinHeight(minHeight);
	}
	stream.clear();
	stream.str("");

	dataElement = componentElement->FirstChildElement("MaxHeight");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float maxHeight;
		stream >> maxHeight;
		heightMapCollisionComponent->SetMaxHeight(maxHeight);
	}
	stream.clear();
	stream.str("");

	dataElement = componentElement->FirstChildElement("Width");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float width;
		stream >> width;
		heightMapCollisionComponent->SetWidth(width);
	}
	stream.clear();
	stream.str("");

	dataElement = componentElement->FirstChildElement("Length");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float length;
		stream >> length;
		heightMapCollisionComponent->SetLength(length);
	}
	stream.clear();
	stream.str("");

	dataElement = componentElement->FirstChildElement("HeightScale");
	if (dataElement && dataElement->GetText())
	{
		stream << dataElement->GetText() << std::endl;
		float heightScale;
		stream >> heightScale;
		heightMapCollisionComponent->SetHeightScale(heightScale);
	}
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

void SceneParser::ParseNavigationTreeComponentValues(NavigationTreeComponent* navigationTreeComponent, tinyxml2::XMLElement* componentElement)
{
	if (!navigationTreeComponent)
	{
		return;
	}

	std::string navigationTreePath;
	if (ReadNavigationTreePath(componentElement, navigationTreePath))
	{
		navigationTreeComponent->SetNavigationTreePath(navigationTreePath);
	}
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

		GOKNAR_CORE_CHECK(relativeMesh);

		if (relativeMesh)
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

	Vector3 objectPosition = object->GetWorldPosition();
	if (ReadVector3Element(objectElement, "WorldPosition", objectPosition))
	{
		object->SetWorldPosition(objectPosition);
	}

	Vector3 objectRotation = object->GetWorldRotation().ToEulerDegrees();
	if (ReadVector3Element(objectElement, "WorldRotation", objectRotation))
	{
		object->SetWorldRotation(Quaternion::FromEulerDegrees(objectRotation));
	}

	Vector3 objectScaling = object->GetWorldScaling();
	if (ReadVector3Element(objectElement, "WorldScaling", objectScaling))
	{
		object->SetWorldScaling(objectScaling);
	}

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

		componentElement = child->FirstChildElement("SkeletalMeshComponent");
		while (componentElement)
		{
			SkeletalMeshComponent* skeletalMeshComponent = object->AddSubComponent<SkeletalMeshComponent>();
			ParseSkeletalMeshComponentValues(skeletalMeshComponent, componentElement);

			ParseComponentValues(skeletalMeshComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("SkeletalMeshComponent");
		}

		componentElement = child->FirstChildElement("InstancedStaticMeshComponent");
		while (componentElement)
		{
			InstancedStaticMeshComponent* instancedStaticMeshComponent = object->AddSubComponent<InstancedStaticMeshComponent>();
			ParseInstancedStaticMeshComponentValues(instancedStaticMeshComponent, componentElement);

			ParseComponentValues(instancedStaticMeshComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("InstancedStaticMeshComponent");
		}

		componentElement = child->FirstChildElement("GPUFoliageComponent");
		while (componentElement)
		{
			GPUFoliageComponent* gpuFoliageComponent = object->AddSubComponent<GPUFoliageComponent>();
			ParseGPUFoliageComponentValues(gpuFoliageComponent, componentElement);

			ParseComponentValues(gpuFoliageComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("GPUFoliageComponent");
		}

		componentElement = child->FirstChildElement("BillboardParticleSystemComponent");
		while (componentElement)
		{
			BillboardParticleSystemComponent* particleSystemComponent = object->AddSubComponent<BillboardParticleSystemComponent>();
			ParseParticleSystemComponentValues(particleSystemComponent, componentElement);

			ParseComponentValues(particleSystemComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("BillboardParticleSystemComponent");
		}

		componentElement = child->FirstChildElement("StaticMeshParticleSystemComponent");
		while (componentElement)
		{
			StaticMeshParticleSystemComponent* particleSystemComponent = object->AddSubComponent<StaticMeshParticleSystemComponent>();
			ParseParticleSystemComponentValues(particleSystemComponent, componentElement);

			ParseComponentValues(particleSystemComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("StaticMeshParticleSystemComponent");
		}

		componentElement = child->FirstChildElement("ParticleSystemComponent");
		while (componentElement)
		{
			const tinyxml2::XMLElement* renderModeElement = componentElement->FirstChildElement("RenderMode");
			const tinyxml2::XMLElement* staticMeshPathElement = componentElement->FirstChildElement("StaticMeshPath");
			const bool shouldCreateStaticMeshParticleSystem =
				(renderModeElement && renderModeElement->GetText() && std::string(renderModeElement->GetText()) == "StaticMesh") ||
				(staticMeshPathElement && staticMeshPathElement->GetText() && staticMeshPathElement->GetText()[0] != '\0');

			ParticleSystemComponent* particleSystemComponent = shouldCreateStaticMeshParticleSystem ?
				static_cast<ParticleSystemComponent*>(object->AddSubComponent<StaticMeshParticleSystemComponent>()) :
				static_cast<ParticleSystemComponent*>(object->AddSubComponent<BillboardParticleSystemComponent>());
			ParseParticleSystemComponentValues(particleSystemComponent, componentElement);

			ParseComponentValues(particleSystemComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("ParticleSystemComponent");
		}

		componentElement = child->FirstChildElement("NavigationTreeComponent");
		bool reusedNavigationTreeComponent = false;
		while (componentElement)
		{
			NavigationTreeComponent* navigationTreeComponent = nullptr;
			if (!reusedNavigationTreeComponent)
			{
				navigationTreeComponent = object->GetFirstComponentOfType<NavigationTreeComponent>();
				reusedNavigationTreeComponent = navigationTreeComponent != nullptr;
			}

			if (!navigationTreeComponent)
			{
				navigationTreeComponent = object->AddSubComponent<NavigationTreeComponent>();
			}

			ParseNavigationTreeComponentValues(navigationTreeComponent, componentElement);

			ParseComponentValues(navigationTreeComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("NavigationTreeComponent");
		}
	}
}

void SceneParser::ParsePhysicsObject(PhysicsObject* physicsObject, tinyxml2::XMLElement* objectElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* child;

	if (RigidBody* rigidBody = dynamic_cast<RigidBody*>(physicsObject))
	{
		child = objectElement->FirstChildElement("Mass");

		if (child)
		{
			stream << child->GetText() << std::endl;
			float mass;
			stream >> mass;
			rigidBody->SetMass(mass);
		}
		stream.clear();
	}

	child = objectElement->FirstChildElement("CollisionGroup");
	if (child)
	{
		stream << child->GetText() << std::endl;
		int collisionGroupInt;
		stream >> collisionGroupInt;
		physicsObject->SetCollisionGroup((CollisionGroup)collisionGroupInt);
	}
	stream.clear();

	child = objectElement->FirstChildElement("CollisionMask");
	if (child)
	{
		stream << child->GetText() << std::endl;
		int collisionMaskInt;
		stream >> collisionMaskInt;
		physicsObject->SetCollisionMask((CollisionMask)collisionMaskInt);
	}
	stream.clear();

	child = objectElement->FirstChildElement("Components");
	if (child)
	{
		tinyxml2::XMLElement* componentElement = child->FirstChildElement("BoxCollisionComponent");
		while (componentElement)
		{
			BoxCollisionComponent* boxCollisionComponent = physicsObject->AddSubComponent<BoxCollisionComponent>();
			ParseBoxCollisionComponentValues(boxCollisionComponent, componentElement);

			ParseComponentValues(boxCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("BoxCollisionComponent");
		}

		componentElement = child->FirstChildElement("SphereCollisionComponent");
		while (componentElement)
		{
			SphereCollisionComponent* sphereCollisionComponent = physicsObject->AddSubComponent<SphereCollisionComponent>();
			ParseSphereCollisionComponentValues(sphereCollisionComponent, componentElement);

			ParseComponentValues(sphereCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("SphereCollisionComponent");
		}

		componentElement = child->FirstChildElement("CapsuleCollisionComponent");
		while (componentElement)
		{
			CapsuleCollisionComponent* capsuleCollisionComponent = physicsObject->AddSubComponent<CapsuleCollisionComponent>();
			ParseCapsuleCollisionComponentValues(capsuleCollisionComponent, componentElement);

			ParseComponentValues(capsuleCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("CapsuleCollisionComponent");
		}

		componentElement = child->FirstChildElement("HeightMapCollisionComponent");
		while (componentElement)
		{
			HeightMapCollisionComponent* heightMapCollisionComponent = physicsObject->AddSubComponent<HeightMapCollisionComponent>();
			ParseHeightMapCollisionComponentValues(heightMapCollisionComponent, componentElement);

			ParseComponentValues(heightMapCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("HeightMapCollisionComponent");
		}

		componentElement = child->FirstChildElement("MovingTriangleMeshCollisionComponent");
		while (componentElement)
		{
			MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent = physicsObject->AddSubComponent<MovingTriangleMeshCollisionComponent>();
			ParseMovingTriangleMeshCollisionComponentValues(movingTriangleMeshCollisionComponent, componentElement);

			ParseComponentValues(movingTriangleMeshCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("MovingTriangleMeshCollisionComponent");
		}

		componentElement = child->FirstChildElement("NonMovingTriangleMeshCollisionComponent");
		while (componentElement)
		{
			NonMovingTriangleMeshCollisionComponent* nonMovingTriangleMeshCollisionComponent = physicsObject->AddSubComponent<NonMovingTriangleMeshCollisionComponent>();
			ParseNonMovingTriangleMeshCollisionComponentValues(nonMovingTriangleMeshCollisionComponent, componentElement);

			ParseComponentValues(nonMovingTriangleMeshCollisionComponent, componentElement);

			componentElement = componentElement->NextSiblingElement("NonMovingTriangleMeshCollisionComponent");
		}
	}
}

void SceneParser::ParseReflectionProbeObject(ReflectionProbeObject* reflectionProbeObject, tinyxml2::XMLElement* objectElement)
{
	std::stringstream stream;

	tinyxml2::XMLElement* child = objectElement->FirstChildElement("Size");
	if (child && child->GetText())
	{
		stream << child->GetText() << std::endl;
		Vector3 size;
		stream >> size.x >> size.y >> size.z;
		reflectionProbeObject->SetSize(size);
	}

	child = objectElement->FirstChildElement("CaptureDistance");
	if (child && child->GetText())
	{
		std::stringstream captureDistanceStream(child->GetText());
		float captureDistance = 0.f;
		captureDistanceStream >> captureDistance;
		reflectionProbeObject->SetCaptureDistance(captureDistance);
	}
}

void SceneParser::GetXMLElement_DirectionalLights(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene)
{
	for (auto directionalLight : scene->GetDirectionalLights())
	{
		if (directionalLight->GetName().find("__Editor__") != std::string::npos ||
			scene->GetIsDirectionalLightFromReferencedScene(directionalLight))
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
		if (spotLight->GetName().find("__Editor__") != std::string::npos ||
			scene->GetIsSpotLightFromReferencedScene(spotLight))
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
		spotLightCoverageAngleElement->SetText(RADIAN_TO_DEGREE(spotLight->GetCoverageAngle()));
		spotLightElement->InsertEndChild(spotLightCoverageAngleElement);

		tinyxml2::XMLElement* spotLightFalloffAngleElement = xmlDocument.NewElement("FalloffAngle");
		spotLightFalloffAngleElement->SetText(RADIAN_TO_DEGREE(spotLight->GetFalloffAngle()));
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
		if (pointLight->GetName().find("__Editor__") != std::string::npos ||
			scene->GetIsPointLightFromReferencedScene(pointLight))
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

void SceneParser::GetXMLElement_SceneReferences(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene)
{
	if (!scene)
	{
		return;
	}

	for (const SceneReference& sceneReference : scene->GetSceneReferences())
	{
		if (sceneReference.path.empty())
		{
			continue;
		}

		tinyxml2::XMLElement* sceneElement = xmlDocument.NewElement("Scene");
		sceneElement->SetAttribute("Path", sceneReference.path.c_str());

		Vector3 worldPosition = sceneReference.position;
		Quaternion worldRotation = sceneReference.rotation;
		Vector3 worldScaling = sceneReference.scaling;
		if (sceneReference.sceneRootObject)
		{
			worldPosition = sceneReference.sceneRootObject->GetWorldPosition();
			worldRotation = sceneReference.sceneRootObject->GetWorldRotation();
			worldScaling = sceneReference.sceneRootObject->GetWorldScaling();
		}

		WriteTransformElementsIfNeeded(
			xmlDocument,
			sceneElement,
			worldPosition,
			worldRotation,
			worldScaling,
			"WorldPosition",
			"WorldRotation",
			"WorldScaling");

		parentElement->InsertEndChild(sceneElement);
	}
}

void SceneParser::GetXMLElement_Objects(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement, Scene* scene)
{
	const std::vector<ObjectBase*>& sceneObjects = scene ? scene->GetObjects() : engine->GetRegisteredObjects();

	const auto shouldWriteObject =
		[scene](ObjectBase* object) -> bool
		{
			if (!object || (scene && scene->GetIsObjectFromReferencedScene(object)))
			{
				return false;
			}

			if (object->GetName().find("__Editor__") != std::string::npos)
			{
				return false;
			}

			return !dynamic_cast<DebugObject*>(object);
		};

	const auto writeObjectElement =
		[&](auto&& writeObjectElement, ObjectBase* object) -> tinyxml2::XMLElement*
		{
			RigidBody* rigidBody = dynamic_cast<RigidBody*>(object);
			ReflectionProbeObject* reflectionProbeObject = dynamic_cast<ReflectionProbeObject*>(object);
			PlayerStart* playerStartObject = dynamic_cast<PlayerStart*>(object);

			std::string objectTypeString = "ObjectBase";
			if (rigidBody)
			{
				objectTypeString = "RigidBody";
			}
			else if (reflectionProbeObject)
			{
				objectTypeString = "ReflectionProbeObject";
			}
			else if (reflectionProbeObject)
			{
				objectTypeString = "ReflectionProbeObject";
			}
			else if (playerStartObject)
			{
				objectTypeString = "PlayerStart";
			}

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
				"WorldRotation",
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

			if (reflectionProbeObject)
			{
				tinyxml2::XMLElement* reflectionProbeSizeElement = xmlDocument.NewElement("Size");
				reflectionProbeSizeElement->SetText(Serialize(reflectionProbeObject->GetSize()).c_str());
				objectElement->InsertEndChild(reflectionProbeSizeElement);

				tinyxml2::XMLElement* reflectionProbeCaptureDistanceElement = xmlDocument.NewElement("CaptureDistance");
				reflectionProbeCaptureDistanceElement->SetText(reflectionProbeObject->GetCaptureDistance());
				objectElement->InsertEndChild(reflectionProbeCaptureDistanceElement);
			}

			tinyxml2::XMLElement* componentsElement = xmlDocument.NewElement("Components");
			GetXMLElement_Components(object, xmlDocument, componentsElement);
			objectElement->InsertEndChild(componentsElement);

			tinyxml2::XMLElement* childrenElement = nullptr;
			for (ObjectBase* childObject : object->GetChildren())
			{
				if (!shouldWriteObject(childObject))
				{
					continue;
				}

				if (!childrenElement)
				{
					childrenElement = xmlDocument.NewElement("Children");
					objectElement->InsertEndChild(childrenElement);
				}

				childrenElement->InsertEndChild(writeObjectElement(writeObjectElement, childObject));
			}

			return objectElement;
		};

	for (ObjectBase* object : sceneObjects)
	{
		if (!shouldWriteObject(object) || object->GetParent())
		{
			continue;
		}

		parentElement->InsertEndChild(writeObjectElement(writeObjectElement, object));
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
		else if (SkeletalMeshComponent* skeletalMeshComponent = dynamic_cast<SkeletalMeshComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("SkeletalMeshComponent");
			GetXMLElement_SkeletalMeshComponent(skeletalMeshComponent, xmlDocument, componentElement);
		}
		else if (GPUFoliageComponent* gpuFoliageComponent = dynamic_cast<GPUFoliageComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("GPUFoliageComponent");
			GetXMLElement_GPUFoliageComponent(gpuFoliageComponent, xmlDocument, componentElement);
		}
		else if (InstancedStaticMeshComponent* instancedStaticMeshComponent = dynamic_cast<InstancedStaticMeshComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("InstancedStaticMeshComponent");
			GetXMLElement_InstancedStaticMeshComponent(instancedStaticMeshComponent, xmlDocument, componentElement);
		}
		else if (BillboardParticleSystemComponent* particleSystemComponent = dynamic_cast<BillboardParticleSystemComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("BillboardParticleSystemComponent");
			GetXMLElement_ParticleSystemComponent(particleSystemComponent, xmlDocument, componentElement);
		}
		else if (StaticMeshParticleSystemComponent* particleSystemComponent = dynamic_cast<StaticMeshParticleSystemComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("StaticMeshParticleSystemComponent");
			GetXMLElement_ParticleSystemComponent(particleSystemComponent, xmlDocument, componentElement);
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
		else if (HeightMapCollisionComponent* heightMapCollisionComponent = dynamic_cast<HeightMapCollisionComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("HeightMapCollisionComponent");
			GetXMLElement_HeightMapCollisionComponent(heightMapCollisionComponent, xmlDocument, componentElement);
		}
		else if (MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent = dynamic_cast<MovingTriangleMeshCollisionComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("MovingTriangleMeshCollisionComponent");
			GetXMLElement_MovingTriangleMeshCollisionComponent(movingTriangleMeshCollisionComponent, xmlDocument, componentElement);
		}
		else if (NavigationTreeComponent* navigationTreeComponent = dynamic_cast<NavigationTreeComponent*>(component))
		{
			componentElement = xmlDocument.NewElement("NavigationTreeComponent");
			GetXMLElement_NavigationTreeComponent(navigationTreeComponent, xmlDocument, componentElement);
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
			"RelativeRotation",
			"RelativeScaling");

		parentElement->InsertEndChild(componentElement);
	}
}

void SceneParser::GetXMLElement_StaticMeshComponent(const StaticMeshComponent* const staticMeshComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	StaticMesh* staticMesh = staticMeshComponent->GetMeshInstance()->GetMesh();
	GOKNAR_CHECK(staticMesh);

	if (!staticMesh || staticMesh->GetPath().empty())
	{
		return;
	}

	tinyxml2::XMLElement* staticMeshComponentMeshPathElement = xmlDocument.NewElement("MeshPath");
	const std::string meshPath = ContentPathUtils::ToContentRelativePath(staticMesh->GetPath());
	staticMeshComponentMeshPathElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(staticMeshComponentMeshPathElement);

	tinyxml2::XMLElement* staticMeshInstanceRenderMaskElement = xmlDocument.NewElement("RenderMask");
	staticMeshInstanceRenderMaskElement->SetText(staticMeshComponent->GetMeshInstance()->GetRenderMask());
	parentElement->InsertEndChild(staticMeshInstanceRenderMaskElement);

	WriteMaterialPaths(xmlDocument, parentElement, GetStaticMeshComponentMaterialPaths(staticMeshComponent));
}

void SceneParser::GetXMLElement_SkeletalMeshComponent(const SkeletalMeshComponent* const skeletalMeshComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const SkeletalMeshInstance* meshInstance = skeletalMeshComponent ? skeletalMeshComponent->GetMeshInstance() : nullptr;
	const SkeletalMesh* skeletalMesh = meshInstance ? meshInstance->GetMesh() : nullptr;
	if (!meshInstance || !skeletalMesh)
	{
		return;
	}

	tinyxml2::XMLElement* skeletalMeshComponentMeshPathElement = xmlDocument.NewElement("MeshPath");
	const std::string meshPath = ContentPathUtils::ToContentRelativePath(skeletalMesh->GetPath());
	skeletalMeshComponentMeshPathElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(skeletalMeshComponentMeshPathElement);

	tinyxml2::XMLElement* skeletalMeshInstanceRenderMaskElement = xmlDocument.NewElement("RenderMask");
	skeletalMeshInstanceRenderMaskElement->SetText(meshInstance->GetRenderMask());
	parentElement->InsertEndChild(skeletalMeshInstanceRenderMaskElement);

	WriteMaterialPaths(xmlDocument, parentElement, GetSkeletalMeshComponentMaterialPaths(skeletalMeshComponent));
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

void SceneParser::GetXMLElement_GPUFoliageComponent(const GPUFoliageComponent* const gpuFoliageComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const StaticMesh* staticMesh = gpuFoliageComponent ? gpuFoliageComponent->GetStaticMesh() : nullptr;
	if (!staticMesh || staticMesh->GetPath().empty())
	{
		return;
	}

	tinyxml2::XMLElement* meshPathElement = xmlDocument.NewElement("MeshPath");
	const std::string meshPath = ContentPathUtils::ToContentRelativePath(staticMesh->GetPath());
	meshPathElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(meshPathElement);

	tinyxml2::XMLElement* castsShadowElement = xmlDocument.NewElement("CastsShadow");
	castsShadowElement->SetText(gpuFoliageComponent->GetCastsShadow());
	parentElement->InsertEndChild(castsShadowElement);

	const std::vector<GPUFoliageInstance>& instances = gpuFoliageComponent->GetInstances();
	if (instances.empty())
	{
		return;
	}

	tinyxml2::XMLElement* instanceTransformationsElement = xmlDocument.NewElement("InstanceTransformations");
	for (const GPUFoliageInstance& instance : instances)
	{
		Vector3 translation = Vector3::ZeroVector;
		Vector3 scaling = Vector3(1.f);
		Quaternion rotation = Quaternion::Identity;
		instance.transform.Decompose(translation, scaling, rotation);

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

		if (instance.color != Vector4(1.f))
		{
			tinyxml2::XMLElement* colorElement = xmlDocument.NewElement("Color");
			colorElement->SetText(SerializeVector4(instance.color).c_str());
			instanceTransformationElement->InsertEndChild(colorElement);
		}

		instanceTransformationsElement->InsertEndChild(instanceTransformationElement);
	}

	parentElement->InsertEndChild(instanceTransformationsElement);
}

void SceneParser::GetXMLElement_ParticleSystemComponent(const ParticleSystemComponent* const particleSystemComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const GPUParticleSpawnDesc& spawnDesc = particleSystemComponent->GetSpawnDesc();
	const auto serializeFloatRange =
		[](const GPUParticleValueRange<float>& range) -> std::string
		{
			return std::to_string(range.minValue) + " " + std::to_string(range.maxValue);
		};
	const auto serializeFloatCurve =
		[](const GPUParticleFloatCurve& curve) -> std::string
		{
			return std::to_string(curve.startValue) + " " + std::to_string(curve.endValue);
		};
	const auto serializeVector2 =
		[](const Vector2& value) -> std::string
		{
			return std::to_string(value.x) + " " + std::to_string(value.y);
		};
	const auto serializeVector4 =
		[](const Vector4& value) -> std::string
		{
			return
				std::to_string(value.x) + " " +
				std::to_string(value.y) + " " +
				std::to_string(value.z) + " " +
				std::to_string(value.w);
		};

	tinyxml2::XMLElement* maxParticleCountElement = xmlDocument.NewElement("MaxParticleCount");
	maxParticleCountElement->SetText(particleSystemComponent->GetMaxParticleCount());
	parentElement->InsertEndChild(maxParticleCountElement);

	tinyxml2::XMLElement* gravityElement = xmlDocument.NewElement("Gravity");
	gravityElement->SetText(Serialize(particleSystemComponent->GetGravity()).c_str());
	parentElement->InsertEndChild(gravityElement);

	tinyxml2::XMLElement* particleSizeElement = xmlDocument.NewElement("ParticleSize");
	particleSizeElement->SetText(particleSystemComponent->GetParticleSize());
	parentElement->InsertEndChild(particleSizeElement);

	tinyxml2::XMLElement* previewParticleCountElement = xmlDocument.NewElement("PreviewParticleCount");
	previewParticleCountElement->SetText(particleSystemComponent->GetPreviewParticleCount());
	parentElement->InsertEndChild(previewParticleCountElement);

	tinyxml2::XMLElement* loopingElement = xmlDocument.NewElement("Looping");
	loopingElement->SetText(spawnDesc.looping ? 1 : 0);
	parentElement->InsertEndChild(loopingElement);

	tinyxml2::XMLElement* infiniteLifetimeElement = xmlDocument.NewElement("InfiniteLifetime");
	infiniteLifetimeElement->SetText(spawnDesc.infiniteLifetime ? 1 : 0);
	parentElement->InsertEndChild(infiniteLifetimeElement);

	tinyxml2::XMLElement* spawnIntervalElement = xmlDocument.NewElement("SpawnInterval");
	spawnIntervalElement->SetText(spawnDesc.spawnInterval);
	parentElement->InsertEndChild(spawnIntervalElement);

	tinyxml2::XMLElement* spawnCountPerIntervalElement = xmlDocument.NewElement("SpawnCountPerInterval");
	spawnCountPerIntervalElement->SetText(spawnDesc.spawnCountPerInterval);
	parentElement->InsertEndChild(spawnCountPerIntervalElement);

	tinyxml2::XMLElement* spawnBoxExtentsElement = xmlDocument.NewElement("SpawnBoxExtents");
	spawnBoxExtentsElement->SetText(Serialize(spawnDesc.spawnBoxExtents).c_str());
	parentElement->InsertEndChild(spawnBoxExtentsElement);

	tinyxml2::XMLElement* lifetimeRangeElement = xmlDocument.NewElement("LifetimeRange");
	lifetimeRangeElement->SetText(serializeFloatRange(spawnDesc.lifetime).c_str());
	parentElement->InsertEndChild(lifetimeRangeElement);

	tinyxml2::XMLElement* initialSizeRangeElement = xmlDocument.NewElement("InitialSizeRange");
	initialSizeRangeElement->SetText(serializeFloatRange(spawnDesc.initialSize).c_str());
	parentElement->InsertEndChild(initialSizeRangeElement);

	tinyxml2::XMLElement* initialVelocityMinElement = xmlDocument.NewElement("InitialVelocityMin");
	initialVelocityMinElement->SetText(Serialize(spawnDesc.initialVelocity.minValue).c_str());
	parentElement->InsertEndChild(initialVelocityMinElement);

	tinyxml2::XMLElement* initialVelocityMaxElement = xmlDocument.NewElement("InitialVelocityMax");
	initialVelocityMaxElement->SetText(Serialize(spawnDesc.initialVelocity.maxValue).c_str());
	parentElement->InsertEndChild(initialVelocityMaxElement);

	tinyxml2::XMLElement* initialRotationMinElement = xmlDocument.NewElement("InitialRotationMin");
	initialRotationMinElement->SetText(Serialize(spawnDesc.initialRotation.minValue).c_str());
	parentElement->InsertEndChild(initialRotationMinElement);

	tinyxml2::XMLElement* initialRotationMaxElement = xmlDocument.NewElement("InitialRotationMax");
	initialRotationMaxElement->SetText(Serialize(spawnDesc.initialRotation.maxValue).c_str());
	parentElement->InsertEndChild(initialRotationMaxElement);

	tinyxml2::XMLElement* angularVelocityMinElement = xmlDocument.NewElement("AngularVelocityMin");
	angularVelocityMinElement->SetText(Serialize(spawnDesc.angularVelocity.minValue).c_str());
	parentElement->InsertEndChild(angularVelocityMinElement);

	tinyxml2::XMLElement* angularVelocityMaxElement = xmlDocument.NewElement("AngularVelocityMax");
	angularVelocityMaxElement->SetText(Serialize(spawnDesc.angularVelocity.maxValue).c_str());
	parentElement->InsertEndChild(angularVelocityMaxElement);

	tinyxml2::XMLElement* accelerationMinElement = xmlDocument.NewElement("AccelerationMin");
	accelerationMinElement->SetText(Serialize(spawnDesc.acceleration.minValue).c_str());
	parentElement->InsertEndChild(accelerationMinElement);

	tinyxml2::XMLElement* accelerationMaxElement = xmlDocument.NewElement("AccelerationMax");
	accelerationMaxElement->SetText(Serialize(spawnDesc.acceleration.maxValue).c_str());
	parentElement->InsertEndChild(accelerationMaxElement);

	tinyxml2::XMLElement* velocityLimitElement = xmlDocument.NewElement("VelocityLimit");
	velocityLimitElement->SetText(spawnDesc.velocityLimit);
	parentElement->InsertEndChild(velocityLimitElement);

	tinyxml2::XMLElement* sizeByLifetimeElement = xmlDocument.NewElement("SizeByLifetime");
	sizeByLifetimeElement->SetText(serializeFloatCurve(spawnDesc.sizeByLifetime).c_str());
	parentElement->InsertEndChild(sizeByLifetimeElement);

	tinyxml2::XMLElement* sizeBySpeedRangeElement = xmlDocument.NewElement("SizeBySpeedRange");
	sizeBySpeedRangeElement->SetText(serializeVector2(spawnDesc.sizeBySpeedRange).c_str());
	parentElement->InsertEndChild(sizeBySpeedRangeElement);

	tinyxml2::XMLElement* sizeBySpeedElement = xmlDocument.NewElement("SizeBySpeed");
	sizeBySpeedElement->SetText(serializeFloatCurve(spawnDesc.sizeBySpeed).c_str());
	parentElement->InsertEndChild(sizeBySpeedElement);

	tinyxml2::XMLElement* colorByLifetimeStartElement = xmlDocument.NewElement("ColorByLifetimeStart");
	colorByLifetimeStartElement->SetText(serializeVector4(spawnDesc.colorByLifetime.startValue).c_str());
	parentElement->InsertEndChild(colorByLifetimeStartElement);

	tinyxml2::XMLElement* colorByLifetimeEndElement = xmlDocument.NewElement("ColorByLifetimeEnd");
	colorByLifetimeEndElement->SetText(serializeVector4(spawnDesc.colorByLifetime.endValue).c_str());
	parentElement->InsertEndChild(colorByLifetimeEndElement);

	tinyxml2::XMLElement* colorBySpeedRangeElement = xmlDocument.NewElement("ColorBySpeedRange");
	colorBySpeedRangeElement->SetText(serializeVector2(spawnDesc.colorBySpeedRange).c_str());
	parentElement->InsertEndChild(colorBySpeedRangeElement);

	tinyxml2::XMLElement* colorBySpeedStartElement = xmlDocument.NewElement("ColorBySpeedStart");
	colorBySpeedStartElement->SetText(serializeVector4(spawnDesc.colorBySpeed.startValue).c_str());
	parentElement->InsertEndChild(colorBySpeedStartElement);

	tinyxml2::XMLElement* colorBySpeedEndElement = xmlDocument.NewElement("ColorBySpeedEnd");
	colorBySpeedEndElement->SetText(serializeVector4(spawnDesc.colorBySpeed.endValue).c_str());
	parentElement->InsertEndChild(colorBySpeedEndElement);

	tinyxml2::XMLElement* emissiveColorStartElement = xmlDocument.NewElement("EmissiveColorStart");
	emissiveColorStartElement->SetText(Serialize(spawnDesc.emissiveColorByLifetime.startValue).c_str());
	parentElement->InsertEndChild(emissiveColorStartElement);

	tinyxml2::XMLElement* emissiveColorEndElement = xmlDocument.NewElement("EmissiveColorEnd");
	emissiveColorEndElement->SetText(Serialize(spawnDesc.emissiveColorByLifetime.endValue).c_str());
	parentElement->InsertEndChild(emissiveColorEndElement);

	if (const StaticMeshParticleSystemComponent* staticMeshParticleSystemComponent = dynamic_cast<const StaticMeshParticleSystemComponent*>(particleSystemComponent))
	{
		if (!staticMeshParticleSystemComponent->GetStaticMeshPath().empty())
		{
			tinyxml2::XMLElement* staticMeshPathElement = xmlDocument.NewElement("StaticMeshPath");
			staticMeshPathElement->SetText(staticMeshParticleSystemComponent->GetStaticMeshPath().c_str());
			parentElement->InsertEndChild(staticMeshPathElement);
		}
	}

	if (const BillboardParticleSystemComponent* billboardParticleSystemComponent = dynamic_cast<const BillboardParticleSystemComponent*>(particleSystemComponent))
	{
		if (!billboardParticleSystemComponent->GetBillboardTexturePath().empty())
		{
			tinyxml2::XMLElement* billboardTexturePathElement = xmlDocument.NewElement("BillboardTexturePath");
			billboardTexturePathElement->SetText(billboardParticleSystemComponent->GetBillboardTexturePath().c_str());
			parentElement->InsertEndChild(billboardTexturePathElement);
		}

		if (!billboardParticleSystemComponent->GetBillboardMaterialPath().empty())
		{
			tinyxml2::XMLElement* billboardMaterialPathElement = xmlDocument.NewElement("BillboardMaterialPath");
			billboardMaterialPathElement->SetText(billboardParticleSystemComponent->GetBillboardMaterialPath().c_str());
			parentElement->InsertEndChild(billboardMaterialPathElement);
		}
	}
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

void SceneParser::GetXMLElement_HeightMapCollisionComponent(const HeightMapCollisionComponent* const heightMapCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	if (!heightMapCollisionComponent || !parentElement)
	{
		return;
	}

	const Image* heightMapImage = heightMapCollisionComponent->GetHeightMapImage();
	if (heightMapImage)
	{
		const std::string heightMapImagePath = ContentPathUtils::ToContentRelativePath(heightMapImage->GetPath());
		if (!heightMapImagePath.empty())
		{
			tinyxml2::XMLElement* heightMapImageElement = xmlDocument.NewElement("HeightMapImage");
			heightMapImageElement->SetText(heightMapImagePath.c_str());
			parentElement->InsertEndChild(heightMapImageElement);
		}
	}

	tinyxml2::XMLElement* heightStickWidthElement = xmlDocument.NewElement("HeightStickWidth");
	heightStickWidthElement->SetText(heightMapCollisionComponent->GetHeightStickWidth());
	parentElement->InsertEndChild(heightStickWidthElement);

	tinyxml2::XMLElement* heightStickLengthElement = xmlDocument.NewElement("HeightStickLength");
	heightStickLengthElement->SetText(heightMapCollisionComponent->GetHeightStickLength());
	parentElement->InsertEndChild(heightStickLengthElement);

	tinyxml2::XMLElement* minHeightElement = xmlDocument.NewElement("MinHeight");
	minHeightElement->SetText(heightMapCollisionComponent->GetMinHeight());
	parentElement->InsertEndChild(minHeightElement);

	tinyxml2::XMLElement* maxHeightElement = xmlDocument.NewElement("MaxHeight");
	maxHeightElement->SetText(heightMapCollisionComponent->GetMaxHeight());
	parentElement->InsertEndChild(maxHeightElement);

	tinyxml2::XMLElement* widthElement = xmlDocument.NewElement("Width");
	widthElement->SetText(heightMapCollisionComponent->GetWidth());
	parentElement->InsertEndChild(widthElement);

	tinyxml2::XMLElement* lengthElement = xmlDocument.NewElement("Length");
	lengthElement->SetText(heightMapCollisionComponent->GetLength());
	parentElement->InsertEndChild(lengthElement);

	tinyxml2::XMLElement* heightScaleElement = xmlDocument.NewElement("HeightScale");
	heightScaleElement->SetText(heightMapCollisionComponent->GetHeightScale());
	parentElement->InsertEndChild(heightScaleElement);
}

void SceneParser::GetXMLElement_MovingTriangleMeshCollisionComponent(const MovingTriangleMeshCollisionComponent* const movingTriangleMeshCollisionComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	tinyxml2::XMLElement* movingTriangleMeshCollisionComponentElement = xmlDocument.NewElement("Mesh");
	const std::string meshPath = ContentPathUtils::ToContentRelativePath(movingTriangleMeshCollisionComponent->GetMesh()->GetPath());
	movingTriangleMeshCollisionComponentElement->SetText(meshPath.c_str());
	parentElement->InsertEndChild(movingTriangleMeshCollisionComponentElement);
}

void SceneParser::GetXMLElement_NavigationTreeComponent(const NavigationTreeComponent* const navigationTreeComponent, tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	if (!navigationTreeComponent || navigationTreeComponent->GetNavigationTreePath().empty())
	{
		return;
	}

	tinyxml2::XMLElement* navigationTreePathElement = xmlDocument.NewElement("NavigationTreePath");
	navigationTreePathElement->SetText(navigationTreeComponent->GetNavigationTreePath().c_str());
	parentElement->InsertEndChild(navigationTreePathElement);
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
