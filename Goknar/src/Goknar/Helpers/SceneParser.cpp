#include "pch.h"

#include "SceneParser.h"

#include <sstream>
#include <stdexcept>

#include "Goknar/Scene.h"
#include "Goknar/Mesh.h"

#include "tinyxml2.h"

void SceneParser::Parse(Scene* scene, char* filePath)
{
	tinyxml2::XMLDocument xmlFile;
	std::stringstream stream;

	auto res = xmlFile.LoadFile(filePath);
	if (res)
	{
		throw std::runtime_error("Error: XML file could not be loaded.");
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
/*	element = root->FirstChildElement("Cameras");
	element = element->FirstChildElement("Camera");
	Camera camera;
	while (element)
	{
		auto child = element->FirstChildElement("Position");
		stream << child->GetText() << std::endl;
		stream >> camera.position.x >> camera.position.y >> camera.position.z;

		child = element->FirstChildElement("ImageResolution");
		stream << child->GetText() << std::endl;
		stream >> camera.imageWidth_ >> camera.imageHeight_;

		child = element->FirstChildElement("NearDistance");
		stream << child->GetText() << std::endl;
		stream >> camera.nearDistance_;

		const char* cameraType = element->Attribute("type");

		if (cameraType && std::string(cameraType) == "simple")
		{
			child = element->FirstChildElement("GazePoint");
			if (child)
			{
				Vector3 gazePoint;
				stream << child->GetText() << std::endl;
				stream >> gazePoint.x >> gazePoint.y >> gazePoint.z;

				camera.gaze = (gazePoint - camera.position).GetNormalized();
			}
			else
			{
				child = element->FirstChildElement("Gaze");
				if (child)
				{
					stream << child->GetText() << std::endl;
					stream >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
				}
			}

			float fovY;
			child = element->FirstChildElement("FovY");
			stream << child->GetText() << std::endl;
			stream >> fovY;

			float resolutionProportion = camera.imageWidth_ / camera.imageHeight_;

			float halfOfFovY = fovY * 0.5f;
			float top, bottom, left, right;
			top = camera.nearDistance_ * tan(DEGREE_TO_RADIAN(halfOfFovY));
			bottom = -top;
			left = bottom * resolutionProportion;
			right = top * resolutionProportion;

			camera.nearPlane = Vector4(left, right, bottom, top);
		}
		else
		{
			child = element->FirstChildElement("Gaze");
			stream << child->GetText() << std::endl;
			stream >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;

			child = element->FirstChildElement("NearPlane");
			stream << child->GetText() << std::endl;
			stream >> camera.nearPlane.x >> camera.nearPlane.y >> camera.nearPlane.z >> camera.nearPlane.w;
		}

		child = element->FirstChildElement("Up");
		stream << child->GetText() << std::endl;
		stream >> camera.up.x >> camera.up.y >> camera.up.z;

		// Set up the right vector and make forward and up vector perpenticular in case they are not
		camera.right = Vector3::Cross(camera.gaze.GetNormalized(), camera.up.GetNormalized());
		camera.up = Vector3::Cross(camera.right.GetNormalized(), camera.gaze.GetNormalized());
		camera.gaze = Vector3::Cross(camera.up.GetNormalized(), camera.right.GetNormalized());

		camera.right.Normalize();
		camera.up.Normalize();
		camera.right.Normalize();

		child = element->FirstChildElement("NumSamples");
		if (child)
		{
			stream << child->GetText() << std::endl;
		}
		else
		{
			stream << "1" << std::endl;;
		}
		stream >> camera.numberOfSamples;

		child = element->FirstChildElement("ImageName");
		stream << child->GetText() << std::endl;
		stream >> camera.imageName;

		scene->cameras.push_back(camera);
		element = element->NextSiblingElement("Camera");
		stream.clear();
	}*/

	//Get Ambient Light
	element = root->FirstChildElement("Lights");
	auto child = element->FirstChildElement("AmbientLight");
	stream << child->GetText() << std::endl;
	Vector3 ambientLight;
	stream >> ambientLight.x >> ambientLight.y >> ambientLight.z;

	scene->SetAmbientLight(ambientLight);

	//Get Lights
/*	element = element->FirstChildElement("PointLight");
	PointLight pointLight;
	while (element)
	{
		child = element->FirstChildElement("Position");
		stream << child->GetText() << std::endl;
		child = element->FirstChildElement("Intensity");
		stream << child->GetText() << std::endl;

		stream >> pointLight.position.x >> pointLight.position.y >> pointLight.position.z;
		stream >> pointLight.intensity.x >> pointLight.intensity.y >> pointLight.intensity.z;

		scene->pointLights.push_back(pointLight);
		element = element->NextSiblingElement("PointLight");
	}
*/

	//Get Materials
/*	element = root->FirstChildElement("Materials");
	element = element->FirstChildElement("Material");
	Material material;
	while (element)
	{
		child = element->FirstChildElement("AmbientReflectance");
		stream << child->GetText() << std::endl;
		stream >> material.ambient.x >> material.ambient.y >> material.ambient.z;

		child = element->FirstChildElement("DiffuseReflectance");
		stream << child->GetText() << std::endl;
		stream >> material.diffuse.x >> material.diffuse.y >> material.diffuse.z;

		child = element->FirstChildElement("SpecularReflectance");
		stream << child->GetText() << std::endl;
		stream >> material.specular.x >> material.specular.y >> material.specular.z;

		child = element->FirstChildElement("MirrorReflectance");
		if (child)
		{
			stream << child->GetText() << std::endl;
		}
		else
		{
			stream << "0 0 0" << std::endl;
		}
		stream >> material.mirror.x >> material.mirror.y >> material.mirror.z;

		child = element->FirstChildElement("Transparency");
		if (child)
		{
			stream << child->GetText() << std::endl;
		}
		else
		{
			stream << "0 0 0" << std::endl;
		}

		stream >> material.transparency.x >> material.transparency.y >> material.transparency.z;

		child = element->FirstChildElement("RefractionIndex");
		if (child)
		{
			stream << child->GetText() << std::endl;
		}
		else
		{
			stream << "0.0" << std::endl;
		}
		stream >> material.refractionIndex;

		child = element->FirstChildElement("PhongExponent");
		if (child)
		{
			stream << child->GetText() << std::endl;
		}
		else
		{
			stream << "1" << std::endl;
		}
		stream >> material.phongExponent;

		scene->materials.push_back(material);
		element = element->NextSiblingElement("Material");
	}
*/

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


		child = element->FirstChildElement("Faces");
		stream << child->GetText() << std::endl;

		unsigned int v0;
		while (!(stream >> v0).eof())
		{
			Face face = Face();
			face.vertexIndices[0] = v0;
			stream >> face.vertexIndices[1] >> face.vertexIndices[2];

			const std::vector<Vector3>& vertices = mesh->GetVertices();
			Vector3 a = vertices[face.vertexIndices[0] - 1];
			Vector3 b = vertices[face.vertexIndices[1] - 1];
			Vector3 c = vertices[face.vertexIndices[2] - 1];
			/*face->normal = Vector3::Cross(c - b, a - b);
			Vector3::Normalize(face->normal);*/

			mesh->AddFace(face);
		}
		stream.clear();

		scene->AddMesh(mesh);
		element = element->NextSiblingElement("Mesh");
	}
	stream.clear();
}