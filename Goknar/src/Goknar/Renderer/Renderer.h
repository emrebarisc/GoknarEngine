#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Types.h"

#include "Goknar/Model/Mesh.h"

#include "glad/glad.h"

#include <vector>

class GOKNAR_API DynamicMesh;
class GOKNAR_API StaticMesh;
class GOKNAR_API DynamicMeshInstance;
class GOKNAR_API StaticMeshInstance;
class GOKNAR_API ShadowManager;

class GOKNAR_API Renderer
{
public:
	Renderer();
	~Renderer();

	void SetStaticBufferData();
	void SetDynamicBufferData();
	void SetBufferData();
	void Init();

	void Render();

	void AddStaticMeshToRenderer(StaticMesh* object);
	void AddStaticMeshInstance(StaticMeshInstance* object);
	void RemoveStaticMeshInstance(StaticMeshInstance* object);

	void AddDynamicMeshToRenderer(DynamicMesh* object);
	void AddDynamicMeshInstance(DynamicMeshInstance* object);
	void RemoveDynamicMeshInstance(DynamicMeshInstance* object);

	void UpdateDynamicMeshVertex(const DynamicMesh* object, int vertexIndex, const VertexData& newVertexData);

	ShadowManager* GetShadowManager()
	{
		return shadowManager_;
	}

	void SetIsRenderingOnlyDepth(bool isRenderingOnlyDepth)
	{
		isRenderingOnlyDepth_ = isRenderingOnlyDepth;
	}

	bool GetIsRenderingOnlyDepth()
	{
		return isRenderingOnlyDepth_;
	}

	void BindShadowTextures(Shader* shader);
	void SetShaderShadowVariables(Shader* shader);

private:
	void BindStaticVBO();
	void BindDynamicVBO();
	void SetAttribPointers();

	std::vector<StaticMesh*> staticMeshes_;
	std::vector<DynamicMesh*> dynamicMeshes_;

	std::vector<StaticMeshInstance*> opaqueStaticMeshInstances_;
	//TODO: Is it really necessary to hold masked objects as a seperate array?
	std::vector<StaticMeshInstance*> maskedStaticMeshInstances_;
	std::vector<StaticMeshInstance*> translucentStaticMeshInstances_;

	std::vector<DynamicMeshInstance*> opaqueDynamicMeshInstances_;
	//TODO: Is it really necessary to hold masked objects as a seperate array?
	std::vector<DynamicMeshInstance*> maskedDynamicMeshInstances_;
	std::vector<DynamicMeshInstance*> translucentDynamicMeshInstances_;

	ShadowManager* shadowManager_;

	unsigned int totalStaticMeshVertexSize_;
	unsigned int totalStaticMeshFaceSize_;
	
	unsigned int totalDynamicMeshVertexSize_;
	unsigned int totalDynamicMeshFaceSize_;

	int totalStaticMeshCount_;
	int totalDynamicMeshCount_;

	GEuint staticVertexBufferId_;
	GEuint staticIndexBufferId_;

	GEuint dynamicVertexBufferId_;
	GEuint dynamicIndexBufferId_;

	bool isRenderingOnlyDepth_;
};

#endif
