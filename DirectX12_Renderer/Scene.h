#pragma once

#include "stdafx.h"
#include "Terrain.h"
#include "Camera.h"
#include "Sky.h"
#include "Moon.h"

using namespace graphics;

#define SPEED 1000.0f
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

private:
	void CloseCommandList();
	void SetViewport();

	Graphics* m_renderer;
	Camera m_camera;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	int m_DrawMode = 1;

	Terrain m_terrain;
	Sky m_sky;
	Moon m_moon;
};