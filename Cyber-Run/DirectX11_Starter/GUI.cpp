#include "GUI.h"

#include "Vertex.h"
#include "WICTextureLoader.h"

#include <iostream>

// values
GUI* GUI::instance = nullptr;

ID3D11Device* GUI::device;
ID3D11DeviceContext* GUI::deviceContext;

SpriteBatch* GUI::spriteBatch;
std::map<std::string, SpriteFont*> GUI::fonts;

std::map<std::string, ID3D11ShaderResourceView*> GUI::images;

SimpleVertexShader* GUI::pixelVS;
SimplePixelShader* GUI::pixelPS;

ID3D11SamplerState* GUI::sampler;

Mesh* GUI::mesh;


// methods
void GUI::Create(ID3D11Device *device, ID3D11DeviceContext *deviceContext) {
	if (instance == nullptr) {
		instance = new GUI(device, deviceContext);
	}
}

void GUI::Destroy() {
	if (instance != nullptr) {
		delete instance;
		instance = nullptr;
	}
}


GUI::GUI(ID3D11Device *device, ID3D11DeviceContext *deviceContext) {
	this->device = device;
	this->deviceContext = deviceContext;

	// fonts
	spriteBatch = new SpriteBatch(deviceContext);

	fonts[std::string("courier")] = new SpriteFont(device, L"fonts/courier.spritefont");
	fonts[std::string("fixedsys")] = new SpriteFont(device, L"fonts/fixedsys.spritefont");

	// images
	LoadImages();
}

GUI::~GUI() {
	delete spriteBatch;

	for (std::map<std::string, SpriteFont*>::iterator it = fonts.begin(); it != fonts.end(); it++) {
		delete it->second;
	}

	for (std::map<std::string, ID3D11ShaderResourceView*>::iterator it = images.begin(); it != images.end(); it++) {
		it->second->Release();	
	}

	delete mesh;
}


// draw text to the screen
void GUI::BeginStringDraw() {
	spriteBatch->Begin();
}

void GUI::EndStringDraw() {
	spriteBatch->End();
}

void GUI::DrawString(const char *font, int x, int y, const wchar_t *msg) {
	SpriteFont *sprite = fonts[std::string(font)];

	sprite->DrawString(spriteBatch, msg, XMFLOAT2(x, y));
}


// draw image to screen
void GUI::LoadImages() {
	// image shaders
	pixelVS = new SimpleVertexShader(device, deviceContext);
	pixelVS->LoadShaderFile(L"SpriteVS.cso");

	pixelPS = new SimplePixelShader(device, deviceContext);
	pixelPS->LoadShaderFile(L"SpritePS.cso");

	D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    device->CreateSamplerState(&samplerDesc, &sampler);

	// image mesh
	Vertex verts[] = {
		{ XMFLOAT3(-1, 1, 0), XMFLOAT2(0, 0), XMFLOAT3(0, 1, 0) },
		{ XMFLOAT3(1, 1, 0), XMFLOAT2(1, 0), XMFLOAT3(0, 1, 0) },
		{ XMFLOAT3(-1, -1, 0),  XMFLOAT2(0, 1), XMFLOAT3(0, 1, 0) },
		{ XMFLOAT3(1, -1, 0), XMFLOAT2(1, 1), XMFLOAT3(0, 1, 0) },
	};

	unsigned int inds[] = {
		0, 1, 2,
		3, 2, 1
	};

	mesh = new Mesh(verts, 4, inds, 6, device);


	// get the images
	ID3D11ShaderResourceView *test;
	CreateWICTextureFromFile(device, deviceContext, L"gui/test.png", 0, &test);
	images["test"] = test;

	ID3D11ShaderResourceView *topbar;
	CreateWICTextureFromFile(device, deviceContext, L"gui/topbar.png", 0, &topbar);
	images["topbar"] = topbar;
}

void GUI::DrawImage(const char *imageName, int x, int y, int w, int h) {
	float posX = -1 + ((x+w/2) * 1.0f / 1000 * 2);
	float posY = 1 - ((y+h/2) * 1.0f / 750 * 2);
	float scaleX = w * 1.0f / 1000;
	float scaleY = h * 1.0f / 750;

	XMFLOAT4X4 worldMatrix;
	XMMATRIX trans = XMMatrixTranslation(posX, posY, 0);
	XMMATRIX sc = XMMatrixScaling(scaleX, scaleY, 0);
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(sc * trans));


	pixelVS->SetMatrix4x4("world", worldMatrix);
	pixelPS->SetSamplerState("samplerState", sampler);
	pixelPS->SetShaderResourceView("image", images[std::string(imageName)]);

	pixelVS->SetShader(true);
	pixelPS->SetShader(true);

	mesh->Draw(deviceContext, false);
}