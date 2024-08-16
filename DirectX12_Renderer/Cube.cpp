#include "Cube.h"

Cube::Cube(Graphics* renderer) : Object(renderer),
m_worldTransform(MathHelper::Identity4x4())
{
	InitPipeline(renderer);

	LoadMesh(renderer);

	m_objectname = "Plane";

}

Cube::~Cube()
{
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
			XMVectorSet(m_scale_x, m_scale_y, m_scale_z, 1.0f),
			XMVectorZero(),
			XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_rotation_x), XMConvertToRadians(m_rotation_y), XMConvertToRadians(m_rotation_z)),
			XMVectorSet(m_translation_x, m_translation_y, m_translation_z, 0.0)
		));

	m_constantBufferData.world = m_worldTransform;
	m_constantBufferData.viewproj = viewproj;
	m_constantBufferData.eye = eye;
	m_constantBufferData.light = m_light.GetDirectionalLight();
	memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(ConstantBuffer));

	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 4, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, cbvHandle);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_Tdirt->GetGPUHandle()); // t0 : Dirt , t1 : Stone, t2 : HeightMap. t3 : Water
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 5, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(2, srvHandle);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // describe how to read the vertex buffer.
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	m_commandList->DrawIndexedInstanced(36, instanceCount, 0, 0, 0);
}

void Cube::CreateDescriptorHeap(Graphics* Renderer)
{
	// Discriptor Heap 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 10;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Renderer->CreateDescriptorHeap(&srvHeapDesc, m_srvHeap);
	m_srvHeap->SetName(L"Cube Descriptor Heap");

	m_srvDescSize = Renderer->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Texture 로딩
	m_Tdirt = std::make_shared<Texture>(Renderer, m_srvHeap, L"resource/textures/blocks/dirt.png", 0);
	m_Tstone = std::make_shared<Texture>(Renderer, m_srvHeap, L"resource/textures/blocks/stone.png", 1);
	m_Tcobblestone = std::make_shared<Texture>(Renderer, m_srvHeap, L"resource/textures/blocks/cobblestone.png", 2);
	m_Twater = std::make_shared<Texture>(Renderer, m_srvHeap, L"resource/textures/blocks/water.png", 3);

	// ConstantBuffer 생성
	UINT64 bufferSize = sizeof(ConstantBuffer);
	Renderer->CreateBuffer(m_CBV, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
	m_CBV->SetName(L"Cube Constant Buffer");

	// ConstantBufferView 생성
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc = {};
	cbvDesc.BufferLocation = m_CBV->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (bufferSize + 255) & ~255; // Constant Buffer는 256 byte aligned

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 4, m_srvDescSize);

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

	//// Occlusion Query를 위한 QueryHeap 생성
	//D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	//queryHeapDesc.Count = 1;
	//queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	//Renderer->GetDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_queryHeap));

	//// QueryResult를 저장할 버퍼 생성
	//D3D12_RESOURCE_DESC queryResultDesc = CD3DX12_RESOURCE_DESC::Buffer(8);
	//Renderer->GetDevice()->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//	D3D12_HEAP_FLAG_NONE,
	//	&queryResultDesc,
	//	D3D12_RESOURCE_STATE_PREDICATION,
	//	nullptr,
	//	IID_PPV_ARGS(&m_queryResultBuffer));
	//m_queryResultBuffer->SetName(L"Cube queryResultBuffer");

	// Root Signature 생성
	CD3DX12_DESCRIPTOR_RANGE range[3];
	CD3DX12_ROOT_PARAMETER paramsRoot[3];
	// Slot : Displacement Map, Register(t0)
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); // t0, t1, t2. t3
	paramsRoot[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_ALL);

	// ConstantBufferview를 위한 Root Parameter
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0
	paramsRoot[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);

	// InstanceBufferView를 위한 Root Parameter
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); // t4
	paramsRoot[2].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_STATIC_SAMPLER_DESC descSamplers[2];
	descSamplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
	descSamplers[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_POINT);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(
		_countof(paramsRoot),
		paramsRoot,
		2,
		descSamplers,
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Renderer->createRootSignature(&rootDesc, m_rootSignature);
	m_rootSignature->SetName(L"Cube Root Signature");

	// Shader Compile
	D3D12_SHADER_BYTECODE PSBytecode = {};
	D3D12_SHADER_BYTECODE VSBytecode = {};
	Renderer->CompileShader(L"shader.fx", "VS", VSBytecode, VERTEX_SHADER);
	Renderer->CompileShader(L"shader.fx", "PS", PSBytecode, PIXEL_SHADER);

	// Input Layout 생성
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	D3D12_INPUT_LAYOUT_DESC	inputLayoutDesc = {};
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.VS = VSBytecode;
	psoDesc.PS = PSBytecode;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	Renderer->createPSO(&psoDesc, m_pipelineState);
	m_pipelineState->SetName(L"Cube Pipeline State");

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	Renderer->createPSO(&psoDesc, m_pipelineStateWireframe);
	m_pipelineStateWireframe->SetName(L"Cube Wireframe Pipeline State");
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

	UINT vbByteSize = sizeof(Vertex) * vertices.size();

	Renderer->CreateCommittedBuffer(m_vertexBuffer, m_vertexBufferUpload, &CD3DX12_RESOURCE_DESC::Buffer(vbByteSize));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = &vertices[0];
	vertexData.RowPitch = vbByteSize;
	vertexData.SlicePitch = vbByteSize;

	UpdateSubresources(Renderer->GetCommandList().Get(), m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
	Renderer->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vbByteSize;

	UINT ibByteSize = sizeof(UINT) * indices.size();

	Renderer->CreateCommittedBuffer(m_indexBuffer, m_indexBufferUpload, &CD3DX12_RESOURCE_DESC::Buffer(ibByteSize));

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = &indices[0];
	indexData.RowPitch = ibByteSize;
	indexData.SlicePitch = ibByteSize;

	UpdateSubresources(Renderer->GetCommandList().Get(), m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);
	Renderer->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = ibByteSize;

	// Bounding Box 생성
	DirectX::SimpleMath::Vector3 minCorner = vertices[0].Position;
	DirectX::SimpleMath::Vector3 maxCorner = vertices[0].Position;


	for (UINT64 i = 1, size = vertices.size(); i < size; ++i)
	{
		minCorner = DirectX::SimpleMath::Vector3::Min(minCorner, vertices[i].Position);
		maxCorner = DirectX::SimpleMath::Vector3::Max(maxCorner, vertices[i].Position);
	}

	DirectX::SimpleMath::Vector3 center = (minCorner + maxCorner) * 0.5f;
	DirectX::SimpleMath::Vector3 extents = (maxCorner - center);

	m_boundingBox.Center = center;
	m_boundingBox.Extents = extents;
}

void Cube::CreateInstanceBuffer(Graphics* renderer, const std::vector<InstanceBuffer> instanceData)
{
	m_instanceData.clear();
	m_instanceData = instanceData;
	// 인스턴스 데이터 크기 계산
	UINT64 instanceBufferSize = sizeof(InstanceBuffer) * instanceData.size();
	instanceCount = static_cast<UINT>(instanceData.size());

	// 업로드 버퍼 생성
	renderer->CreateBuffer(m_StructuredBuffer, &CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize));
	m_StructuredBuffer->SetName(L"Cube InstanceBuffer");

	// 데이터 복사
	CD3DX12_RANGE readRange(0, 0); // 읽기 범위는 0으로 설정
	if (FAILED(m_StructuredBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_StructuredBufferDataBegin))))
	{
		throw std::runtime_error("Failed to map instance buffer upload resource.");
	}
	memcpy(m_StructuredBufferDataBegin, instanceData.data(), instanceBufferSize);
	m_StructuredBuffer->Unmap(0, nullptr);

	// SRV 생성
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = static_cast<UINT>(instanceData.size());
	srvDesc.Buffer.StructureByteStride = sizeof(InstanceBuffer);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandleInstance(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 5, m_srvDescSize);

	renderer->CreateSRV(m_StructuredBuffer.Get(), &srvDesc, srvHandleInstance);
}

void Cube::UpdateInstanceBuffer(const std::vector<InstanceBuffer> instanceData)
{
	m_instanceData.clear();
	m_instanceData = instanceData;
	// 인스턴스 데이터 크기 계산
	UINT64 instanceBufferSize = sizeof(InstanceBuffer) * instanceData.size();

	memcpy(m_StructuredBufferDataBegin, instanceData.data(), instanceBufferSize);
}

bool Cube::Intersects(DirectX::SimpleMath::Ray ray, float& distance, float& mindistance)
{
	// Bounding Box와 Ray의 교차 검사
	// 인스턴스 데이터를 순회하며 교차 검사
	for (auto& instance : m_instanceData)
	{
		DirectX::BoundingBox instanceAABB = m_boundingBox;
		instanceAABB.Center = XMFLOAT3(instance.world._41 * 0.5 + m_boundingBox.Center.x, instance.world._42 * 0.5 + m_boundingBox.Center.y, instance.world._43 * 0.5 + m_boundingBox.Center.z) ;

		if (instanceAABB.Intersects(ray.position, ray.direction, distance))
		{
			if (distance < mindistance)
			{
				mindistance = distance;
			}
		}
	}

	return true;
}

void Cube::SetIntersectBlock(DirectX::SimpleMath::Ray ray)
{
	float mindistance = FLT_MAX;
	float distance = 0;

	InstanceBuffer* closestInstance = nullptr;

	for (auto& instance : m_instanceData)
	{
		DirectX::BoundingBox instanceAABB = m_boundingBox;
		instanceAABB.Center = XMFLOAT3(instance.world._41 * 0.5 + m_boundingBox.Center.x, instance.world._42 * 0.5 + m_boundingBox.Center.y, instance.world._43 * 0.5 + m_boundingBox.Center.z);

		if (instanceAABB.Intersects(ray.position, ray.direction, distance))
		{
			if (distance < mindistance)
			{
				mindistance = distance;
				closestInstance = &instance;
			}
		}
	}

	if (closestInstance != nullptr)
	{
		closestInstance->isvisible = SetPicked(closestInstance->isvisible);
	}

	// 선택된 블럭 삭제
	//m_instanceData.erase(
	//	std::remove_if(m_instanceData.begin(), m_instanceData.end(), [](InstanceBuffer& instance) { return instance.isvisible == true; }),
	//	m_instanceData.end());

	UINT64 instanceBufferSize = sizeof(InstanceBuffer) * m_instanceData.size();
	memcpy(m_StructuredBufferDataBegin, m_instanceData.data(), instanceBufferSize);
}
