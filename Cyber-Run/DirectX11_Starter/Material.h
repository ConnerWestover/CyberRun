#pragma once
#include <DirectXMath.h>
#include <string>
#include <vector>
#include "DirectXGameCore.h"
#include "SimpleShader.h"

using namespace DirectX;
using namespace std;

class Material
{
public:
	Material(SimpleVertexShader* vS, SimplePixelShader* pS, vector<ID3D11ShaderResourceView*> srv, vector<string> locs, ID3D11SamplerState* sampler);
	Material();
	~Material();
	SimpleVertexShader* getVert();
	SimplePixelShader* getPix();
	vector<ID3D11ShaderResourceView*> getSRV();
	ID3D11SamplerState* getSampler();
	void prepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	vector<ID3D11ShaderResourceView*> srvs;
	vector<string> locations;
	ID3D11SamplerState* sampler;
};