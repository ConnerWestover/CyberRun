#pragma once

#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"

class GameEntity
{
public:
	GameEntity(Mesh* mesh, Material* mat, bool sky);
	~GameEntity(void);

	void UpdateWorldMatrix();

	void Move(float x, float y, float z)		{ position.x += x;	position.y += y;	position.z += z; }
	void Rotate(float x, float y, float z)		{ rotation.x += x;	rotation.y += y;	rotation.z += z; }

	void SetPosition(float x, float y, float z) { position.x = x;	position.y = y;		position.z = z; }
	void SetRotation(float x, float y, float z) { rotation.x = x;	rotation.y = y;		rotation.z = z; }
	void SetScale(float x, float y, float z)	{ scale.x = x;		scale.y = y;		scale.z = z; }
	
	DirectX::XMFLOAT3 position;

	Mesh* GetMesh() { return mesh; }
	DirectX::XMFLOAT4X4* GetWorldMatrix() { return &worldMatrix; }
	void Draw(ID3D11DeviceContext * deviceContext, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
private:

	Mesh* mesh;
	Material* material;
	
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

	bool skyBox;
};

