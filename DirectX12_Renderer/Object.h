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

	size_t m_indexcount;
	size_t m_vertexcount;

private:

	ComPtr<ID3D12DescriptorHeap> m_srvHeap = nullptr;
	ComPtr<ID3D12Resource> m_uploadHeap = nullptr;

	UINT m_srvDescSize;

	ComPtr<ID3D12Resource> m_vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_indexBuffer = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferUpload = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform = MathHelper::Identity4x4();
};