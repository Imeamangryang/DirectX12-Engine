#include "Camera.h"

Camera::Camera(int height, int width)
{
	m_Yaw = m_Pitch = m_Roll = 0.0f;

	// Projection Matrix 설정
	XMMATRIX viewproj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100000.0f);
	XMStoreFloat4x4(&m_projection, viewproj);

	// 카메라의 Position, Look, Up vector 설정
	//m_pos = XMFLOAT4(0.0f, 0.0f, 20000.0f, 0.0f);
	m_pos = XMFLOAT4(0.0f, 0.0f, 2000.0f, 0.0f);

	m_lookat = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR look = XMVector3Normalize(XMLoadFloat4(&m_lookat));
	XMStoreFloat4(&m_lookat, look);

	m_left = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR left = XMVector3Cross(look, XMLoadFloat4(&m_left));
	XMStoreFloat4(&m_left, left);

	XMVECTOR up = XMVector3Cross(left, look);
	XMStoreFloat4(&m_up, up);
}

Camera::~Camera()
{
}

// MVP Matrix 생성
XMFLOAT4X4 Camera::GetViewProjectionMatrixTransposed()
{
	XMVECTOR look = XMLoadFloat4(&m_lookat);
	XMVECTOR up = XMLoadFloat4(&m_up);
	XMVECTOR left = XMLoadFloat4(&m_left);

	if (m_Pitch != 0 || m_Yaw != 0 || m_Roll != 0) {
		float pitch_rad = XMConvertToRadians(m_Pitch);
		float yaw_rad = XMConvertToRadians(m_Yaw);
		float roll_rad = XMConvertToRadians(m_Roll);

		XMMATRIX rot, rotp, roty, rotr;
		rotp = XMMatrixRotationAxis(left, pitch_rad);
		roty = XMMatrixRotationAxis(up, yaw_rad);
		rotr = XMMatrixRotationAxis(look, roll_rad);
		rot = rotp * roty * rotr;
		look = XMVector3Normalize(XMVector3Transform(look, rot));
		left = XMVector3Normalize(XMVector3Transform(left, rot));
		up = XMVector3Cross(left, look);
	}

	XMVECTOR camera = XMLoadFloat4(&m_pos);
	XMVECTOR target = camera + look; 
	XMMATRIX view = XMMatrixLookAtLH(camera, target, up);

	XMMATRIX proj = XMLoadFloat4x4(&m_projection);
	XMMATRIX viewproj = XMMatrixTranspose(view * proj);
	XMFLOAT4X4 final;
	XMStoreFloat4x4(&final, viewproj);
	return final;
}

XMFLOAT4 Camera::Translate(XMFLOAT3 move)
{
	XMVECTOR look = XMLoadFloat4(&m_lookat);
	XMVECTOR up = XMLoadFloat4(&m_up);
	XMVECTOR left = XMLoadFloat4(&m_left);
	if (m_Pitch != 0 || m_Yaw != 0 || m_Roll != 0) {
		float pitch_rad = XMConvertToRadians(m_Pitch);
		float yaw_rad = XMConvertToRadians(m_Yaw);
		float roll_rad = XMConvertToRadians(m_Roll);

		XMMATRIX rot, rotp, roty, rotr;
		rotp = XMMatrixRotationAxis(left, pitch_rad);
		roty = XMMatrixRotationAxis(up, yaw_rad);
		rotr = XMMatrixRotationAxis(look, roll_rad);
		rot = rotp * roty * rotr;
		look = XMVector3Normalize(XMVector3Transform(look, rot));
		left = XMVector3Normalize(XMVector3Transform(left, rot));
		up = XMVector3Cross(left, look);
	}

	XMVECTOR tmp = XMLoadFloat4(&m_pos);

	tmp += look * move.x + left * move.y + up * move.z;

	XMStoreFloat4(&m_pos, tmp);
	return m_pos;
}

void Camera::Pitch(float theta)
{
	m_Pitch += theta;
	m_Pitch = m_Pitch > 360 ? m_Pitch - 360 : m_Pitch < -360 ? m_Pitch + 360 : m_Pitch;
}

void Camera::Yaw(float theta)
{
	m_Yaw += theta;
	m_Yaw = m_Yaw > 360 ? m_Yaw - 360 : m_Yaw < -360 ? m_Yaw + 360 : m_Yaw;
}

void Camera::Roll(float theta)
{
	m_Roll += theta;
	m_Roll = m_Roll > 360 ? m_Roll - 360 : m_Roll < -360 ? m_Roll + 360 : m_Roll;
}
