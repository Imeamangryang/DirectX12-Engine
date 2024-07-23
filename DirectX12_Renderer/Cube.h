#pragma once
#pragma once
#include "Renderer.h"
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "DDSTextureLoader.h"
#include "DirectionalLight.h"

using namespace graphics;

class Cube : public Object
{
public:
	Cube(Graphics* renderer);
	~Cube();

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void ClearUnusedUploadBuffersAfterInit();

	void SetIsWireframe(bool isWireframe) { this->isWireframe = isWireframe; }

private:
	void CreateDescriptorHeap(Graphics* Renderer);

	void InitPipeline(Graphics* Renderer);

	void LoadMesh(Graphics* Renderer);

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ID3D12Resource* m_uploadHeap;

	UINT m_width;
	UINT m_height;

	bool isWireframe = false;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateWireframe;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ID3D12Resource* m_CBV;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;
	UINT m_srvDescSize;

	ID3D12Resource* m_vertexBuffer;
	ID3D12Resource* m_vertexBufferUpload;
	ID3D12Resource* m_indexBuffer;
	ID3D12Resource* m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform;

	DirectionalLight m_light;
};