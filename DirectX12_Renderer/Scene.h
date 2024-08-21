#pragma once

#include "stdafx.h"
#include "Camera.h"
#include "Cube.h"
#include "ImGuiLoader.h"
#include "Chunk.h"
#include "Dragon.h"
#include "Skybox.h"

using namespace graphics;
using namespace DirectX::SimpleMath;

#define SPEED 100.0f
#define ROT_ANGLE 0.75f

struct InputDirections
{
	BOOL bFront;
	BOOL bBack;
	BOOL bLeft;
	BOOL bRight;
	BOOL bUp;
	BOOL bDown;
	BOOL bMode1;
	BOOL bMode2;
};

class Scene
{
public:
	Scene(int height, int width, Graphics* renderer);
	~Scene();

	void Draw();

	void HandleInput(const InputDirections& directions, float deltaTime);

	void HandleMouseInput(float x, float y);

	void HandleMouseClick(float x, float y);

private:
	void CloseCommandList();
	void SetViewport();
	void SetImGuiWindow();

	Graphics* m_renderer;
	Camera m_camera;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	int m_DrawMode = 1;

	ImGuiLoader m_imguiLoader;

	Cube m_cube;
	Chunk m_chunk;
	Skybox m_skybox;
};