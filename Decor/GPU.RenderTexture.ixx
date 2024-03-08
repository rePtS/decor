module;

#include <wrl/client.h>
#include "DirectXHelpers.h"
#include <stdexcept>

export module GPU.RenderTexture;

import Utils;

using Microsoft::WRL::ComPtr;

export class RenderTexture
{
public:
    explicit RenderTexture(DXGI_FORMAT format) noexcept :
        m_format(format),
        m_width(0),
        m_height(0)
    {
    }

    RenderTexture(RenderTexture&&) = default;
    RenderTexture& operator= (RenderTexture&&) = default;

    RenderTexture(RenderTexture const&) = delete;
    RenderTexture& operator= (RenderTexture const&) = delete;

    void SetDevice(_In_ ID3D11Device* device)
    {
        if (device == m_device.Get())
            return;

        if (m_device)
        {
            ReleaseDevice();
        }

        {
            UINT formatSupport = 0;
            if (FAILED(device->CheckFormatSupport(m_format, &formatSupport)))
            {
                throw std::runtime_error("CheckFormatSupport");
            }

            constexpr UINT32 required = D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_RENDER_TARGET;
            if ((formatSupport & required) != required)
            {
#ifdef _DEBUG
                char buff[128] = {};
                sprintf_s(buff, "RenderTexture: Device does not support the requested format (%u)!\n", m_format);
                OutputDebugStringA(buff);
#endif
                throw std::runtime_error("RenderTexture");
            }
        }

        m_device = device;
    }

    void SizeResources(size_t width, size_t height)
    {
        if (width == m_width && height == m_height)
            return;

        if (m_width > UINT32_MAX || m_height > UINT32_MAX)
        {
            throw std::out_of_range("Invalid width/height");
        }

        if (!m_device)
            return;

        m_width = m_height = 0;

        // Create a render target
        CD3D11_TEXTURE2D_DESC renderTargetDesc(
            m_format,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            1, // The render target view has only one texture.
            1, // Use a single mipmap level.
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
            D3D11_USAGE_DEFAULT,
            0,
            1
        );

        Utils::ThrowIfFailed(m_device->CreateTexture2D(
            &renderTargetDesc,
            nullptr,
            m_renderTarget.ReleaseAndGetAddressOf()
        ));

        Utils::SetResourceName(m_renderTarget, "RenderTexture RT");

        // Create RTV.
        CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, m_format);

        Utils::ThrowIfFailed(m_device->CreateRenderTargetView(
            m_renderTarget.Get(),
            &renderTargetViewDesc,
            m_renderTargetView.ReleaseAndGetAddressOf()
        ));

        Utils::SetResourceName(m_renderTargetView, "RenderTexture RTV");

        // Create SRV.
        CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(D3D11_SRV_DIMENSION_TEXTURE2D, m_format);

        Utils::ThrowIfFailed(m_device->CreateShaderResourceView(
            m_renderTarget.Get(),
            &shaderResourceViewDesc,
            m_shaderResourceView.ReleaseAndGetAddressOf()
        ));

        Utils::SetResourceName(m_shaderResourceView, "RenderTexture SRV");

        m_width = width;
        m_height = height;
    }

    void ReleaseDevice() noexcept
    {
        m_renderTargetView.Reset();
        m_shaderResourceView.Reset();
        m_renderTarget.Reset();

        m_device.Reset();

        m_width = m_height = 0;
    }

    ID3D11Texture2D* GetRenderTarget() const noexcept { return m_renderTarget.Get(); }
    ID3D11RenderTargetView* GetRenderTargetView() const noexcept { return m_renderTargetView.Get(); }
    ID3D11ShaderResourceView* GetShaderResourceView() const noexcept { return m_shaderResourceView.Get(); }

    DXGI_FORMAT GetFormat() const noexcept { return m_format; }

private:
    ComPtr<ID3D11Device>                m_device;
    ComPtr<ID3D11Texture2D>             m_renderTarget;
    ComPtr<ID3D11RenderTargetView>      m_renderTargetView;
    ComPtr<ID3D11ShaderResourceView>    m_shaderResourceView;

    DXGI_FORMAT                         m_format;

    size_t                              m_width;
    size_t                              m_height;
};