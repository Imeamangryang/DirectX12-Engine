#pragma once
#include "Renderer.h"
#include "stdafx.h"
#include "Structures.h"

using namespace graphics;

class Object
{
public:
	Object(Graphics* renderer);
	~Object() = default;

	// GUI ±¸¼º¿ë
	string m_objectname = "";

	float m_translation_x = 0.0f;
	float m_translation_y = 0.0f;
	float m_translation_z = 0.0f;

	float m_rotation_x = 0.0f;
	float m_rotation_y = 0.0f;
	float m_rotation_z = 0.0f;

	float m_scale_x = 1.0f;
	float m_scale_y = 1.0f;
	float m_scale_z = 1.0f;

	UINT m_indexcount;
	UINT m_vertexcount;

private:

	ComPtr<ID3D12DescriptorHeap> m_srvHeap = nullptr;
	ID3D12Resource* m_uploadHeap = nullptr;
	UINT m_width = 0; 
	UINT m_height = 0;

	ID3D12Resource* m_CBV = nullptr;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;
	UINT m_srvDescSize;

	ID3D12Resource* m_vertexBuffer = nullptr;
	ID3D12Resource* m_vertexBufferUpload = nullptr;
	ID3D12Resource* m_indexBuffer = nullptr;
	ID3D12Resource* m_indexBufferUpload = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform = MathHelper::Identity4x4();
};