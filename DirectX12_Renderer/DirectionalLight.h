#pragma once
#include "Light.h"

class DirectionalLight : public Light {
public:
	DirectionalLight() : Light() {}
	DirectionalLight(XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT4 specular, XMFLOAT3 direction) : 
		Light(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), ambient, diffuse, specular, XMFLOAT3(0.0f, 0.0f, 0.0f), 0.0f, direction, 0.0f) {}
	~DirectionalLight() {}
};