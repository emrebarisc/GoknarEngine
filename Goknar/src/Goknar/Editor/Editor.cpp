#include "pch.h"

#include "Editor.h"

#include "EditorCameraController.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Model/StaticMesh.h"

Editor::Editor()
{
}

Editor::~Editor()
{

}

void Editor::Init()
{
	editorCameraController = new EditorCameraController();
}