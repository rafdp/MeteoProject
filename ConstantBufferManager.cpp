#include "Builder.h"

	
void Direct3DConstantBufferManager::ok ()
{
	DEFAULT_OK_BLOCK
}

Direct3DConstantBufferManager::Direct3DConstantBufferManager ()
try :
	buffers_ ()
{}
_END_EXCEPTION_HANDLING (CTOR)

Direct3DConstantBufferManager::~Direct3DConstantBufferManager ()
{
	for (auto i = buffers_.begin ();
	          i < buffers_.end ();
		      i++)
	{ if (i->buffer) i->buffer->Release (); } 

	buffers_.clear ();
}

ConstantBufferIndex_t Direct3DConstantBufferManager::Bind (void* data,
														   size_t size, 
														   UINT slot,
														   ID3D11Device* device)
{
	BEGIN_EXCEPTION_HANDLING
	BufferInfo_t newBuffer = {nullptr, data, slot};
	D3D11_BUFFER_DESC bufferDesc = {};
	HRESULT result = S_OK;

	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = static_cast <UINT> (size);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	result = device->CreateBuffer (&bufferDesc,
								   NULL,
								   &newBuffer.buffer);

	buffers_.push_back (newBuffer);

	if (result != S_OK)
		_EXC_N (CREATE_CONSTANT_BUFFER, "D3D: Failed to create constant buffer")

	return buffers_.size () - 1;
	END_EXCEPTION_HANDLING (BIND)
}

void Direct3DConstantBufferManager::Update (ConstantBufferIndex_t n,
											ID3D11DeviceContext* deviceContext)
{
	deviceContext->UpdateSubresource (buffers_[n].buffer,
									  0,
									  NULL,
									  buffers_[n].data,
									  0,
									  0);
}
void  Direct3DConstantBufferManager::SendVSBuffer (ConstantBufferIndex_t n, 
												   ID3D11DeviceContext* deviceContext)
{
	deviceContext->VSSetConstantBuffers (buffers_[n].slot,
										 1,
										 &buffers_[n].buffer);
}

void  Direct3DConstantBufferManager::SendPSBuffer (ConstantBufferIndex_t n, 
												   ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetConstantBuffers (buffers_[n].slot,
										 1,
										 &buffers_[n].buffer);
}

void  Direct3DConstantBufferManager::SendGSBuffer (ConstantBufferIndex_t n, 
												   ID3D11DeviceContext* deviceContext)
{
	deviceContext->GSSetConstantBuffers (buffers_[n].slot,
										 1,
										 &buffers_[n].buffer);
}