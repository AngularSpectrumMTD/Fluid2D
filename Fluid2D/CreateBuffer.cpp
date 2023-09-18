#include "Fluid2D.h"

using namespace DirectX;

void Fluid2D::CreateResultBuffer()
{
    auto width = GetWidth();
    auto height = GetHeight();

    for (u32 i = 0; i < 2; i++)
    {
        mResultBufferTbl[i] = mDevice->CreateTexture2D(
            width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_HEAP_TYPE_DEFAULT,
            (i == 0) ? L"ResultBuffer0" : L"ResultBuffer1"
        );

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        mResultSRVTbl[i] = mDevice->CreateShaderResourceView(mResultBufferTbl[i].Get(), &srvDesc);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        mResultUAVTbl[i] = mDevice->CreateUnorderedAccessView(mResultBufferTbl[i].Get(), &uavDesc);

        mResultStateTbl[i] = D3D12_RESOURCE_STATE_COPY_SOURCE;
    }

    for (u32 i = 0; i < 2; i++)
    {
        mVelocityBufferTbl[i] = mDevice->CreateTexture2D(
            width, height, DXGI_FORMAT_R32G32_FLOAT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_HEAP_TYPE_DEFAULT,
            (i == 0) ? L"VelocityBuffer0" : L"VelocityBuffer1"
        );

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        mVelocitySRVTbl[i] = mDevice->CreateShaderResourceView(mVelocityBufferTbl[i].Get(), &srvDesc);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        mVelocityUAVTbl[i] = mDevice->CreateUnorderedAccessView(mVelocityBufferTbl[i].Get(), &uavDesc);

        mVelocityStateTbl[i] = D3D12_RESOURCE_STATE_COPY_SOURCE;
    }

    for (u32 i = 0; i < 2; i++)
    {
        mPressureBufferTbl[i] = mDevice->CreateTexture2D(
            width, height, DXGI_FORMAT_R32_FLOAT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_HEAP_TYPE_DEFAULT,
            (i == 0) ? L"PressureBuffer0" : L"PressureBuffer1"
        );

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        mPressureSRVTbl[i] = mDevice->CreateShaderResourceView(mPressureBufferTbl[i].Get(), &srvDesc);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        mPressureUAVTbl[i] = mDevice->CreateUnorderedAccessView(mPressureBufferTbl[i].Get(), &uavDesc);

        mPressureStateTbl[i] = D3D12_RESOURCE_STATE_COPY_SOURCE;
    }

    mDivergenceBuffer = mDevice->CreateTexture2D(
        width, height, DXGI_FORMAT_R32_FLOAT,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_HEAP_TYPE_DEFAULT,
        L"DivergenceBuffer"
    );

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    mDivergenceSRV = mDevice->CreateShaderResourceView(mDivergenceBuffer.Get(), &srvDesc);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    mDivergenceUAV = mDevice->CreateUnorderedAccessView(mDivergenceBuffer.Get(), &uavDesc);

    mDivergenceState = D3D12_RESOURCE_STATE_COPY_SOURCE;
}

void Fluid2D::CreateRegularBuffer()
{
    CreateResultBuffer();
}

void Fluid2D::CreateConstantBuffer()
{
    mSceneCB = mDevice->CreateConstantBuffer(sizeof(SceneParam), nullptr, L"SceneCB");
}