#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
	Camera(int height, int width);
	~Camera();

	XMFLOAT4X4 GetViewProjectionMatrixTransposed();
	XMFLOAT4X4 GetProjectionMatrix() { return m_projection; }
	XMMATRIX GetViewMatrix() { return m_view; }

	XMFLOAT4 GetEyePosition() { return m_pos; }
	

	XMFLOAT4 Translate(XMFLOAT3 move);

	void Pitch(float theta);

	void Yaw(float theta);

	void Roll(float theta);

private:
	XMFLOAT4X4 m_projection;
	XMMATRIX m_view;
	XMFLOAT4 m_pos;
	XMFLOAT4 m_lookat;
	XMFLOAT4 m_up;

	XMFLOAT4 m_left;

	float m_Yaw;
	float m_Pitch;
	float m_Roll;
};