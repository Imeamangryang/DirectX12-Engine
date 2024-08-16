#define NUM_CONTROL_POINTS 3
Texture2D Tex_table[300] : register(t0);

SamplerState dmsampler : register(s0);
SamplerState cmsampler : register(s1);

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
    uint instanceID : SV_InstanceID;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
    uint instanceID : SV_InstanceID;
};

struct LightData
{
    float4 pos;
    float4 amb;
    float4 dif;
    float4 spec;
    float3 att;
    float rng;
    float3 dir;
    float sexp;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 viewproj;
    float4 eye;
    LightData light;
}

struct InstanceBuffer
{
    float4x4 instanceTrans;
    int blocktype;
    bool isvisible;
};

StructuredBuffer<InstanceBuffer> instanceTransforms : register(t4); // �ν��Ͻ� ��ȯ ���

// blocktype�� ���� �ٸ� ������ �������� ����
float4 GetColor(int blocktype, float2 tex)
{
    if (blocktype == 0) return float4(1.0f, 0.0f, 0.0f, 1.0f);
    else if (blocktype == 1) return float4(Tex_table[0].Sample(cmsampler, tex));
    else if (blocktype == 2) return float4(Tex_table[1].Sample(cmsampler, tex));
    else if (blocktype == 3) return float4(Tex_table[2].Sample(cmsampler, tex));
    else if (blocktype == 5) return float4(Tex_table[3].Sample(cmsampler, tex));
    else return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

float3 GetSamplePixel(int blocktype, float2 tex, int i, int j, float2 texelsize)
{
    if (blocktype == 0)
        return Tex_table[0].Sample(cmsampler, tex + float2(i, j) * texelsize).rgb;
    else if (blocktype == 1)
        return Tex_table[0].Sample(cmsampler, tex + float2(i, j) * texelsize).rgb;
    else if (blocktype == 2)
        return Tex_table[1].Sample(cmsampler, tex + float2(i, j) * texelsize).rgb;
    else if (blocktype == 3)
        return Tex_table[2].Sample(cmsampler, tex + float2(i, j) * texelsize).rgb;
    else if (blocktype == 5)
        return Tex_table[3].Sample(cmsampler, tex + float2(i, j) * texelsize).rgb;
    else
        return Tex_table[0].Sample(cmsampler, tex + float2(i, j) * texelsize).rgb;
}


// Vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.pos = float4(input.pos, 1.0f);
    output.pos += mul(instanceTransforms[input.instanceID].instanceTrans, output.pos);
 
    output.pos = mul(world, output.pos);
    output.pos = mul(output.pos, viewproj);
    
    output.norm = input.norm;
    
    output.tan = input.tan;
    
    output.tex = input.tex;
    
    output.instanceID = input.instanceID;

    return output;
}

// Pixel shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float2 texelSize = float2(1.0 / 128.0, 1.0 / 128.0); // �ؽ�ó ũ�⿡ �°� ����

    // Sobel Ŀ��
    float3x3 sobelX = float3x3(
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );

    float3x3 sobelY = float3x3(
        -1, -2, -1,
         0, 0, 0,
         1, 2, 1
    );
    
    // �ֺ� �ȼ� ���ø�
    float3 sample[3][3];
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            sample[i + 1][j + 1] = GetSamplePixel(instanceTransforms[input.instanceID].blocktype, input.tex, i, j, texelSize);
        }
    }

    // Sobel ���� ����
    float3 Gx = float3(0.0, 0.0, 0.0);
    float3 Gy = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            Gx += sobelX[i][j] * sample[i][j];
            Gy += sobelY[i][j] * sample[i][j];
        }
    }

    // �׶���Ʈ ũ�� ���
    float3 gradient = sqrt(Gx * Gx + Gy * Gy);

    // �ܰ��� ���� ���
    float edge = length(gradient);
    
    // �Ӱ谪�� �̿��� �ܰ��� �β� ����
    float threshold = 0.1; // �Ӱ谪 ����
    edge = edge > threshold ? 1.0 : 0.0;

    // ���� ���� ���
    float3 norm = input.norm;
    norm = normalize(norm);
    float4 color = GetColor(instanceTransforms[input.instanceID].blocktype, input.tex);

    // World space������ light pos�� dir ���
    float3 worldlightpos = mul(light.pos, world).xyz;
    float3 lightDir = normalize(input.pos.xyz - worldlightpos);

    float4 ambient = light.amb;
    float4 diffuse = light.dif * dot(-lightDir, norm);
    float3 V = reflect(lightDir, norm);
    float3 toEye = normalize(eye.xyz - input.pos.xyz);
    float4 specular = 0.1f * light.spec * pow(max(dot(V, toEye), 0.0f), 4.0f);

    float4 finalColor = float4(saturate((ambient + diffuse + specular).rgb * color.rgb), color.a);

    float4 edgeColor = float4(edge, edge, edge, 1.0f);
    
    if (instanceTransforms[input.instanceID].isvisible == 1)
    {
        // �ܰ����� ���� ������ ����
        return edgeColor * 1.0 + finalColor * 1.0;
    }
    
    return finalColor;
}
