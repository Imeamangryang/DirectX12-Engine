#pragma once
#include "stdafx.h"

using namespace graphics;

// ======================================================================================================
// Texture class
// �ؽ�ó �ε�, ���ҽ� ����, ��ũ���� �� ���� ���� ����ϰ� �˴ϴ�.
// ======================================================================================================
class Texture 
{
public:
	Texture(Graphics* renderer, ComPtr<ID3D12DescriptorHeap> descriptorHeap ,const std::wstring& filePath, const size_t registernum);
	~Texture();

	ID3D12Resource* GetTexture() { return m_texture.Get(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() { return m_gpuDescriptorHandle; }

private:
	void LoadTexture(Graphics* renderer, ComPtr<ID3D12DescriptorHeap> descriptorHeap ,const std::wstring& filePath, const size_t registernum);

	ComPtr<ID3D12DescriptorHeap>	m_srvHeap;

	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12Resource> m_uploadHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuDescriptorHandle;

};