#include "pch.h"

#include "ModelLoader.h"

#include "Goknar/Application.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
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

aiNode* GetRootBone(const BoneNameToIdMap* boneNameToIdMap, aiNode* assimpNode)
{
	if (boneNameToIdMap->find(assimpNode->mName.C_Str()) != boneNameToIdMap->end())
	{
		return assimpNode;
	}

	for (unsigned int childIndex = 0; childIndex < assimpNode->mNumChildren; ++childIndex)
	{
		aiNode* childNode = GetRootBone(boneNameToIdMap, assimpNode->mChildren[childIndex]);
		if (childNode != nullptr)
		{
			return childNode;
		}
	}

	return nullptr;
}

void SetupArmature(SkeletalMesh* skeletalMesh, Bone* bone, aiNode* assimpNode)
{
	if (assimpNode->mNumChildren == 0)
	{
		return;
	}

	const BoneNameToIdMap* boneNameToIdMap = skeletalMesh->GetBoneNameToIdMap();
	for (unsigned int childrenIndex = 0; childrenIndex < assimpNode->mNumChildren; ++childrenIndex)
	{
		aiNode* assimpChildNode = assimpNode->mChildren[childrenIndex];
		std::string assimpChildNodeName = assimpChildNode->mName.C_Str();

		if (boneNameToIdMap->find(assimpChildNodeName) != boneNameToIdMap->end())
		{
			Bone* childBone = skeletalMesh->GetBone(skeletalMesh->GetBoneId(assimpChildNodeName));
			childBone->transformation = Matrix(
				assimpChildNode->mTransformation.a1, assimpChildNode->mTransformation.a2, assimpChildNode->mTransformation.a3, assimpChildNode->mTransformation.a4,
				assimpChildNode->mTransformation.b1, assimpChildNode->mTransformation.b2, assimpChildNode->mTransformation.b3, assimpChildNode->mTransformation.b4,
				assimpChildNode->mTransformation.c1, assimpChildNode->mTransformation.c2, assimpChildNode->mTransformation.c3, assimpChildNode->mTransformation.c4,
				assimpChildNode->mTransformation.d1, assimpChildNode->mTransformation.d2, assimpChildNode->mTransformation.d3, assimpChildNode->mTransformation.d4
			);
			bone->children.push_back(childBone);
			SetupArmature(skeletalMesh, childBone, assimpChildNode);
		}
	}
}

StaticMesh* ModelLoader::LoadModel(const std::string& path)
{
	Scene* gameScene = engine->GetApplication()->GetMainScene();

	StaticMesh* staticMesh = nullptr;
	SkeletalMesh* skeletalMesh = nullptr;

	Assimp::Importer importer;
	
	// aiProcess_Triangulate caused problems with vertex weights
	// Try exporting skeletal meshes as triangulated in modeling software
	const aiScene* assimpScene = importer.ReadFile((ContentDir + path).c_str(), 0);// aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_LimitBoneWeights);
	if (assimpScene)
	{
		for (unsigned int meshIndex = 0; meshIndex < assimpScene->mNumMeshes; ++meshIndex)
		{
			aiMesh* assimpMesh = assimpScene->mMeshes[meshIndex];

			if (assimpMesh->HasBones())
			{
				skeletalMesh = new SkeletalMesh();
				skeletalMesh->ResizeVertexToBonesArray(assimpMesh->mNumVertices);

				for (unsigned int boneIndex = 0; boneIndex < assimpMesh->mNumBones; ++boneIndex)
				{
					aiBone* assimpBone = assimpMesh->mBones[boneIndex];

					unsigned int boneId = skeletalMesh->GetBoneId(assimpBone->mName.C_Str());

					skeletalMesh->AddBone(new Bone(assimpBone->mName.C_Str(),
						Matrix
						(
							assimpBone->mOffsetMatrix.a1, assimpBone->mOffsetMatrix.a2, assimpBone->mOffsetMatrix.a3, assimpBone->mOffsetMatrix.a4,
							assimpBone->mOffsetMatrix.b1, assimpBone->mOffsetMatrix.b2, assimpBone->mOffsetMatrix.b3, assimpBone->mOffsetMatrix.b4,
							assimpBone->mOffsetMatrix.c1, assimpBone->mOffsetMatrix.c2, assimpBone->mOffsetMatrix.c3, assimpBone->mOffsetMatrix.c4,
							assimpBone->mOffsetMatrix.d1, assimpBone->mOffsetMatrix.d2, assimpBone->mOffsetMatrix.d3, assimpBone->mOffsetMatrix.d4
						)));

					for (unsigned int weightIndex = 0; weightIndex < assimpBone->mNumWeights; ++weightIndex)
					{
						const aiVertexWeight& assimpVertexWeight = assimpBone->mWeights[weightIndex];
						skeletalMesh->AddVertexBoneData(assimpVertexWeight.mVertexId, boneId, assimpVertexWeight.mWeight);
					}
				}

				aiNode* rootAssimpBone = GetRootBone(skeletalMesh->GetBoneNameToIdMap(), assimpScene->mRootNode);

				if (rootAssimpBone)
				{
					Matrix rootTransformation
					(
						rootAssimpBone->mTransformation.a1, rootAssimpBone->mTransformation.a2, rootAssimpBone->mTransformation.a3, rootAssimpBone->mTransformation.a4,
						rootAssimpBone->mTransformation.b1, rootAssimpBone->mTransformation.b2, rootAssimpBone->mTransformation.b3, rootAssimpBone->mTransformation.b4,
						rootAssimpBone->mTransformation.c1, rootAssimpBone->mTransformation.c2, rootAssimpBone->mTransformation.c3, rootAssimpBone->mTransformation.c4,
						rootAssimpBone->mTransformation.d1, rootAssimpBone->mTransformation.d2, rootAssimpBone->mTransformation.d3, rootAssimpBone->mTransformation.d4
					);

					skeletalMesh->GetArmature()->globalInverseTransform = rootTransformation.GetInverse();

					skeletalMesh->GetArmature()->root = skeletalMesh->GetBone(skeletalMesh->GetBoneId(rootAssimpBone->mName.C_Str()));
					skeletalMesh->GetArmature()->root->transformation = rootTransformation;
					SetupArmature(skeletalMesh, skeletalMesh->GetArmature()->root, rootAssimpBone);

					for (unsigned int assimpAnimationIndex = 0; assimpAnimationIndex < assimpScene->mNumAnimations; ++assimpAnimationIndex)
					{
						const aiAnimation* assimpAnimation = assimpScene->mAnimations[assimpAnimationIndex];
						SkeletalAnimation* skeletalAnimation = new SkeletalAnimation();

						skeletalAnimation->name = assimpAnimation->mName.C_Str();
						skeletalAnimation->animationNodeSize = assimpAnimation->mNumChannels;
						skeletalAnimation->animationNodes = new SkeletalAnimationNode * [skeletalAnimation->animationNodeSize];
						skeletalAnimation->duration = assimpAnimation->mDuration;
						skeletalAnimation->ticksPerSecond = assimpAnimation->mTicksPerSecond;

						for (unsigned int animationNodeIndex = 0; animationNodeIndex < skeletalAnimation->animationNodeSize; ++animationNodeIndex)
						{
							aiNodeAnim* assimpAnimationNode = assimpAnimation->mChannels[animationNodeIndex];

							SkeletalAnimationNode* skeletalAnimationNode = new SkeletalAnimationNode();
							skeletalAnimationNode->affectedBoneName = assimpAnimationNode->mNodeName.C_Str();

							skeletalAnimationNode->rotationKeySize = assimpAnimationNode->mNumRotationKeys;
							skeletalAnimationNode->rotationKeys = new AnimationQuaternionKey[skeletalAnimationNode->rotationKeySize];
							for (unsigned int rotationKeyIndex = 0; rotationKeyIndex < skeletalAnimationNode->rotationKeySize; ++rotationKeyIndex)
							{
								const aiQuatKey& assimpQuaternionKey = assimpAnimationNode->mRotationKeys[rotationKeyIndex];

								skeletalAnimationNode->rotationKeys[rotationKeyIndex].time = assimpQuaternionKey.mTime;
								skeletalAnimationNode->rotationKeys[rotationKeyIndex].value = Quaternion(assimpQuaternionKey.mValue.x, assimpQuaternionKey.mValue.y, assimpQuaternionKey.mValue.z, assimpQuaternionKey.mValue.w);
							}

							skeletalAnimationNode->positionKeySize = assimpAnimationNode->mNumPositionKeys;
							skeletalAnimationNode->positionKeys = new AnimationVectorKey[skeletalAnimationNode->positionKeySize];
							for (unsigned int positionKeyIndex = 0; positionKeyIndex < skeletalAnimationNode->positionKeySize; ++positionKeyIndex)
							{
								const aiVectorKey& assimpVectorKey = assimpAnimationNode->mPositionKeys[positionKeyIndex];

								skeletalAnimationNode->positionKeys[positionKeyIndex].time = assimpVectorKey.mTime;
								skeletalAnimationNode->positionKeys[positionKeyIndex].value = Vector3(assimpVectorKey.mValue.x, assimpVectorKey.mValue.y, assimpVectorKey.mValue.z);
							}

							skeletalAnimationNode->scalingKeySize = assimpAnimationNode->mNumScalingKeys;
							skeletalAnimationNode->scalingKeys = new AnimationVectorKey[skeletalAnimationNode->scalingKeySize];
							for (unsigned int scalingKeyIndex = 0; scalingKeyIndex < skeletalAnimationNode->scalingKeySize; ++scalingKeyIndex)
							{
								const aiVectorKey& assimpVectorKey = assimpAnimationNode->mScalingKeys[scalingKeyIndex];

								skeletalAnimationNode->scalingKeys[scalingKeyIndex].time = assimpVectorKey.mTime;
								skeletalAnimationNode->scalingKeys[scalingKeyIndex].value = Vector3(assimpVectorKey.mValue.x, assimpVectorKey.mValue.y, assimpVectorKey.mValue.z);
							}

							skeletalAnimation->AddSkeletalAnimationNode(animationNodeIndex, skeletalAnimationNode);
						}
						skeletalMesh->AddSkeletalAnimation(skeletalAnimation);
					}
				}
				staticMesh = skeletalMesh;
			}
			else
			{
				staticMesh = new StaticMesh();
			}

			staticMesh->SetName(assimpMesh->mName.C_Str());

			for(unsigned int vertexIndex = 0; vertexIndex < assimpMesh->mNumVertices; ++vertexIndex)
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

			if(assimpMesh->HasFaces())
			{
				for (unsigned int faceIndex = 0; faceIndex < assimpMesh->mNumFaces; faceIndex++)
				{
					aiFace& face = assimpMesh->mFaces[faceIndex];
					GOKNAR_CORE_ASSERT(face.mNumIndices == 3u, "ONLY TRIANGLE MESH FACES ARE SUPPORTED!");
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
					//assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, value);
					//material->SetAmbientReflectance(Vector3(value.r, value.g, value.b));
					material->SetAmbientReflectance(Vector3(1.f, 1.f, 1.f));

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

					float blendFunction = 0.f;
					assimpMaterial->Get(AI_MATKEY_BLEND_FUNC, blendFunction);

					aiString name;
					assimpMaterial->Get(AI_MATKEY_NAME, name);
					material->SetName(name.C_Str());

					Shader* shader = nullptr;

					aiString diffuseTexturePath;
					if (assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
					{
						shader = new Shader();
						gameScene->AddShader(shader);

						material->SetShader(shader);

						std::string imagePath = ContentDir;
						std::string diffuseImagePathStdString = std::string(diffuseTexturePath.C_Str());
						if (diffuseImagePathStdString.find(".fbm") != std::string::npos)
						{
							long long lastSlashIndex = path.find_last_of('/');
							if (lastSlashIndex != std::string::npos)
							{
								imagePath += path.substr(0, lastSlashIndex + 1);
							}
						}

						imagePath += diffuseTexturePath.C_Str();

						Image* image = engine->GetResourceManager()->GetContent<Image>(imagePath);
						if (image)
						{
							image->SetTextureUsage(TextureUsage::Diffuse);
							material->GetShader()->AddTextureImage(image);
						}
					}

					aiString normalTexturePath;
					if (assimpMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
					{
						if (!shader)
						{
							shader = new Shader();
							gameScene->AddShader(shader);
						}

						material->SetShader(shader);

						std::string normalImagePath = ContentDir;
						std::string normalImagePathStdString = std::string(normalTexturePath.C_Str());
						if (normalImagePathStdString.find(".fbm") != std::string::npos)
						{
							long long lastSlashIndex = path.find_last_of('/');
							if (lastSlashIndex != std::string::npos)
							{
								normalImagePath += path.substr(0, lastSlashIndex + 1);
							}
						}

						normalImagePath += normalTexturePath.C_Str();

						Image* image = engine->GetResourceManager()->GetContent<Image>(normalImagePath);
						image->SetTextureUsage(TextureUsage::Normal);
						shader->AddTextureImage(image);
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

	if (skeletalMesh)
	{
		return skeletalMesh;
	}
	else if (staticMesh)
	{
		return staticMesh;
	}

	return nullptr;
}