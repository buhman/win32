struct VS_INPUT
{
  float3 Position : POSITION;
  float4 Color : COLOR0;
};

struct VS_OUTPUT
{
  float4 Position   : POSITION;   // vertex position
  float4 Diffuse    : COLOR0;     // vertex diffuse color
  float2 Texture    : TEXCOORD0;
};

float4x4 mWorldViewProj;

VS_OUTPUT Main(VS_INPUT Input)
{
  VS_OUTPUT Output;

  //Output.Position = mul(float4(Input.Position, 1.0), mWorldViewProj);
  Output.Position = float4(Input.Position, 1.0);
  Output.Diffuse = Input.Color;
  Output.Texture = Input.Position.xy;

  return Output;
}
