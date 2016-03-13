#pragma once

#include "includes.h"

class Direct3DCamera : NZA_t
{
	WindowClass* wnd_;
	XMVECTOR position_;
	XMVECTOR direction_;
	XMVECTOR up_;
	XMFLOAT3 projectionSettings_;

	XMMATRIX view_;
	XMMATRIX projection_;

public:
	void ok () override;

	Direct3DCamera (WindowClass* wnd,
					float posX, float posY, float posZ,
					float tgtX, float tgtY, float tgtZ,
					float upX, float upY, float upZ,
					float fov = 0.3f, float zNear = 1.0f, float zFar = 1000.0f);

	void Update ();

	const XMMATRIX& GetView () const;
	const XMMATRIX& GetProjection () const;

	void StorePos (XMFLOAT4& pos) const;
	void SetPos (XMFLOAT4 pos);
	void SetDir (XMFLOAT4 dir);

	XMVECTOR& GetDir ();
	XMVECTOR& GetPos ();

	float& GetFOV ();

	void TranslatePos (XMFLOAT3 move);
	void TranslatePos (XMVECTOR move);
	void RotateDir (float hor, float ver);
	void MoveForward (float d);
	void MoveBackward (float d);
	void MoveRight (float d);
	void MoveLeft (float d);

	void RotateHorizontal (float a);
	void RotateVertical (float a);
};
struct Vertex_t
{
	float x, y, z;
	void SetPos (float x_,
				 float y_,
				 float z_);


#ifndef IGNORE_VERTEX_NORMAL
	float nx, ny, nz;
	void SetNormal (float nx_,
					float ny_,
					float nz_);
#endif

#ifndef IGNORE_VERTEX_TEXTURE
	float u, v, w;
	void SetTexture (float u_,
					 float v_,
					 float w_ = 0.0f);
#endif

#ifndef IGNORE_VERTEX_COLOR
	float r, g, b, a;
	void SetColor (float r_,
				   float g_,
				   float b_,
				   float a_ = 0.0f);
#endif
	
};
struct Direct3DObjectBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX InverseView;
	XMMATRIX InverseProjection;
};

__declspec (align (16))
struct CurrentMatrices
{
	Direct3DObjectBuffer objData_;
	XMMATRIX world_;
};

__declspec (align (16))
class Direct3DObject : NZA_t
{
	bool drawIndexed_;
	std::vector<Vertex_t> vertices_;
	std::vector<UINT> indices_;
	CurrentMatrices currM_;
	D3D11_PRIMITIVE_TOPOLOGY topology_;
	bool blending_;
	uint64_t objectId_;

	ID3D11Buffer* vertexBuffer_;
	ID3D11Buffer* indexBuffer_;
	Direct3DConstantBufferManager* cbManager_;
	ConstantBufferIndex_t objectBufferN_;
	bool buffersSet_;
	bool objectBufferSet_;

	ShaderIndex_t vertexShader_;
	ShaderIndex_t pixelShader_;
	ShaderIndex_t geometryShader_;
	LayoutIndex_t layoutN_;
	CRITICAL_SECTION draw_;

	friend class Direct3DProcessor;

	void SetID (uint64_t id);

	void ok () override;

	void SaveLayout (LayoutIndex_t n);

	void SetCBManager (Direct3DConstantBufferManager* cbManager);
	
public:
	Direct3DObject (XMMATRIX& world, 
					bool blending = false,
					bool drawIndexed = false,
					D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	~Direct3DObject ();

	void AddVertexArray (Vertex_t* vert, 
						 size_t n);

	void ClearVertexArray ();

	void AddIndexArray (UINT* ind,
						size_t n);

	void SetupBuffers (ID3D11Device* device);

	//void SetWVP (XMMATRIX& matrix);
	void SetWorld (XMMATRIX& matrix);
	XMMATRIX& GetWorld ();

	void Draw (ID3D11DeviceContext* deviceContext,
			   Direct3DCamera* cam);


	void ClearBuffers ();
	void SetVertexBuffer (ID3D11Device* device);
	void SetIndexBuffer  (ID3D11Device* device);
	void SetObjectBuffer (ID3D11Device* device);
	void EnterCriticalSection ();
	void ExitCriticalSection ();

	std::vector<Vertex_t>& GetVertices ();

	uint64_t GetID();
};


void* GetValidObjectPtr ();