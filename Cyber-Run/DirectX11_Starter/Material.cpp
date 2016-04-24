#include "Material.h"



Material::Material(SimpleVertexShader * vS, SimplePixelShader * pS, vector<ID3D11ShaderResourceView*> srv, vector<string> locs, ID3D11SamplerState* _sampler)
{
	vertexShader = vS;
	pixelShader = pS;
	srvs = srv;
	locations = locs;
	sampler = _sampler;
}

Material::Material()
{
}


Material::~Material()
{
}

SimpleVertexShader * Material::getVert()
{
	return vertexShader;
}

SimplePixelShader * Material::getPix()
{
	return pixelShader;
}

vector<ID3D11ShaderResourceView*> Material::getSRV()
{
	return srvs;
}

ID3D11SamplerState * Material::getSampler()
{
	return sampler;
}

void Material::prepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	vertexShader->SetMatrix4x4("view", viewMatrix);
	vertexShader->SetMatrix4x4("projection", projectionMatrix);

	for (int i = 0; i < srvs.size(); i++)
	{
		pixelShader->SetShaderResourceView(locations[i], srvs[i]);
	}
	
	pixelShader->SetSamplerState("samplerState", sampler);

	vertexShader->SetShader(true);
	pixelShader->SetShader(true);

	vertexShader->CopyAllBufferData();
}
