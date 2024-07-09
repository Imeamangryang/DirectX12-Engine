#include "Scene.h"

Scene::Scene(int height, int width, Graphics* renderer) : 
	m_terrain(renderer),
	m_sky(renderer),
	m_renderer(renderer),
	m_camera(height, width)
{
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = (float)width;
	m_viewport.Height = (float)height;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;

	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = width;
	m_scissorRect.bottom = height;

	CloseCommandList();
	m_renderer->LoadAsset();


	// Object
	m_terrain.ClearUnusedUploadBuffersAfterInit();
	m_sky.ClearUnusedUploadBuffersAfterInit();
}

Scene::~Scene()
{
	m_renderer->ClearAllFrames();
	m_renderer = nullptr;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Scene::Draw()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Details", NULL, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (!ImGui::CollapsingHeader("Draw Modes")) {
			if (ImGui::Button("Solid")) {
				m_DrawMode = 1;
			}
			if (ImGui::Button("WireFrame")) {
				m_DrawMode = 2;
			}
		}
		if (!ImGui::CollapsingHeader("Objects")) {
			ImGui::Text("Terrain");
			ImGui::Text("Sky");
		}

		
		ImGui::End();
	}

	m_renderer->ResetPipeline();

	const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	m_renderer->SetBackBufferRender(m_renderer->GetCommandList(), clearColor);

	SetViewport();

	if (m_DrawMode == 1)
	{
		m_terrain.DrawTes(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
		m_sky.Draw3D(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
	}
	else
	{
		m_terrain.DrawTes_Wireframe(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
		m_sky.Draw3D(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
	}

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_renderer->GetCommandList().Get());

	m_renderer->SetBackBufferPresent(m_renderer->GetCommandList());
	CloseCommandList();

	m_renderer->Render();
}

void Scene::HandleInput(const InputDirections& directions, float deltaTime)
{
	if (directions.bFront)
	{
		m_camera.Translate(XMFLOAT3(SPEED * deltaTime, 0.0f, 0.0f));
	}

	if (directions.bBack)
	{
		m_camera.Translate(XMFLOAT3(-SPEED * deltaTime, 0.0f, 0.0f));
	}

	if (directions.bLeft)
	{
		m_camera.Translate(XMFLOAT3(0.0f, SPEED * deltaTime, 0.0f));
	}

	if (directions.bRight)
	{
		m_camera.Translate(XMFLOAT3(0.0f, -SPEED * deltaTime, 0.0f));
	}

	if (directions.bUp)
	{
		m_camera.Translate(XMFLOAT3(0.0f, 0.0f, SPEED * deltaTime));
	}

	if (directions.bDown)
	{
		m_camera.Translate(XMFLOAT3(0.0f, 0.0f, -SPEED * deltaTime));
	}
	if (directions.bMode1)
	{
		m_DrawMode = 1;
	}
	if (directions.bMode2)
	{
		m_DrawMode = 2;
	}
}

void Scene::HandleMouseInput(int x, int y)
{
	m_camera.Pitch(ROT_ANGLE * y);
	m_camera.Yaw(-ROT_ANGLE * x);
}

void Scene::CloseCommandList()
{
	if (FAILED(m_renderer->GetCommandList()->Close()))
	{
		throw GFX_Exception("CommandList Close failed.");
	}
}

void Scene::SetViewport()
{
	m_renderer->GetCommandList()->RSSetViewports(1, &m_viewport);
	m_renderer->GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
}
