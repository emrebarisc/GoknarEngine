#include "pch.h"

#include "SkeletalMeshUnit.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Scene.h"
#include "Materials/Material.h"
#include "Renderer/Shader.h"

#include "Managers/CameraManager.h"
#include "IO/IOManager.h"

SkeletalMeshUnit::SkeletalMeshUnit() :
	MeshUnit()
{
}

SkeletalMeshUnit::~SkeletalMeshUnit()
{
	delete vertexBoneDataArray_;
}

void SkeletalMeshUnit::PreInit()
{
	MeshUnit::PreInit();
}

void SkeletalMeshUnit::Init()
{
	MeshUnit::Init();
}

void SkeletalMeshUnit::PostInit()
{
	MeshUnit::PostInit();
}