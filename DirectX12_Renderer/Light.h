#pragma once

#include <DirectXMath.h>
#include <Windows.h>

using namespace DirectX;

struct LightSource {
	LightSource() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4	pos;
	XMFLOAT4	intensityAmbient;
	XMFLOAT4	intensityDiffuse;
	XMFLOAT4	intensitySpecular;
	XMFLOAT3	attenuation;
	float		range;
	XMFLOAT3	direction;
	float		spotlightConeExponent;
};

class Light {
public:
	Light() {}
	Light(XMFLOAT4 position, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT4 specular, XMFLOAT3 attenuation, float range, XMFLOAT3 direction, float sexp);
	~Light() {}

	LightSource GetLight() { return m_lightData; }
	void SetLightDirection(XMFLOAT3 dir) { m_lightData.direction = dir; }

protected:
	LightSource m_lightData;
};
