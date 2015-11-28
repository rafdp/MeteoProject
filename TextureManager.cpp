#include "Builder.h"

void Direct3DTextureManager::ok()
{
	DEFAULT_OK_BLOCK
}

Direct3DTextureManager::Direct3DTextureManager()
try :
	textures_(),
	loaded_()
{

}
_END_EXCEPTION_HANDLING(CTOR)

Direct3DTextureManager::~Direct3DTextureManager()
{
	for (auto i = textures_.begin();
	i < textures_.end();
		i++)
	{
		if (*i) (*i)->Release();
	}
	textures_.clear();
	loaded_.clear();
}

TextureIndex_t Direct3DTextureManager::LoadTexture(std::string filename,
	ID3D11Device* device)
{
	BEGIN_EXCEPTION_HANDLING
		auto found = loaded_.find(filename);
	if (found != loaded_.end()) return found->second;
	ID3D11ShaderResourceView* newResource = nullptr;
	HRESULT result = S_OK;
	result = D3DX11CreateShaderResourceViewFromFileA(device,
		filename.c_str(),
		NULL,
		NULL,
		&newResource,
		NULL);
	if (result != S_OK)
		_EXC_N(LOAD_TEXTURE_FILE,
			"D3D: Failed to load texture from file (%s)" _
			filename.c_str())

		textures_.push_back(newResource);
	TextureIndex_t index = textures_.size() - 1;
	loaded_[filename] = index;

	return index;

	END_EXCEPTION_HANDLING(LOAD_TEXTURE)
}

TextureIndex_t Direct3DTextureManager::RegisterTexture(ID3D11ShaderResourceView* text)
{
	BEGIN_EXCEPTION_HANDLING
		auto found = find (textures_.begin(), textures_.end(), text);
	if (found != textures_.end()) return found - textures_.begin ();
	if (!text) return -1;
	HRESULT result = S_OK;
	textures_.push_back(text);

	return textures_.size() - 1;

	END_EXCEPTION_HANDLING(REGISTER_TEXTURE)
}

ID3D11ShaderResourceView* Direct3DTextureManager::GetTexture(TextureIndex_t n)
{
	BEGIN_EXCEPTION_HANDLING
		if (n >= textures_.size() || textures_[n] == nullptr)
			_EXC_N(OUT_OF_RANGE, "D3D: Invalid texture index (%d of %d available)" _
				n _
				textures_.size())
			return textures_[n];

	END_EXCEPTION_HANDLING(GET_TEXTURE)
}
