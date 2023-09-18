struct fluidCB
{
    float2 mouseXY;
    float2 velocityXY;
    float2 bufferWH;
    float deltaTime;
};

SamplerState imageSampler : register(s0);
ConstantBuffer<fluidCB> fluidConstantBuffer : register(b0);
Texture2D<float> sourcePressure : register(t0);
Texture2D<float> divergence : register(t1);
RWTexture2D<float> destPressure : register(u0);

[numthreads(8, 8, 1)]
void updatePressure(uint2 dtid : SV_DispatchThreadID)
{
    const float2 uv = (dtid + 0.5.xx) / fluidConstantBuffer.bufferWH;

    float pressure_nx = sourcePressure.SampleLevel(imageSampler, uv - float2(1 / fluidConstantBuffer.bufferWH.x, 0), 0);
    float pressure_px = sourcePressure.SampleLevel(imageSampler, uv + float2(1 / fluidConstantBuffer.bufferWH.x, 0), 0);
    float pressure_ny = sourcePressure.SampleLevel(imageSampler, uv - float2(0, 1 / fluidConstantBuffer.bufferWH.y), 0);
    float pressure_py = sourcePressure.SampleLevel(imageSampler, uv + float2(0, 1 / fluidConstantBuffer.bufferWH.y), 0);
    
    float div = divergence[dtid];
    destPressure[dtid] = (pressure_nx + pressure_px + pressure_ny + pressure_py) / 4 - div / 4;//avg
}