#include "Fluid2D.h"
#include <fstream>
#include <wincodec.h>

void Fluid2D::GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, u32 pathSize)
{
    if (path == nullptr)
    {
        throw std::exception();
    }

    DWORD size = GetModuleFileName(nullptr, path, pathSize);
    if (size == 0 || size == pathSize)
    {
        // Method failed or path was truncated.
        throw std::exception();
    }

    WCHAR* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';
    }
}

std::wstring Fluid2D::GetAssetFullPath(LPCWSTR assetName)
{
    return mAssetPath + assetName;
}

void Fluid2D::CreateComputeShaderStateObject(const LPCWSTR& compiledComputeShaderName, ComPtr<ID3D12PipelineState>& computePipelineState, ComPtr<ID3D12RootSignature> rootSig)
{
    u32 fileSize = 0;
    UINT8* shaderCodePtr;
    utility::ReadDataFromFile(GetAssetFullPath(compiledComputeShaderName).c_str(), &shaderCodePtr, &fileSize);

    D3D12_COMPUTE_PIPELINE_STATE_DESC copmputePSODesc = {};
    copmputePSODesc.pRootSignature = rootSig.Get();
    copmputePSODesc.CS = CD3DX12_SHADER_BYTECODE((void*)shaderCodePtr, fileSize);

    HRESULT hr = mDevice->GetDevice()->CreateComputePipelineState(&copmputePSODesc, IID_PPV_ARGS(&computePipelineState));

    if (FAILED(hr)) {
        throw std::runtime_error("CreateComputePipelineStateObject failed.");
    }
}

void Fluid2D::CreateComputeRootSignatureAndPSO()
{
    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsClearFloat = rsCreater.Create(mDevice, false, L"rsClearFloat");
        CreateComputeShaderStateObject(ComputeShaders::ClearFloat, mClearFloatPSO, mRsClearFloat);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsClearFloat2 = rsCreater.Create(mDevice, false, L"rsClearFloat2");
        CreateComputeShaderStateObject(ComputeShaders::ClearFloat2, mClearFloat2PSO, mRsClearFloat2);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.PushStaticSampler(0, 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp);
        rsCreater.Push(utility::RootSignatureCreater::RootType::CBV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsApplyExternalForce = rsCreater.Create(mDevice, false, L"rsApplyExternalForce");
        CreateComputeShaderStateObject(ComputeShaders::ApplyExternalForce, mApplyExternalForcePSO, mRsApplyExternalForce);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.PushStaticSampler(0, 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp);
        rsCreater.Push(utility::RootSignatureCreater::RootType::CBV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsGenerateDivergence = rsCreater.Create(mDevice, false, L"rsGenerateDivergence");
        CreateComputeShaderStateObject(ComputeShaders::GenerateDivergence, mGenerateDivergencePSO, mRsGenerateDivergence);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.PushStaticSampler(0, 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp);
        rsCreater.Push(utility::RootSignatureCreater::RootType::CBV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsSetTexture = rsCreater.Create(mDevice, false, L"rsSetTexture");
        CreateComputeShaderStateObject(ComputeShaders::SetTexture, mSetTexturePSO, mRsSetTexture);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.PushStaticSampler(0, 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp);
        rsCreater.Push(utility::RootSignatureCreater::RootType::CBV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsUpdateAdvection = rsCreater.Create(mDevice, false, L"rsUpdateAdvection");
        CreateComputeShaderStateObject(ComputeShaders::UpdateAdvection, mUpdateAdvectionPSO, mRsUpdateAdvection);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.PushStaticSampler(0, 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp);
        rsCreater.Push(utility::RootSignatureCreater::RootType::CBV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 1);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsUpdatePressure = rsCreater.Create(mDevice, false, L"rsUpdatePressure");
        CreateComputeShaderStateObject(ComputeShaders::UpdatePressure, mUpdatePressurePSO, mRsUpdatePressure);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.PushStaticSampler(0, 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
        rsCreater.Push(utility::RootSignatureCreater::RootType::CBV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 1);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsUpdateResult = rsCreater.Create(mDevice, false, L"rsUpdateResult");
        CreateComputeShaderStateObject(ComputeShaders::UpdateResult, mUpdateResultPSO, mRsUpdateResult);
    }

    {
        utility::RootSignatureCreater rsCreater;
        rsCreater.PushStaticSampler(0, 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp, utility::RootSignatureCreater::AddressMode::Clamp);
        rsCreater.Push(utility::RootSignatureCreater::RootType::CBV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 0);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::SRV, 1);
        rsCreater.Push(utility::RootSignatureCreater::RangeType::UAV, 0);
        mRsUpdateVelocity = rsCreater.Create(mDevice, false, L"rsUpdateVelocity");
        CreateComputeShaderStateObject(ComputeShaders::UpdateVelocity, mUpdateVelocityPSO, mRsUpdateVelocity);
    }
}