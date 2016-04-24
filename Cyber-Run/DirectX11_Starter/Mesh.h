#pragma once

#include <d3d11.h>

#include "Vertex.h"

struct OBJTriangle
{
	int Position[3];
	int Normal[3];
	int UV[3];
};

class Mesh
{
public:
	Mesh(Vertex* vertArray, int numVerts, unsigned int* indexArray, int numIndices, ID3D11Device* device);
	Mesh(char* objFile, ID3D11Device* device, ID3D11RasterizerState* rasterState, ID3D11DepthStencilState* depthState);
	~Mesh(void);

	ID3D11Buffer* GetVertexBuffer() { return vb; }
	ID3D11Buffer* GetIndexBuffer() { return ib; }
	int GetIndexCount() { return numIndices; }

	void Draw(ID3D11DeviceContext* deviceContext, bool sky);
private:
	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	ID3D11RasterizerState* rasterState;
	ID3D11DepthStencilState* depthState;
	int numIndices;
	//bool skyBox;

	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	void CreateBuffers(Vertex* vertArray, int numVerts, unsigned int* indexArray, int numIndices, ID3D11Device* device);
};

