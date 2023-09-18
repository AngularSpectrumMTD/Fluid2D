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
Texture2D<float2> sourceVelocity : register(t1);
RWTexture2D<float2> destVelocity : register(u0);

[numthreads(8, 8, 1)]
void updateVelocity(uint2 dtid : SV_DispatchThreadID)
{
    const float2 uv = (dtid + 0.5.xx) / fluidConstantBuffer.bufferWH;

    float pressure_nx = sourcePressure.SampleLevel(imageSampler, uv - float2(1 / fluidConstantBuffer.bufferWH.x, 0), 0);
    float pressure_px = sourcePressure.SampleLevel(imageSampler, uv + float2(1 / fluidConstantBuffer.bufferWH.x, 0), 0);
    float pressure_ny = sourcePressure.SampleLevel(imageSampler, uv - float2(0, 1 / fluidConstantBuffer.bufferWH.y), 0);
    float pressure_py = sourcePressure.SampleLevel(imageSampler, uv + float2(0, 1 / fluidConstantBuffer.bufferWH.y), 0);
    
    float2 srcVelocity = sourceVelocity.SampleLevel(imageSampler, uv, 0);
    destVelocity[dtid] = srcVelocity - (float2(pressure_px - pressure_nx, pressure_py - pressure_ny)) / 2;
}