#pragma once

#include "Renderer.h"
#include "stdafx.h"

using namespace graphics;

class ImGuiLoader
{
public:
	ImGuiLoader(Graphics* renderer);
	~ImGuiLoader();

	void Draw();

private:
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescSize;
};
