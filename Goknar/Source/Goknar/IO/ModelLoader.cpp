#include "pch.h"

#include "ModelLoader.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Log.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Texture.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

StaticMesh* ModelLoader::LoadPlyFile(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		return nullptr;
	}

	std::string line;
	std::stringstream stringStream;

	StaticMesh* mesh = new StaticMesh();
	int vertexCount = 0;
	int faceCount = 0;

	bool headerEnded = false;

	bool readVertices = false;
	bool readNormals = false;
	bool readUVs = false;
	bool readColors = false;

	// Read header
	while (!headerEnded && std::getline(file, line))
	{
		std::string keyword, type, value;
		stringStream << line;
		while (stringStream >> keyword)
		{
			if (keyword == "element")
			{
				stringStream >> type;

				if (type == "vertex")
				{
					stringStream >> vertexCount;
				}

				if (type == "face")
				{
					stringStream >> faceCount;
				}
			}
			else if (keyword == "property")
			{
				stringStream >> type >> value;

				if (value == "x" || value == "y" || value == "z")
				{
				readVertices = true;
				}
				else if (value == "nx" || value == "ny" || value == "nz")
				{
				readNormals = true;
				}
				else if (value == "s" || value == "t")
				{
				readUVs = true;
				}
				else if (value == "red" || value == "green" || value == "blue" || value == "alpha")
				{
				readColors = true;
				}
			}
			else if (keyword == "end_header")
			{
			headerEnded = true;
			break;
			}
		}
		stringStream.clear();
	}

	// Read data
	float x = 0;
	float y = 0;
	float z = 0;
	float nx = 0;
	float ny = 0;
	float nz = 0;
	float s = 0;
	float t = 0;
	float red = 255.f;
float green = 255.f;
float blue = 255.f;
float alpha = 255.f;

for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
{
	std::getline(file, line);
	stringStream << line;
	if (readVertices)
	{
		stringStream >> x >> y >> z;
	}
	if (readNormals)
	{
		stringStream >> nx >> ny >> nz;
	}
	if (readUVs)
	{
		stringStream >> s >> t;
	}
	if (readColors)
	{
		stringStream >> red >> green >> blue >> alpha;
	}

	mesh->AddVertexData(VertexData(Vector3(x, y, z), Vector3(nx, ny, nz), Vector4(red / 255.f, green / 255.f, blue / 255.f, alpha / 255.f), Vector2(s, t)));
	stringStream.clear();
}

for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
{
	std::getline(file, line);
	stringStream << line;

	int faceVertexCount;
	int facePoint1, facePoint2, facePoint3;
	stringStream >> faceVertexCount >> facePoint1 >> facePoint2 >> facePoint3;
	Face face(facePoint1, facePoint2, facePoint3);
	mesh->AddFace(face);

	stringStream.clear();
}

return mesh;
}

StaticMesh* ModelLoader::LoadModel(const std::string& path)
{
	Scene* gameScene = engine->GetApplication()->GetMainScene();

	StaticMesh* staticMesh = nullptr;

	Assimp::Importer importer;
	const aiScene* assimpScene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (assimpScene)
	{
		staticMesh = new StaticMesh();

		for (unsigned int meshIndex = 0; meshIndex < assimpScene->mNumMeshes; ++meshIndex)
		{
			aiMesh* assimpMesh = assimpScene->mMeshes[meshIndex];

			for (unsigned int vertexIndex = 0; vertexIndex < assimpMesh->mNumVertices; ++vertexIndex)
			{
				aiVector3D& vertexPosition = assimpMesh->mVertices[vertexIndex];
				aiVector3D& normal = assimpMesh->mNormals[vertexIndex];

				Vector4 vertexColor = Vector4::ZeroVector;
				if (assimpMesh->HasVertexColors(vertexIndex))
				{
					aiColor4D* color = assimpMesh->mColors[vertexIndex];
					vertexColor = Vector4(color->r / 255, color->g / 255, color->b / 255, color->a / 255);
				}

				Vector2 vertexUV = Vector2::ZeroVector;
				if (assimpMesh->HasTextureCoords(0))
				{
					aiVector3D assimpUV = assimpMesh->mTextureCoords[0][vertexIndex];
					vertexUV.x = assimpUV.x;
					vertexUV.y = assimpUV.y;
				}

				staticMesh->AddVertexData(
					VertexData(
						Vector3(vertexPosition.x, vertexPosition.y, vertexPosition.z),
						Vector3(normal.x, normal.y, normal.z),
						vertexColor,
						vertexUV
					)
				);
			}

			if (assimpMesh->HasFaces())
			{
				for (unsigned int faceIndex = 0; faceIndex < assimpMesh->mNumFaces; faceIndex++)
				{
					aiFace& face = assimpMesh->mFaces[faceIndex];
					GOKNAR_CORE_ASSERT(face.mNumIndices == 3, "ONLY TRIANGLE MESH FACES ARE SUPPORTED!");
					staticMesh->AddFace(Face(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
				}
			}

			if(assimpScene->HasMaterials())
			{
				aiMaterial* assimpMaterial = assimpScene->mMaterials[assimpMesh->mMaterialIndex];

				if (assimpMaterial)
				{
					Material* material = new Material();

					aiColor3D value(0.f, 0.f, 0.f);
					assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, value);
					material->SetAmbientReflectance(Vector3(value.r, value.g, value.b));

					assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, value);
					material->SetDiffuseReflectance(Vector3(value.r, value.g, value.b));

					assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, value);
					material->SetSpecularReflectance(Vector3(value.r, value.g, value.b));

					float floatValue = -1.f;
					assimpMaterial->Get(AI_MATKEY_SPECULAR_FACTOR, floatValue);
					if (0.f < floatValue)
					{
						material->SetPhongExponent(floatValue);
					}

					bool isTwoSided = false;
					assimpMaterial->Get(AI_MATKEY_TWOSIDED, isTwoSided);
					material->SetShadingModel(isTwoSided ? MaterialShadingModel::TwoSided : MaterialShadingModel::Default);

					aiString name;
					assimpMaterial->Get(AI_MATKEY_NAME, name);
					material->SetName(name.C_Str());

					aiString texturePath;
					if (assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
					{
						Shader* shader = new Shader();
						gameScene->AddShader(shader);

						shader->SetHolderMaterial(material);
						material->SetShader(shader);

						Texture* texture = new Texture();
						texture->SetTextureImagePath(texturePath.C_Str());
						texture->SetTextureUsage(TextureUsage::Diffuse);
						gameScene->AddTexture(texture);
						material->GetShader()->AddTexture(texture);
					}

					gameScene->AddMaterial(material);
					staticMesh->SetMaterial(material);
				}
			}
		}
	}
	else
	{
		GOKNAR_CORE_ERROR("Error occured while loading the asset({}). What went wrong: {}", path, importer.GetErrorString());
	}

	return staticMesh;
}