struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 psmain(PS_INPUT input) : SV_Target
{
    // simple flat grey color for untextured primitives
    return float4(0.8f, 0.8f, 0.8f, 1.0f);
}
