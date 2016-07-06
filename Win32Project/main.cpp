#define _CRT_SECURE_NO_WARNINGS 

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#include <math.h>

//////////////////////////////////////////////////////////////////

// TODO: Global Variables for Direct3D
ID3D11Device * d3dDevice;
ID3D11DeviceContext * immediateContext;
IDXGISwapChain * dxgiSwapChain;

ID3D11RenderTargetView * renderTargetView;

ID3D11VertexShader * vertexShader;
ID3D11InputLayout * inputLayout;
ID3D11PixelShader * pixelShader;
ID3D11Buffer * vertexBuffer;

ID3D11Buffer * constantBuffer;

ID3D11RasterizerState * rasterizerState;

struct MyVertex { float x, y, z; };
struct MyConstantBuffer { float world[16], view[16], proj[16]; };

#define cot(x) 1/tan(x)
// row방식으로 벡터를 저장한다.
void transpose(float v[16]) {
	float temp[16] = { v[0], v[4], v[8], v[12], v[1], v[5], v[9], v[13], v[2], v[6], v[10], v[14], v[3], v[7], v[11], v[15] };
	memcpy(v, temp, sizeof(float) * 16);
}

void ReadData(const char* filename, void** buffer, int* length) {
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	*length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	*buffer = new char[*length];
	fread(*buffer, *length, 1, fp);
	fclose(fp);
}

bool InitializeDirect3D(HWND hWnd)
{
	// TODO: Initializing Direct3D
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0, };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc = { 1280, 720, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_STRETCHED };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = true;

	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
		&swapChainDesc, &dxgiSwapChain, &d3dDevice, nullptr, &immediateContext)))
		return false;

	ID3D11Resource * backBuffer;
	dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	if (FAILED(d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView)))
		return false;

	backBuffer->Release();

	immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

	void * vertexShaderData;
	int vertexShaderLength;
	ReadData("MyVertexShader.cso", (void**)&vertexShaderData, &vertexShaderLength);
	d3dDevice->CreateVertexShader(vertexShaderData, vertexShaderLength, nullptr, &vertexShader);
	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	d3dDevice->CreateInputLayout(inputElementDescs, _countof(inputElementDescs), vertexShaderData, vertexShaderLength, &inputLayout);
	delete[] vertexShaderData;

	void * pixelShaderData;
	int pixelShaderLength;
	ReadData("MyPixelShader.cso", (void**)&pixelShaderData, &pixelShaderLength);
	d3dDevice->CreatePixelShader(pixelShaderData, pixelShaderLength, nullptr, &pixelShader);
	delete[] pixelShaderData;

	MyVertex vertices[] = {
		{ -0.5f, -0.5f, +0.0f },
		{ +0.0f, +0.5f, +0.0f },
		{ +0.5f, -0.5f, +0.0f }
	};
	D3D11_BUFFER_DESC vertexBufferDesc = { sizeof(vertices), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	D3D11_SUBRESOURCE_DATA vertexBufferSubResourceData = { vertices, sizeof(vertices), 0 };
	d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferSubResourceData, &vertexBuffer);

	D3D11_VIEWPORT viewport = { 0, };
	viewport.Width = 1280;
	viewport.Height = 720;
	viewport.MaxDepth = 1.0f;
	immediateContext->RSSetViewports(1, &viewport);

	D3D11_BUFFER_DESC constantBufferDesc = {sizeof(MyConstantBuffer), D3D11_USAGE_DEFAULT, 
		D3D11_BIND_CONSTANT_BUFFER, 0, 0, 0};
	d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	memset(&rasterizerDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	d3dDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState);

	return true;
}

void UninitializeDirect3D ()
{
	// TODO: Uninitializing Direct3D
	rasterizerState->Release();
	constantBuffer->Release();

	vertexBuffer->Release();

	inputLayout->Release();
	pixelShader->Release();
	vertexShader->Release();

	renderTargetView->Release();

	immediateContext->Release();
	d3dDevice->Release();
	dxgiSwapChain->Release();
}

void Loop ()
{
	float clearColor[] = { 0x65/255.0f, 0x9C/255.0f, 0xEF/255.0f, 1 };
	immediateContext->ClearRenderTargetView(renderTargetView, clearColor);

	immediateContext->RSSetState(rasterizerState);

	MyConstantBuffer constantBufferData = { 0, };
	static float angle = 0;
	angle += 0.001f;
	float world[16] = { cos(angle), 0, -sin(angle), 0, 0, 1, 0, 0, sin(angle), 0, cos(angle), 0, 0, 0, 0, 1 };
	transpose(world);
	float view[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	transpose(view);
	float yScale = cot((3.141592f / 4) / 2);
	float xScale = yScale / (1280 / 720.0f);
	float zn = 0.001f, zf = 1000.0f;
	float proj[16] = { xScale, 0, 0, 0, 0, yScale, 0, 0, 0, 0, zf / (zf - zn), 1, 0, 0, -zn*zf / (zf - zn), 1 };
	transpose(proj);
	memcpy(constantBufferData.world, world, sizeof(float) * 16);
	memcpy(constantBufferData.view, view, sizeof(float) * 16);
	memcpy(constantBufferData.proj, proj, sizeof(float) * 16);
	immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &constantBufferData,
		sizeof(constantBufferData), 0);

	
	immediateContext->VSSetShader(vertexShader, nullptr, 0);
	/*
	immediateContext->PSSetShader 함수를 이용하여 색을 바꿀 수 있다.
	*/
	immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	immediateContext->PSSetShader(pixelShader, nullptr, 0);

	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediateContext->IASetInputLayout(inputLayout);
	UINT stride = sizeof(MyVertex), offset = 0;
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	immediateContext->Draw(3, 0);

	dxgiSwapChain->Present(0, 0);
	// TODO: Rendering

}

//////////////////////////////////////////////////////////////////

#pragma region Precode
LRESULT CALLBACK WndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
	case WM_CLOSE: PostQuitMessage ( 0 ); break;
	default: return DefWindowProc ( hWnd, uMsg, wParam, lParam );
	}
	return 0;
}

HWND InitializeWindow ( int width = 1280, int height = 720 )
{
	WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, nullptr,
		LoadIcon ( nullptr, IDI_APPLICATION ), LoadCursor ( nullptr, IDC_ARROW ),
		nullptr, nullptr, TEXT ( "Win32AppWindow" ) };
	if ( RegisterClass ( &wndClass ) == 0 )
		return 0;

	RECT rect = { 0, 0, width, height };
	AdjustWindowRect ( &rect, WS_OVERLAPPEDWINDOW, false );
	int w = rect.right - rect.left, h = rect.bottom - rect.top;
	int x = GetSystemMetrics ( SM_CXSCREEN ) / 2 - w / 2, y = GetSystemMetrics ( SM_CYSCREEN ) / 2 - h / 2;

	return CreateWindow ( TEXT ( "Win32AppWindow" ), TEXT ( "Application" ), WS_OVERLAPPEDWINDOW,
		x, y, w, h, nullptr, nullptr, GetModuleHandle ( nullptr ), nullptr );
}

void RunWindow ( HWND hWnd )
{
	ShowWindow ( hWnd, SW_SHOW );
	UpdateWindow ( hWnd );

	MSG msg;
	while ( true )
	{
		if ( PeekMessage ( &msg, nullptr, 0, 0, PM_NOREMOVE ) )
		{
			if ( !GetMessage ( &msg, nullptr, 0, 0 ) )
				return;
			TranslateMessage ( &msg );
			DispatchMessage ( &msg );
		}
		else
		{
			Loop ();
			Sleep ( 1 );
		}
	}
}

int WINAPI WinMain ( HINSTANCE, HINSTANCE, LPSTR, int )
{
	HWND hWnd = InitializeWindow ();
	if ( hWnd == nullptr ) return -1;
	if ( !InitializeDirect3D ( hWnd ) ) return -2;

	RunWindow ( hWnd );

	UninitializeDirect3D ();

	return 0;
}
#pragma endregion