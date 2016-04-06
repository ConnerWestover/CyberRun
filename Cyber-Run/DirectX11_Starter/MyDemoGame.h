#pragma once

#include <DirectXMath.h>
#include "DirectXGameCore.h"
#include "SimpleShader.h"

#include "Mesh.h"
#include "Camera.h"
#include "GameEntity.h"

#include <vector>

// Include run-time memory checking in debug builds, so 
// we can be notified of memory leaks
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// --------------------------------------------------------
// Game class which extends the base DirectXGameCore class
// --------------------------------------------------------
class MyDemoGame : public DirectXGameCore
{
public:
	MyDemoGame(HINSTANCE hInstance);
	~MyDemoGame();

	// Overrides for base level methods
	bool Init();
	void OnResize();
	void UpdateScene(float deltaTime, float totalTime);
	void DrawScene(float deltaTime, float totalTime);

	// For handing mouse input
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
    // Input and mesh swapping
    bool prevSpaceBar;
    unsigned int currentEntity;

    // Keep track of "stuff"
    std::vector<Mesh*> meshes;
    std::vector<GameEntity*> entities;

	// Initialization for our "game" demo - Feel free to
	// expand, alter, rename or remove these once you
	// start doing something more advanced!
	void CreateGeometry();
	void LoadShaders();
	void CreateMatrices();

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Sky stuff
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;
	ID3D11ShaderResourceView*	skyTexture;
	ID3D11RasterizerState*		rasterState;
	ID3D11DepthStencilState*	depthState;

	// Post process stuff
	ID3D11RenderTargetView* ppRTV;
	ID3D11ShaderResourceView* ppSRV;
	SimpleVertexShader* ppVS;
	SimplePixelShader* ppPS;


    // Texture stuff
    ID3D11ShaderResourceView* texture;
	ID3D11ShaderResourceView* normalMap;
    ID3D11SamplerState* sampler;

	// Basic debug camera
	Camera* camera;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};