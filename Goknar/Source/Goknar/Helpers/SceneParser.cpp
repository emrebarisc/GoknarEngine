#include "pch.h"

#include "SceneParser.h"

#include <sstream>
#include <stdexcept>

#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/IO/ModelLoader.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Texture.h"


#include "Goknar/Managers/ResourceManager.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/SkeletalMesh.h"

#include "Goknar/Components/DynamicMeshComponent.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Components/SkeletalMeshComponent.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"

#include "tinyxml2.h"

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

	//Get BackgroundColor
	auto element = root->FirstChildElement("BackgroundColor");
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

			child = element->FirstChildElement("ImageResolution");
			stream << child->GetText() << std::endl;
			int width, height;
			stream >> width >> height;
			camera->SetImageWidth(width);
			camera->SetImageHeight(height);

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

			if (shadowMapResolutionElementFound)
			{
				int x = 1024;
				int y = 1024;
				stream >> x >> y;
				pointLight->SetShadowWidth(x);
				pointLight->SetShadowHeight(y);
			}

			scene->AddPointLight(pointLight);
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

			if (shadowMapResolutionElementFound)
			{
				int x = 1024;
				int y = 1024;
				stream >> x >> y;
				directionalLight->SetShadowWidth(x);
				directionalLight->SetShadowHeight(y);
			}

			scene->AddDirectionalLight(directionalLight);
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

			if (shadowMapResolutionElementFound)
			{
				int x = 1024;
				int y = 1024;
				stream >> x >> y;
				spotLight->SetShadowWidth(x);
				spotLight->SetShadowHeight(y);
			}

			scene->AddSpotLight(spotLight);
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
				material->SetDiffuseReflectance(diffuseReflectance);
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
			material->SetPhongExponent(phongExponent);

			resourceManager->AddMaterial(material);
			element = element->NextSiblingElement("Material");
		}
		stream.clear();
	}

	//Get Static Meshes
	element = root->FirstChildElement("Meshes");
	if (element)
	{
		element = element->FirstChildElement("StaticMesh");
		while (element)
		{
			StaticMesh* mesh = nullptr;
			int materialId = -1;
			child = element->FirstChildElement("Material");
			if (child)
			{
				stream << child->GetText() << std::endl;
				stream >> materialId;
			}

			bool loadedWithFile = false;

			child = element->FirstChildElement("Path");
			if (child)
			{
				std::string modelFilePath;
				stream << child->GetText() << std::endl;
				stream >> modelFilePath;

				mesh = engine->GetResourceManager()->GetContent<StaticMesh>(modelFilePath);
				if (mesh != nullptr)
				{
					if (0 <= materialId)
					{
						mesh->SetMaterial(resourceManager->GetMaterial(materialId));
					}
					loadedWithFile = true;
				}
				stream.clear();
				element = element->NextSiblingElement("StaticMesh");
				continue;
			}

			if (!mesh)
			{
				mesh = new StaticMesh();
				if (0 <= materialId)
				{
					mesh->SetMaterial(resourceManager->GetMaterial(materialId));
				}

				child = element->FirstChildElement("Vertices");
				stream << child->GetText() << std::endl;
				Vector3 vertex;
				while (!(stream >> vertex.x).eof())
				{
					stream >> vertex.y >> vertex.z;
					mesh->AddVertex(vertex);
				}
				stream.clear();

				child = element->FirstChildElement("Normals");
				if (child)
				{
					stream << child->GetText() << std::endl;
					int normalIndex = 0;
					Vector3 normal;
					while (!(stream >> normal.x).eof())
					{
						stream >> normal.y >> normal.z;
						mesh->SetVertexNormal(normalIndex++, normal);
					}
				}
				stream.clear();

				child = element->FirstChildElement("Faces");
				stream << child->GetText() << std::endl;
				unsigned int v0;
				while (!(stream >> v0).eof())
				{
					Face face = Face();
					face.vertexIndices[0] = v0;
					stream >> face.vertexIndices[1] >> face.vertexIndices[2];
					mesh->AddFace(face);
				}
				stream.clear();

				child = element->FirstChildElement("UVs");
				if (child)
				{
					stream << child->GetText() << std::endl;
					unsigned int uvIndex = 0;
					Vector2 UV;
					while (!(stream >> UV.x).eof())
					{
						stream >> UV.y;
						if (uvIndex < mesh->GetVertexCount())
						{
							mesh->SetVertexUV(uvIndex++, UV);
						}
						else
						{
							break;
						}
					}
				}
				stream.clear();
			}

			if (!loadedWithFile && 0 < mesh->GetVertexCount())
			{
				scene->AddStaticMesh(mesh);
			}
			element = element->NextSiblingElement("StaticMesh");
		}
		stream.clear();
	}

	//Get Skeletal Meshes
	element = root->FirstChildElement("Meshes");
	if (element)
	{
		element = element->FirstChildElement("SkeletalMesh");
		while (element)
		{
			int materialId = -1;
			child = element->FirstChildElement("Material");
			if (child)
			{
				stream << child->GetText() << std::endl;
				stream >> materialId;
			}

			child = element->FirstChildElement("Path");
			SkeletalMesh* skeletalMesh;
			if (child)
			{
				std::string modelFilePath;
				stream << child->GetText() << std::endl;
				stream >> modelFilePath;

				skeletalMesh = engine->GetResourceManager()->GetContent<SkeletalMesh>(modelFilePath);
				if (skeletalMesh != nullptr)
				{
					if (0 <= materialId)
					{
						skeletalMesh->SetMaterial(resourceManager->GetMaterial(materialId));
					}
				}
				stream.clear();
			}
			element = element->NextSiblingElement("SkeletalMesh");
		}
		stream.clear();
	}

	//Get Static Objects
	element = root->FirstChildElement("Objects");
	if (element)
	{
		element = element->FirstChildElement("StaticMeshObject");
		ObjectBase* object = nullptr;
		while (element)
		{
			object = new ObjectBase();
			StaticMeshComponent* staticMeshComponent = object->AddSubComponent<StaticMeshComponent>();

			child = element->FirstChildElement("Mesh");
			if (child)
			{
				stream << child->GetText() << std::endl;
				int meshIndex;
				stream >> meshIndex;
				staticMeshComponent->SetMesh(dynamic_cast<StaticMesh*>(engine->GetResourceManager()->GetResourceContainer()->GetMesh(meshIndex)));
			}

			child = element->FirstChildElement("PivotPoint");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 pivotPoint;
				stream >> pivotPoint.x >> pivotPoint.y >> pivotPoint.z;
				staticMeshComponent->SetPivotPoint(pivotPoint);
			}
			stream.clear();

			child = element->FirstChildElement("RelativePosition");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativePosition;
				stream >> relativePosition.x >> relativePosition.y >> relativePosition.z;
				staticMeshComponent->SetRelativePosition(relativePosition);
			}
			stream.clear();

			child = element->FirstChildElement("RelativeRotation");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativeRotation;
				stream >> relativeRotation.x >> relativeRotation.y >> relativeRotation.z;
				staticMeshComponent->SetRelativeRotation(Quaternion::FromEulerDegrees(relativeRotation));
			}
			stream.clear();

			child = element->FirstChildElement("RelativeScaling");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativeScaling;
				stream >> relativeScaling.x >> relativeScaling.y >> relativeScaling.z;
				staticMeshComponent->SetRelativeScaling(relativeScaling);
			}
			stream.clear();

			child = element->FirstChildElement("WorldPosition");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldPosition;
				stream >> worldPosition.x >> worldPosition.y >> worldPosition.z;
				object->SetWorldPosition(worldPosition);
			}
			stream.clear();

			child = element->FirstChildElement("WorldRotation");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldRotation;
				stream >> worldRotation.x >> worldRotation.y >> worldRotation.z;
				object->SetWorldRotation(Quaternion::FromEulerDegrees(worldRotation));
			}
			stream.clear();

			child = element->FirstChildElement("WorldScaling");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldScaling;
				stream >> worldScaling.x >> worldScaling.y >> worldScaling.z;
				object->SetWorldScaling(worldScaling);
			}
			stream.clear();
			element = element->NextSiblingElement("StaticMeshObject");

		}
		stream.clear();
	}

	//Get Skeletal Objects
	element = root->FirstChildElement("Objects");
	if (element)
	{
		element = element->FirstChildElement("SkeletalMeshObject");
		ObjectBase* object = nullptr;
		while (element)
		{
			object = new ObjectBase();
			SkeletalMeshComponent* skeletalMeshComponent = object->AddSubComponent<SkeletalMeshComponent>();

			child = element->FirstChildElement("PivotPoint");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 pivotPoint;
				stream >> pivotPoint.x >> pivotPoint.y >> pivotPoint.z;
				skeletalMeshComponent->SetPivotPoint(pivotPoint);
			}
			stream.clear();

			child = element->FirstChildElement("RelativePosition");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativePosition;
				stream >> relativePosition.x >> relativePosition.y >> relativePosition.z;
				skeletalMeshComponent->SetRelativePosition(relativePosition);
			}
			stream.clear();

			child = element->FirstChildElement("RelativeRotation");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativeRotation;
				stream >> relativeRotation.x >> relativeRotation.y >> relativeRotation.z;
				skeletalMeshComponent->SetRelativeRotation(Quaternion::FromEulerDegrees(relativeRotation));
			}
			stream.clear();

			child = element->FirstChildElement("RelativeScaling");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativeScaling;
				stream >> relativeScaling.x >> relativeScaling.y >> relativeScaling.z;
				skeletalMeshComponent->SetRelativeScaling(relativeScaling);
			}
			stream.clear();

			child = element->FirstChildElement("Mesh");
			if (child)
			{
				stream << child->GetText() << std::endl;
				int meshIndex;
				stream >> meshIndex;
				skeletalMeshComponent->SetMesh(dynamic_cast<SkeletalMesh*>(engine->GetResourceManager()->GetResourceContainer()->GetMesh(meshIndex)));
			}

			child = element->FirstChildElement("WorldPosition");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldPosition;
				stream >> worldPosition.x >> worldPosition.y >> worldPosition.z;
				object->SetWorldPosition(worldPosition);
			}
			stream.clear();

			child = element->FirstChildElement("WorldRotation");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldRotation;
				stream >> worldRotation.x >> worldRotation.y >> worldRotation.z;
				object->SetWorldRotation(Quaternion::FromEulerDegrees(worldRotation));
			}
			stream.clear();

			child = element->FirstChildElement("WorldScaling");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldScaling;
				stream >> worldScaling.x >> worldScaling.y >> worldScaling.z;
				object->SetWorldScaling(worldScaling);
			}
			stream.clear();
			element = element->NextSiblingElement("SkeletalMeshObject");

		}
		stream.clear();
	}

	//Get Dynamic Objects
	element = root->FirstChildElement("Objects");
	if(element)
	{
		// DynamicMeshObject
		element = element->FirstChildElement("DynamicMeshObject");
		ObjectBase* object = nullptr;
		while (element)
		{
			object = new ObjectBase();
			DynamicMeshComponent* dynamicMeshComponent = object->AddSubComponent<DynamicMeshComponent>();

			child = element->FirstChildElement("PivotPoint");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 pivotPoint;
				stream >> pivotPoint.x >> pivotPoint.y >> pivotPoint.z;
				dynamicMeshComponent->SetPivotPoint(pivotPoint);
			}
			stream.clear();

			child = element->FirstChildElement("RelativePosition");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativePosition;
				stream >> relativePosition.x >> relativePosition.y >> relativePosition.z;
				dynamicMeshComponent->SetRelativePosition(relativePosition);
			}
			stream.clear();

			child = element->FirstChildElement("RelativeRotation");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativeRotation;
				stream >> relativeRotation.x >> relativeRotation.y >> relativeRotation.z;
				dynamicMeshComponent->SetRelativeRotation(Quaternion::FromEulerDegrees(relativeRotation));
			}
			stream.clear();

			child = element->FirstChildElement("RelativeScaling");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 relativeScaling;
				stream >> relativeScaling.x >> relativeScaling.y >> relativeScaling.z;
				dynamicMeshComponent->SetRelativeScaling(relativeScaling);
			}
			stream.clear();

			child = element->FirstChildElement("Mesh");
			if (child)
			{
				stream << child->GetText() << std::endl;
				int meshIndex;
				stream >> meshIndex;
				dynamicMeshComponent->SetMesh(dynamic_cast<DynamicMesh*>(engine->GetResourceManager()->GetResourceContainer()->GetMesh(meshIndex)));
			}

			child = element->FirstChildElement("WorldPosition");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldPosition;
				stream >> worldPosition.x >> worldPosition.y >> worldPosition.z;
				object->SetWorldPosition(worldPosition);
			}
			stream.clear();

			child = element->FirstChildElement("WorldRotation");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldRotation;
				stream >> worldRotation.x >> worldRotation.y >> worldRotation.z;
				object->SetWorldRotation(Quaternion::FromEulerDegrees(worldRotation));
			}
			stream.clear();

			child = element->FirstChildElement("WorldScaling");
			if (child)
			{
				stream << child->GetText() << std::endl;
				Vector3 worldScaling;
				stream >> worldScaling.x >> worldScaling.y >> worldScaling.z;
				object->SetWorldScaling(worldScaling);
			}
			stream.clear();

			scene->AddObject(object);
			element = element->NextSiblingElement("DynamicMeshObject");

		}
		stream.clear();

	}
}