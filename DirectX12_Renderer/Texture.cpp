#include "Texture.h"

Texture::Texture(Graphics* renderer, ComPtr<ID3D12DescriptorHeap> descriptorHeap, const std::wstring& filePath, const size_t registernum)
{
	LoadTexture(renderer, descriptorHeap, filePath, registernum);
}

Texture::~Texture()
{
    if (m_uploadHeap) {
        m_uploadHeap->Release();
        m_uploadHeap = nullptr;
    }
}

void Texture::LoadTexture(Graphics* renderer, ComPtr<ID3D12DescriptorHeap> descriptorHeap, const std::wstring& filePath, const size_t registernum)
{
    std::unique_ptr<uint8_t[]> decodeData;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    D3D12_SUBRESOURCE_DATA subresource;

    if (filePath.substr(filePath.find_last_of(L".") + 1) == L"dds") {
        LoadDDSTextureFromFile(renderer->GetDevice().Get(), filePath.c_str(), &m_texture, decodeData, subresources);

        const size_t textureSize = GetRequiredIntermediateSize(m_texture.Get(), 0, static_cast<UINT>(subresources.size()));

        renderer->GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(textureSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadHeap));

        UpdateSubresources(renderer->GetCommandList().Get(), m_texture.Get(), m_uploadHeap.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());
    }
    else {
        LoadWICTextureFromFileEx(renderer->GetDevice().Get(), filePath.c_str(), 0, D3D12_RESOURCE_FLAG_NONE, WIC_LOADER_FORCE_RGBA32, &m_texture, decodeData, subresource);

        const size_t textureSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

        renderer->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(textureSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadHeap));
        UpdateSubresources(renderer->GetCommandList().Get(), m_texture.Get(), m_uploadHeap.Get(), 0, 0, 1, &subresource);
    }

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    renderer->GetCommandList()->ResourceBarrier(1, &barrier);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = m_texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = m_texture->GetDesc().MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    size_t m_srvDescSize = renderer->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), registernum, m_srvDescSize);

    renderer->CreateSRV(m_texture.Get(), &srvDesc, srvHandle);

    m_gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), registernum, m_srvDescSize);
}
