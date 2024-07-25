#pragma once

// Linkages
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "D3DX12.h"
#include <dxgi1_5.h>
#include <DirectXMath.h>
#include <D3DCompiler.h>
#include <stdexcept>
#include "stdafx.h"

namespace graphics {
	using Microsoft::WRL::ComPtr;
	using namespace DirectX;

	static const float SCREEN_DEPTH = 1000.0f;
	static const float SCREEN_NEAR = 0.1f;
	static const UINT FACTORY_DEBUG = DXGI_CREATE_FACTORY_DEBUG; // set to 0 if not debugging, DXGI_CREATE_FACTORY_DEBUG if debugging.
	static const DXGI_FORMAT DESIRED_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const int FRAME_BUFFER_COUNT = 3; // triple buffering.
	static const D3D_FEATURE_LEVEL	FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_0; // minimum feature level necessary for DirectX 12 compatibility.
	
	enum ShaderType { PIXEL_SHADER, VERTEX_SHADER, GEOMETRY_SHADER, HULL_SHADER, DOMAIN_SHADER };

	class GFX_Exception : public std::runtime_error {
	public:
		GFX_Exception(const char *msg) : std::runtime_error(msg) {}
	};
	
	class Graphics	{
	public:
		Graphics(int height, int width, HWND win, bool fullscreen);
		~Graphics();

		void Render();
		void ResetPipeline();
		void SetBackBufferRender(ComPtr<ID3D12GraphicsCommandList> commnadList, const float clearColor[4]);
		void SetBackBufferPresent(ComPtr<ID3D12GraphicsCommandList> commnadList);
		void createRootSignature(CD3DX12_ROOT_SIGNATURE_DESC* rootDesc, ComPtr<ID3D12RootSignature>& rootSignature);
		void createPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc, ComPtr<ID3D12PipelineState>& pipelineState);
		void CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC* heapDesc, ComPtr<ID3D12DescriptorHeap>& heap);
		void CreateSRV(ID3D12Resource* texture, D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		void CreateCBV(D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		void CreateBuffer(ID3D12Resource*& buffer, D3D12_RESOURCE_DESC* texDesc);
		void CreateDefaultBuffer(ID3D12Resource*& buffer, D3D12_RESOURCE_DESC* texDesc);
		void CompileShader(LPCWSTR filename, LPCSTR entryname, D3D12_SHADER_BYTECODE& shaderBytecode, ShaderType shadertype);
		void LoadAsset();
		void CreateCommittedBuffer(ID3D12Resource*& buffer, ID3D12Resource*& upload, D3D12_RESOURCE_DESC* texDesc);
		void ClearAllFrames();

		ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return m_commandList; }
		ComPtr<ID3D12Device> GetDevice() { return m_device; }
		ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_commandQueue; }

		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heaptype);

	private:
		void initializePipeLine(HWND win);
		void NextFrame();

		ComPtr<IDXGIFactory4>				m_dxgiFactory;
		ComPtr<IDXGIAdapter1>				m_adapter;
		ComPtr<ID3D12Device>				m_device;
		ComPtr<ID3D12CommandQueue>			m_commandQueue;
		ComPtr<ID3D12CommandAllocator>		m_commandAllocator[FRAME_BUFFER_COUNT];
		ComPtr<ID3D12GraphicsCommandList>	m_commandList;
		ComPtr<IDXGISwapChain3>			m_swapChain;
		ComPtr<ID3D12DescriptorHeap>		m_RTVHeap; // Render Target View Heap
		ComPtr<ID3D12DescriptorHeap>		m_DSVHeap; // Render Target View Heap
		ID3D12Resource*				m_RenderTarget[FRAME_BUFFER_COUNT];
		ID3D12Resource*				m_DepthStencilBuffer;
		ComPtr<ID3D12Fence>				m_fence[FRAME_BUFFER_COUNT];
		HANDLE						m_fenceEvent;
		D3D12_VIEWPORT				m_viewport;
		D3D12_RECT					m_scissorRect;
		UINT						m_fenceValues[FRAME_BUFFER_COUNT];
		UINT						m_BufferIndex;
		UINT						m_RTVDescSize; // Descriptor sizes may vary from device to device, so keep the size around so we can increment an offset when necessary.
		int							m_width;
		int							m_height;
		bool						m_fullscreen;
	};
};
