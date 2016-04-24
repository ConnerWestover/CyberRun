#include "GameEntity.h"

using namespace DirectX;

GameEntity::GameEntity(Mesh* mesh, Material* mat, bool sky)
{
	// Save the mesh
	this->mesh = mesh;
	this->material = mat;
	skyBox = sky;

	// Set up transform
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	position = XMFLOAT3(0,0,0);
	rotation = XMFLOAT3(0,0,0);
	scale = XMFLOAT3(1,1,1);
}

GameEntity::~GameEntity(void)
{
}

// Update the world matrix
void GameEntity::UpdateWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotX = XMMatrixRotationX(rotation.x);
	XMMATRIX rotY = XMMatrixRotationY(rotation.y);
	XMMATRIX rotZ = XMMatrixRotationZ(rotation.z);
	XMMATRIX sc = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX total = sc * rotZ * rotY * rotX * trans;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(total));
}

void GameEntity::Draw(ID3D11DeviceContext * deviceContext, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	UpdateWorldMatrix();
	material->getVert()->SetMatrix4x4("world", worldMatrix);
	material->prepareMaterial(viewMatrix, projectionMatrix);
	mesh->Draw(deviceContext,skyBox);
}