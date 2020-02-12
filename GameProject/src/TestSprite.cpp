#include "TestSprite.h"

#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Model/SpriteMesh.h"
#include "Goknar/Renderer/Texture.h"

TestSprite::TestSprite()
{
	Texture* texture = new Texture("./Content/Textures/T_Airplane.png");
	texture->SetName("texture0");
	SpriteMesh* spriteMesh = new SpriteMesh(texture);
	spriteMeshComponent_ = new MeshComponent(this);
	spriteMeshComponent_->SetMesh(spriteMesh);
}

TestSprite::~TestSprite()
{
}

void TestSprite::BeginGame()
{
}

void TestSprite::Tick(float deltaTime)
{
}
