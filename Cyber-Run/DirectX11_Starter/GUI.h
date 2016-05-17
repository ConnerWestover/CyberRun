#pragma once

#include <map>
#include <string>

#include "SimpleShader.h"
#include "Mesh.h"

#include <SpriteFont.h>
using namespace DirectX;

class GUI {
public:
	static void Create(ID3D11Device*, ID3D11DeviceContext*);
	static void Destroy();

	static void BeginStringDraw();
	static void EndStringDraw();
	static void DrawString(const char*, int, int, const wchar_t*);

	static void DrawImage(const char*, int, int, int, int);

private:
	// singleton stuff
	static GUI *instance;

	GUI(ID3D11Device*, ID3D11DeviceContext*);
	~GUI();
	GUI(GUI const&);
	void operator=(GUI const&);

	// device stuff
	static ID3D11Device *device;
	static ID3D11DeviceContext *deviceContext;

	// font stuff
	static SpriteBatch *spriteBatch;
	static std::map<std::string, SpriteFont*> fonts;

	// image stuff
	static std::map<std::string, ID3D11ShaderResourceView*> images;
	static void LoadImages();

	static SimpleVertexShader *pixelVS;
	static SimplePixelShader *pixelPS;

	static ID3D11SamplerState *sampler;

	static Mesh *mesh;
};