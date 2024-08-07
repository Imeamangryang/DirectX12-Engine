#include "ImGuiLoader.h"

ImGuiLoader::ImGuiLoader(Graphics* renderer)
{
	// SRV Discriptor Heap 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	renderer->CreateDescriptorHeap(&srvHeapDesc, m_srvHeap);
	m_srvHeap->SetName(L"ImGui SRV Heap");

	m_srvDescSize = renderer->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// imgui 초기화
	ImGui_ImplDX12_Init(renderer->GetDevice().Get(), FRAME_BUFFER_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM,
		m_srvHeap.Get(),
		// You'll need to designate a descriptor from your descriptor heap for Dear ImGui to use internally for its font texture's SRV
		CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), 0, m_srvDescSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_srvDescSize));
}

ImGuiLoader::~ImGuiLoader()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLoader::Draw()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
}
