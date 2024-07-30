#include "Scene.h"

// Draon Bone Tree Draw
void DrawTree(const vector<shared_ptr<FbxBoneInfo>>& bones, UINT& index)
{
	if (index >= bones.size())
		return;

	auto& bone = bones[index];

	// 자식 노드 없음
	if (bone->childcount == 0)
	{
		ImGui::BulletText(bone->boneName.c_str());
		return;
	}
	// 자식 노드 있음
	else {
		if (ImGui::TreeNode(bones[index]->boneName.c_str())) {
			UINT parentindex = index;
			for (UINT i = 0; i < bone->childcount; i++) {
				index += 1;
				while (parentindex != bones[index]->parentIndex) {
					index += 1;
				}
				DrawTree(bones, index);
			}
			ImGui::TreePop();
		}
	}
}

Scene::Scene(int height, int width, Graphics* renderer) : 
	m_renderer(renderer),
	m_camera(height, width),
	m_imguiLoader(renderer),
	m_cube(renderer),
	m_chunk(renderer)
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

}

Scene::~Scene()
{
	m_renderer->ClearAllFrames();
	m_renderer = nullptr;
	m_imguiLoader.~ImGuiLoader();
}

void Scene::Draw()
{
	// ImGui Render
	m_imguiLoader.Draw();
	SetImGuiWindow();

	// DirectX Render
	m_renderer->ResetPipeline();

	m_renderer->SetBackBufferRender(m_renderer->GetCommandList(), DirectX::Colors::LightBlue);

	SetViewport();


	// WireFrame Draw Setting
	if (m_DrawMode == 1)
	{
		m_cube.SetIsWireframe(false);
	}
	else
	{
		m_cube.SetIsWireframe(true);
	}

	// Object Draw
	{
		//m_cube.Draw(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
		m_chunk.Draw(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
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

void Scene::HandleMouseInput(float x, float y)
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

void Scene::SetImGuiWindow()
{
	ImGui::NewFrame();
	{
		ImGui::Begin("Details", NULL, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (!ImGui::CollapsingHeader("Draw Modes")) {
			if (ImGui::Button("Solid")) {
				m_DrawMode = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("WireFrame")) {
				m_DrawMode = 2;
			}
		}
		if (!ImGui::CollapsingHeader("Objects")) {
			if (ImGui::TreeNode(m_cube.m_objectname.c_str())) {
				ImGui::Text("Translation");
				ImGui::DragFloat("Translation X", &m_cube.m_translation_x, 1.0f, -100000.0f, 100000.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::DragFloat("Translation Y", &m_cube.m_translation_y, 1.0f, -100000.0f, 100000.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::DragFloat("Translation Z", &m_cube.m_translation_z, 1.0f, -100000.0f, 100000.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::Text("Rotation");
				ImGui::DragFloat("Rotation X", &m_cube.m_rotation_x, 1.0f, 0.0f, 360.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::DragFloat("Rotation Y", &m_cube.m_rotation_y, 1.0f, 0.0f, 360.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::DragFloat("Rotation Z", &m_cube.m_rotation_z, 1.0f, 0.0f, 360.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::Text("Scaling");
				ImGui::DragFloat("Scale X", &m_cube.m_scale_x, 0.1f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::DragFloat("Scale Y", &m_cube.m_scale_y, 0.1f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_None);
				ImGui::DragFloat("Scale Z", &m_cube.m_scale_z, 0.1f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_None);

				ImGui::BulletText("Vertex Count : %d", m_cube.m_vertexcount);
				ImGui::SameLine();
				ImGui::BulletText("Index Count : %d", m_cube.m_indexcount);

				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
}
