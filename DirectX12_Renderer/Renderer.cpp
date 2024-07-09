#include "Renderer.h"

namespace graphics {
	Graphics::Graphics(int height, int width, HWND win, bool fullscreen) {
		m_device = nullptr;
		m_commandQueue = nullptr;
		m_commandList = nullptr;
		m_swapChain = nullptr;
		m_RTVHeap = nullptr;
		m_DSVHeap = nullptr;
		m_DepthStencilBuffer = nullptr;
		m_fenceEvent = nullptr;
		for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
			m_commandAllocator[i] = nullptr;
			m_RenderTarget[i] = nullptr;
			m_fence[i] = nullptr;
		}
		m_width = width;
		m_height = height;
		m_fullscreen = fullscreen;

		// 파이프라인 초기화
		initializePipeLine(win);
	}

	Graphics::~Graphics() {
		// ensure swap chain in windows mode.
		if (m_swapChain) {
			m_swapChain->SetFullscreenState(false, NULL);
			m_swapChain->Release();
			m_swapChain = nullptr;
		}

		CloseHandle(m_fenceEvent);

		if (m_DepthStencilBuffer) {
			m_DepthStencilBuffer->Release();
			m_DepthStencilBuffer = nullptr;
		}

		for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
			if (m_RenderTarget[i]) {
				m_RenderTarget[i]->Release();
				m_RenderTarget[i] = nullptr;
			}
		}
	}

	UINT Graphics::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heaptype)
	{
		return m_device->GetDescriptorHandleIncrementSize(heaptype);
	}

	// Graphics Pipeline 초기화
	void Graphics::initializePipeLine(HWND win)
	{		
	    // 1. DXGI Factory 생성
		{
			if (FAILED(CreateDXGIFactory2(FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory))))
			{
				throw GFX_Exception("Create DXGIFactory2 failed on init");
			}
		}

		// 2. Device 생성
		{
			int adapterIndex = 0;
			bool adapterFound = false;

			while (m_dxgiFactory->EnumAdapters1(adapterIndex, &m_adapter) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_ADAPTER_DESC1 desc;
				m_adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
					// we don't want a software device (emulator).
					++adapterIndex;
					continue;
				}

				if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), FEATURE_LEVEL, _uuidof(ID3D12Device), NULL))) {
					adapterFound = true;
					break;
				}

				++adapterIndex;
			}
			if (!adapterFound) 
			{
				throw GFX_Exception("No DirectX 12 compatible graphics card found on init.");
			}

			// attempt to create the device.
			if (FAILED(D3D12CreateDevice(m_adapter.Get(), FEATURE_LEVEL, IID_PPV_ARGS(&m_device))))
			{
				throw GFX_Exception("Create Device failed on init.");
			}
		}

		// 3. Command Queue 생성
		{
			D3D12_COMMAND_QUEUE_DESC cmdQDesc = {};
			ZeroMemory(&cmdQDesc, sizeof(cmdQDesc));
			cmdQDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			cmdQDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			cmdQDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			cmdQDesc.NodeMask = 0;

			if (FAILED(m_device->CreateCommandQueue(&cmdQDesc, IID_PPV_ARGS(&m_commandQueue))))
			{
				throw GFX_Exception("Create CommandQueue failed on init.");
			}
		}

		// 4. Swap chain 생성
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
			swapChainDesc.BufferCount = FRAME_BUFFER_COUNT; // double buffering.
			swapChainDesc.BufferDesc.Height = m_height;
			swapChainDesc.BufferDesc.Width = m_width;
			swapChainDesc.BufferDesc.Format = DESIRED_FORMAT;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
			swapChainDesc.OutputWindow = win;
			swapChainDesc.Windowed = !m_fullscreen;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.SampleDesc.Count = 1;

			IDXGISwapChain* swapChain;
			if (FAILED(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &swapChain)))
			{
				throw GFX_Exception("Create SwapChain failed on init.");
			}

			m_swapChain = static_cast<IDXGISwapChain3*>(swapChain);

			// clean up.
			swapChain = NULL;
			if (m_dxgiFactory) 
			{
				m_dxgiFactory->Release();
				m_dxgiFactory = nullptr;
			}

			m_BufferIndex = m_swapChain->GetCurrentBackBufferIndex();
		}

		// 5. RenderTargetView(RTV) 생성
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			ZeroMemory(&rtvHeapDesc, sizeof(rtvHeapDesc));
			rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap)))) 
			{
				throw GFX_Exception("Create DescriptorHeap failed on init.");
			}

			m_RTVDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

			for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) 
			{
				if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i])))) 
				{
					throw GFX_Exception("Swap Chain GetBuffer failed on init.");
				}
				m_device->CreateRenderTargetView(m_RenderTarget[i], NULL, rtvHandle);

				rtvHandle.Offset(1, m_RTVDescSize);
			}
		}

		// 6. Depth-Stencil View(DSV) 생성
		{		
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			if (FAILED(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)))) 
			{
				throw GFX_Exception("Create descriptor heap failed for Depth/Stencil.");
			}

			D3D12_CLEAR_VALUE dsOptimizedClearValue = {};
			dsOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			dsOptimizedClearValue.DepthStencil.Depth = 1.0f;
			dsOptimizedClearValue.DepthStencil.Stencil = 0.0f;
			if (FAILED(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0,
					1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&dsOptimizedClearValue,
				IID_PPV_ARGS(&m_DepthStencilBuffer)))) 
			{
				throw GFX_Exception("Failed to create heap for Depth/Stencil buffer.");
			}

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			m_device->CreateDepthStencilView(m_DepthStencilBuffer, &dsvDesc, m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
		}

		// 7. CommandAllocator & FenceObject 생성
		{
			for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
				if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])))) 
				{
					throw GFX_Exception("Create CommandAllocator failed on init.");
				}

				if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i])))) 
				{
					throw GFX_Exception("Create Fence failed on init.");
				}

				m_fenceValues[i] = 0;
			}
		}

		// 8. CommandList 생성
		{
			if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_BufferIndex].Get(), NULL, IID_PPV_ARGS(&m_commandList))))
			{
				throw GFX_Exception("Create CommandList failed on init.");
			}
		}

		// 9. FenceEvent 생성
		{
			m_fenceEvent = CreateEvent(NULL, false, false, NULL);
			if (!m_fenceEvent) 
			{
				throw GFX_Exception("Create Fence Event failed on init.");
			}
		}

		// 10. ImGUI 초기화
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// ImGUI 초기화
		ImGui_ImplWin32_Init(win);
	}

	// CPU & GPU 동기화
	void Graphics::NextFrame()
	{
		if (FAILED(m_commandQueue->Signal(m_fence[m_BufferIndex].Get(), m_fenceValues[m_BufferIndex])))
		{
			throw GFX_Exception("CommandQueue Signal Fence failed on Render.");
		}

		m_BufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		if (m_fence[m_BufferIndex]->GetCompletedValue() < m_fenceValues[m_BufferIndex]) {
			if (FAILED(m_fence[m_BufferIndex]->SetEventOnCompletion(m_fenceValues[m_BufferIndex], m_fenceEvent))) {
				throw GFX_Exception("Failed to SetEventOnCompletion for fence in NextFrame.");
			}

			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		++m_fenceValues[m_BufferIndex];
	}

	// Render 함수
	void Graphics::Render() 
	{
		ID3D12CommandList* lCmds[] = { m_commandList.Get()};
		m_commandQueue->ExecuteCommandLists(__crt_countof(lCmds), lCmds);

		// swap the back buffers.
		if (FAILED(m_swapChain->Present(0, 0))) 
		{
			throw GFX_Exception("SwapChain Present failed on Render.");
		}
	}

	// Reset Pipeline
	void Graphics::ResetPipeline()
	{
		NextFrame();

		if (FAILED(m_commandAllocator[m_BufferIndex]->Reset()))
		{
			throw GFX_Exception("CommandAllocator Reset failed on UpdatePipeline.");
		}
		if (FAILED(m_commandList->Reset(m_commandAllocator[m_BufferIndex].Get(), NULL)))
		{
			throw GFX_Exception("CommandList Reset failed on UpdatePipeline.");
		}
	}

	// BackBuffer Render 대상 지정
	void Graphics::SetBackBufferRender(ComPtr<ID3D12GraphicsCommandList> commnadList, const float clearColor[4])
	{
		// 백버퍼를 RT으로 쓸것이라고 전달(barrier)
		commnadList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_RenderTarget[m_BufferIndex],
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET)
		);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), m_BufferIndex, m_RTVDescSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
		commnadList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

		commnadList->ClearRenderTargetView(rtvHandle, clearColor, 0, NULL);
		commnadList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	// BackBuffer Present 대상 지정
	void Graphics::SetBackBufferPresent(ComPtr<ID3D12GraphicsCommandList> commnadList)
	{
		commnadList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_RenderTarget[m_BufferIndex],
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT)
		);
	}

	// RootSignature 생성 함수
	void Graphics::createRootSignature(CD3DX12_ROOT_SIGNATURE_DESC* rootDesc, ID3D12RootSignature*& rootSignature)
	{
		ID3DBlob* error;
		ID3DBlob* signature;

		if (FAILED(D3D12SerializeRootSignature(rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) 
		{
			throw GFX_Exception((char*)error->GetBufferPointer());
		}
		if (FAILED(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature))))
		{
			throw GFX_Exception("Failed to create Root Signature.");
		}
		signature->Release();
	}

	// PipelineStateObject 생성 함수
	void Graphics::createPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc, ID3D12PipelineState*& pipelineState)
	{
		if (FAILED(m_device->CreateGraphicsPipelineState(psoDesc, IID_PPV_ARGS(&pipelineState))))
		{
			throw GFX_Exception("Failed to Create GraphicsPipeline.");
		}
	}

	// DescriptorHeap 생성 함수
	void Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC* heapDesc, ComPtr<ID3D12DescriptorHeap>& heap)
	{
		if FAILED(m_device->CreateDescriptorHeap(heapDesc, IID_PPV_ARGS(&heap))) 
		{
			throw GFX_Exception("Failed to create descriptor heap.");
		}

	}

	// Shader Resource View 생성 함수
	void Graphics::CreateSRV(ID3D12Resource*& texture, D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		m_device->CreateShaderResourceView(texture, srvDesc, handle);
	}

	// Constant buffer view 생성 함수
	void Graphics::CreateCBV(D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		m_device->CreateConstantBufferView(desc, handle);
;	}

	// Buffer 생성 함수
	void Graphics::CreateBuffer(ID3D12Resource*& buffer, D3D12_RESOURCE_DESC* texDesc)
	{
		if (FAILED(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&buffer))))
		{
			throw GFX_Exception("Failed to create buffer.");
		}
	}

	// DefaultBuffer 생성 함수
	void Graphics::CreateDefaultBuffer(ID3D12Resource*& buffer, D3D12_RESOURCE_DESC* texDesc)
	{
		if (FAILED(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			NULL,
			IID_PPV_ARGS(&buffer))))
		{
			throw GFX_Exception("Failed to create default heap on CreateSRV.");
		}
	}

	// Shader Compile 함수
	void Graphics::CompileShader(LPCWSTR filename, LPCSTR entryname, D3D12_SHADER_BYTECODE& shaderBytecode, ShaderType shadertype)
	{
		ID3DBlob* shader;
		ID3DBlob* error;
		LPCSTR version;

		switch (shadertype)
		{
		case PIXEL_SHADER:
			version = "ps_5_0";
			break;
		case VERTEX_SHADER:
			version = "vs_5_0";
			break;
		case GEOMETRY_SHADER:
			version = "gs_5_0";
			break;
		case HULL_SHADER:
			version = "hs_5_0";
			break;
		case DOMAIN_SHADER:
			version = "ds_5_0";
			break;
		default:
			version = ""; // will break on attempting to compile as not valid.
		}

		if (FAILED(D3DCompileFromFile(filename, NULL, NULL, entryname, version, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shader, &error))) {
			if (shader) shader->Release();
			if (error)
			{
				throw GFX_Exception((char*)error->GetBufferPointer());
			}
			else {
				throw GFX_Exception("Failed to compile Shader. No error returned from compiler.");
			}
		}
		shaderBytecode.BytecodeLength = shader->GetBufferSize();
		shaderBytecode.pShaderBytecode = shader->GetBufferPointer();
	}

	// Render Object 함수
	void Graphics::LoadAsset()
	{
		ID3D12CommandList* lCmds[] = { m_commandList.Get()};
		m_commandQueue->ExecuteCommandLists(__crt_countof(lCmds), lCmds);

		++m_fenceValues[m_BufferIndex];

		if (FAILED(m_commandQueue->Signal(m_fence[m_BufferIndex].Get(), m_fenceValues[m_BufferIndex])))
		{
			throw GFX_Exception("CommandQueue Signal Fence failed on Render.");
		}

		if (FAILED(m_fence[m_BufferIndex]->SetEventOnCompletion(m_fenceValues[m_BufferIndex], m_fenceEvent)))
		{
			throw GFX_Exception("Failed to SetEventOnCompletion for fence in WaitForGPU.");
		}

		WaitForSingleObject(m_fenceEvent, INFINITE);

		++m_fenceValues[m_BufferIndex];
	}

	// 버퍼의 데이터 GPU로 제출하는 함수
	void Graphics::CreateCommittedBuffer(ID3D12Resource*& buffer, ID3D12Resource*& upload, D3D12_RESOURCE_DESC* texDesc)
	{
		if (FAILED(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			NULL,
			IID_PPV_ARGS(&buffer))))
		{
			throw GFX_Exception("Failed to create default heap on CreateSRV.");
		}

		const auto bufferSize = GetRequiredIntermediateSize(buffer, 0, 1);
		if (FAILED(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			NULL,
			IID_PPV_ARGS(&upload)))) 
		{
			throw GFX_Exception("Failed to create upload heap on CreateSRV.");
		}
	}

	// GPU에서 모든 렌더링이 완료되도록 하는 함수
	void Graphics::ClearAllFrames()
	{
		for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
		{
			// Add Signal command to set fence to the fence value that indicates the GPU is done with that buffer. maFenceValues[i] contains the frame count for that buffer.
			if (FAILED(m_commandQueue->Signal(m_fence[i].Get(), m_fenceValues[i])))
			{
				throw GFX_Exception("CommandQueue Signal Fence failed on Render.");
			}

			// if the current value returned by the fence is less than the current fence value for this buffer, then we know the GPU is not done with the buffer, so wait.
			if (m_fence[i]->GetCompletedValue() < m_fenceValues[i]) 
			{
				if (FAILED(m_fence[i]->SetEventOnCompletion(m_fenceValues[i], m_fenceEvent))) 
				{
					throw GFX_Exception("Failed to SetEventOnCompletion for fence in NextFrame.");
				}
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}
		}
	}
}