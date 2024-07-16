#pragma once
#include "stdafx.h"
#include "Light.h"

class DirectionalLight : public Light {
public:
	DirectionalLight() : Light() {}
	DirectionalLight(XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT4 specular, XMFLOAT3 direction) : 
		Light(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), ambient, diffuse, specular, XMFLOAT3(0.0f, 0.0f, 0.0f), 0.0f, direction, 0.0f) {}
	~DirectionalLight() {}

	LightSource GetDirectionalLight() { return m_light.GetLight(); }
private:

	Light m_light = Light(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 0.0f, XMFLOAT3(1.0f, 0.0f, 0.0f), 0.0f);
};