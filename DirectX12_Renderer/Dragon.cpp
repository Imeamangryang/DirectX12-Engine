#include "Dragon.h"

Dragon::Dragon(Graphics* renderer) : Object(renderer),
m_worldTransform(MathHelper::Identity4x4())
{
	InitPipeline(renderer);

	LoadFBXModel(renderer, "models/Dragon/Dragon.fbx");

	m_objectname = "Dragon";

	m_rotation_x = 90.0f;

	m_translation_z = 300.0f;

	m_scale_x = 1.0f;
	m_scale_y = 1.0f;
	m_scale_z = 1.0f;
}

Dragon::~Dragon()
{
}

void Dragon::Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye)
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
			XMVectorSet(m_scale_x, m_scale_y, m_scale_z, 0.0f),
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
	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 2, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(2, cbvHandle);
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvhandle2(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 1, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, srvhandle2);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle2(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 3, m_srvDescSize);
	m_commandList->SetGraphicsRootDescriptorTable(3, cbvHandle2);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // describe how to read the vertex buffer.
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	for (const auto& mesh : meshes)
	{
		// Draw
		m_commandList->DrawIndexedInstanced(mesh.IndexSize, 1, mesh.StartIndexLocation, mesh.BaseVertexLocation, 0);
	}
}

void Dragon::InitPipeline(Graphics* Renderer)
{
	CreateDescriptorHeap(Renderer);

	CD3DX12_DESCRIPTOR_RANGE range[4];
	CD3DX12_ROOT_PARAMETER paramsRoot[4];
	// Root Signature 생성
	// Slot 0 : BumpColor
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
	paramsRoot[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_ALL);

	// Slot 2 : Normal Map
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // t1
	paramsRoot[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);

	// ConstantBuffer
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0
	paramsRoot[2].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);

	// ConstantBuffer
	range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // b1
	paramsRoot[3].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_ALL);

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

	// Shader Compile
	D3D12_SHADER_BYTECODE PSBytecode = {};
	D3D12_SHADER_BYTECODE VSBytecode = {};
	Renderer->CompileShader(L"Dragon.fx", "VS", VSBytecode, VERTEX_SHADER);
	Renderer->CompileShader(L"Dragon.fx", "PS", PSBytecode, PIXEL_SHADER);

	// Input Layout 생성
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "BOMEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	Renderer->createPSO(&psoDesc, m_pipelineStateWireframe);
	m_pipelineStateWireframe->SetName(L"Cube Wireframe Pipeline State");
}

void Dragon::CreateDescriptorHeap(Graphics* Renderer)
{
	// SRV Discriptor Heap 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Renderer->CreateDescriptorHeap(&srvHeapDesc, m_srvHeap);

	m_srvDescSize = Renderer->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_TColor = std::make_shared<Texture>(Renderer, m_srvHeap, L"models/Dragon/textures/Dragon_Bump_Col2.jpg", 0);
	m_TNormal = std::make_shared<Texture>(Renderer, m_srvHeap, L"models/Dragon/textures/Dragon_Nor_mirror2.jpg", 1);

	UINT64 bufferSize = sizeof(ConstantBuffer);
	Renderer->CreateBuffer(m_CBV, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
	m_CBV->SetName(L"CBV");

	// ConstantBufferView 생성
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc = {};
	cbvDesc.BufferLocation = m_CBV->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (bufferSize + 255) & ~255; // Constant Buffer는 256 byte aligned

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 2, m_srvDescSize);

	Renderer->CreateCBV(&cbvDesc, srvHandle);

	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

	CD3DX12_RANGE readRange(0, 0);
	if (FAILED(m_CBV->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin))))
	{
		throw (GFX_Exception("Failed to map CBV in Moon."));
	}

	UINT64 bufferSize2 = sizeof(AnimationConstantBuffer);
	Renderer->CreateBuffer(m_StructuredBuffer, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize2));
	m_StructuredBuffer->SetName(L"CBV");

	// ConstantBufferView 생성
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc2 = {};
	cbvDesc2.BufferLocation = m_StructuredBuffer->GetGPUVirtualAddress();
	cbvDesc2.SizeInBytes = (bufferSize + 255) & ~255; // Constant Buffer는 256 byte aligned

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle2(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 3, m_srvDescSize);

	Renderer->CreateCBV(&cbvDesc2, srvHandle2);

	ZeroMemory(&m_instanceData, sizeof(m_instanceData));

	CD3DX12_RANGE readRange2(0, 0);
	if (FAILED(m_StructuredBuffer->Map(0, &readRange2, reinterpret_cast<void**>(&m_StructuredBufferDataBegin))))
	{
		throw (GFX_Exception("Failed to map CBV in Moon."));
	}
}

void Dragon::LoadFBXModel(Graphics* Renderer, string path)
{
	FBXLoader loader;
	loader.LoadFbx(path);

	std::vector<Vertex> vertices;
	std::vector<std::uint32_t> indices;

	m_vertexcount = 0;
	m_indexcount = 0;

	for (UINT i = 0; i < loader.GetMeshCount(); i++)
	{
		const FbxMeshInfo& meshInfo = loader.GetMesh(i);

		FbxMeshData meshdata;
		meshdata.MeshName = meshInfo.name;
		meshdata.VertexSize = meshInfo.vertices.size();
		meshdata.IndexSize = meshInfo.indices.size();
		meshdata.StartIndexLocation = m_indexcount;
		meshdata.BaseVertexLocation = m_vertexcount;

		vertices.insert(vertices.end(), meshInfo.vertices.begin(), meshInfo.vertices.end());
		indices.insert(indices.end(), meshInfo.indices.begin(), meshInfo.indices.end());

		m_vertexcount += meshInfo.vertices.size();
		m_indexcount += meshInfo.indices.size();

		meshes.push_back(meshdata);
	}

	m_boneInfos = loader.GetBones();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const std::uint32_t ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

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

	// Animation clip
	UINT frameCount = 0;
	vector<shared_ptr<FbxAnimClipInfo>>& animClips = loader.GetAnimClip();
	for (shared_ptr<FbxAnimClipInfo>& ac : animClips)
	{
		AnimClipInfo info = {};

		info.animName = ac->name;
		info.duration = ac->endTime.GetSecondDouble() - ac->startTime.GetSecondDouble();

		UINT startFrame = static_cast<UINT>(ac->startTime.GetFrameCount(ac->mode));
		UINT endFrame = static_cast<UINT>(ac->endTime.GetFrameCount(ac->mode));
		info.frameCount = endFrame - startFrame;

		info.keyFrames.resize(ac->keyFrames.size());

		const UINT boneCount = static_cast<UINT>(ac->keyFrames.size());
		for (UINT b = 0; b < boneCount; b++)
		{
			auto& vec = ac->keyFrames[b];

			const UINT64 size = vec.size();
			frameCount = max(frameCount, size);
			info.keyFrames[b].resize(size*2);

			for (UINT f = 0; f < size; f++)
			{
				FbxKeyFrameInfo& kf = vec[f];
				// FBX에서 파싱한 정보들로 채워준다
				KeyFrameInfo& kfInfo = info.keyFrames[b][f];
				kfInfo.time = kf.time;
				kfInfo.frame = static_cast<UINT>(size);
				kfInfo.scale.x = static_cast<float>(kf.matTransform.GetS().mData[0]);
				kfInfo.scale.y = static_cast<float>(kf.matTransform.GetS().mData[1]);
				kfInfo.scale.z = static_cast<float>(kf.matTransform.GetS().mData[2]);
				kfInfo.rotation.x = static_cast<float>(kf.matTransform.GetQ().mData[0]);
				kfInfo.rotation.y = static_cast<float>(kf.matTransform.GetQ().mData[1]);
				kfInfo.rotation.z = static_cast<float>(kf.matTransform.GetQ().mData[2]);
				kfInfo.rotation.w = static_cast<float>(kf.matTransform.GetQ().mData[3]);
				kfInfo.translate.x = static_cast<float>(kf.matTransform.GetT().mData[0]);
				kfInfo.translate.y = static_cast<float>(kf.matTransform.GetT().mData[1]);
				kfInfo.translate.z = static_cast<float>(kf.matTransform.GetT().mData[2]);
				kfInfo.matTransform = GetMatrix(kf.matTransform);
			}
		}

		m_animClips.push_back(info);
	}

	// bone info
	vector<shared_ptr<FbxBoneInfo>>& bones = loader.GetBones();
	for (shared_ptr<FbxBoneInfo>& bone : bones)
	{
		BoneInfo boneInfo = {};
		boneInfo.parentIdx = bone->parentIndex;
		boneInfo.matOffset = GetMatrix(bone->matOffset);
		boneInfo.boneName = bone->boneName;
		m_bones.push_back(boneInfo);
	}
}

XMFLOAT4X4 Dragon::GetMatrix(FbxAMatrix& mat)
{
	XMFLOAT4X4 ret;
	for (UINT y = 0; y < 4; y++)
	{
		for (UINT x = 0; x < 4; x++)
		{
			ret.m[y][x] = static_cast<float>(mat.Get(x, y));
		}
	}
	return ret;
}

XMMATRIX Dragon::Interpolate(const KeyFrameInfo& kf1, const KeyFrameInfo& kf2, double time)
{
	// 보간 비율 계산
	double lerpFactor = (time - kf1.time) / (kf2.time - kf1.time);

	// 스케일, 회전, 이동 보간
	XMVECTOR scale1 = XMLoadFloat3(&kf1.scale);
	XMVECTOR scale2 = XMLoadFloat3(&kf2.scale);
	XMVECTOR rotation1 = XMLoadFloat4(&kf1.rotation);
	XMVECTOR rotation2 = XMLoadFloat4(&kf2.rotation);
	XMVECTOR translate1 = XMLoadFloat3(&kf1.translate);
	XMVECTOR translate2 = XMLoadFloat3(&kf2.translate);

	XMVECTOR scale = XMVectorLerp(scale1, scale2, lerpFactor);
	XMVECTOR rotation = XMQuaternionSlerp(rotation1, rotation2, lerpFactor);
	XMVECTOR translate = XMVectorLerp(translate1, translate2, lerpFactor);

	// 변환 행렬 생성
	XMMATRIX matScale = XMMatrixScalingFromVector(scale);
	XMMATRIX matRotation = XMMatrixRotationQuaternion(rotation);
	XMMATRIX matTranslate = XMMatrixTranslationFromVector(translate);

	return matScale * matRotation * matTranslate;
}

void Dragon::ComputeBoneTransforms(const AnimClipInfo& animClip, double time, const vector<BoneInfo>& bones, vector<XMMATRIX>& boneTransforms)
{
	size_t boneCount = bones.size();
	boneTransforms.resize(boneCount, XMMatrixIdentity());

	for (size_t i = 0; i < boneCount; ++i)
	{
		// 현재 시간에 가장 가까운 두 개의 KeyFrame을 찾기
		const vector<KeyFrameInfo>& keyFrames = animClip.keyFrames[i];
		KeyFrameInfo kf1 = keyFrames.front();
		KeyFrameInfo kf2 = keyFrames.back();

		for (size_t j = 0; j < keyFrames.size() - 1; ++j)
		{
			if (time >= keyFrames[j].time && time <= keyFrames[j + 1].time)
			{
				kf1 = keyFrames[j];
				kf2 = keyFrames[j + 1];
				break;
			}
		}

		// 보간된 변환 행렬 계산
		XMMATRIX boneTransform = Interpolate(kf1, kf2, time);

		// 부모 뼈의 변환 행렬과 곱하여 최종 변환 행렬 계산
		if (bones[i].parentIdx != UINT_MAX)
		{
			boneTransform = boneTransform * boneTransforms[bones[i].parentIdx];
		}

		// 오프셋 행렬 적용
		XMMATRIX offsetMatrix = XMLoadFloat4x4(&bones[i].matOffset);
		boneTransforms[i] = offsetMatrix * boneTransform;
	}
}
