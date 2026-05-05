#include "pch.h"

#include "ModelLoader.h"

#include "Goknar/Application.h"
#include "Goknar/Contents/Content.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/SkeletalMeshUnit.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Log.h"

// Replaced Assimp includes with ufbx
#include "IO/ufbx.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>

#ifdef GOKNAR_PLATFORM_UNIX
std::string ConvertToLinuxPath(const std::string& input)
{
	std::string output;
	output.reserve(input.size());

	for (char c : input) {
		if (c == '\\')
		{
			c = '/';
		}

		output += c;
	}

	return output;
}
#endif

struct UnifiedVertex
{
	uint32_t posIndex{ 0 };
	uint32_t normIndex{ 0 };
	uint32_t uvIndex{ 0 };
	uint32_t colIndex{ 0 };

	bool operator==(const UnifiedVertex& other) const {
		return posIndex == other.posIndex &&
			normIndex == other.normIndex &&
			uvIndex == other.uvIndex &&
			colIndex == other.colIndex;
	}
};

struct UnifiedVertexHash {
	size_t operator()(const UnifiedVertex& v) const {
		size_t h = v.posIndex;
		h ^= v.normIndex + 0x9e3779b9 + (h << 6) + (h >> 2);
		h ^= v.uvIndex + 0x9e3779b9 + (h << 6) + (h >> 2);
		h ^= v.colIndex + 0x9e3779b9 + (h << 6) + (h >> 2);
		return h;
	}
};

Matrix ConvertMatrix(const ufbx_matrix& m)
{
	return Matrix(
		m.m00, m.m01, m.m02, m.m03,
		m.m10, m.m11, m.m12, m.m13,
		m.m20, m.m21, m.m22, m.m23,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

ufbx_node* GetRootBone(const BoneNameToIdMap* boneNameToIdMap, ufbx_node* node)
{
	if (boneNameToIdMap->find(node->name.data) != boneNameToIdMap->end())
	{
		return node;
	}

	for (size_t childIndex = 0; childIndex < node->children.count; ++childIndex)
	{
		ufbx_node* childNode = GetRootBone(boneNameToIdMap, node->children.data[childIndex]);
		if (childNode != nullptr)
		{
			return childNode;
		}
	}

	return nullptr;
}

void SetupArmature(SkeletalMesh* skeletalMesh, Bone* bone, ufbx_node* node)
{
	if (node->children.count == 0)
	{
		return;
	}

	const BoneNameToIdMap* boneNameToIdMap = skeletalMesh->GetBoneNameToIdMap();
	for (size_t childrenIndex = 0; childrenIndex < node->children.count; ++childrenIndex)
	{
		ufbx_node* childNode = node->children.data[childrenIndex];
		std::string childNodeName = childNode->name.data;

		if (boneNameToIdMap->find(childNodeName) != boneNameToIdMap->end())
		{
			Bone* childBone = skeletalMesh->GetBone(skeletalMesh->GetBoneId(childNodeName));

			ufbx_matrix localMat = ufbx_transform_to_matrix(&childNode->local_transform);
			childBone->transformation = ConvertMatrix(localMat);

			bone->children.push_back(childBone);
			SetupArmature(skeletalMesh, childBone, childNode);
		}
	}
}

Content* ModelLoader::LoadModel(const std::string& path)
{
	StaticMesh* staticMeshAsset = nullptr;
	SkeletalMesh* skeletalMeshAsset = nullptr;

	ufbx_load_opts opts = {};
	opts.generate_missing_normals = true;
	opts.target_axes = ufbx_axes_right_handed_y_up;
	opts.filename = ufbx_string{ path.c_str(), path.size() };

	ufbx_error error;
	std::vector<uint8_t> modelBytes;
	if (!DataEncryption::ReadBinaryFile(path, modelBytes))
	{
		GOKNAR_CORE_ERROR("\n\tError occured while loading the asset(%s)\n\tWhat went wrong: The file could not be read.", path.c_str());
		return nullptr;
	}

	ufbx_scene* scene = ufbx_load_memory(modelBytes.data(), modelBytes.size(), &opts, &error);

	if (scene)
	{
		bool sceneHasBones = false;
		for (size_t meshIndex = 0; meshIndex < scene->meshes.count; ++meshIndex)
		{
			if (scene->meshes.data[meshIndex]->skin_deformers.count > 0)
			{
				sceneHasBones = true;
				break;
			}
		}

		if (sceneHasBones)
		{
			skeletalMeshAsset = new SkeletalMesh();
		}
		else
		{
			staticMeshAsset = new StaticMesh();
		}

		for (size_t meshIndex = 0; meshIndex < scene->meshes.count; ++meshIndex)
		{
			ufbx_mesh* ufbxMesh = scene->meshes.data[meshIndex];
			const bool hasBones = sceneHasBones && ufbxMesh->skin_deformers.count > 0;

			size_t numMaterials = ufbxMesh->materials.count;
			if (numMaterials == 0) numMaterials = 1; // Fallback to 1 submesh if no materials exist

			struct SubMeshData {
				MeshUnit* meshUnit = nullptr;
				SkeletalMeshUnit* skeletalMeshUnit = nullptr;
				std::unordered_map<UnifiedVertex, uint32_t, UnifiedVertexHash> uniqueVertices;
				std::vector<VertexData> tempVertices;
				uint32_t currentVertexId = 0;
				std::vector<size_t> originalFaceIndices;
				std::vector<uint32_t> unifiedIndices;
				std::vector<std::vector<uint32_t>> posToVertexIndices;
			};

			std::vector<SubMeshData> subMeshes(numMaterials);

			for (size_t i = 0; i < numMaterials; ++i)
			{
				if (hasBones)
				{
					subMeshes[i].skeletalMeshUnit = new SkeletalMeshUnit();
					subMeshes[i].meshUnit = subMeshes[i].skeletalMeshUnit;
				}
				else
				{
					subMeshes[i].meshUnit = new MeshUnit();
				}

				std::string subMeshName = ufbxMesh->name.data;
				if (numMaterials > 1)
				{
					subMeshName += "_" + std::to_string(i);
				}
				subMeshes[i].meshUnit->SetName(subMeshName);
				subMeshes[i].unifiedIndices.resize(ufbxMesh->num_indices, 0xFFFFFFFF);

				if (hasBones)
				{
					subMeshes[i].posToVertexIndices.resize(ufbxMesh->num_vertices);
				}
			}

			// 1. Distribute faces based on material index
			for (size_t faceIndex = 0; faceIndex < ufbxMesh->faces.count; ++faceIndex)
			{
				size_t matIndex = 0;
				if (ufbxMesh->face_material.data && faceIndex < ufbxMesh->face_material.count)
				{
					int32_t matId = ufbxMesh->face_material.data[faceIndex];
					if (matId >= 0 && matId < (int32_t)numMaterials)
					{
						matIndex = matId;
					}
				}
				subMeshes[matIndex].originalFaceIndices.push_back(faceIndex);
			}

			// 2. Build vertices, indices, and materials for each submesh
			for (size_t matIndex = 0; matIndex < numMaterials; ++matIndex)
			{
				SubMeshData& subMesh = subMeshes[matIndex];

				if (subMesh.originalFaceIndices.empty())
				{
					delete subMesh.meshUnit;
					subMesh.meshUnit = nullptr;
					subMesh.skeletalMeshUnit = nullptr;
					continue;
				}

				// Collect vertices uniquely per submesh
				for (size_t faceIndex : subMesh.originalFaceIndices)
				{
					ufbx_face face = ufbxMesh->faces.data[faceIndex];
					for (size_t i = 0; i < face.num_indices; ++i)
					{
						uint32_t index_in_mesh = face.index_begin + i;

						if (subMesh.unifiedIndices[index_in_mesh] != 0xFFFFFFFF)
						{
							continue;
						}

						UnifiedVertex key;
						key.posIndex = ufbxMesh->vertex_position.indices.data[index_in_mesh];

						if (ufbxMesh->vertex_normal.exists)
							key.normIndex = ufbxMesh->vertex_normal.indices.data[index_in_mesh];
						if (ufbxMesh->vertex_uv.exists)
							key.uvIndex = ufbxMesh->vertex_uv.indices.data[index_in_mesh];
						if (ufbxMesh->vertex_color.exists)
							key.colIndex = ufbxMesh->vertex_color.indices.data[index_in_mesh];

						auto it = subMesh.uniqueVertices.find(key);
						if (it != subMesh.uniqueVertices.end())
						{
							subMesh.unifiedIndices[index_in_mesh] = it->second;
						}
						else
						{
							subMesh.unifiedIndices[index_in_mesh] = subMesh.currentVertexId;
							subMesh.uniqueVertices[key] = subMesh.currentVertexId;

							ufbx_vec3 pos = ufbxMesh->vertex_position.values.data[key.posIndex];

							ufbx_vec3 norm = { 0, 0, 0 };
							if (ufbxMesh->vertex_normal.exists)
								norm = ufbxMesh->vertex_normal.values.data[key.normIndex];

							Vector4 col = Vector4(1.f);
							if (ufbxMesh->vertex_color.exists)
							{
								ufbx_vec4 c = ufbxMesh->vertex_color.values.data[key.colIndex];
								col = Vector4(c.x, c.y, c.z, c.w);
							}

							Vector2 uv = Vector2::ZeroVector;
							if (ufbxMesh->vertex_uv.exists)
							{
								ufbx_vec2 u = ufbxMesh->vertex_uv.values.data[key.uvIndex];
								uv = Vector2(u.x, u.y);
							}

							subMesh.tempVertices.push_back(VertexData(
								Vector3(pos.x, pos.y, pos.z),
								Vector3(norm.x, norm.y, norm.z),
								col,
								uv
							));

							if (hasBones)
							{
								subMesh.posToVertexIndices[key.posIndex].push_back(subMesh.currentVertexId);
							}

							subMesh.currentVertexId++;
						}
					}
				}

				// Inject temp vertices
				for (const VertexData& vertexData : subMesh.tempVertices)
				{
					subMesh.meshUnit->AddVertexData(vertexData);
				}

				if (subMesh.meshUnit->GetVertexCount() <= 0)
				{
					delete subMesh.meshUnit;
					subMesh.meshUnit = nullptr;
					subMesh.skeletalMeshUnit = nullptr;
					continue;
				}

				// Distribute Skin/Bone weights
				if (hasBones && skeletalMeshAsset && subMesh.skeletalMeshUnit)
				{
					subMesh.skeletalMeshUnit->ResizeVertexToBonesArray(subMesh.currentVertexId);

					ufbx_skin_deformer* skin = ufbxMesh->skin_deformers.data[0];

					for (auto& vec : subMesh.posToVertexIndices)
					{
						if (!vec.empty()) {
							std::sort(vec.begin(), vec.end());
							vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
						}
					}

					for (size_t boneIndex = 0; boneIndex < skin->clusters.count; ++boneIndex)
					{
						ufbx_skin_cluster* cluster = skin->clusters.data[boneIndex];
						const std::string boneName = cluster->bone_node->name.data;
						const BoneNameToIdMap* boneNameToIdMap = skeletalMeshAsset->GetBoneNameToIdMap();
						if (boneNameToIdMap->find(boneName) == boneNameToIdMap->end())
						{
							skeletalMeshAsset->GetBoneId(boneName);
							skeletalMeshAsset->AddBone(new Bone(boneName, ConvertMatrix(cluster->geometry_to_bone)));
						}

						const unsigned int boneId = skeletalMeshAsset->GetBoneId(boneName);

						for (size_t weightIndex = 0; weightIndex < cluster->vertices.count; ++weightIndex)
						{
							uint32_t posIndex = cluster->vertices.data[weightIndex];
							float weight = cluster->weights.data[weightIndex];

							if (posIndex < subMesh.posToVertexIndices.size())
							{
								for (uint32_t vertexId : subMesh.posToVertexIndices[posIndex])
								{
									subMesh.skeletalMeshUnit->AddVertexBoneData(vertexId, boneId, weight);
								}
							}
						}
					}
				}

				// Triangulate local face data
				for (size_t faceIndex : subMesh.originalFaceIndices)
				{
					ufbx_face face = ufbxMesh->faces.data[faceIndex];
					uint32_t triIndices[256];
					uint32_t triangleCount = ufbx_triangulate_face(triIndices, 256, ufbxMesh, face);

					for (uint32_t triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
					{
						uint32_t originalMeshIndex0 = triIndices[triangleIndex * 3 + 0];
						uint32_t originalMeshIndex1 = triIndices[triangleIndex * 3 + 1];
						uint32_t originalMeshIndex2 = triIndices[triangleIndex * 3 + 2];

						uint32_t i0 = subMesh.unifiedIndices[originalMeshIndex0];
						uint32_t i1 = subMesh.unifiedIndices[originalMeshIndex1];
						uint32_t i2 = subMesh.unifiedIndices[originalMeshIndex2];
						subMesh.meshUnit->AddFace(Face(i0, i1, i2));
					}
				}

				Material* material = new Material();

				if (matIndex < ufbxMesh->materials.count)
				{
					ufbx_material* ufbxMaterial = ufbxMesh->materials.data[matIndex];
					if (ufbxMaterial)
					{
						material->SetAmbientOcclusion(1.f);
						material->SetMetallic(0.f);
						material->SetRoughness(0.5f);

						if (ufbxMaterial->pbr.base_color.has_value)
						{
							material->SetBaseColor(Vector3(ufbxMaterial->pbr.base_color.value_vec3.x, ufbxMaterial->pbr.base_color.value_vec3.y, ufbxMaterial->pbr.base_color.value_vec3.z));
						}

						if (ufbxMaterial->pbr.emission_color.has_value)
						{
							material->SetEmisiveColor(Vector3(
								ufbxMaterial->pbr.emission_color.value_vec3.x,
								ufbxMaterial->pbr.emission_color.value_vec3.y,
								ufbxMaterial->pbr.emission_color.value_vec3.z));
						}

						if (ufbxMaterial->pbr.metalness.has_value)
						{
							material->SetMetallic(ufbxMaterial->pbr.metalness.value_real);
						}

						if (ufbxMaterial->pbr.roughness.has_value)
						{
							material->SetRoughness(ufbxMaterial->pbr.roughness.value_real);
						}

						if (ufbxMaterial->pbr.ambient_occlusion.has_value)
						{
							material->SetAmbientOcclusion(ufbxMaterial->pbr.ambient_occlusion.value_real);
						}

						material->SetShadingModel(MaterialShadingModel::Default);
						material->SetName(ufbxMaterial->name.data);

						auto LoadMaterialTexture = [&](const ufbx_texture* ufbxTexture, TextureUsage textureUsage)
						{
							if (!ufbxTexture)
							{
								return;
							}

							std::string imagePath = "";
							std::string texturePath = ufbxTexture->filename.data;

							if (texturePath.find(".fbm") != std::string::npos)
							{
								long long lastSlashIndex = path.find_last_of('/');
								if (lastSlashIndex != std::string::npos)
								{
									imagePath += path.substr(0, lastSlashIndex + 1);
								}
							}

							imagePath += texturePath;

#ifdef GOKNAR_PLATFORM_UNIX
							imagePath = ConvertToLinuxPath(imagePath);
#endif

							Image* image = engine->GetResourceManager()->GetContent<Image>(imagePath);
							if (image)
							{
								image->SetTextureUsage(textureUsage);
								material->AddTextureImage(image);
							}
						};

						LoadMaterialTexture(ufbxMaterial->pbr.base_color.texture, TextureUsage::Diffuse);
						LoadMaterialTexture(ufbxMaterial->pbr.normal_map.texture, TextureUsage::Normal);
						LoadMaterialTexture(ufbxMaterial->pbr.emission_color.texture, TextureUsage::Emmisive);
						LoadMaterialTexture(ufbxMaterial->pbr.ambient_occlusion.texture, TextureUsage::AmbientOcclusion);
						LoadMaterialTexture(ufbxMaterial->pbr.metalness.texture, TextureUsage::Metallic);
						LoadMaterialTexture(ufbxMaterial->pbr.roughness.texture, TextureUsage::Roughness);
					}
				}

				subMesh.meshUnit->SetMaterial(material);

				if (sceneHasBones && skeletalMeshAsset)
				{
					skeletalMeshAsset->AddMesh(subMesh.skeletalMeshUnit);
				}
				else if (staticMeshAsset)
				{
					staticMeshAsset->AddMesh(subMesh.meshUnit);
				}
			}
		}

		if (skeletalMeshAsset && skeletalMeshAsset->GetBoneSize() > 0)
		{
			ufbx_node* rootBoneNode = GetRootBone(skeletalMeshAsset->GetBoneNameToIdMap(), scene->root_node);
			if (rootBoneNode)
			{
				ufbx_matrix rootLocalMat = ufbx_transform_to_matrix(&rootBoneNode->local_transform);
				Matrix rootTransformation = ConvertMatrix(rootLocalMat);

				skeletalMeshAsset->GetArmature()->globalInverseTransform = rootTransformation.GetInverse();

				skeletalMeshAsset->GetArmature()->root = skeletalMeshAsset->GetBone(skeletalMeshAsset->GetBoneId(rootBoneNode->name.data));
				skeletalMeshAsset->GetArmature()->root->transformation = rootTransformation;
				SetupArmature(skeletalMeshAsset, skeletalMeshAsset->GetArmature()->root, rootBoneNode);

				for (size_t animIndex = 0; animIndex < scene->anim_stacks.count; ++animIndex)
				{
					ufbx_anim_stack* animStack = scene->anim_stacks.data[animIndex];
					SkeletalAnimation* skeletalAnimation = new SkeletalAnimation();

					skeletalAnimation->name = animStack->name.data;

					double durationInSeconds = animStack->time_end - animStack->time_begin;
					double fps = 30.0;
					int keyframeCount = (int)(durationInSeconds * fps);
					if (keyframeCount <= 0)
					{
						keyframeCount = 1;
					}

					skeletalAnimation->duration = durationInSeconds;
					skeletalAnimation->ticksPerSecond = fps;
					skeletalAnimation->maxKeyframeCount = keyframeCount;

					int channelCount = skeletalMeshAsset->GetBoneNameToIdMap()->size();
					skeletalAnimation->animationKeyframeCount = channelCount;
					skeletalAnimation->animationKeyframes = new SkeletalAnimationKeyframe * [channelCount];

					int channelIndex = 0;
					for (const auto& bonePair : *(skeletalMeshAsset->GetBoneNameToIdMap()))
					{
						std::string boneName = bonePair.first;
						ufbx_node* animNode = ufbx_find_node(scene, boneName.c_str());
						if (!animNode)
						{
							continue;
						}

						SkeletalAnimationKeyframe* keyframe = new SkeletalAnimationKeyframe();
						keyframe->affectedBoneName = boneName;

						keyframe->positionKeySize = keyframeCount;
						keyframe->positionKeys = new AnimationVectorKey[keyframeCount];
						keyframe->rotationKeySize = keyframeCount;
						keyframe->rotationKeys = new AnimationQuaternionKey[keyframeCount];
						keyframe->scalingKeySize = keyframeCount;
						keyframe->scalingKeys = new AnimationVectorKey[keyframeCount];

						for (int keyframeIndex = 0; keyframeIndex < keyframeCount; ++keyframeIndex)
						{
							double timeInSeconds = (double)keyframeIndex / fps;
							ufbx_transform transform = ufbx_evaluate_transform(animStack->anim, animNode, animStack->time_begin + timeInSeconds);

							keyframe->positionKeys[keyframeIndex].time = timeInSeconds;
							keyframe->positionKeys[keyframeIndex].value = Vector3(transform.translation.x, transform.translation.y, transform.translation.z);

							keyframe->rotationKeys[keyframeIndex].time = timeInSeconds;
							keyframe->rotationKeys[keyframeIndex].value = Quaternion(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

							keyframe->scalingKeys[keyframeIndex].time = timeInSeconds;
							keyframe->scalingKeys[keyframeIndex].value = Vector3(transform.scale.x, transform.scale.y, transform.scale.z);
						}

						skeletalAnimation->AddSkeletalAnimationKeyframe(channelIndex, keyframe);
						channelIndex++;
					}

					skeletalAnimation->animationKeyframeCount = channelIndex;
					skeletalMeshAsset->AddSkeletalAnimation(skeletalAnimation);
				}
			}
		}

		ufbx_free_scene(scene);
	}
	else
	{
		GOKNAR_CORE_ERROR("\n\tError occured while loading the asset(%s)\n\tWhat went wrong: %s", path.c_str(), error.description.data);
	}

	if (skeletalMeshAsset)
	{
		return skeletalMeshAsset;
	}
	if (staticMeshAsset)
	{
		return staticMeshAsset;
	}

	return nullptr;
}
