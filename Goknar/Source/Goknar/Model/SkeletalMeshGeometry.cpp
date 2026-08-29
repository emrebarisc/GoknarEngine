#include "pch.h"

#include "SkeletalMeshGeometry.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Scene.h"
#include "Materials/Material.h"
#include "Renderer/Shader.h"

#include "Managers/CameraManager.h"
#include "IO/IOManager.h"

SkeletalMeshGeometry::SkeletalMeshGeometry() :
	MeshGeometry()
{
}

SkeletalMeshGeometry::~SkeletalMeshGeometry()
{
	delete vertexBoneDataArray_;
}

void SkeletalMeshGeometry::PreInit()
{
	MeshGeometry::PreInit();
}

void SkeletalMeshGeometry::Init()
{
	MeshGeometry::Init();
}

void SkeletalMeshGeometry::PostInit()
{
	MeshGeometry::PostInit();
}