#pragma once

#include "includes.h"

class WindowClass;


class Direct3DProcessor : NZA_t
{
	IDXGISwapChain*			  swapChain_;
	ID3D11Device*			  device_;
	ID3D11DeviceContext*      deviceContext_;
	WindowClass*              wnd_;

	uint8_t                   nBuffers_;
	ID3D11RenderTargetView*   currentBuffer_;
	ID3D11Texture2D*		  depthBuffer_;
	ID3D11DepthStencilView*   depthStencilView_;

	std::vector<ID3D11DepthStencilState*> depthStencilStates_;
	std::vector<ID3D11RasterizerState*>   rasterizerStates_;
	std::vector<ID3D11BlendState*>        blendStates_;
	std::vector<ID3D11SamplerState*>      samplerStates_;
	std::vector<ID3D11InputLayout*>       layouts_;
	std::vector<IUnknown*>                toDelete_;

	std::vector<Direct3DObject*> objects_;
	Direct3DShaderManager        shaderManager_;
	ShaderIndex_t				 currentVertexShader_;
	ShaderIndex_t                currentPixelShader_;
	ShaderIndex_t				 currentGeometryShader_;
	UINT						 currentLayout_;

	Direct3DConstantBufferManager cbManager_;
	Direct3DTextureManager        textureManager_;

	void InitDeviceAndSwapChain ();
	void InitViewport ();
	void InitDepthStencilView ();
	void EnableShader (ShaderIndex_t desc);
	void EnableObjectSettings (Direct3DObject* obj);
	

public:
	void ok () override;

	Direct3DProcessor (WindowClass* wnd, uint8_t buffers = 1);
	~Direct3DProcessor ();

	void ProcessDrawing (Direct3DCamera* cam, bool clean = true);
	void Present ();

	DepthStencilIndex_t AddDepthStencilState (bool enableDepth = true,
							   bool enableStencil = false);
	void ApplyDepthStencilState (DepthStencilIndex_t n);

	RasterizerIndex_t AddRasterizerState   (bool clockwise = true,
							   bool wireframe = false, 
							   bool cullNone  = false);
	void ApplyRasterizerState (RasterizerIndex_t n);


	BlendIndex_t AddBlendState (bool blend = false);
	void ApplyBlendState (BlendIndex_t n);

	SamplerIndex_t AddSamplerState (D3D11_TEXTURE_ADDRESS_MODE mode = D3D11_TEXTURE_ADDRESS_WRAP, 
								XMFLOAT4 border = {1.0f, 1.0f, 1.0f, 1.0f});
	void SendSamplerStateToPS (SamplerIndex_t n, UINT slot);

	void RegisterObject (Direct3DObject* obj);
	void RemoveObject (Direct3DObject* obj);
	std::vector<Direct3DObject*>& GetObjectsVector ();

	void ProcessObjects ();

	ShaderIndex_t LoadShader (std::string filename,
							  std::string function,
							  SHADER_TYPES shaderType);

	ID3D11VertexShader*   GetVertexShader   (ShaderIndex_t desc);
	ID3D11PixelShader*    GetPixelShader    (ShaderIndex_t desc);
	ID3D11GeometryShader* GetGeometryShader (ShaderIndex_t desc);

	LayoutIndex_t AddLayout (ShaderIndex_t desc,
					bool position = false,
					bool normal = false,
					bool texture = false,
					bool color = false);

	void EnableLayout (LayoutIndex_t n);
	void SetLayout (Direct3DObject* obj, LayoutIndex_t n);

	ConstantBufferIndex_t RegisterConstantBuffer (void* data,
							     size_t size,
							     UINT slot);
	void UpdateConstantBuffer (ConstantBufferIndex_t n);
	void SendCBToVS (ConstantBufferIndex_t n);
	void SendCBToPS (ConstantBufferIndex_t n);
	void SendCBToGS (ConstantBufferIndex_t n);

	void AttachShaderToObject (Direct3DObject* obj, ShaderIndex_t n);

	TextureIndex_t LoadTexture (std::string filename);
	void SendTextureToPS (TextureIndex_t index, UINT slot);

	void ReloadShaders ();

	ID3D11Device* GetDevice ();

	WindowClass* GetWindowPtr ();

	void AddToDelete (IUnknown* ptr);

	Direct3DTextureManager& GetTextureManager ();
};