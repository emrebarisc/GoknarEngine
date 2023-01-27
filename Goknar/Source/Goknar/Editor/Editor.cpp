#include "pch.h"

#include "Editor.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Model/StaticMesh.h"

#include "EditorCameraController.h"
#include "EditorGroundGrid.h"

Editor::Editor()
{
}

Editor::~Editor()
{
	
}

void Editor::Init()
{
	editorCameraController_ = new EditorCameraController();
	editorGroundGrid_ = new EditorGroundGrid();
	editorGroundGrid_->Init();
}