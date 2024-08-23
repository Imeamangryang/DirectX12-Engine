#include "Particle.h"

Particle::Particle(Graphics* renderer) : Object(renderer)
{
	InitPipeline(renderer);
}

Particle::~Particle()
{
}

void Particle::Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye)
{
}

void Particle::CreateDescriptorHeap(Graphics* Renderer)
{
}

void Particle::InitPipeline(Graphics* Renderer)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(Renderer->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Graphics root signature.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        
        CD3DX12_ROOT_PARAMETER1 rootParameters[2];
        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;

        if(FAILED(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error)))
		{
			OutputDebugStringA((char*)error->GetBufferPointer());
			return;
		}
        Renderer->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
        m_rootSignature->SetName(L"Particle Root Signature");
    }

    // Compute root signature.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 rootParameters[3];
        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
        computeRootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        if (FAILED(D3DX12SerializeVersionedRootSignature(&computeRootSignatureDesc, featureData.HighestVersion, &signature, &error)))
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
            return;
        }
        Renderer->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_computeRootSignature));
        m_computeRootSignature->SetName(L"Particle Compute Root Signature");
    }

    // shader compile
    D3D12_SHADER_BYTECODE VSBytecode = {};
    D3D12_SHADER_BYTECODE GSBytecode = {};
    D3D12_SHADER_BYTECODE PSBytecode = {};
    ComPtr<ID3DBlob> computeShader;
    ComPtr<ID3DBlob> error;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    Renderer->CompileShader(L"shader.fx", "VS", VSBytecode, VERTEX_SHADER);
    Renderer->CompileShader(L"shader.fx", "GS", GSBytecode, GEOMETRY_SHADER);
    Renderer->CompileShader(L"shader.fx", "PS", PSBytecode, PIXEL_SHADER);
    if (FAILED(D3DCompileFromFile(L"NBodyGravityCS.hlsl", nullptr, nullptr, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &error)))
    {
        OutputDebugStringA((char*)error->GetBufferPointer());
		return;
    }

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Describe the blend and depth states.
    CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = VSBytecode;
    psoDesc.GS = GSBytecode;
    psoDesc.PS = PSBytecode;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;
    Renderer->createPSO(&psoDesc, m_pipelineState);
    m_pipelineState->SetName(L"Particle PSO");

    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = m_computeRootSignature.Get();
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());

    Renderer->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_computePipelineState));
    m_computePipelineState->SetName(L"Particle Compute PSO");
}

void Particle::LoadMesh(Graphics* Renderer)
{
    std::vector<ParticleVertex> vertices;
    vertices.resize(ParticleCount);
    for (UINT i = 0; i < ParticleCount; i++)
    {
        vertices[i].color = XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f);
    }
    const UINT vbByteSize = ParticleCount * sizeof(ParticleVertex);

    Renderer->CreateCommittedBuffer(m_vertexBuffer, m_vertexBufferUpload, &CD3DX12_RESOURCE_DESC::Buffer(vbByteSize));

    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = &vertices[0];
    vertexData.RowPitch = vbByteSize;
    vertexData.SlicePitch = vbByteSize;

    UpdateSubresources(Renderer->GetCommandList().Get(), m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
    Renderer->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = static_cast<UINT>(vbByteSize);
    m_vertexBufferView.StrideInBytes = sizeof(ParticleVertex);
}
