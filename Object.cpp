
#include "Builder.h"

void Vertex_t::SetPos (float x_,
					   float y_,
					   float z_)
{
	x = x_;
	y = y_;
	z = z_;
}

#ifndef IGNORE_VERTEX_NORMAL
void Vertex_t::SetNormal (float nx_,
					      float ny_,
					      float nz_)
{
	nx = nx_;
	ny = ny_;
	nz = nz_;
}
#endif

#ifndef IGNORE_VERTEX_TEXTURE
void Vertex_t::SetTexture (float u_,
						   float v_,
						   float w_)
{
	u = u_;
	v = v_;
	w = w_;
}
#endif
#ifndef IGNORE_VERTEX_COLOR
void Vertex_t::SetColor (float r_,
						 float g_,
						 float b_,
						 float a_)
{
	r = r_;
	g = g_;
	b = b_;
	a = a_;
}
#endif

void Direct3DObject::ok ()
{
	DEFAULT_OK_BLOCK
}

Direct3DObject::Direct3DObject (XMMATRIX& world,
								bool blending,
								bool drawIndexed,
								D3D11_PRIMITIVE_TOPOLOGY topology) :
	vertices_        (),
	indices_         (),
	topology_        (topology),
	drawIndexed_     (drawIndexed),
	objectId_        (),
	blending_        (blending),
	currM_           ({ { XMMatrixIdentity (), XMMatrixIdentity () } , world }),
	vertexBuffer_    (nullptr),
	indexBuffer_     (nullptr),
	cbManager_       (nullptr),
	objectBufferN_   (0),
	buffersSet_      (false),
	objectBufferSet_ (false),
	vertexShader_    (-1),
	pixelShader_     (-1),
	geometryShader_  (-1),
	layoutN_         (-1),
	draw_            ()
{
	InitializeCriticalSection (&draw_);
}

void Direct3DObject::SetCBManager (Direct3DConstantBufferManager* cbManager)
{
	cbManager_ = cbManager;
}

Direct3DObject::~Direct3DObject ()
{
	vertices_.clear ();
	indices_.clear ();
	objectId_ = 0;
	if (buffersSet_)
	{
		vertexBuffer_->Release ();
		vertexBuffer_ = nullptr;
		if (drawIndexed_)
		{
			indexBuffer_->Release ();
			indexBuffer_ = nullptr;
		}
	}
	DeleteCriticalSection (&draw_);
}

void Direct3DObject::SetID (uint64_t objectId)
{
	BEGIN_EXCEPTION_HANDLING
	objectId_ = objectId;
	END_EXCEPTION_HANDLING (SET_ID)
}

void Direct3DObject::AddVertexArray (Vertex_t* vert, size_t n)
{
	BEGIN_EXCEPTION_HANDLING
	/*if (buffersSet_)
		_EXC_N (BUFFERS_SET, "D3D: Cannot add vertices to buffered object")*/
		vertices_.insert (vertices_.begin (), vert, vert + n);
	END_EXCEPTION_HANDLING (ADD_VERTEX_ARRAY)
}

void Direct3DObject::ClearVertexArray ()
{
	BEGIN_EXCEPTION_HANDLING
	vertices_.clear ();
	END_EXCEPTION_HANDLING (ADD_VERTEX_ARRAY)
}

void Direct3DObject::AddIndexArray (UINT* ind, size_t n)
{
	BEGIN_EXCEPTION_HANDLING
	/*if (buffersSet_)
		_EXC_N (BUFFERS_SET, "D3D: Cannot add indices to buffered object")*/
	indices_.insert (indices_.begin (), ind, ind + n);
	END_EXCEPTION_HANDLING (ADD_INDEX_ARRAY)
}

void Direct3DObject::SetupBuffers (ID3D11Device* device)
{
	BEGIN_EXCEPTION_HANDLING
	if (buffersSet_) ClearBuffers ();
	HRESULT result = S_OK;

	SetVertexBuffer (device);
	SetIndexBuffer (device);
	SetObjectBuffer (device);

	buffersSet_ = true;

	END_EXCEPTION_HANDLING (SETUP_BUFFERS)
}

void Direct3DObject::SetWVP (XMMATRIX& matrix)
{
	currM_.objData_.WVP = matrix;
}

void Direct3DObject::SetWorld (XMMATRIX& matrix)
{
	currM_.objData_.World = matrix;
}

XMMATRIX& Direct3DObject::GetWorld ()
{
	return currM_.world_;
}

void Direct3DObject::Draw (ID3D11DeviceContext* deviceContext, 
						   Direct3DCamera* cam)
{
	BEGIN_EXCEPTION_HANDLING

	EnterCriticalSection ();

	//printf ("%d\n", vertices_.size());
	uint16_t i = rand () % vertices_.size ();
	//printf ("%f %f %f\n", vertices_[i].x, vertices_[i].y, vertices_[i].z);

	if (buffersSet_ == false)
		_EXC_N (BUFFERS_NOT_SET, 
				"D3D: Unable to draw with the buffers not set (obj %d)" _ 
				objectId_)

	if (vertices_.size () == 0)
		_EXC_N (EMPTY_VERTICES, 
				"D3D: Unable to draw object with empty vertex buffer (obj %d)" _ 
				objectId_)

	UINT stride = sizeof (Vertex_t);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers (0, 1, &vertexBuffer_, &stride, &offset);
	if (drawIndexed_)
	{
		if (indices_.size () == 0)
			_EXC_N (EMPTY_INDICES,
					"D3D: Unable to draw object with empty index buffer (obj %d)" _
					objectId_)
			deviceContext->IASetIndexBuffer (indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
	}
	XMMATRIX tempWVP = /*currM_.world_ **/ cam->GetView () * cam->GetProjection ();
	currM_.objData_.WVP = XMMatrixTranspose (tempWVP);
	currM_.objData_.World = XMMatrixTranspose (currM_.world_);



	cbManager_->Update (objectBufferN_, deviceContext);
	cbManager_->SendVSBuffer (objectBufferN_, deviceContext);
	cbManager_->SendGSBuffer (objectBufferN_, deviceContext);

	deviceContext->IASetPrimitiveTopology (topology_);

	if (drawIndexed_)
		deviceContext->DrawIndexed (static_cast<UINT> (indices_.size ()), 0, 0);
	else
		deviceContext->Draw (static_cast<UINT> (vertices_.size ()), 0);
	ExitCriticalSection ();

	END_EXCEPTION_HANDLING (DRAW_OBJECT)

}

void Direct3DObject::SaveLayout (LayoutIndex_t n)
{
	layoutN_ = n;
}


void Direct3DObject::ClearBuffers ()
{
	BEGIN_EXCEPTION_HANDLING

	if (!buffersSet_) return;
		//_EXC_N (BUFFERS_NOT_SET, "D3D: Unable to clear empty buffers (obj %d)" _
		//		objectId_)

	buffersSet_ = false;

	if (vertexBuffer_)
		vertexBuffer_->Release ();
	vertexBuffer_ = nullptr;

	if (indexBuffer_)
		indexBuffer_->Release ();
	indexBuffer_ = nullptr;

	END_EXCEPTION_HANDLING (CLEAR_BUFFERS)
}

void Direct3DObject::SetVertexBuffer (ID3D11Device* device)
{
	BEGIN_EXCEPTION_HANDLING

		//printf ("Setting\n");

	if (vertexBuffer_ != nullptr)
	{
		vertexBuffer_->Release ();
		vertexBuffer_ = nullptr;
	}

	HRESULT result = S_OK;
	//printf ("vertex A\n");
	if (vertices_.size () == 0)
	_EXC_N (EMPTY_VERTICES, "D3D: Cannot create empty vertex buffer (obj %d)" _ objectId_)
	D3D11_BUFFER_DESC bufferDesc = {};

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = static_cast<UINT> (sizeof (Vertex_t) * vertices_.size ());
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA vertexSd = { vertices_.data () };

	result = device->CreateBuffer (&bufferDesc,
								   &vertexSd,
								   &vertexBuffer_);
	//printf ("vertex B\n");

	if (result != S_OK)
		_EXC_N (VERTEX_BUFFER, "D3D: Failed to create vertex buffer (obj %d)" _ objectId_)

	
	END_EXCEPTION_HANDLING (SET_VERTEX_BUFFER)
}

void Direct3DObject::SetIndexBuffer (ID3D11Device* device)
{
	BEGIN_EXCEPTION_HANDLING

	if (drawIndexed_ && indices_.size () != 0)
	{
		if (indexBuffer_ != nullptr)
		{
			indexBuffer_->Release ();
			indexBuffer_ = nullptr;
		}
		D3D11_BUFFER_DESC bufferDesc = {};
		HRESULT result = S_OK;

		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = static_cast<UINT> (indices_.size () * sizeof (UINT));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA indexSd = { indices_.data () };

		result = device->CreateBuffer (&bufferDesc,
										&indexSd,
										&indexBuffer_);
		//printf ("index A\n");
		if (result != S_OK)
			_EXC_N (INDEX_BUFFER, "D3D: Failed to create index buffer (obj %d)" _ objectId_)
	}
	//printf ("index B\n");
	if (drawIndexed_ && indices_.size () == 0)
		_EXC_N (EMPTY_INDICES, "D3D: Cannot create empty index buffer (obj %d)" _ objectId_)
	END_EXCEPTION_HANDLING (SET_INDEX_BUFFER)
}

void Direct3DObject::SetObjectBuffer (ID3D11Device* device)
{
	BEGIN_EXCEPTION_HANDLING

		//printf ("object A\n");
	if (!cbManager_)
		_EXC_N (NULL_CB_MANAGER, "D3D: Cannot set up object buffer without buffer manager (obj %d)" _ objectId_)

		//printf ("object B\n");
		if (objectBufferSet_) return;
		//_EXC_N (OBJECT_BUFFER_SET, "D3D: Object buffer has already been set (obj %d)" _ objectId_)

		//printf ("object C\n");
	
	objectBufferN_ = cbManager_->Bind (&currM_.objData_,
									   sizeof (Direct3DObjectBuffer), 
									   0, 
									   device);
	objectBufferSet_ = true;

	END_EXCEPTION_HANDLING (SET_OBJECT_BUFFER)
}



void Direct3DObject::EnterCriticalSection ()
{
	::EnterCriticalSection (&draw_);
}
void Direct3DObject::ExitCriticalSection ()
{
	::LeaveCriticalSection (&draw_);
}

std::vector<Vertex_t>& Direct3DObject::GetVertices ()
{
	return vertices_;
}


void Direct3DCamera::ok ()
{
	DEFAULT_OK_BLOCK
		if (wnd_ == nullptr)
			_EXC_N (NULL_WND, "D3D: Null window");
}

Direct3DCamera::Direct3DCamera (WindowClass* wnd,
								float posX, float posY, float posZ,
								float tgtX, float tgtY, float tgtZ,
								float upX, float upY, float upZ,
								float fov, float zNear, float zFar)
	try :
	wnd_ (wnd),
	position_ (XMVectorSet (posX, posY, posZ, 0.0f)),
	target_ (XMVectorSet (tgtX, tgtY, tgtZ, 0.0f)),
	up_ (XMVectorSet (upX, upY, upZ, 0.0f)),
	projectionSettings_ (fov, zNear, zFar),
	view_ (XMMatrixLookAtLH (position_, target_, up_)),
	projection_ (XMMatrixIdentity ())
{
	if (wnd == nullptr)
		_EXC_N (NULL_WND,
				"D3D: Cannot create camera over null window");

	projection_ = XMMatrixPerspectiveFovLH (projectionSettings_.x * 3.141592f,
											((float)wnd_->width ()) / wnd_->height (),
											projectionSettings_.y,
											projectionSettings_.z);
}
_END_EXCEPTION_HANDLING (CTOR)

void Direct3DCamera::Update ()
{
	BEGIN_EXCEPTION_HANDLING

		view_ = XMMatrixLookAtLH (position_, target_, up_);
	projection_ = XMMatrixPerspectiveFovLH (projectionSettings_.x * 3.141592f,
											((float)wnd_->width ()) / wnd_->height (),
											projectionSettings_.y,
											projectionSettings_.z);

	END_EXCEPTION_HANDLING (UPDATE_CAMERA)
}


const XMMATRIX& Direct3DCamera::GetView ()
{
	return view_;
}
const XMMATRIX& Direct3DCamera::GetProjection ()
{
	return projection_;
}


void* GetValidObjectPtr ()
{
	return _aligned_malloc (sizeof (Direct3DObject), 16);
}
