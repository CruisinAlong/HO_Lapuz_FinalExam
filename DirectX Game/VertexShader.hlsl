cbuffer CBPerObject : register(b0)
{
    row_major float4x4 world;
    row_major float4x4 view;
    row_major float4x4 projection;
    float time;
    float padding[3];
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

VS_OUTPUT vsmain(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 pos = float4(input.position, 1.0f);
    float4 worldPos = mul(pos, world);
    float4 viewPos = mul(worldPos, view);
    output.position = mul(viewPos, projection);
    output.texcoord = input.texcoord;
    return output;
}

// Instanced variant: consumes per-instance matrix rows as instance inputs
struct VS_INPUT_INST
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    // Use TEXCOORD1..4 for instance rows to be portable and accepted by CreateInputLayout
    float4 instanceRow0 : TEXCOORD1;
    float4 instanceRow1 : TEXCOORD2;
    float4 instanceRow2 : TEXCOORD3;
    float4 instanceRow3 : TEXCOORD4;
};

VS_OUTPUT vsmain_instanced(VS_INPUT_INST input)
{
    VS_OUTPUT output;
    float4 pos = float4(input.position, 1.0f);
    float4x4 instanceWorld = float4x4(
        input.instanceRow0,
        input.instanceRow1,
        input.instanceRow2,
        input.instanceRow3
    );
    float4 worldPos = mul(pos, instanceWorld);
    float4 viewPos = mul(worldPos, view);
    output.position = mul(viewPos, projection);
    output.texcoord = input.texcoord;
    return output;
}