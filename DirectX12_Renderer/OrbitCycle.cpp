#include "OrbitCycle.h"

OrbitCycle::OrbitCycle(UINT period) : 
	m_Sun(XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)),
	m_Moon(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)),
	m_period(period)
{
	m_Last = system_clock::now();
}

OrbitCycle::~OrbitCycle()
{
}

void OrbitCycle::Update()
{
	time_point<system_clock> now = system_clock::now();

	// get the amount of time in ms since the last time we updated.
	milliseconds elapsed = duration_cast<milliseconds>(now - m_Last);

	// calculate how far to rotate.
	double angletorotate = elapsed.count() * m_period * XMConvertToRadians(DEG_PER_MILLI);

	// rotate the sun's direction vector.
	XMFLOAT3 tmp = m_Sun.GetLight().direction;
	XMVECTOR dir = XMLoadFloat3(&tmp);
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(0.0f, (float)angletorotate, 0.0f);
	dir = XMVector3Normalize(XMVector3Rotate(dir, rot));
	XMStoreFloat3(&tmp, dir);
	m_Sun.SetLightDirection(tmp);

	// rotate the moon's direction vector.
	tmp = m_Moon.GetLight().direction;
	dir = XMLoadFloat3(&tmp);
	rot = XMQuaternionRotationRollPitchYaw(0.0f, (float)angletorotate, 0.0f);
	dir = XMVector3Normalize(XMVector3Rotate(dir, rot));
	XMStoreFloat3(&tmp, dir);
	m_Moon.SetLightDirection(tmp);

	// update the time for the next pass.
	m_Last = now;
}



