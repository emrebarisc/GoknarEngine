#include "pch.h"

#include "ModelLoader.h"

#include "Goknar/Application.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Log.h"

// Replaced Assimp includes with ufbx
#include "IO/ufbx.h"

#include <vector>

#include <unordered_map>
#include <algorithm>

#ifdef GOKNAR_PLATFORM_UNIX
std::string ConvertToLinuxPath(const std::string& input)
{
    std::string output;
    output.reserve(input.size());

    for (char c : input) {
		if(c == '\\')
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

// Helper to convert ufbx 3x4 affine matrix to Goknar 4x4 Matrix
Matrix ConvertMatrix(const ufbx_matrix& m)
{
	return Matrix(
		m.m00, m.m01, m.m02, m.m03,
		m.m10, m.m11, m.m12, m.m13,
		m.m20, m.m21, m.m22, m.m23,
		0.0f,  0.0f,  0.0f,  1.0f
	);
}

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
            
            // Convert the local transform of the node
			ufbx_matrix localMat = ufbx_transform_to_matrix(&childNode->local_transform);
			childBone->transformation = ConvertMatrix(localMat);
			
            bone->children.push_back(childBone);
			SetupArmature(skeletalMesh, childBone, childNode);
		}
	}
}

StaticMesh* ModelLoader::LoadModel(const std::string& path)
{
	StaticMesh* staticMesh = nullptr;
	SkeletalMesh* skeletalMesh = nullptr;

	ufbx_load_opts opts = {};
	opts.generate_missing_normals = true;
	opts.target_axes = ufbx_axes_right_handed_y_up; 

	ufbx_error error;
	ufbx_scene* scene = ufbx_load_file(path.c_str(), &opts, &error);
	
	if (scene)
	{
		for (size_t meshIndex = 0; meshIndex < scene->meshes.count; ++meshIndex)
		{
			ufbx_mesh* ufbxMesh = scene->meshes.data[meshIndex];

			bool hasBones = ufbxMesh->skin_deformers.count > 0;
			if (hasBones)
			{
				skeletalMesh = new SkeletalMesh();
				staticMesh = skeletalMesh;
			}
			else
			{
				staticMesh = new StaticMesh();
			}

			staticMesh->SetName(ufbxMesh->name.data);

			// --- 1. GATHER UNIQUE VERTICES (Don't add to engine yet!) ---
			std::unordered_map<UnifiedVertex, uint32_t, UnifiedVertexHash> uniqueVertices;
			std::vector<uint32_t> unifiedIndices(ufbxMesh->num_indices);
			std::vector<VertexData> tempVertices; // Store temporarily
			uint32_t currentVertexId = 0;

			for (size_t i = 0; i < ufbxMesh->num_indices; ++i)
			{
				UnifiedVertex key;
				key.posIndex = ufbxMesh->vertex_position.indices.data[i];

				if (ufbxMesh->vertex_normal.exists)
					key.normIndex = ufbxMesh->vertex_normal.indices.data[i];
				if (ufbxMesh->vertex_uv.exists)
					key.uvIndex = ufbxMesh->vertex_uv.indices.data[i];
				if (ufbxMesh->vertex_color.exists)
					key.colIndex = ufbxMesh->vertex_color.indices.data[i];

				auto it = uniqueVertices.find(key);
				if (it != uniqueVertices.end())
				{
					unifiedIndices[i] = it->second;
				}
				else
				{
					unifiedIndices[i] = currentVertexId;
					uniqueVertices[key] = currentVertexId;

					ufbx_vec3 pos = ufbxMesh->vertex_position.values.data[key.posIndex];

					ufbx_vec3 norm = { 0, 0, 0 };
					if (ufbxMesh->vertex_normal.exists)
						norm = ufbxMesh->vertex_normal.values.data[key.normIndex];

					Vector4 col = Vector4(1.f);
					if (ufbxMesh->vertex_color.exists) {
						ufbx_vec4 c = ufbxMesh->vertex_color.values.data[key.colIndex];
						col = Vector4(c.x, c.y, c.z, c.w);
					}

					Vector2 uv = Vector2::ZeroVector;
					if (ufbxMesh->vertex_uv.exists) {
						ufbx_vec2 u = ufbxMesh->vertex_uv.values.data[key.uvIndex];
						uv = Vector2(u.x, u.y);
					}

					tempVertices.push_back(VertexData(
						Vector3(pos.x, pos.y, pos.z),
						Vector3(norm.x, norm.y, norm.z),
						col,
						uv
					));

					currentVertexId++;
				}
			}

			// --- 2. BONES AND SKINNED ANIMATION (Mirroring Assimp's flow) ---
			if (hasBones)
			{
				// Resize FIRST, before AddVertexData is called!
				skeletalMesh->ResizeVertexToBonesArray(currentVertexId);

				ufbx_skin_deformer* skin = ufbxMesh->skin_deformers.data[0];

				std::vector<std::vector<uint32_t>> posToVertexIndices(ufbxMesh->num_vertices);
				for (size_t i = 0; i < ufbxMesh->num_indices; ++i)
				{
					uint32_t compactedIndex = unifiedIndices[i];
					uint32_t posIndex = ufbxMesh->vertex_position.indices.data[i];
					posToVertexIndices[posIndex].push_back(compactedIndex);
				}

				for (auto& vec : posToVertexIndices)
				{
					std::sort(vec.begin(), vec.end());
					vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
				}

				for (size_t boneIndex = 0; boneIndex < skin->clusters.count; ++boneIndex)
				{
					ufbx_skin_cluster* cluster = skin->clusters.data[boneIndex];
					unsigned int boneId = skeletalMesh->GetBoneId(cluster->bone_node->name.data);

					skeletalMesh->AddBone(new Bone(cluster->bone_node->name.data, ConvertMatrix(cluster->geometry_to_bone)));

					for (size_t weightIndex = 0; weightIndex < cluster->vertices.count; ++weightIndex)
					{
						uint32_t posIndex = cluster->vertices.data[weightIndex];
						float weight = cluster->weights.data[weightIndex];

						for (uint32_t vertexId : posToVertexIndices[posIndex])
						{
							skeletalMesh->AddVertexBoneData(vertexId, boneId, weight);
						}
					}
				}

				// KEEP your existing GetRootBone and Animation Keyframe loops right here!
				ufbx_node* rootBoneNode = GetRootBone(skeletalMesh->GetBoneNameToIdMap(), scene->root_node);
				if (rootBoneNode)
				{
					ufbx_matrix rootLocalMat = ufbx_transform_to_matrix(&rootBoneNode->local_transform);
					Matrix rootTransformation = ConvertMatrix(rootLocalMat);

					skeletalMesh->GetArmature()->globalInverseTransform = rootTransformation.GetInverse();

					skeletalMesh->GetArmature()->root = skeletalMesh->GetBone(skeletalMesh->GetBoneId(rootBoneNode->name.data));
					skeletalMesh->GetArmature()->root->transformation = rootTransformation;
					SetupArmature(skeletalMesh, skeletalMesh->GetArmature()->root, rootBoneNode);

					// [Your scene->anim_stacks loop goes here]
				}
			}

			// --- 3. APPLY VERTEX DATA ---
			// Now that the bones are fully allocated, safely push to the engine
			for (const VertexData& vd : tempVertices)
			{
				staticMesh->AddVertexData(vd);
			}

			// --- 4. TRIANGULATE FACES ---
			for (size_t faceIndex = 0; faceIndex < ufbxMesh->faces.count; ++faceIndex)
			{
				ufbx_face face = ufbxMesh->faces.data[faceIndex];
				uint32_t tri_indices[256];
				uint32_t num_tris = ufbx_triangulate_face(tri_indices, 256, ufbxMesh, face);

				for (uint32_t t = 0; t < num_tris; t++)
				{
					uint32_t i0 = unifiedIndices[tri_indices[t * 3 + 0]];
					uint32_t i1 = unifiedIndices[tri_indices[t * 3 + 1]];
					uint32_t i2 = unifiedIndices[tri_indices[t * 3 + 2]];
					staticMesh->AddFace(Face(i0, i1, i2));
				}
			}

			Material* material = new Material();

			if (ufbxMesh->materials.count > 0)
			{
				ufbx_material* ufbxMaterial = ufbxMesh->materials.data[0];

				if (ufbxMaterial)
				{
					material->SetAmbientReflectance(Vector3(1.f, 1.f, 1.f));

					if (ufbxMaterial->pbr.base_color.has_value) {
						material->SetBaseColor(Vector3(ufbxMaterial->pbr.base_color.value_vec3.x, ufbxMaterial->pbr.base_color.value_vec3.y, ufbxMaterial->pbr.base_color.value_vec3.z));
					}

					if (ufbxMaterial->pbr.specular_color.has_value) {
						material->SetSpecularReflectance(Vector3(ufbxMaterial->pbr.specular_color.value_vec3.x, ufbxMaterial->pbr.specular_color.value_vec3.y, ufbxMaterial->pbr.specular_color.value_vec3.z));
					}

					material->SetShadingModel(MaterialShadingModel::Default);
					material->SetName(ufbxMaterial->name.data);

					if (ufbxMaterial->pbr.base_color.texture)
					{
						std::string imagePath = "";
						std::string diffuseTexturePath = ufbxMaterial->pbr.base_color.texture->filename.data;

						if (diffuseTexturePath.find(".fbm") != std::string::npos)
						{
							long long lastSlashIndex = path.find_last_of('/');
							if (lastSlashIndex != std::string::npos)
							{
								imagePath += path.substr(0, lastSlashIndex + 1);
							}
						}

						imagePath += diffuseTexturePath;

#ifdef GOKNAR_PLATFORM_UNIX
						imagePath = ConvertToLinuxPath(imagePath);
#endif

						Image* image = engine->GetResourceManager()->GetContent<Image>(imagePath);
						if (image)
						{
							image->SetTextureUsage(TextureUsage::Diffuse);
							material->AddTextureImage(image);
						}
					}

					if (ufbxMaterial->pbr.normal_map.texture)
					{
						std::string normalImagePath = ContentDir;
						std::string normalTexturePath = ufbxMaterial->pbr.normal_map.texture->filename.data;
						
                        if (normalTexturePath.find(".fbm") != std::string::npos)
						{
							long long lastSlashIndex = path.find_last_of('/');
							if (lastSlashIndex != std::string::npos)
							{
								normalImagePath += path.substr(0, lastSlashIndex + 1);
							}
						}

						normalImagePath += normalTexturePath;

#ifdef GOKNAR_PLATFORM_UNIX
						normalImagePath = ConvertToLinuxPath(normalImagePath);
#endif

						Image* image = engine->GetResourceManager()->GetContent<Image>(normalImagePath);
						if(image)
						{
							image->SetTextureUsage(TextureUsage::Normal);
							material->AddTextureImage(image);
						}
					}
				}
			}

			staticMesh->SetMaterial(material);
		}
        
        ufbx_free_scene(scene);
	}
	else
	{
		GOKNAR_CORE_ERROR("Error occured while loading the asset({}). What went wrong: {}", path, error.description.data);
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