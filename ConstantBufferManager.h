#pragma once

#include "includes.h"

struct BufferInfo_t
{
	ID3D11Buffer* buffer;
	void* data;
	UINT slot;
};


class Direct3DConstantBufferManager : NZA_t
{
	std::vector<BufferInfo_t> buffers_;
public:
	void ok () override;
	Direct3DConstantBufferManager ();
	~Direct3DConstantBufferManager ();

	ConstantBufferIndex_t Bind (void* data,
							    size_t size, 
							    UINT slot,
							    ID3D11Device* device);

	void Update (ConstantBufferIndex_t n, 
				 ID3D11DeviceContext* deviceContext);

	void SendVSBuffer (ConstantBufferIndex_t n, 
					   ID3D11DeviceContext* deviceContext);
	void SendPSBuffer (ConstantBufferIndex_t n, 
					   ID3D11DeviceContext* deviceContext);
	void SendGSBuffer (ConstantBufferIndex_t n, 
					   ID3D11DeviceContext* deviceContext);
};