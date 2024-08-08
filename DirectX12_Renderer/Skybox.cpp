#include "Skybox.h"

Skybox::Skybox(Graphics* renderer) : Object(renderer)
{
	m_renderer = renderer;

	InitPipeline();

	m_objectname = "SkyVolume";
}

Skybox::~Skybox()
{
}

void Skybox::Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye)
{
	m_commandList->SetPipelineState(m_pipelineState.Get());
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// Transform
	XMStoreFloat4x4(&m_worldTransform,
		XMMatrixTransformation(
			XMVectorZero(),
			XMVectorZero(),
			XMVectorSet(m_scale_x, m_scale_y, m_scale_z, 1.0f),
			XMVectorZero(),
			XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_rotation_x), XMConvertToRadians(m_rotation_y), XMConvertToRadians(m_rotation_z)),
			XMVectorSet(eye.x, eye.y, eye.z, 0.0)
		));

	m_constantBufferData.world = m_worldTransform;
	m_constantBufferData.viewproj = viewproj;

	memcpy(m_constantBufferDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->SetGraphicsRootConstantBufferView(1, m_constantBuffer->GetGPUVirtualAddress());

	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->DrawIndexedInstanced(m_indexcount, 1, 0, 0, 0);
}

void Skybox::InitPipeline()
{
	CreateDescriptorHeap();

	// Root Signature 积己
	CD3DX12_DESCRIPTOR_RANGE range[1];
	CD3DX12_ROOT_PARAMETER paramsRoot[2];
	// Slot : SkyCube Map
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); // t0
	paramsRoot[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_ALL);

	// Slot : ConstantBuffer
	paramsRoot[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // b0

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

	m_renderer->createRootSignature(&rootDesc, m_rootSignature);
	m_rootSignature->SetName(L"SkyBox Root Signature");

	// Shader Compile
	D3D12_SHADER_BYTECODE PSBytecode = {};
	D3D12_SHADER_BYTECODE VSBytecode = {};
	m_renderer->CompileShader(L"Skybox.fx", "VS", VSBytecode, VERTEX_SHADER);
	m_renderer->CompileShader(L"Skybox.fx", "PS", PSBytecode, PIXEL_SHADER);

	// Input Layout 积己
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
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	m_renderer->createPSO(&psoDesc, m_pipelineState);
	m_pipelineState->SetName(L"SkyBox Pipeline State");

	LoadMesh();
}

void Skybox::CreateDescriptorHeap()
{
	// Discriptor Heap 积己
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 2;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_renderer->CreateDescriptorHeap(&srvHeapDesc, m_srvHeap);
	m_srvHeap->SetName(L"SkyBox Descriptor Heap");

	m_srvDescSize = m_renderer->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_Tsky = std::make_shared<Texture>(m_renderer, m_srvHeap, L"resource/TychoSkymapII.t4_08192x04096.tif", 0);

	// ConstantBuffer 积己
	UINT64 bufferSize = sizeof(SkyConstantBuffer);
	m_renderer->CreateBuffer(m_constantBuffer, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
	m_constantBuffer->SetName(L"SkyBox Constant Buffer");

	// ConstantBufferView 积己
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc = {};
	cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (bufferSize + 255) & ~255; // Constant Buffer绰 256 byte aligned

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 1, m_srvDescSize);

	m_renderer->CreateCBV(&cbvDesc, srvHandle);

	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

	// ConstantBuffer甫 困茄 皋葛府 概俏
	CD3DX12_RANGE readRange(0, 0);
	if (FAILED(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_constantBufferDataBegin))))
	{
		throw (GFX_Exception("Failed to map CBV in Cube."));
	}
}

void Skybox::LoadMesh()
{
	UINT numSubdivisions = min(10, 5u);
	float radius = 20000.0f;

	const float X = 0.525731f;
	const float Z = 0.850651f;

	XMFLOAT3 pos[12] =
	{
		XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
		XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
		XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
		XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
		XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
	};

	DWORD k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	vertices.resize(12);
	indices.resize(60);

	for (UINT i = 0; i < 12; ++i)
	{
		vertices[i].Position = pos[i];
	}

	for (UINT i = 0; i < 60; ++i)
	{
		indices[i] = k[i];
	}

	for (UINT i = 0; i < numSubdivisions; ++i)
	{
		std::vector<Vertex> tempvertices = vertices;
		std::vector<UINT> tempindices = indices;

		vertices.resize(0);
		indices.resize(0);

		//       v1
		//       *
		//      / \
		//     /   \
		//  m0*-----*m1
		//   / \   / \
		//  /   \ /   \
		// *-----*-----*
		// v0    m2     v2

		UINT numTris = tempindices.size() / 3;
		for (UINT i = 0; i < numTris; ++i)
		{
			Vertex v0 = tempvertices[tempindices[i * 3 + 0]];
			Vertex v1 = tempvertices[tempindices[i * 3 + 1]];
			Vertex v2 = tempvertices[tempindices[i * 3 + 2]];

			//
			// Generate the midpoints.
			//

			Vertex m0, m1, m2;

			// For subdivision, we just care about the position component.  We derive the other
			// vertex components in CreateGeosphere.

			m0.Position = XMFLOAT3(
				0.5f * (v0.Position.x + v1.Position.x),
				0.5f * (v0.Position.y + v1.Position.y),
				0.5f * (v0.Position.z + v1.Position.z));

			m1.Position = XMFLOAT3(
				0.5f * (v1.Position.x + v2.Position.x),
				0.5f * (v1.Position.y + v2.Position.y),
				0.5f * (v1.Position.z + v2.Position.z));

			m2.Position = XMFLOAT3(
				0.5f * (v0.Position.x + v2.Position.x),
				0.5f * (v0.Position.y + v2.Position.y),
				0.5f * (v0.Position.z + v2.Position.z));

			//
			// Add new geometry.
			//

			vertices.push_back(v0); // 0
			vertices.push_back(v1); // 1
			vertices.push_back(v2); // 2
			vertices.push_back(m0); // 3
			vertices.push_back(m1); // 4
			vertices.push_back(m2); // 5

			indices.push_back(i * 6 + 0);
			indices.push_back(i * 6 + 3);
			indices.push_back(i * 6 + 5);

			indices.push_back(i * 6 + 3);
			indices.push_back(i * 6 + 4);
			indices.push_back(i * 6 + 5);

			indices.push_back(i * 6 + 5);
			indices.push_back(i * 6 + 4);
			indices.push_back(i * 6 + 2);

			indices.push_back(i * 6 + 3);
			indices.push_back(i * 6 + 1);
			indices.push_back(i * 6 + 4);
		}
	}

	// Project vertices onto sphere and scale.
	for (UINT i = 0; i < vertices.size(); ++i)
	{
		// Project onto unit sphere.
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&vertices[i].Position));

		// Project onto sphere.
		XMVECTOR p = - radius * n;

		XMStoreFloat3(&vertices[i].Position, p);
		XMStoreFloat3(&vertices[i].Normal, n);

		// Derive texture coordinates from spherical coordinates.
		float theta = MathHelper::AngleFromXY(
			vertices[i].Position.x,
			vertices[i].Position.z);


		float phi = acosf(vertices[i].Position.y / radius);

		vertices[i].TexC.x = theta / XM_2PI;
		vertices[i].TexC.y = phi / XM_PI;

		// Partial derivative of P with respect to theta
		vertices[i].TangentU.x = -radius * sinf(phi) * sinf(theta);
		vertices[i].TangentU.y = 0.0f;
		vertices[i].TangentU.z = +radius * sinf(phi) * cosf(theta);

		XMVECTOR T = XMLoadFloat3(&vertices[i].TangentU);
		XMStoreFloat3(&vertices[i].TangentU, XMVector3Normalize(T));
	}

	int bufferSize = sizeof(Vertex) * vertices.size();

	m_renderer->CreateCommittedBuffer(m_vertexBuffer, m_vertexBufferUpload, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = &vertices[0];
	vertexData.RowPitch = bufferSize;
	vertexData.SlicePitch = bufferSize;

	UpdateSubresources(m_renderer->GetCommandList().Get(), m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
	m_renderer->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = bufferSize;

	bufferSize = sizeof(UINT) * indices.size();

	m_renderer->CreateCommittedBuffer(m_indexBuffer, m_indexBufferUpload, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize));

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = &indices[0];
	indexData.RowPitch = bufferSize;
	indexData.SlicePitch = bufferSize;

	UpdateSubresources(m_renderer->GetCommandList().Get(), m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);
	m_renderer->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = bufferSize;

	m_indexcount = indices.size();
}
