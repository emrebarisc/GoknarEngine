#include "pch.h"

#include "SceneParser.h"

#include <sstream>
#include <stdexcept>

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Material.h"
#include "Goknar/Mesh.h"

#include "Goknar/Lights/PointLight.h"

#include "Goknar/Managers/CameraManager.h"

#include "tinyxml2.h"

void SceneParser::Parse(Scene* scene, char* filePath)
{
	tinyxml2::XMLDocument xmlFile;
	std::stringstream stream;

	tinyxml2::XMLError res;

	try
	{
		res = xmlFile.LoadFile(filePath);
		if (res)
		{
			throw std::runtime_error("Error: Scene XML file could not be loaded.");
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

	Colori backgroundColor;
	stream >> backgroundColor.r >> backgroundColor.g >> backgroundColor.b;

	scene->SetBackgroundColor(backgroundColor);

	stream.clear();

	//Get Cameras
	element = root->FirstChildElement("Cameras");
	element = element->FirstChildElement("Camera");
	Camera* camera = new Camera();
	while (element)
	{
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

			float resolutionProportion = camera->GetImageWidth() / camera->GetImageHeight();

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

		// Set up the right vector and make forward and up vector perpenticular in case they are not
		camera->SetRightVector(Vector3::Cross(camera->GetForwardVector().GetNormalized(), camera->GetUpVector().GetNormalized()));
		camera->SetUpVector(Vector3::Cross(camera->GetRightVector().GetNormalized(), camera->GetForwardVector().GetNormalized()));
		camera->SetForwardVector(Vector3::Cross(camera->GetUpVector().GetNormalized(), camera->GetRightVector().GetNormalized()));
		
		camera->InitMatrices();

		engine->GetCameraManager()->AddCamera(camera);
		element = element->NextSiblingElement("Camera");
		stream.clear();
	}

	//Get Ambient Light
	element = root->FirstChildElement("Lights");
	auto child = element->FirstChildElement("AmbientLight");
	stream << child->GetText() << std::endl;
	Vector3 ambientLight;
	stream >> ambientLight.x >> ambientLight.y >> ambientLight.z;

	scene->SetAmbientLight(ambientLight);

	//Get Lights
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

		Vector3 position;
		stream >> position.x >> position.y >> position.z;
		pointLight->SetPosition(position);

		Vector3 color;
		stream >> color.x >> color.y >> color.z;
		pointLight->SetColor(color);

		float intensity;
		stream >> intensity;
		pointLight->SetIntensity(intensity);

		scene->AddPointLight(pointLight);
		element = element->NextSiblingElement("PointLight");
	}
	stream.clear();

	//Get Materials
	element = root->FirstChildElement("Materials");
	element = element->FirstChildElement("Material");
	Material* material;
	while (element)
	{
		material = new Material();
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

		scene->AddMaterial(material);
		element = element->NextSiblingElement("Material");
	}

	stream.clear();

	//Get Meshes
	element = root->FirstChildElement("Objects");
	element = element->FirstChildElement("Mesh");

	Mesh* mesh;
	while (element)
	{
		mesh = new Mesh();
		child = element->FirstChildElement("Material");
		stream << child->GetText() << std::endl;

		int materialId;
		stream >> materialId;
		mesh->SetMaterialId(materialId);

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

			/*const std::vector<Vector3>* vertices = mesh->GetVerticesPointer();
			Vector3 a = vertices->at(face.vertexIndices[0]);
			Vector3 b = vertices->at(face.vertexIndices[1]);
			Vector3 c = vertices->at(face.vertexIndices[2]);
			face->normal = Vector3::Cross(c - b, a - b);
			Vector3::Normalize(face->normal);*/

			mesh->AddFace(face);
		}
		stream.clear();

		scene->AddMesh(mesh);
		element = element->NextSiblingElement("Mesh");
	}
	stream.clear();
}