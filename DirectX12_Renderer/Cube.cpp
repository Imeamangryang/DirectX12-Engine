#include "Cube.h"

Cube::Cube(Graphics* renderer) : Object(renderer),
m_pipelineState(nullptr),
m_rootSignature(nullptr),
m_srvHeap(nullptr),
m_uploadHeap(nullptr),
m_width(0),
m_height(0),
m_CBV(nullptr),
m_cbvDataBegin(nullptr),
m_vertexBuffer(nullptr),
m_vertexBufferUpload(nullptr),
m_indexBuffer(nullptr),
m_indexBufferUpload(nullptr),
m_worldTransform(MathHelper::Identity4x4())
{
	InitPipeline(renderer);

	LoadMesh(renderer);

	m_objectname = "Plane";

	m_translation_x = -200.0f;

}

Cube::~Cube()
{
	if (m_srvHeap)
	{
		m_srvHeap->Release();
		m_srvHeap = nullptr;
	}
	if (m_uploadHeap)
	{
		m_uploadHeap->Release();
		m_uploadHeap = nullptr;
	}
	if (m_indexBufferUpload)
	{
		m_indexBufferUpload->Release();
		m_indexBufferUpload = nullptr;
	}
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}
	if (m_vertexBufferUpload)
	{
		m_vertexBufferUpload->Release();
		m_vertexBufferUpload = nullptr;
	}
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
	if (m_CBV)
	{
		m_CBV->Unmap(0, nullptr);
		m_cbvDataBegin = nullptr;
		m_CBV->Release();
		m_CBV = nullptr;
	}
}

void Cube::Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye)
{
	if (isWireframe == false) {
		m_commandList->SetPipelineState(m_pipelineState.Get());
	}
	else {
		m_commandList->SetPipelineState(m_pipelineStateWireframe.Get());
	}
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// Transform
	XMStoreFloat4x4(&m_worldTransform,
		XMMatrixTransformation(
			XMVectorZero(),
			XMVectorZero(),
			XMVectorSet(m_scale_x * 100.0f, m_scale_y * 100.0f, m_scale_z * 100.0f, 100.0f),
			XMVectorZero(),
			XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_rotation_x), XMConvertToRadians(m_rotation_y), XMConvertToRadians(m_rotation_z)),
			XMVectorSet(m_translation_x, m_translation_y, m_translation_z, 0.0)
		));

	m_constantBufferData.world = m_worldTransform;
	m_constantBufferData.viewproj = viewproj;
	m_constantBufferData.eye = eye;
	m_constantBufferData.height = m_height;
	m_constantBufferData.width = m_width;
	m_constantBufferData.light = m_light.GetDirectionalLight();
	memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(ConstantBuffer));

	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 3, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, cbvHandle);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cubeMap->GetGPUHandle()); // t0 : CubeMap , t1 : NormalMap, t2 : HeightMap

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST); // describe how to read the vertex buffer.
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	m_commandList->DrawIndexedInstanced(m_indexcount, 1, 0, 0, 0);
}

void Cube::ClearUnusedUploadBuffersAfterInit()
{
	if (m_uploadHeap)
	{
		m_uploadHeap->Release();
		m_uploadHeap = nullptr;
	}
	if (m_indexBufferUpload)
	{
		m_indexBufferUpload->Release();
		m_indexBufferUpload = nullptr;
	}
	if (m_vertexBufferUpload)
	{
		m_vertexBufferUpload->Release();
		m_vertexBufferUpload = nullptr;
	}
}

void Cube::CreateDescriptorHeap(Graphics* Renderer)
{
	// Discriptor Heap 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Renderer->CreateDescriptorHeap(&srvHeapDesc, m_srvHeap);

	m_srvDescSize = Renderer->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Texture 로딩
	m_cubeMap = std::make_unique<Texture>(Renderer, m_srvHeap, L"resource/tile.dds", 0);
	m_normalMap = std::make_unique<Texture>(Renderer, m_srvHeap, L"resource/tile_normal.png", 1);
	m_heightMap = std::make_unique<Texture>(Renderer, m_srvHeap, L"resource/tile_height.png", 2);

	// ConstantBuffer 생성
	UINT64 bufferSize = sizeof(ConstantBuffer);
	Renderer->CreateBuffer(m_CBV, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
	m_CBV->SetName(L"CBV");

	// ConstantBufferView 생성
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc = {};
	cbvDesc.BufferLocation = m_CBV->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (bufferSize + 255) & ~255; // Constant Buffer는 256 byte aligned

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 3, m_srvDescSize);

	Renderer->CreateCBV(&cbvDesc, srvHandle);

	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

	// ConstantBuffer를 위한 메모리 매핑
	CD3DX12_RANGE readRange(0, 0);
	if (FAILED(m_CBV->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin))))
	{
		throw (GFX_Exception("Failed to map CBV in Cube."));
	}
}

void Cube::InitPipeline(Graphics* Renderer)
{
	CreateDescriptorHeap(Renderer);

	// Root Signature 생성
	CD3DX12_DESCRIPTOR_RANGE range[2];
	CD3DX12_ROOT_PARAMETER paramsRoot[2];
	// Slot : Displacement Map, Register(t0)
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); // t0, t1, t2
	paramsRoot[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_ALL);

	// ConstantBufferview를 위한 Root Parameter
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	paramsRoot[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_STATIC_SAMPLER_DESC descSamplers[2];
	descSamplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
	descSamplers[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(
		_countof(paramsRoot),
		paramsRoot,
		2,
		descSamplers,
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Renderer->createRootSignature(&rootDesc, m_rootSignature);

	// Shader Compile
	D3D12_SHADER_BYTECODE PSBytecode = {};
	D3D12_SHADER_BYTECODE VSBytecode = {};
	D3D12_SHADER_BYTECODE HSBytecode = {};
	D3D12_SHADER_BYTECODE DSBytecode = {};
	Renderer->CompileShader(L"shader.fx", "VS", VSBytecode, VERTEX_SHADER);
	Renderer->CompileShader(L"shader.fx", "PS", PSBytecode, PIXEL_SHADER);
	Renderer->CompileShader(L"shader.fx", "HS", HSBytecode, HULL_SHADER);
	Renderer->CompileShader(L"shader.fx", "DS", DSBytecode, DOMAIN_SHADER);

	// Input Layout 생성
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_INPUT_LAYOUT_DESC	inputLayoutDesc = {};
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.VS = VSBytecode;
	psoDesc.PS = PSBytecode;
	psoDesc.HS = HSBytecode;
	psoDesc.DS = DSBytecode;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	Renderer->createPSO(&psoDesc, m_pipelineState);

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	Renderer->createPSO(&psoDesc, m_pipelineStateWireframe);
}

void Cube::LoadMesh(Graphics* Renderer)
{
	std::vector<Vertex> vertices;

	// Fill in the front face vertex data.
	vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(+1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

	// Fill in the back face vertex data.
	vertices.push_back(Vertex(-1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(+1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(+1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

	// Fill in the top face vertex data.
	vertices.push_back(Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

	// Fill in the bottom face vertex data.
	vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(+1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(+1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(-1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

	// Fill in the left face vertex data.
	vertices.push_back(Vertex(-1.0f, -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(-1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(-1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));

	// Fill in the right face vertex data.
	vertices.push_back(Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(+1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

	std::vector<UINT> indices;

	// Fill in the front face index data
	indices.push_back(0); indices.push_back(1); indices.push_back(2);
	indices.push_back(0); indices.push_back(2); indices.push_back(3);

	// Fill in the back face index data
	indices.push_back(4); indices.push_back(5); indices.push_back(6);
	indices.push_back(4); indices.push_back(6); indices.push_back(7);

	// Fill in the top face index data
	indices.push_back(8); indices.push_back(9); indices.push_back(10);
	indices.push_back(8); indices.push_back(10); indices.push_back(11);

	// Fill in the bottom face index data
	indices.push_back(12); indices.push_back(13); indices.push_back(14);
	indices.push_back(12); indices.push_back(14); indices.push_back(15);

	// Fill in the left face index data
	indices.push_back(16);  indices.push_back(17); indices.push_back(18);
	indices.push_back(16); indices.push_back(18); indices.push_back(19);

	// Fill in the right face index data
	indices.push_back(20); indices.push_back(21); indices.push_back(22);
	indices.push_back(20); indices.push_back(22); indices.push_back(23);

	m_vertexcount = vertices.size();
	m_indexcount = indices.size();

	int vbByteSize = sizeof(Vertex) * vertices.size();

	Renderer->CreateCommittedBuffer(m_vertexBuffer, m_vertexBufferUpload, &CD3DX12_RESOURCE_DESC::Buffer(vbByteSize));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = &vertices[0];
	vertexData.RowPitch = vbByteSize;
	vertexData.SlicePitch = vbByteSize;

	UpdateSubresources(Renderer->GetCommandList().Get(), m_vertexBuffer, m_vertexBufferUpload, 0, 0, 1, &vertexData);
	Renderer->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vbByteSize;

	int ibByteSize = sizeof(UINT) * indices.size();

	Renderer->CreateCommittedBuffer(m_indexBuffer, m_indexBufferUpload, &CD3DX12_RESOURCE_DESC::Buffer(ibByteSize));

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = &indices[0];
	indexData.RowPitch = ibByteSize;
	indexData.SlicePitch = ibByteSize;

	UpdateSubresources(Renderer->GetCommandList().Get(), m_indexBuffer, m_indexBufferUpload, 0, 0, 1, &indexData);
	Renderer->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = ibByteSize;
}