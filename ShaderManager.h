#pragma once

#include "includes.h"


enum SHADER_TYPES
{
	SHADER_VERTEX = 1,
	SHADER_PIXEL,
	SHADER_GEOMETRY,
	SHADER_NOT_SET
};


struct ShaderDesc_t
{
	UINT shaderType;
	ID3D10Blob* blob;
	void* shader;
};

class Direct3DShaderManager : NZA_t
{
	std::vector<ShaderDesc_t>   shaders_;
	std::map<std::string, ShaderIndex_t> loaded_;

public:
	Direct3DShaderManager ();
	~Direct3DShaderManager ();

	void ok () override;

	ShaderIndex_t LoadShader (std::string filename,
							  std::string function,
							  SHADER_TYPES shaderType,
							  ID3D11Device* device);

	void* GetShader (ShaderIndex_t n);
	ID3D10Blob* GetBlob (ShaderIndex_t n);
	UINT GetType (ShaderIndex_t n);
	bool CheckShaderType (ShaderIndex_t n);
	void ReloadShaders (ID3D11Device* device);
};