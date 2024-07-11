#include "Character.h"

Character::Character(Graphics* renderer) : Object(renderer),
	m_srvHeap(nullptr),
	m_uploadHeap(nullptr),
	m_width(0),
	m_height(0),
	m_pipelineState(nullptr),
	m_pipelineStateWireframe(nullptr),
	m_rootSignature(nullptr),
	m_rootSignatureWireframe(nullptr),
	m_CBV(nullptr),
	m_cbvDataBegin(nullptr),
	m_srvDescSize(0),
	m_vertexBuffer(nullptr),
	m_vertexBufferUpload(nullptr),
	m_indexBuffer(nullptr),
	m_indexBufferUpload(nullptr),
	m_vertexBufferView(),
	m_indexBufferView(),
	m_worldTransform(MathHelper::Identity4x4())
{
	CreateDescriptorHeap(renderer);

	InitPipeline(renderer);;
	InitPipelineWireframe(renderer);

	LoadCharacter(renderer, L"models/Remy.fbx");

	m_objectname = "Character";

}

Character::~Character()
{
	if (m_srvHeap) m_srvHeap.Reset();
	if (m_uploadHeap) m_uploadHeap->Release();
	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_vertexBufferUpload) m_vertexBufferUpload->Release();
	if (m_indexBuffer) m_indexBuffer->Release();
	if (m_indexBufferUpload) m_indexBufferUpload->Release();
	if (m_CBV)
	{
		m_CBV->Unmap(0, nullptr);
		m_cbvDataBegin = nullptr;
		m_CBV->Release();
		m_CBV = nullptr;
	}

}

void Character::Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye)
{
	m_commandList->SetPipelineState(m_pipelineState.Get());
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// Transform
	XMStoreFloat4x4(&m_worldTransform,
		XMMatrixTransformation(
			XMVectorZero(),
			XMVectorZero(),
			XMVectorSet(m_scale_x, m_scale_y, m_scale_z, 0.0f),
			XMVectorZero(),
			XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_rotation_x), XMConvertToRadians(m_rotation_y), XMConvertToRadians(m_rotation_z)),
			XMVectorSet(m_translation_x, m_translation_y, m_translation_z, 0.0)
		));

	m_constantBufferData.world = m_worldTransform;
	m_constantBufferData.viewproj = viewproj;
	m_constantBufferData.eye = eye;
	m_constantBufferData.height = m_height;
	m_constantBufferData.width = m_width;
	memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(SkyConstantBuffer));

	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 1, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, cbvHandle);
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvhandle2(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 2, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(2, srvhandle2);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // describe how to read the vertex buffer.
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	for(const auto& mesh : meshes)
	{
		// Draw
		m_commandList->DrawIndexedInstanced(mesh.IndexSize, 1, mesh.IndexSize, mesh.VertexSize, 0);
	}
}

void Character::DrawWireframe(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye)
{
}

void Character::ClearUnusedUploadBuffersAfterInit()
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

void Character::InitPipeline(Graphics* Renderer)
{
	CD3DX12_DESCRIPTOR_RANGE range[3];
	CD3DX12_ROOT_PARAMETER paramsRoot[3];
	// Root Signature 생성
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	paramsRoot[0].InitAsDescriptorTable(1, &range[0]);

	// ConstantBufferview를 위한 Root Parameter
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	paramsRoot[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);

	// Slot3 : Color Map, Register(t1)
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	paramsRoot[2].InitAsDescriptorTable(1, &range[2]);

	CD3DX12_STATIC_SAMPLER_DESC descSamplers[2];
	descSamplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
	descSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	descSamplers[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
	descSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(
		_countof(paramsRoot),
		paramsRoot,
		2,
		descSamplers,
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Renderer->createRootSignature(&rootDesc, m_rootSignature);

	CreateConstantBuffer(Renderer);

	// Shader Compile
	D3D12_SHADER_BYTECODE PSBytecode = {};
	D3D12_SHADER_BYTECODE VSBytecode = {};
	Renderer->CompileShader(L"VertexShader2D.hlsl", "VS2D", VSBytecode, VERTEX_SHADER);
	Renderer->CompileShader(L"PixelShader2D.hlsl", "PS2D", PSBytecode, PIXEL_SHADER);

	// Input Layout 생성
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
}

void Character::InitPipelineWireframe(Graphics* Renderer)
{
}

void Character::CreateDescriptorHeap(Graphics* Renderer)
{
	// SRV Discriptor Heap 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 3;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Renderer->CreateDescriptorHeap(&srvHeapDesc, m_srvHeap);

	m_srvDescSize = Renderer->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Character::CreateConstantBuffer(Graphics* Renderer)
{
	UINT64 bufferSize = sizeof(SkyConstantBuffer);
	Renderer->CreateBuffer(m_CBV, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
	m_CBV->SetName(L"CBV");

	// ConstantBufferView 생성
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc = {};
	cbvDesc.BufferLocation = m_CBV->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (bufferSize + 255) & ~255; // Constant Buffer는 256 byte aligned

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 1, m_srvDescSize);

	Renderer->CreateCBV(&cbvDesc, srvHandle);

	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

	CD3DX12_RANGE readRange(0, 0);
	if (FAILED(m_CBV->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin))))
	{
		throw (GFX_Exception("Failed to map CBV in Moon."));
	}
}

void Character::LoadCharacter(Graphics* Renderer, const wchar_t* path)
{
	FbxManager* mfbxManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(mfbxManager, IOSROOT);
	mfbxManager->SetIOSettings(ios);
	FbxImporter* mfbxImporter = FbxImporter::Create(mfbxManager, "");
	FbxScene* mfbxScene = FbxScene::Create(mfbxManager, "");

	mfbxImporter->Initialize("models/Remy.fbx", -1, mfbxManager->GetIOSettings());
	mfbxImporter->Import(mfbxScene);

	mfbxScene->GetGlobalSettings().SetAxisSystem(FbxAxisSystem::DirectX);

	FbxGeometryConverter geometryConverter(mfbxManager);
	geometryConverter.Triangulate(mfbxScene, true);

	mfbxImporter->Destroy();

	FbxNode* lRootNode = mfbxScene->GetRootNode();

	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	for (int k = 0; k < lRootNode->GetChildCount(); k++) {
		FbxMeshData meshdata;

		FbxNode* mNode = lRootNode->GetChild(k);

		FbxNodeAttribute* attribute = mNode->GetNodeAttribute();

		if (attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
			FbxMesh* mesh = mNode->GetMesh();
			int vertexcount = mesh->GetControlPointsCount();

			FbxVector4* controlPoints = mesh->GetControlPoints();
			for (int i = 0; i < vertexcount; ++i) {
				Vertex tempvertex;
				tempvertex.Position.x = static_cast<float>(controlPoints[i].mData[0]);
				tempvertex.Position.y = static_cast<float>(controlPoints[i].mData[2]);
				tempvertex.Position.z = static_cast<float>(controlPoints[i].mData[1]);

				vertices.push_back(tempvertex);
			}

			UINT arrIdx[3];
			int polygonCount = mesh->GetPolygonCount();

			for (int i = 0; i < polygonCount; i++) // 삼각형의 개수
			{
				for (int j = 0; j < 3; j++) // 삼각형은 세 개의 정점으로 구성
				{
					UINT controlPointIndex = mesh->GetPolygonVertex(i, j); // 제어점의 인덱스 추출
					arrIdx[j] = controlPointIndex;
				}

				indices.push_back(arrIdx[0]);
				indices.push_back(arrIdx[2]);
				indices.push_back(arrIdx[1]);
			}

			meshdata.MeshName = mesh->GetName();
			meshdata.VertexSize = vertexcount;
			meshdata.IndexSize = polygonCount * 3;

			meshes.push_back(meshdata);
		}
	}

	m_vertexcount = vertices.size();
	m_indexcount = indices.size();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

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