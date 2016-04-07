// ----------------------------------------------------------------------------
//  A few notes on project settings
//
//  - The project is set to use the UNICODE character set
//    - This was changed in Project Properties > Config Properties > General > Character Set
//    - This basically adds a "#define UNICODE" to the project
//
//  - The include directories were automagically correct, since the DirectX 
//    headers and libs are part of the windows SDK
//    - For instance, $(WindowsSDK_IncludePath) is set as a project include 
//      path by default.  That's where the DirectX headers are located.
//
//  - Two libraries had to be manually added to the Linker Input Dependencies
//    - d3d11.lib
//    - d3dcompiler.lib
//    - This was changed in Project Properties > Config Properties > Linker > Input > Additional Dependencies
//
//  - The Working Directory was changed to match the actual .exe's 
//    output directory, since we need to load the compiled shader files at run time
//    - This was changed in Project Properties > Config Properties > Debugging > Working Directory
//
// ----------------------------------------------------------------------------

#include "MyDemoGame.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

float playerX =0;
bool ATrigger = false;
bool DTrigger = false;


#pragma region Win32 Entry Point (WinMain)
// --------------------------------------------------------
// Win32 Entry Point - Where your program starts
// --------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Create the game object.
	MyDemoGame game(hInstance);
	
	// This is where we'll create the window, initialize DirectX, 
	// set up geometry and shaders, etc.
	if( !game.Init() )
		return 0;
	
	// All set to run the game loop
	return game.Run();
}

#pragma endregion

#pragma region Constructor / Destructor
// --------------------------------------------------------
// Base class constructor will set up all of the underlying
// fields, and then we can overwrite any that we'd like
// --------------------------------------------------------
MyDemoGame::MyDemoGame(HINSTANCE hInstance) 
	: DirectXGameCore(hInstance)
{
	// Set up a custom caption for the game window.
	// - The "L" before the string signifies a "wide character" string
	// - "Wide" characters take up more space in memory (hence the name)
	// - This allows for an extended character set (more fancy letters/symbols)
	// - Lots of Windows functions want "wide characters", so we use the "L"
	windowCaption = L"My Super Fancy GGP Game";

	// Custom window size - will be created by Init() later
	windowWidth = 800;
	windowHeight = 600;

	camera = 0;
}

// --------------------------------------------------------
// Cleans up our DirectX stuff and any objects we need to delete
// - When you make new DX resources, you need to release them here
// - If you don't, you get a lot of scary looking messages in Visual Studio
// --------------------------------------------------------
MyDemoGame::~MyDemoGame()
{
	// Delete our simple shaders
	delete vertexShader;
	delete pixelShader;
	delete skyVS;
	delete skyPS;
	delete ppVS;
	delete ppPS;

    // Loop and remove Game Entities
    for (unsigned int i = 0; i < entities.size(); i++)
        delete entities[i];

    for (unsigned int i = 0; i < meshes.size(); i++)
        delete meshes[i];

    delete camera;

	texture->Release();
	normalMap->Release();
	sampler->Release();

	skyTexture->Release();
	depthState->Release();
	rasterState->Release();

	ppRTV->Release();
	ppSRV->Release();
}

#pragma endregion

#pragma region Initialization

// --------------------------------------------------------
// Initializes the base class (including the window and D3D),
// sets up our geometry and loads the shaders (among other things)
// --------------------------------------------------------
bool MyDemoGame::Init()
{
	// Call the base class's Init() method to create the window,
	// initialize DirectX, etc.
	if( !DirectXGameCore::Init() )
		return false;

	// Helper methods to create something to draw, load shaders to draw it 
	// with and set up matrices so we can see how to pass data to the GPU.
	//  - For your own projects, feel free to expand/replace these.
	CreateGeometry();
	LoadShaders();
	CreateMatrices();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives we'll be using and how to interpret them
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Successfully initialized
	return true;
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void MyDemoGame::CreateGeometry()
{
    XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
    XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
    XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

    
    Mesh* player = new Mesh("MaleLow.obj", device);
    Mesh* floor = new Mesh("cube.obj", device);
	
    meshes.push_back(player);
    meshes.push_back(floor);

    // Make some entities
    GameEntity* person = new GameEntity(player);
    GameEntity* ground = new GameEntity(floor);

	entities.push_back(ground);
    entities.push_back(person);
   
	for (int i = 0; i < 20; i++)
	{	
		Mesh* sphere = new Mesh("sphere.obj", device);
		meshes.push_back(sphere);
		GameEntity* collectMe = new GameEntity(sphere);
		int x = rand() % 3;
		switch (x)
		{
			case 0:
				collectMe->SetPosition(-.75f, 0.0f, 2.0f * i);
				break;
			case 1:
				collectMe->SetPosition(0.0f, 0.0f, 2.0f * i);
				break;
			case 2:
				collectMe->SetPosition(.75f, 0.0f, 2.0f * i);
				break;
			default:
				collectMe->SetPosition(0.0f, 0.0f, 2.0f * i);
				break;
		}
		entities.push_back(collectMe);
	}
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// - These simple shaders provide helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void MyDemoGame::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, deviceContext);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, deviceContext);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	skyVS = new SimpleVertexShader(device, deviceContext);
	skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, deviceContext);
	skyPS->LoadShaderFile(L"SkyPS.cso");

	ppVS = new SimpleVertexShader(device, deviceContext);
	ppVS->LoadShaderFile(L"BlurVS.cso");

	ppPS = new SimplePixelShader(device, deviceContext);
	ppPS->LoadShaderFile(L"BlurPS.cso");

    // Set up texture stuff
    DirectX::CreateWICTextureFromFile(device, deviceContext, L"grid.jpg", 0, &texture);
	DirectX::CreateWICTextureFromFile(device, deviceContext, L"gridNormals.jpg", 0, &normalMap);
	DirectX::CreateDDSTextureFromFile(device, deviceContext, L"SunnyCubeMap.dds", 0, &skyTexture);

    // Create the sampler
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&samplerDesc, &sampler);

	// Create the sky rasterizer state
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rastDesc, &rasterState);

	// Sky's depth state
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthDesc, &depthState);


	// Post processing ---------------------------------

	// Create a texture
	D3D11_TEXTURE2D_DESC tDesc = {};
	tDesc.Width = windowWidth;
	tDesc.Height = windowHeight;
	tDesc.ArraySize = 1;
	tDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tDesc.CPUAccessFlags = 0;
	tDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tDesc.MipLevels = 1;
	tDesc.MiscFlags = 0;
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 0;
	tDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&tDesc, 0, &ppTexture);

	// Make a render target view for rendering into the texture
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = tDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(ppTexture, &rtvDesc, &ppRTV);

	// Make an SRV 
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = tDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0; 
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(ppTexture, &srvDesc, &ppSRV);

	// Get rid of ONE of the texture references
	ppTexture->Release();
}

// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void MyDemoGame::CreateMatrices()
{
	camera = new Camera(0, 0, -5);
	camera->UpdateProjectionMatrix(aspectRatio);
}

#pragma endregion

#pragma region Window Resizing

// --------------------------------------------------------
// Handles resizing DirectX "stuff" to match the (usually) new
// window size and updating our projection matrix to match
// --------------------------------------------------------
void MyDemoGame::OnResize()
{
	// Handle base-level DX resize stuff
	DirectXGameCore::OnResize();

	if (camera != 0)
	{
		camera->UpdateProjectionMatrix(aspectRatio);
	}
}
#pragma endregion

#pragma region Game

// --------------------------------------------------------
// Update your game here - take input, move objects, etc.
// --------------------------------------------------------
void MyDemoGame::UpdateScene(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	entities[0]->SetPosition(0.0f, -2.0f, 495.0f - (totalTime * 2));
	entities[0]->SetScale(3.0f, 2.0f, 1000.0f);
	entities[0]->UpdateWorldMatrix();

	entities[1]->SetPosition(playerX, -1.0f, -3.0f);
	entities[1]->SetScale(.05f, .05f, .05f);
	entities[1]->UpdateWorldMatrix();

	if (GetKeyState('A') & 0x8000) { 
		ATrigger = true;
	}
	if (ATrigger && !(GetKeyState('A') & 0x8000)) {
		playerX = (playerX - .75f);
		if (playerX <= -.75f) {
			playerX = -.75f;
		}
		ATrigger = false;
	}
	if (GetKeyState('D') & 0x8000) {
		DTrigger = true;
	}
	if (DTrigger && !(GetKeyState('D') & 0x8000)) {
		playerX = (playerX + .75f);
		if (playerX >= .75f) {
			playerX = .75f;
		}
		DTrigger = false;
	}
    
	for (int i = 2; i < entities.size(); i++)
	{
		XMFLOAT3 pos = entities[i]->position;
		
		entities[i]->SetPosition(pos.x, -.5f, (pos.z - (deltaTime *2)));
		entities[i]->SetScale(.1f, .1f, .1f);
		if (pos.z <= -2.8f && pos.x == playerX) {
			//TODO: Add Point to Score
			entities[i]->SetPosition(pos.x, -.5f, 40.0f);
		}
		else if (pos.z <= -3.0f) {
			entities[i]->SetPosition(pos.x, -.5f, 40.0f);
		}
		entities[i]->UpdateWorldMatrix();
	}

	// Update the camera
	camera->Update(deltaTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void MyDemoGame::DrawScene(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = {0,0,0,0};// {0.4f, 0.6f, 0.75f, 0.0f};

	// Swap to the new render target
	deviceContext->OMSetRenderTargets(1, &ppRTV, depthStencilView);



	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of DrawScene (before drawing *anything*)
	deviceContext->ClearRenderTargetView(ppRTV, color);
	deviceContext->ClearDepthStencilView(
		depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	for (int i = 0; i < entities.size(); i++)
	{
		//Object code
		GameEntity* ge = entities[i];
		ID3D11Buffer* vb = ge->GetMesh()->GetVertexBuffer();
		ID3D11Buffer* ib = ge->GetMesh()->GetIndexBuffer();

		// Set buffers in the input assembler
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		deviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		vertexShader->SetMatrix4x4("world", *ge->GetWorldMatrix());
		vertexShader->SetMatrix4x4("view", camera->GetView());
		vertexShader->SetMatrix4x4("projection", camera->GetProjection());

		// Pass in some light data to the pixel shader
		pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(0, -1, 0));
		pixelShader->SetFloat4("DirLightColor", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(0, 2, 0));
		pixelShader->SetFloat4("PointLightColor", XMFLOAT4(0.3f, 0.3f, 1.0f, 0.0f));

		pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

		pixelShader->SetShaderResourceView("diffuse", texture);
		pixelShader->SetShaderResourceView("normalMap", normalMap);
		pixelShader->SetShaderResourceView("skyTexture", skyTexture);
		pixelShader->SetSamplerState("trilinear", sampler);

		vertexShader->SetShader(true);
		pixelShader->SetShader(true);

		// Finally do the actual drawingw
		deviceContext->DrawIndexed(ge->GetMesh()->GetIndexCount(), 0, 0);

	}

	GameEntity* ge = entities[2];
	ID3D11Buffer* vb = ge->GetMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = ge->GetMesh()->GetIndexBuffer();
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	deviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		// Draw the sky ----------------------------
		ID3D11Buffer* skyVB = meshes[2]->GetVertexBuffer();
		ID3D11Buffer* skyIB = meshes[2]->GetIndexBuffer();

		// Set the in the input assembler
		deviceContext->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
		deviceContext->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

		// Set up the shaders
		skyVS->SetMatrix4x4("view", camera->GetView());
		skyVS->SetMatrix4x4("projection", camera->GetProjection());
		skyVS->SetShader();

		skyPS->SetShaderResourceView("sky", skyTexture);
		skyPS->SetShader();

		// Set up states and draw
		deviceContext->RSSetState(rasterState);
		deviceContext->OMSetDepthStencilState(depthState, 0);
		deviceContext->DrawIndexed(meshes[2]->GetIndexCount(), 0, 0);

		// Reset states
		deviceContext->RSSetState(0);
		deviceContext->OMSetDepthStencilState(0, 0);


		// Done with "regular" rendering - swap to post process
		deviceContext->OMSetRenderTargets(1, &renderTargetView, 0);
		deviceContext->ClearRenderTargetView(renderTargetView, color);


		// Draw the post process
		ppVS->SetShader();


		ppPS->SetInt("blurAmount", 1.5f);
		ppPS->SetFloat("pixelWidth", 1.0f / windowWidth);
		ppPS->SetFloat("pixelHeight", 1.0f / windowHeight);
		ppPS->SetShaderResourceView("pixels", ppSRV);
		ppPS->SetSamplerState("trilinear", sampler);
		ppPS->SetShader();

		// Turn off existing vert/index buffers
		ID3D11Buffer* nothing = 0;
		deviceContext->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		deviceContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		// Finally - DRAW!
		deviceContext->Draw(3, 0);

		// Unbind the SRV so the underlying texture isn't bound for
		// both input and output at the start of next frame
		ppPS->SetShaderResourceView("pixels", 0);
		// Present the buffer
		//  - Puts the image we're drawing into the window so the user can see it
		//  - Do this exactly ONCE PER FRAME
		//  - Always at the very end of the frame
		HR(swapChain->Present(0, 0));
}

#pragma endregion

#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
//
// Feel free to add code to this method
// --------------------------------------------------------
void MyDemoGame::OnMouseDown(WPARAM btnState, int x, int y)
{
	// Save the previous mouse position, so we have it for the future
	//prevMousePos.x = x;
	//prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	//SetCapture(hMainWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
//
// Feel free to add code to this method
// --------------------------------------------------------
void MyDemoGame::OnMouseUp(WPARAM btnState, int x, int y)
{
	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	//ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
//
// Feel free to add code to this method
// --------------------------------------------------------
void MyDemoGame::OnMouseMove(WPARAM btnState, int x, int y)
{
	// Calc differences
	/*if (btnState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera->Rotate(yDiff, xDiff);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;*/
}
#pragma endregion