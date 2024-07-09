#include "Light.h"

Light::Light(XMFLOAT4 position, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT4 specular, XMFLOAT3 attenuation, float range, XMFLOAT3 direction, float sexp)
{
	m_lightData.pos = position;
	m_lightData.intensityAmbient = ambient;
	m_lightData.intensityDiffuse = diffuse;
	m_lightData.intensitySpecular = specular;
	m_lightData.attenuation = attenuation;
	m_lightData.range = range;
	m_lightData.spotlightConeExponent = sexp;

	XMVECTOR v = XMLoadFloat3(&direction);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&m_lightData.direction, v);
}
