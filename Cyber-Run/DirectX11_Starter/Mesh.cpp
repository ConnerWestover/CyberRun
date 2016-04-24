#include "Mesh.h"
#include <DirectXMath.h>
#include <vector>
#include <fstream>

using namespace DirectX;

Mesh::Mesh(Vertex* vertArray, int numVerts, unsigned int* indexArray, int numIndices, ID3D11Device* device)
{
	CalculateTangents(vertArray, numVerts, indexArray, numIndices);
	CreateBuffers(vertArray, numVerts, indexArray, numIndices, device);
}

Mesh::Mesh(char* objFile, ID3D11Device* device,
	ID3D11RasterizerState* _rasterState, ID3D11DepthStencilState* _depthState)
{
	rasterState = _rasterState;
	depthState = _depthState;
	// String to hold a single line
	char chars[512];

	// Vector to hold the verts
	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> uvs;
	std::vector<OBJTriangle> triangles;

	// Amounts
	int numVerts = 0;
	int numNormals = 0;
	int numUVs = 0;
	int numTriangles = 0;

	// File input
	std::ifstream obj(objFile);

	// Scan the file for info
	if (obj.is_open())
	{
		while (obj.good())
		{
			// Get the first 2 characters of a line
			obj.getline(chars, 512);
			if (chars[0] == 'v' && chars[1] == 'n') numNormals++;
			else if (chars[0] == 'v' && chars[1] == 't') numUVs++;
			else if (chars[0] == 'v') numVerts++;
			else if (chars[0] == 'f') numTriangles++;
		}
	}

	// Reset position
	obj.clear();
	obj.seekg(0, obj.beg);

	positions.resize(numVerts);
	normals.resize(numNormals);
	uvs.resize(numUVs);
	triangles.resize(numTriangles * 3);

	// Set up counts
	int vertCounter = 0;
	int normalCounter = 0;
	int uvCounter = 0;
	int triangleCounter = 0;

	// Check for successful open
	if (obj.is_open())
	{
		// Still good?
		while (obj.good())
		{
			// Get the line
			obj.getline(chars, 512);

			// Check the type of line
			if (chars[0] == 'v' && chars[1] == 'n')
			{
				sscanf_s(
					chars,
					"vn %f %f %f",
					&normals[normalCounter].x,
					&normals[normalCounter].y,
					&normals[normalCounter].z);
				normalCounter++;
			}
			else if (chars[0] == 'v' && chars[1] == 't')
			{
				sscanf_s(
					chars,
					"vt %f %f",
					&uvs[uvCounter].x,
					&uvs[uvCounter].y);
				uvCounter++;
			}
			else if (chars[0] == 'v')
			{
				sscanf_s(
					chars,
					"v %f %f %f",
					&positions[vertCounter].x,
					&positions[vertCounter].y,
					&positions[vertCounter].z);
				vertCounter++;
			}
			else if (chars[0] == 'f')
			{
				sscanf_s(
					chars,
					"f %d/%d/%d %d/%d/%d %d/%d/%d",
					&triangles[triangleCounter].Position[0],
					&triangles[triangleCounter].UV[0],
					&triangles[triangleCounter].Normal[0],
					&triangles[triangleCounter].Position[1],
					&triangles[triangleCounter].UV[1],
					&triangles[triangleCounter].Normal[1],
					&triangles[triangleCounter].Position[2],
					&triangles[triangleCounter].UV[2],
					&triangles[triangleCounter].Normal[2]);
				triangleCounter++;
			}
		}

		// Close
		obj.close();

		// Make a vector for the verts
		std::vector<Vertex> verts(triangleCounter * 3);
		std::vector<UINT> indices(triangleCounter * 3);

		// Get the verts ready (OBJ indices are 1-based!)
		UINT indexCounter = 0;
		for (int t = 0; t < triangleCounter; t++)
		{
			for (int i = 0; i < 3; i++)
			{
				// Set up verts
				int index = t * 3 + i;
				verts[index].Position = positions[triangles[t].Position[i] - 1];
				verts[index].Normal = normals[triangles[t].Normal[i] - 1];
				verts[index].UV = uvs[triangles[t].UV[i] - 1];

				// Set up indices too
				indices[index] = indexCounter;
				indexCounter++;
			}
		}

		// Create the buffers
		CalculateTangents(&verts[0], triangleCounter * 3, &indices[0], triangleCounter * 3);
		CreateBuffers(&verts[0], triangleCounter * 3, &indices[0], triangleCounter * 3, device);
	}
}


Mesh::~Mesh(void)
{
	vb->Release(); vb = 0;
	ib->Release(); ib = 0;
}

// Calculates the tangents of the vertices in a mesh
// Code adapted from: http://www.terathon.com/code/tangent.html
void Mesh::CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices)
{
	// Reset tangents
	for (int i = 0; i < numVerts; i++)
	{
		verts[i].Tangent = XMFLOAT3(0, 0, 0);
	}

	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < numVerts;)
	{
		// Grab indices and vertices of first triangle
		unsigned int i1 = indices[i++];
		unsigned int i2 = indices[i++];
		unsigned int i3 = indices[i++];
		Vertex* v1 = &verts[i1];
		Vertex* v2 = &verts[i2];
		Vertex* v3 = &verts[i3];

		// Calculate vectors relative to triangle positions
		float x1 = v2->Position.x - v1->Position.x;
		float y1 = v2->Position.y - v1->Position.y;
		float z1 = v2->Position.z - v1->Position.z;

		float x2 = v3->Position.x - v1->Position.x;
		float y2 = v3->Position.y - v1->Position.y;
		float z2 = v3->Position.z - v1->Position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->UV.x - v1->UV.x;
		float t1 = v2->UV.y - v1->UV.y;

		float s2 = v3->UV.x - v1->UV.x;
		float t2 = v3->UV.y - v1->UV.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);
		
		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->Tangent.x += tx; 
		v1->Tangent.y += ty; 
		v1->Tangent.z += tz;

		v2->Tangent.x += tx; 
		v2->Tangent.y += ty; 
		v2->Tangent.z += tz;

		v3->Tangent.x += tx; 
		v3->Tangent.y += ty; 
		v3->Tangent.z += tz;
	}

	// Ensure all of the tangents are orthogonal to the normals
	for (int i = 0; i < numVerts; i++)
	{
		// Grab the two vectors
		XMVECTOR normal = XMLoadFloat3(&verts[i].Normal);
		XMVECTOR tangent = XMLoadFloat3(&verts[i].Tangent);

		// Use Gram-Schmidt orthogonalize
		tangent = XMVector3Normalize(
			tangent - normal * XMVector3Dot(normal, tangent));
		
		// Store the tangent
		XMStoreFloat3(&verts[i].Tangent, tangent);
	}
}

void Mesh::CreateBuffers(Vertex* vertArray, int numVerts, unsigned int* indexArray, int numIndices, ID3D11Device* device)
{
	// Create the vertex buffer
	D3D11_BUFFER_DESC vbd;
    vbd.Usage					= D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth				= sizeof(Vertex) * numVerts; // Number of vertices
    vbd.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags			= 0;
    vbd.MiscFlags				= 0;
	vbd.StructureByteStride		= 0;
    D3D11_SUBRESOURCE_DATA initialVertexData;
    initialVertexData.pSysMem	= vertArray;
    device->CreateBuffer(&vbd, &initialVertexData, &vb);

	// Create the index buffer
	D3D11_BUFFER_DESC ibd;
    ibd.Usage					= D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth				= sizeof(unsigned int) * numIndices; // Number of indices
    ibd.BindFlags				= D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags			= 0;
    ibd.MiscFlags				= 0;
	ibd.StructureByteStride		= 0;
    D3D11_SUBRESOURCE_DATA initialIndexData;
    initialIndexData.pSysMem	= indexArray;
    device->CreateBuffer(&ibd, &initialIndexData, &ib);

	// Save the indices
	this->numIndices = numIndices;
}

void Mesh::Draw(ID3D11DeviceContext * deviceContext, bool sky)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	if (!sky)
	{
		deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		deviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
		deviceContext->DrawIndexed(numIndices, 0, 0);
	}
	else
	{
		deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		deviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
		
		deviceContext->RSSetState(rasterState);
		deviceContext->OMSetDepthStencilState(depthState, 0);
		deviceContext->DrawIndexed(numIndices, 0, 0);

		// Reset states
		deviceContext->RSSetState(0);
		deviceContext->OMSetDepthStencilState(0, 0);
	}
}