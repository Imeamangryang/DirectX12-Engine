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
	m_chunk(renderer, m_camera.GetEyePosition()),
	m_skybox(renderer)
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

	m_chunk.UpdateChunks(m_camera.GetEyePosition());

	// Object Draw
	{
		m_chunk.Draw(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
		m_skybox.Draw(m_renderer->GetCommandList(), m_camera.GetViewProjectionMatrixTransposed(), m_camera.GetEyePosition());
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

void Scene::HandleMouseClick(float x, float y)
{
	float screen_x = (2.0f * x / m_viewport.Width) - 1.0f;
	float screen_y = (-2.0F * y / m_viewport.Height) + 1.0f;

	Vector3 clipSpaceCoord = Vector3(screen_x, screen_y, 0);
	Vector3 viewSpaceCoord = XMVector3TransformCoord(clipSpaceCoord, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera.GetProjectionMatrix())));
	Vector3 worldSpaceCoord = XMVector3TransformCoord(viewSpaceCoord, XMMatrixInverse(nullptr, m_camera.GetViewMatrix()));
	
	Vector3 rayDir = worldSpaceCoord - Vector3(XMLoadFloat4(&m_camera.GetEyePosition()));
	rayDir.Normalize();

	const Vector3 camPosV3 = Vector3(XMLoadFloat4(&m_camera.GetEyePosition()));

	Ray picking_ray(camPosV3, rayDir);

	//// View Space에서의 Ray
	//Vector3 rayOrigin = Vector3(0.0f, 0.0f, 0.0f);
	//Vector3 rayDir = Vector3(viewSpaceCoord.x, viewSpaceCoord.y, 1.0f);

	//Matrix view = m_camera.GetViewMatrix();
	//Matrix viewInv = view.Invert();
	//
	//// World Space에서의 Ray
	//Vector3 worldRayOrigin = XMVector3TransformCoord(rayOrigin, viewInv);
	//Vector3 worldRayDir = XMVector3TransformNormal(rayDir, viewInv);

	//worldRayDir.Normalize();

	//// Ray 생성
	//Ray picking_ray(worldRayOrigin, worldRayDir);

	// Cube와의 충돌 검사
	float closest_distance = FLT_MAX;
	float closest_chunk_distance = FLT_MAX;
	float distance = 0.0f;

	Cube* closest_cube = nullptr;

	for(auto& cube : m_chunk.GetBlocks())
	{
		// Cube들중 가장 작은 distance를 가진 Cube 찾는다.
		if (cube.Intersects(picking_ray, distance, closest_distance))
		{
			if (closest_distance < closest_chunk_distance)
			{
				closest_chunk_distance = closest_distance;
				closest_cube = &cube;
			}
		}
	}

	if (closest_cube != nullptr)
	{
		closest_cube->SetIntersectBlock(picking_ray);
	}
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

		ImGui::BulletText("Camera Position : %.2f %.2f %.2f", m_camera.GetEyePosition().x, m_camera.GetEyePosition().y, m_camera.GetEyePosition().z);

		ImGui::BulletText("Continentalness : %.2f", m_chunk.GetContinentNoise(m_camera.GetEyePosition().x, m_camera.GetEyePosition().y));
		ImGui::BulletText("Erosion : %.2f", m_chunk.GetErosionNoise(m_camera.GetEyePosition().x, m_camera.GetEyePosition().y));
		ImGui::BulletText("PeaksValleys : %.2f", m_chunk.GetPeaksValleysNoise(m_camera.GetEyePosition().x, m_camera.GetEyePosition().y));

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
			//if (ImGui::TreeNode(m_dragon.m_objectname.c_str())) {
			//	ImGui::Text("Translation");
			//	ImGui::DragFloat("Translation X", &m_dragon.m_translation_x, 1.0f, -100000.0f, 100000.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::DragFloat("Translation Y", &m_dragon.m_translation_y, 1.0f, -100000.0f, 100000.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::DragFloat("Translation Z", &m_dragon.m_translation_z, 1.0f, -100000.0f, 100000.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::Text("Rotation");
			//	ImGui::DragFloat("Rotation X", &m_dragon.m_rotation_x, 1.0f, 0.0f, 360.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::DragFloat("Rotation Y", &m_dragon.m_rotation_y, 1.0f, 0.0f, 360.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::DragFloat("Rotation Z", &m_dragon.m_rotation_z, 1.0f, 0.0f, 360.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::Text("Scaling");
			//	ImGui::DragFloat("Scale X", &m_dragon.m_scale_x, 0.1f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::DragFloat("Scale Y", &m_dragon.m_scale_y, 0.1f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_None);
			//	ImGui::DragFloat("Scale Z", &m_dragon.m_scale_z, 0.1f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_None);

			//	ImGui::BulletText("Vertex Count : %d", m_dragon.m_vertexcount);
			//	ImGui::SameLine();
			//	ImGui::BulletText("Index Count : %d", m_dragon.m_indexcount);

			//	if (ImGui::TreeNode("Bone Tree")) {
			//		for (UINT i = 1; i < m_dragon.m_boneInfos.size(); i++)
			//		{
			//			if (m_dragon.m_boneInfos[i]->parentIndex == 0)
			//			{
			//				DrawTree(m_dragon.m_boneInfos, i);
			//			}
			//		}
			//		ImGui::TreePop();
			//	}

			//	ImGui::TreePop();
			//}
		}
		ImGui::End();

		//ImGui::Begin("CommandList", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		//ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//ImGui::End();
	}
}
