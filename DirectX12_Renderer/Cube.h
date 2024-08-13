#pragma once
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "DirectionalLight.h"
#include "Texture.h"

using namespace graphics;

//// Indirect Command 구조 정의
//// cbv : 상수 버퍼의 GPU 가상 주소
//// drawArguments : DrawInstancedIndirect 함수에 전달되는 인자
//struct IndirectCommand
//{
//	D3D12_GPU_VIRTUAL_ADDRESS cbv;
//	D3D12_DRAW_ARGUMENTS drawArguments;
//};
//
//// Graphics root signature parameter offsets.
//enum GraphicsRootParameters
//{
//	Cbv,
//	GraphicsRootParametersCount
//};

class Cube : public Object
{
public:
	Cube(Graphics* renderer);
	~Cube();

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void SetIsWireframe(bool isWireframe) { this->isWireframe = isWireframe; }
	void CreateInstanceBuffer(Graphics* renderer, const std::vector<InstanceBuffer> instanceData);
	void UpdateInstanceBuffer(const std::vector<InstanceBuffer> instanceData);

	DirectX::BoundingBox GetBoundingBox();
	bool Intersects(DirectX::SimpleMath::Ray ray, float& distance);
	void SetPicked(bool ispicked) { isPicked = !isPicked; }

private:
	void CreateDescriptorHeap(Graphics* Renderer);

	void InitPipeline(Graphics* Renderer);

	void LoadMesh(Graphics* Renderer);
	

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescSize;

	bool isWireframe = false;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateWireframe;
	ComPtr<ID3D12RootSignature> m_rootSignature;

	//ComPtr<ID3D12RootSignature> m_computeRootSignature;
	//ComPtr<ID3D12CommandSignature> m_commandSignature;
	ComPtr<ID3D12QueryHeap> m_queryHeap;
	ComPtr<ID3D12Resource> m_queryResultBuffer;


	ComPtr<ID3D12Resource> m_CBV;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;

	ComPtr<ID3D12Resource> m_StructuredBuffer;
	UINT8* m_StructuredBufferDataBegin;
	UINT instanceCount;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUpload;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexBufferUpload;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform;

	DirectionalLight m_light;

	std::shared_ptr<Texture> m_Tdirt;
	std::shared_ptr<Texture> m_Tstone;
	std::shared_ptr<Texture> m_Tcobblestone;
	std::shared_ptr<Texture> m_Twater;

	// Colider Box
	DirectX::BoundingBox m_boundingBox;
	bool isPicked = false;
};