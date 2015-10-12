#include "Builder.h"

bool Direct3DShaderManager::CheckShaderType (ShaderIndex_t n)
{
	if (n >= shaders_.size ())
		_EXC_N (OUT_OF_RANGE, "D3D: Out of range index (%d of %d)" _ n _ shaders_.size ())
	if (shaders_[n].shaderType < SHADER_VERTEX || 
		shaders_[n].shaderType >= SHADER_NOT_SET)
		return false;
	return true;
}

void Direct3DShaderManager::ReloadShaders (ID3D11Device* device)
{
	for (auto i = loaded_.begin ();
	i != loaded_.end ();
		i++)
	{
		std::string file (i->first.begin (),
						  i->first.begin () + i->first.find (':'));

		std::string shader (i->first.begin () + i->first.find (':') + 1,
						    i->first.end ());

		HRESULT result = S_OK;

		//shaders_.push_back ({});
		ShaderDesc_t& currShader = shaders_[i->second];

		std::string version, name;
		switch (shaders_[i->second].shaderType)
		{
			case SHADER_VERTEX:
				version = "vs_4_0";
				name = "vertex";
				break;
			case SHADER_PIXEL:
				version = "ps_4_0";
				name = "pixel";
				break;
			case SHADER_GEOMETRY:
				version = "gs_4_0";
				name = "geometry";
				break;
			default:
				_EXC_N (SHADER_TYPE, "D3D: Invalid shader type, cannot load")
		}


		result = D3DX11CompileFromFileA (file.c_str (),
										 0,
										 0,
										 shader.c_str (),
										 version.c_str (),
										 0,
										 0,
										 0,
										 &currShader.blob,
										 0,
										 0);

		if (result != S_OK)
			_EXC_N (LOAD_SHADER_BLOB, "D3D: Failed to load %s shader (%s) from file (%s) (0x%x)" _
					name.c_str () _
					shader.c_str () _
					file.c_str () _
					result)

			switch (shaders_[i->second].shaderType)
			{
				case SHADER_VERTEX:
					result = device->CreateVertexShader (currShader.blob->GetBufferPointer (),
														 currShader.blob->GetBufferSize (),
														 NULL,
														 reinterpret_cast <ID3D11VertexShader**> (&currShader.shader));
					break;
				case SHADER_PIXEL:
					result = device->CreatePixelShader (currShader.blob->GetBufferPointer (),
														currShader.blob->GetBufferSize (),
														NULL,
														reinterpret_cast <ID3D11PixelShader**> (&currShader.shader));
					break;
				case SHADER_GEOMETRY:
					result = device->CreateGeometryShader (currShader.blob->GetBufferPointer (),
														   currShader.blob->GetBufferSize (),
														   NULL,
														   reinterpret_cast <ID3D11GeometryShader**> (&currShader.shader));
					break;
				default:
					break;
			}
		if (result != S_OK)
			_EXC_N (CREATE_SHADER, "Failed to create %s shader from blob" _ name.c_str ())
	}
}

void Direct3DShaderManager::ok ()
{
	DEFAULT_OK_BLOCK
}

Direct3DShaderManager::Direct3DShaderManager ()
try :
	shaders_ (),
	loaded_  ()
{

}
_END_EXCEPTION_HANDLING (CTOR)

Direct3DShaderManager::~Direct3DShaderManager ()
{
	for (auto i = shaders_.begin ();
	i < shaders_.end ();
		i++)
	{
		if (i->blob) i->blob->Release ();
		if (i->shader)
		{
			(reinterpret_cast <ID3D11DeviceChild*> (i->shader))->Release ();
		}
	}
	shaders_.clear ();

	loaded_.clear ();
}

ShaderIndex_t Direct3DShaderManager::LoadShader (std::string filename,
											 	 std::string function,
										 		 SHADER_TYPES shaderType,
									 			 ID3D11Device* device)
{
	BEGIN_EXCEPTION_HANDLING
	std::string storing (filename);
	storing += ':';
	storing += function;
	auto found = loaded_.find (storing);
	if (found != loaded_.end ()) return found->second;

	HRESULT result = S_OK;

	shaders_.push_back ({});
	ShaderDesc_t& currShader = shaders_.back ();

	std::string version, name;
	switch (shaderType)
	{
		case SHADER_VERTEX:
			version = "vs_4_0";
			name = "vertex";
			currShader.shaderType = SHADER_VERTEX;
			break;
		case SHADER_PIXEL:
			version = "ps_4_0";
			name = "pixel";
			currShader.shaderType = SHADER_PIXEL;
			break;
		case SHADER_GEOMETRY:
			version = "gs_4_0";
			name = "geometry";
			currShader.shaderType = SHADER_GEOMETRY;
			break;
		default:
			_EXC_N (SHADER_TYPE, "D3D: Invalid shader type, cannot load")
			return -1;
	}


	result = D3DX11CompileFromFileA (filename.c_str (),
									 0,
									 0,
									 function.c_str (),
									 version.c_str (),
									 0,
									 0,
									 0,
									 &currShader.blob,
									 0,
									 0);

	if (result != S_OK)
		_EXC_N (LOAD_SHADER_BLOB, "D3D: Failed to load %s shader (%s) from file (%s) (0x%x)" _
				name.c_str () _
				function.c_str () _
				filename.c_str () _ 
				result)

	switch (shaderType)
	{
		case SHADER_VERTEX:
			result = device->CreateVertexShader (currShader.blob->GetBufferPointer (),
												 currShader.blob->GetBufferSize (),
												 NULL,
												 reinterpret_cast <ID3D11VertexShader**> (&currShader.shader));
			break;
		case SHADER_PIXEL:
			result = device->CreatePixelShader (currShader.blob->GetBufferPointer (),
											    currShader.blob->GetBufferSize (),
											    NULL,
											    reinterpret_cast <ID3D11PixelShader**> (&currShader.shader));
			break;
		case SHADER_GEOMETRY:
			result = device->CreateGeometryShader (currShader.blob->GetBufferPointer (),
												   currShader.blob->GetBufferSize (),
												   NULL,
												   reinterpret_cast <ID3D11GeometryShader**> (&currShader.shader));
			break;
		default:
			break;
	}
	if (result != S_OK)
		_EXC_N (CREATE_SHADER, "Failed to create %s shader from blob" _ name.c_str ())

	ShaderIndex_t index = shaders_.size () - 1;
	loaded_[storing] = index;
	return index;
	END_EXCEPTION_HANDLING (LOAD_SHADER)
}

void* Direct3DShaderManager::GetShader (ShaderIndex_t n)
{
	BEGIN_EXCEPTION_HANDLING
	if (n >= shaders_.size ())
		_EXC_N (OUT_OF_RANGE, "D3D: Out of range shader")

	if (!CheckShaderType (n))
		_EXC_N (SHADER_TYPE, "D3D: Invalid shader type")


	if (shaders_[n].blob == nullptr)
		_EXC_N (NULL_BLOB, "D3D: Invalid index")

	return shaders_[n].shader;

	END_EXCEPTION_HANDLING (GET_VERTEX_SHADER)
}


ID3D10Blob* Direct3DShaderManager::GetBlob (ShaderIndex_t n)
{
	BEGIN_EXCEPTION_HANDLING

	if (n >= shaders_.size ())
		_EXC_N (OUT_OF_RANGE, "D3D: Out of range shader")

	if (shaders_[n].blob == nullptr)
		_EXC_N (NULL_BLOB, "D3D: Invalid index")

	return shaders_[n].blob;

	END_EXCEPTION_HANDLING (GET_BLOB)
}

UINT Direct3DShaderManager::GetType (ShaderIndex_t n)
{
	if (n >= shaders_.size ())
		_EXC_N (OUT_OF_RANGE, "D3D: Out of range shader")

	return shaders_[n].shaderType;
}
