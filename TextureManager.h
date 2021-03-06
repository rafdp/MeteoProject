
#pragma once

#include "includes.h"


class Direct3DTextureManager : NZA_t
{
	std::vector<ID3D11ShaderResourceView*> textures_;
	std::map<std::string, TextureIndex_t> loaded_;

public:
	void ok() override;
	Direct3DTextureManager();
	~Direct3DTextureManager();

	TextureIndex_t LoadTexture(std::string filename,
		ID3D11Device* device);
	TextureIndex_t RegisterTexture (ID3D11ShaderResourceView* text);
	ID3D11ShaderResourceView* GetTexture (TextureIndex_t n);

};