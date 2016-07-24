/*
DirectX-11 main class implementation
*/

#include "pch.h"

#include "DX11Base.h"
#include "DX11Utils.h"
#include "DXGIFormats.h"
#include "DX11ShaderCompiler.h"
#include "DX11RenderTarget.h"

#include <C3E\core\threading.h>
#include <C3E\core\console.h>

#include <sstream>

using namespace std;
using namespace C3E;
using namespace C3E::ABI;

namespace C3E
{
	namespace DX11
	{
		class DX11Render;

		/*
		struct DX11Shader
		{
		ShaderType shaderType = SHADER_TYPE_UNKNOWN;
		ComPtr<ID3D11DeviceChild> shaderInterface;
		};

		struct DX11ShaderResource
		{
		ComPtr<ID3D11Resource> m_resource;
		ComPtr<ID3D11ShaderResourceView> m_resourceView;
		};
		*/

		class DX11RenderContext : public ABI::IRenderContext
		{
		private:

			DX11Render* m_renderer = nullptr;
			ComPtr<ID3D11DeviceContext> m_context;
			ComPtr<ID3D11CommandList> m_context_commandlist;

		public:

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			DX11RenderContext(DX11Render* renderer);
			~DX11RenderContext() {}

			ComPtr<ID3D11DeviceContext> GetDeviceContext() const { return m_context; }

			void ExecuteCommandlist(const ComPtr<ID3D11DeviceContext>& immdediatecontext)
			{
				if (m_context_commandlist.Get())
					immdediatecontext->ExecuteCommandList(m_context_commandlist.Get(), false);

				m_context_commandlist.Reset();
				m_context->ClearState();
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			void ContextBegin() override;
			void ContextEnd() override;

			void ResourceSet(ResourceHandle buffer, const void* mem, uint32 size) override
			{
				auto buf = (ID3D11Buffer*)buffer;
				D3D11_BUFFER_DESC desc;
				buf->GetDesc(&desc);

				if (desc.Usage == D3D11_USAGE_DEFAULT || desc.Usage == D3D11_USAGE_STAGING)
				{
					m_context->UpdateSubresource(buf, 0, nullptr, mem, 0, 0);
				}
				else if (desc.Usage == D3D11_USAGE_DYNAMIC)
				{
					D3D11_MAPPED_SUBRESOURCE ms;

					HRESULT hr = m_context->Map(buf, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
					memcpy(ms.pData, mem, size);
					m_context->Unmap(buf, NULL);

					C3E_ASSERT(SUCCEEDED(hr));
				}
			}

			void ResourceCopy(ResourceHandle source, ResourceHandle dest) override
			{
				auto sresv = (ID3D11View*)source;
				auto dresv = (ID3D11View*)dest;

				ID3D11Resource* sres = nullptr;
				ID3D11Resource* dres = nullptr;

				sresv->GetResource(&sres);
				dresv->GetResource(&dres);

				m_context->CopyResource(dres, sres);

				sres->Release();
				dres->Release();
			}

			void ResourceResolve(ResourceHandle source, ResourceHandle dest) override
			{
				auto sresv = (ID3D11View*)source;
				auto dresv = (ID3D11View*)dest;

				ID3D11Resource* sres = nullptr;
				ID3D11Resource* dres = nullptr;

				sresv->GetResource(&sres);
				dresv->GetResource(&dres);

				m_context->ResolveSubresource(dres, 0, sres, 0, DXGI_FORMAT_UNKNOWN);

				sres->Release();
				dres->Release();
			}

			void SetRenderTarget(ResourceHandle rtv, ResourceHandle dbv, uint32 depthBufferIndex, uint32 renderTargetIndex) override
			{
				auto rt = (DX11RenderTarget*)rtv;
				auto db = (DX11DepthBuffer*)dbv;

				ID3D11RenderTargetView* r = (rt) ? rt->GetRenderTargetView(renderTargetIndex) : nullptr;
				ID3D11DepthStencilView* d = (db) ? db->GetDepthStencilView(depthBufferIndex) : nullptr;

				D3D11_VIEWPORT vp;

				if (rt) rt->GetViewportStruct(vp);

				m_context->RSSetViewports(1, &vp);

				m_context->OMSetRenderTargets(1, &r, d);
			}

			void ClearRenderTarget(ResourceHandle rtv, const Colour& c) override
			{
				auto r = (DX11RenderTarget*)rtv;
				int count = (r->isCubeMap()) ? 6 : 1;

				for (int i = 0; i < count; i++)
					m_context->ClearRenderTargetView(r->GetRenderTargetView(i), c);
			}

			void ResizeRenderTarget(ResourceHandle rtv, uint32 w, uint32 h) const override
			{
				auto r = (DX11RenderTarget*)rtv;
				r->ResizeView(w, h);

				//throw exception("not implemented");
			}

			void ResizeDepthBuffer(ResourceHandle dbv, uint32 w, uint32 h) const override
			{
				auto d = (DX11DepthBuffer*)dbv;
				d->ResizeView(w, h);

				//throw exception("not implemented");
			}

			void SetShader(ResourceHandle h, ShaderType type) override
			{
				if (type == ShaderType::SHADER_TYPE_VERTEX)
				{
					m_context->VSSetShader((ID3D11VertexShader*)h, 0, 0);
				}
				else if (type == ShaderType::SHADER_TYPE_PIXEL)
				{
					m_context->PSSetShader((ID3D11PixelShader*)h, 0, 0);
				}
				else if (type == ShaderType::SHADER_TYPE_GEOMETRY)
				{
					m_context->GSSetShader((ID3D11GeometryShader*)h, 0, 0);
				}
			}

			void SetShaderInputDescriptor(ResourceHandle h) override
			{
				auto inputlayout = (ID3D11InputLayout*)h;
				m_context->IASetInputLayout(inputlayout);
			}

			void SetShaderResource(uint32 index, ResourceHandle h) override
			{
				auto res = (ID3D11ShaderResourceView*)h;
				m_context->PSSetShaderResources(index, 1, &res);
				m_context->GSGetShaderResources(index, 1, &res);
				m_context->VSSetShaderResources(index, 1, &res);
			}

			void SetVertexTopology(VertexTopology vt)
			{
				D3D11_PRIMITIVE_TOPOLOGY t;

				switch (vt)
				{
				case (VertexTopology::TopologyTriangleList) : { t = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break; }
				case (VertexTopology::TopologyTriangleStrip) : { t = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break; }
				case (VertexTopology::TopologyLineList) : { t = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break; }
				case (VertexTopology::TopologyLineStrip) : { t = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break; }
				case (VertexTopology::TopologyPointList) : { t = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break; }
				default: { t = D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED; }
				}

				m_context->IASetPrimitiveTopology(t);
			}

			void SetIndexBuffer(ResourceHandle ibuffer, uint32 offset) override
			{
				auto buf = (ID3D11Buffer*)ibuffer;
				m_context->IASetIndexBuffer(buf, dxgi_format_helper<Index>::value, offset);
			}

			void SetVertexBuffer(uint32 index, ResourceHandle vbuffer, uint32 stride, uint32 offset) override
			{
				auto buf = (ID3D11Buffer*)vbuffer;
				m_context->IASetVertexBuffers(index, 1, &buf, &stride, &offset);
			}

			void SetShaderBuffer(uint32 index, ResourceHandle ebuffer) override
			{
				auto buf = (ID3D11Buffer*)ebuffer;
				m_context->VSSetConstantBuffers(index, 1, &buf);
				m_context->PSSetConstantBuffers(index, 1, &buf);
				m_context->GSSetConstantBuffers(index, 1, &buf);
				//m_context->HSSetConstantBuffers(index, 1, &buf);
				//m_context->DSSetConstantBuffers(index, 1, &buf);
			}

			void Draw(uint32 voffset, uint32 vcount) override;
			void DrawIndexed(uint32 ioffset, uint32 icount, uint32 voffset) override;

			void EnableAlphaBlending(bool b) override;

			////////////////////////////////////////////////////////////////////////////////////////////////////////
		};

		class DX11Render : public ABI::IRenderApi
		{
		public:

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			DX11Render(const Graphics::Configuration& settings)
			{
				HWND hwnd = reinterpret_cast<HWND>(settings.windowId);

				C3E_ASSERT(IsWindow(hwnd));

				m_hwnd = hwnd;

				HRESULT hr = S_OK;

				//////////////////////////////////////////////////////////////////////////////////////////////
				//DXGI
				//////////////////////////////////////////////////////////////////////////////////////////////

				ComPtr<IDXGIFactory1> DXGIfactory;
				ComPtr<IDXGIAdapter> DXGIadapter;

				hr = CreateDXGIFactory1(IID_OF(IDXGIFactory1), (void**)DXGIfactory.GetAddressOf());

				if (FAILED(hr))
					throw GraphicsException("DXGI factory: IDXGIFactory1::CreateDXGIFactory1()");

				hr = DXGIfactory->EnumAdapters(0, DXGIadapter.GetAddressOf());

				if (hr == DXGI_ERROR_NOT_FOUND)
					throw GraphicsException("Failed to obtain adapter: IDXGIFactory1::EnumAdapters()");

				DXGIadapter->GetDesc(&m_dxgiDeviceDescription);
				wcscpy_s(m_deviceDescription.description, 128, m_dxgiDeviceDescription.Description);
				m_deviceDescription.SharedSystemMemory = m_dxgiDeviceDescription.SharedSystemMemory;
				m_deviceDescription.VideoMemory = m_dxgiDeviceDescription.DedicatedVideoMemory;
				m_deviceDescription.SystemMemory = m_dxgiDeviceDescription.DedicatedSystemMemory;

				DXGIfactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);

				//////////////////////////////////////////////////////////////////////////////////////////////
				//initialize direct3D
				//////////////////////////////////////////////////////////////////////////////////////////////

				//Create swap chain
				DXGI_SWAP_CHAIN_DESC scd;
				ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

				//Swap chain description
				scd.BufferCount = 1;                                    // one back buffer
				scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				//scd.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
				scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
				scd.OutputWindow = m_hwnd;								// HWND of host window

				scd.SampleDesc.Count = settings.AAcount;
				scd.SampleDesc.Quality = settings.AAquality;
				//scd.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
				scd.Windowed = !settings.viewport.fullscreen;

				scd.BufferDesc.Width = settings.viewport.width;
				scd.BufferDesc.Height = settings.viewport.height;

				scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				//scd.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;

				scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				//VSync
				if (settings.vsync)
				{
					scd.BufferDesc.RefreshRate.Numerator = settings.refreshrate;
					scd.BufferDesc.RefreshRate.Denominator = 1;
				}

				m_config = settings;

				//////////////////////////////////////////////////////////////////////////////////////////////

				UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; //for D2D and D3D interop
				flags = 0;

				D3D_FEATURE_LEVEL featureLevels[] =
				{
					D3D_FEATURE_LEVEL_11_1,
					D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1,
					D3D_FEATURE_LEVEL_10_0,
				};

				if (settings.debug)
				{
					flags |= D3D11_CREATE_DEVICE_DEBUG;
				}

				try
				{
					hr = D3D11CreateDeviceAndSwapChain(
						DXGIadapter.Get(),
						D3D_DRIVER_TYPE_UNKNOWN,//D3D_DRIVER_TYPE_HARDWARE,
						NULL,
						flags,
						featureLevels,
						ARRAYSIZE(featureLevels),
						D3D11_SDK_VERSION,
						&scd,
						m_dxgiSwapchain.GetAddressOf(),
						m_device.GetAddressOf(),
						&m_feature_level,
						m_immediatecontext.GetAddressOf()
					);

					if (FAILED(hr))
					{
						throw _com_error(hr);
					}

				}
				catch (_com_error& e)
				{
					cout << hex << "D3D11CreateDeviceAndSwapChain failure. HRESULT (0x" << hr << "): "
						<< (const char*)e.ErrorMessage() << endl;

					throw GraphicsException("Failed to create device and swapchain:\nD3D11CreateDeviceAndSwapChain()");
				}

				m_renderTargetDefault = DX11RenderTarget(m_dxgiSwapchain.Get());
				m_depthBufferDefault = DX11DepthBuffer(m_device.Get(), settings.viewport.width, settings.viewport.height, DXGI_FORMAT_UNKNOWN, DXGI_SAMPLE_DESC{ settings.AAcount, settings.AAquality }, false, 0);

				SetDebugObjectName(m_immediatecontext.Get(), "D3D11:ImmediateContext");

				/*
				D3D11_RASTERIZER_DESC rd;
				ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
				rd.FillMode = D3D11_FILL_SOLID;
				rd.CullMode = D3D11_CULL_NONE;
				rd.AntialiasedLineEnable = true;
				rd.MultisampleEnable = true;

				m_device->CreateRasterizerState(&rd, m_rasterstate.GetAddressOf());
				SetDebugObjectName(m_rasterstate.Get(), "D3D11:RasterizerState-CullNone");
				*/

				D3D11_SAMPLER_DESC sd;
				ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));

				if (m_config.anisotropicFiltering == Graphics::AnisoptropyNone)
				{
					sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					sd.MaxAnisotropy = 1;
					sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
				}
				else
				{
					sd.Filter = D3D11_FILTER_ANISOTROPIC;
					sd.MaxAnisotropy = (uint32)m_config.anisotropicFiltering;
					sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
				}

				sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

				sd.MinLOD = -FLT_MAX;
				sd.MaxLOD = FLT_MAX;
				sd.MipLODBias = 0.0f;
				memset(sd.BorderColor, 1, sizeof(float[4]));

				m_device->CreateSamplerState(&sd, m_samplerStateWrap.GetAddressOf());

				sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

				m_device->CreateSamplerState(&sd, m_samplerStateClamp.GetAddressOf());

				//Depth stencil state
				D3D11_DEPTH_STENCIL_DESC dsd;
				ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));

				dsd.DepthEnable = true;
				dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				dsd.DepthFunc = D3D11_COMPARISON_LESS;
				dsd.StencilEnable = true;
				dsd.StencilReadMask = 0xFF;
				dsd.StencilWriteMask = 0xFF;

				//Front facing operations
				dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
				dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

				//Back facing operations
				dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
				dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

				//Create the Depth/Stencil Views
				m_device->CreateDepthStencilState(&dsd, m_depthState.GetAddressOf());

				//Blend state
				D3D11_BLEND_DESC bd;
				ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));

				if (m_config.AAcount > 1) bd.AlphaToCoverageEnable = true;
				bd.IndependentBlendEnable = true;

				bd.RenderTarget[0].BlendEnable = true;
				bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
				bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
				bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

				m_device->CreateBlendState(&bd, m_blendState.GetAddressOf());
			}

			~DX11Render() { SetFullscreen(false); }

			////////////////////////////////////////////////////////////////////////////////////////////////////////
			//Resource methods
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			ResourceHandle CreateShader(
				const byte* bytecode,
				size_t bytecodesize,
				ShaderType type
				) override
			{
				if (type == ShaderType::SHADER_TYPE_VERTEX)
				{
					ID3D11VertexShader* vshader = nullptr;
					C3E_ASSERT(SUCCEEDED(m_device->CreateVertexShader(bytecode, bytecodesize, 0, &vshader)));
					SetDebugObjectName(vshader, "D3D11:VertexShader");
					return (ResourceHandle)vshader;
				}
				else if (type == ShaderType::SHADER_TYPE_PIXEL)
				{
					ID3D11PixelShader* pshader = nullptr;
					C3E_ASSERT(SUCCEEDED(m_device->CreatePixelShader(bytecode, bytecodesize, 0, &pshader)));
					SetDebugObjectName(pshader, "D3D11:PixelShader");
					return (ResourceHandle)pshader;
				}
				else if (type == ShaderType::SHADER_TYPE_GEOMETRY)
				{
					ID3D11GeometryShader* gshader = nullptr;
					C3E_ASSERT(SUCCEEDED(m_device->CreateGeometryShader(bytecode, bytecodesize, 0, &gshader)));
					SetDebugObjectName(gshader, "D3D11:GeometryShader");
					return (ResourceHandle)gshader;
				}
				else
				{
					return (ResourceHandle)0;
				}
			}

			bool DestroyShader(ResourceHandle h) override
			{
				auto s = static_cast<ID3D11DeviceChild*>(h);

				if (s)
				{
					s->Release();
					return true;
				}

				return false;
			}

			ResourceHandle CreateShaderInputDescriptor(
				const byte* bytecode,
				size_t bytecodesize,
				const ShaderInputDescriptor* sids,
				uint32 sid_num
				) override
			{
				ID3D11InputLayout* inputlayout = nullptr;

				vector<D3D11_INPUT_ELEMENT_DESC> desc(sid_num);
				desc.resize(sid_num);

				for (uint32 i = 0; i < sid_num; i++)
				{
					desc[i].AlignedByteOffset = sids[i].byteOffset;
					desc[i].InputSlot = sids[i].slot;
					desc[i].SemanticName = sids[i].semanticName;

					desc[i].SemanticIndex = 0;
					desc[i].InstanceDataStepRate = 0;
					desc[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;

					switch (sids[i].vectorComponents)
					{
					case 1: { desc[i].Format = DXGI_FORMAT_R32_FLOAT; break; }
					case 2: { desc[i].Format = DXGI_FORMAT_R32G32_FLOAT; break; }
					case 3: { desc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break; }
					case 4: { desc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
					default: { desc[i].Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN; }
					}
				}

				C3E_ASSERT(SUCCEEDED(m_device->CreateInputLayout(&desc[0], sid_num, bytecode, bytecodesize, &inputlayout)));
				SetDebugObjectName(inputlayout, "D3D11:InputLayout");

				/*
				//float4 vector offset - 16bytes
				#define VECTOR_OFFSET(x) (sizeof(Vector) * x)


				D3D11_INPUT_ELEMENT_DESC layout[] =
				{
				//float3 - DXGI_FORMAT_R32G32B32_FLOAT
				//float4 - DXGI_FORMAT_R32G32B32A32_FLOAT

				//1st Vertex element
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, VECTOR_OFFSET((uint32)VertexAttributeIndex::Position), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
				//4th Vertex element
				{ "COLOUR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, VECTOR_OFFSET((uint32)VertexAttributeIndex::Colour), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
				};

				C3E_ASSERT(SUCCEEDED(m_device->CreateInputLayout(layout, ARRAYSIZE(layout), bytecode, bytecodesize, m_inputlayout.GetAddressOf())));
				SetDebugObjectName(m_inputlayout.Get(), "D3D11:InputLayout");
				*/


				return (ResourceHandle)inputlayout;
			}

			bool DestroyShaderInputDescriptor(ResourceHandle h) override
			{
				auto inputlayout = static_cast<ID3D11InputLayout*>(h);

				if (inputlayout)
				{
					inputlayout->Release();
					return true;
				}

				return false;
			}

			ResourceHandle CreateBuffer(
				const void* mem,
				uint32 memsize,
				EResourceType rsctype,
				EResourceUsage rscusage) override
			{
				ID3D11Buffer* buffer = nullptr;

				D3D11_SUBRESOURCE_DATA data;
				D3D11_BUFFER_DESC desc;

				ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
				ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

				string debugname;

				switch ((EResourceType)rsctype)
				{
					case EResourceType::TypeVertexBuffer: { desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; debugname = "D3D11:VertexBuffer"; break; }
					case EResourceType::TypeIndexBuffer: { desc.BindFlags = D3D11_BIND_INDEX_BUFFER; debugname = "D3D11:IndexBuffer"; break; }
					case EResourceType::TypeShaderBuffer: { desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; debugname = "D3D11:ShaderBuffer"; break; }
				}

				switch ((EResourceUsage)rscusage)
				{
					case EResourceUsage::UsageDynamic: { desc.Usage = D3D11_USAGE_DYNAMIC; debugname += "-dynamic"; break; }
					case EResourceUsage::UsageStaging: { desc.Usage = D3D11_USAGE_STAGING; debugname += "-staging"; break; }
					case EResourceUsage::UsageStatic: { desc.Usage = D3D11_USAGE_IMMUTABLE; debugname += "-immutable"; break; }
					default: { desc.Usage = D3D11_USAGE_DEFAULT;  debugname += "-default"; }
				}

				//Only dynamic resources are allowed direct access to buffer memory
				if (desc.Usage == D3D11_USAGE_DYNAMIC)
				{
					desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				}

				desc.MiscFlags = 0;
				desc.ByteWidth = memsize;

				data.pSysMem = mem;
				data.SysMemPitch = 0;
				data.SysMemSlicePitch = 0;

				if (FAILED(m_device->CreateBuffer(&desc, &data, &buffer)))
					return 0;

				SetDebugObjectName(buffer, debugname.c_str());

				return (ResourceHandle)buffer;
			}

			bool DestroyBuffer(ResourceHandle h) override
			{
				auto buf = (ID3D11Buffer*)h;

				if (!buf)
					return false;

				buf->Release();

				return true;
			}

			ResourceHandle CreateShaderResource(
				const ShaderResourceData* data,
				const ShaderResourceDescriptor& desc
				) override
			{
				DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
				D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
				D3D11_CPU_ACCESS_FLAG access = D3D11_CPU_ACCESS_READ;

				format = TextureFormatToDXGIFormat(desc.texformat);

				switch ((EResourceUsage)desc.rscusage)
				{
					case EResourceUsage::UsageDefault: { usage = D3D11_USAGE_DEFAULT; break; }
					case EResourceUsage::UsageDynamic: { usage = D3D11_USAGE_DYNAMIC; break; }
					case EResourceUsage::UsageStaging: { usage = D3D11_USAGE_STAGING; break; }
					case EResourceUsage::UsageStatic: { usage = D3D11_USAGE_IMMUTABLE; break; }
				}

				//Force default usage
				if (desc.useMips)
				{
					usage = D3D11_USAGE_DEFAULT;
				}

				if (desc.arraySize > 1)
				{
					usage = D3D11_USAGE_DEFAULT;
				}

				if (usage == D3D11_USAGE_DEFAULT)
				{
					access = D3D11_CPU_ACCESS_FLAG(0);
				}
				else if (usage == D3D11_USAGE_DYNAMIC)
				{
					access = D3D11_CPU_ACCESS_WRITE;
				}
				else
				{
					access = D3D11_CPU_ACCESS_READ;
				}

				uint32 miplevels = (desc.useMips) ? GetNumMipLevels(desc.width, desc.height) : 1;
				uint32 miscFlags = (desc.isCubemap) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

				if (!data) return 0;

				vector<D3D11_SUBRESOURCE_DATA> subresources(desc.arraySize);

				for (uint32 i = 0; i < desc.arraySize; i++)
				{
					subresources[i].pSysMem = data[i].mem;
					subresources[i].SysMemPitch = data[i].memPitch;
					subresources[i].SysMemSlicePitch = data[i].memSlicePitch;
				}

				D3D11_SUBRESOURCE_DATA* pSubresource = (desc.useMips) ? nullptr : &subresources[0];

				uint32 fmtSupport;
				m_device->CheckFormatSupport(format, &fmtSupport);
				C3E_ASSERT(fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN);

				HRESULT hr = S_OK;

				//Create texture resource view
				D3D11_SHADER_RESOURCE_VIEW_DESC sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.Format = format;
				sd.Texture1D.MipLevels = 1;

				ComPtr<ID3D11Resource> resource;

				//Create texture resource
				switch (desc.textype)
				{
				case (ETextureType::TypeTexture1D) :
				{
					D3D11_TEXTURE1D_DESC dtd;
					ZeroMemory(&dtd, sizeof(dtd));

					dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
					dtd.Width = desc.width;
					dtd.ArraySize = desc.arraySize;
					dtd.MipLevels = miplevels;
					dtd.Format = format;
					dtd.MiscFlags = miscFlags;
					dtd.Usage = usage;
					dtd.CPUAccessFlags = access;

					if (desc.useMips)
					{
						dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
						dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
						sd.Texture1D.MipLevels = GetNumMipLevels(desc.width, desc.height);
					}

					sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;

					hr = m_device->CreateTexture1D(&dtd, pSubresource, (ID3D11Texture1D**)resource.GetAddressOf());

					break;
				}
				case (ETextureType::TypeTexture2D) :
				{
					D3D11_TEXTURE2D_DESC dtd;
					ZeroMemory(&dtd, sizeof(dtd));

					dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
					dtd.Width = desc.width;
					dtd.Height = desc.height;
					dtd.Format = format;
					dtd.MiscFlags = miscFlags;
					dtd.ArraySize = desc.arraySize;
					dtd.Usage = usage;
					dtd.CPUAccessFlags = access;
					dtd.MipLevels = miplevels;
					dtd.SampleDesc.Count = 1;
					dtd.SampleDesc.Quality = 0;

					if (desc.useMips)
					{
						dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
						dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
						sd.Texture2D.MipLevels = GetNumMipLevels(desc.width, desc.height);
					}

					if (desc.isCubemap)
					{
						sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					}
					else
					{
						sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					}

					hr = m_device->CreateTexture2D(&dtd, pSubresource, (ID3D11Texture2D**)resource.GetAddressOf());

					break;
				}
				case (ETextureType::TypeTexture3D) :
				{
					D3D11_TEXTURE3D_DESC dtd;
					ZeroMemory(&dtd, sizeof(dtd));

					dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
					dtd.Width = desc.width;
					dtd.Height = desc.height;
					dtd.Depth = desc.depth;
					dtd.Format = format;
					dtd.MipLevels = miplevels;
					dtd.MiscFlags = miscFlags;
					dtd.Usage = usage;
					dtd.CPUAccessFlags = access;

					if (desc.useMips)
					{
						dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
						dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
						sd.Texture3D.MipLevels = GetNumMipLevels(desc.width, desc.height);
					}

					sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;

					hr = m_device->CreateTexture3D(&dtd, pSubresource, (ID3D11Texture3D**)resource.GetAddressOf());

					break;
				}
				}

				if (FAILED(hr))
				{
					return 0;
				}

				ID3D11ShaderResourceView* resourceView = nullptr;

				hr = m_device->CreateShaderResourceView(resource.Get(), &sd, &resourceView);

				if (desc.useMips)
				{
					lock_guard<mutex>lk(m_rtv_lock);

					for (int i = 0; (size_t)i < subresources.size(); i++)
					{
						m_immediatecontext->UpdateSubresource(resource.Get(), 0, nullptr, subresources[i].pSysMem, subresources[i].SysMemPitch, subresources[i].SysMemSlicePitch);
					}

					m_immediatecontext->GenerateMips(resourceView);
				}

				if (FAILED(hr)) return 0;

				return (ResourceHandle)resourceView;
			}

			bool DestroyShaderResource(ResourceHandle h) override
			{
				auto rsc = (ID3D11ShaderResourceView*)h;

				if (!rsc)
					return false;

				rsc->Release();

				return true;
			}

			void SetResourceString(ResourceHandle h, const char* debugstring) override
			{
				if (auto res = static_cast<ID3D11DeviceChild*>(h))
				{
					SetDebugObjectName(res, debugstring);
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////
			//Pipeline methods
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			void InitPipeline(ID3D11DeviceContext* context)
			{
				//m_renderTargetDefault.BindTarget(context, 0);

				context->VSSetSamplers(0, 1, m_samplerStateWrap.GetAddressOf());
				context->PSSetSamplers(0, 1, m_samplerStateWrap.GetAddressOf());

				context->VSSetSamplers(1, 1, m_samplerStateClamp.GetAddressOf());
				context->PSSetSamplers(1, 1, m_samplerStateClamp.GetAddressOf());

				context->OMSetDepthStencilState(m_depthState.Get(), 0);

				float factor[] = { 1, 1, 1, 1 };
				//context->OMSetBlendState(m_blendState.Get(), factor, 0xffffffff);
			}

			void DrawBegin(const Colour& c) override
			{
				lock_guard<mutex>lk(m_rtv_lock);

				for (auto& rt : m_rendertargets)
				{
					int count = (rt->isCubeMap()) ? 6 : 1;

					for (int i = 0; i < count; i++)
						m_immediatecontext->ClearRenderTargetView(rt->GetRenderTargetView(i), c);
				}

				for (auto& dp : m_depthbuffers)
				{
					int count = (dp->isCubeMap()) ? 6 : 1;

					for (int i = 0; i < count; i++)
						m_immediatecontext->ClearDepthStencilView(dp->GetDepthStencilView(i), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
				}

				InitPipeline(m_immediatecontext.Get());

				m_immediatecontext->ClearRenderTargetView(m_renderTargetDefault.GetRenderTargetView(0), c);
				m_immediatecontext->ClearDepthStencilView(m_depthBufferDefault.GetDepthStencilView(0), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

				//m_immediatecontext->ClearRenderTargetView(m_rendertarget.Get(), c);
				//m_immediatecontext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

				m_drawCallCount = 0;
			}

			void DrawEnd() override
			{
				m_dxgiSwapchain->Present(0, 0);

				if (m_cacheResModified.load())
				{
					lock_guard<mutex>lk(m_cacheRes_lock);
					Resize(m_cacheResW, m_cacheResH);
					m_cacheResModified = false;
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			ABI::IRenderContext* CreateContext() override
			{
				ComPtr<ID3D11DeviceContext> deferredcontext;

				unique_ptr<DX11RenderContext> context(new DX11RenderContext(this));

				lock_guard<mutex>lk(m_rendercontext_lock);
				m_rendercontexts.push_back(move(context));

				return m_rendercontexts.back().get();
			}

			void DestroyContext(ABI::IRenderContext* context) override
			{
				auto rc = static_cast<DX11RenderContext*>(context);
				C3E_ASSERT(rc);

				lock_guard<mutex>lk(m_rendercontext_lock);

				auto& rcs = m_rendercontexts;

				auto i = find_if(
					rcs.begin(),
					rcs.end(),
					[&](const unique_ptr<DX11RenderContext>& ptr){
					return (ptr.get() == rc);
				});

				if (i != rcs.end())
				{
					rcs.erase(i);
				}
			}

			void DispatchContext(ABI::IRenderContext* context) override
			{
				auto rc = static_cast<DX11RenderContext*>(context);
				C3E_ASSERT(rc);

				rc->ExecuteCommandlist(m_immediatecontext.Get());
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			ResourceHandle CreateRenderTarget(const RenderTargetDescriptor& desc) override
			{
				lock_guard<mutex>lk(m_rtv_lock);

				DXGI_SAMPLE_DESC sampling;
				sampling.Count = desc.sampling.count;
				sampling.Quality = desc.sampling.quality;

				int flags = 0;

				if (desc.useMips) flags |= EDX11ViewFlags::MIP_AUTOGEN;

				auto ptr = new DX11RenderTarget(m_device.Get(), desc.width, desc.height, TextureFormatToDXGIFormat(desc.texformat), sampling, desc.isCubemap, desc.useMips);
				m_rendertargets.push_back(unique_ptr<DX11RenderTarget>(ptr));

				return ptr;
			}

			ResourceHandle CreateDepthBuffer(const DepthBufferDescriptor& desc) override
			{
				lock_guard<mutex>lk(m_rtv_lock);

				DXGI_SAMPLE_DESC sampling;
				sampling.Count = desc.sampling.count;
				sampling.Quality = desc.sampling.quality;

				int flags = 0;

				auto ptr = new DX11DepthBuffer(m_device.Get(), desc.width, desc.height, DXGI_FORMAT_UNKNOWN, sampling, desc.isCubemap, flags);
				m_depthbuffers.push_back(unique_ptr<DX11DepthBuffer>(ptr));

				return ptr;
			}

			bool DestroyDepthBuffer(ResourceHandle h) override
			{
				lock_guard<mutex>lk(m_rtv_lock);

				auto rt = (DX11DepthBuffer*)h;

				auto i = find_if(m_depthbuffers.begin(), m_depthbuffers.end(), [&](const unique_ptr<DX11DepthBuffer>& r){ return (r.get() == rt); });

				if (i != m_depthbuffers.end())
				{
					m_depthbuffers.erase(i);
					return true;
				}

				return false;
			}

			bool DestroyRenderTarget(ResourceHandle h) override
			{
				lock_guard<mutex>lk(m_rtv_lock);

				auto rt = (DX11RenderTarget*)h;

				auto i = find_if(m_rendertargets.begin(), m_rendertargets.end(), [&](const unique_ptr<DX11RenderTarget>& r){ return (r.get() == rt); });

				if (i != m_rendertargets.end())
				{
					m_rendertargets.erase(i);
					return true;
				}

				return false;
			}

			ResourceHandle GetDefaultRenderTarget() const override
			{
				return (ResourceHandle)&m_renderTargetDefault;
			}

			ResourceHandle GetDefaultDepthBuffer() const override
			{
				return (ResourceHandle)&m_depthBufferDefault;
			}

			ResourceHandle GetViewTexture(ResourceHandle h) const override
			{
				auto v = (DX11View*)h;

				if (v)
				{
					return (ResourceHandle)v->GetTextureView(0);
				}

				return 0;
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			void SetViewport(const Graphics::Viewport& vp) override
			{
				if ((vp.height != (uint32)m_config.viewport.height) || (vp.width != (uint32)m_config.viewport.width))
				{
					lock_guard<mutex>lk(m_cacheRes_lock);

					m_cacheResH = vp.height;
					m_cacheResW = vp.width;
					m_cacheResModified = true;
					//Resize(vp.hRes, vp.vRes);

					m_renderTargetDefault.ResizeView(vp.width, vp.height);
					m_depthBufferDefault.ResizeView(vp.width, vp.height);
				}

				if (vp.fullscreen != m_config.viewport.fullscreen)
				{
					SetFullscreen(vp.fullscreen);
				}
			}

			void GetViewport(Graphics::Viewport& vp) override
			{
				vp = m_config.viewport;
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			void QueryDebugStatistics(Graphics::DebugInfo& info) override
			{
				info.drawcalls = m_drawCallCount.load();
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////

			void SetFullscreen(bool on)
			{
				if (m_dxgiSwapchain.Get() && on != m_config.viewport.fullscreen)
				{
					m_dxgiSwapchain->SetFullscreenState(on, 0);
					m_config.viewport.fullscreen = on;
				}
			}

			void Resize(uint32 w, uint32 h)
			{
				C3E_ASSERT(m_renderTargetDefault.ResizeView(w, h));

				//Resize view port
				m_config.viewport.height = h;
				m_config.viewport.width = w;

				InitPipeline(m_immediatecontext.Get());
			}

			ComPtr<ID3D11Device> GetDevice() const { return m_device; }
			ComPtr<ID3D11DeviceContext> GetImmediateContext() const { return m_immediatecontext; }
			size_t GetNumRenderContexts() const { return m_rendercontexts.size(); }

			void IncrementDrawCallCounter() { m_drawCallCount++; }

			ComPtr<ID3D11SamplerState> m_samplerStateWrap;
			ComPtr<ID3D11SamplerState> m_samplerStateClamp;
			ComPtr<ID3D11BlendState> m_blendState;
			ComPtr<ID3D11DepthStencilState> m_depthState;

		private:

			////////////////////////////////////////////////////////////////////////////////////////////////////////
			//D3D11 Resources
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			//Devices
			ComPtr<ID3D11Device> m_device;							// Device interface
			ComPtr<ID3D11DeviceContext> m_immediatecontext;			// Device context

			//Primary render target
			DX11RenderTarget m_renderTargetDefault;
			DX11DepthBuffer m_depthBufferDefault;

			//DXGI
			ComPtr<IDXGISwapChain> m_dxgiSwapchain;		// Swap chain interface

			DXGI_ADAPTER_DESC m_dxgiDeviceDescription;
			Graphics::DeviceInfo m_deviceDescription;
			Graphics::ApiInfo m_apiDescription;
			Graphics::Configuration m_config;

			atomic<bool> m_cacheResModified = false;
			int m_cacheResH = 0;
			int m_cacheResW = 0;
			mutex m_cacheRes_lock;

			D3D_FEATURE_LEVEL m_feature_level;
			HWND m_hwnd = nullptr;

			vector<unique_ptr<DX11RenderContext>> m_rendercontexts;
			vector<unique_ptr<DX11RenderTarget>> m_rendertargets;
			vector<unique_ptr<DX11DepthBuffer>> m_depthbuffers;
			mutex m_rendercontext_lock;
			mutex m_rtv_lock;

			atomic<int> m_drawCallCount;

			////////////////////////////////////////////////////////////////////////////////////////////////////////
		};

		////////////////////////////////////////////////////////////////////////////////////////////////////////

		DX11RenderContext::DX11RenderContext(DX11Render* renderer) :
			m_renderer(renderer)
		{
			C3E_ASSERT(SUCCEEDED(m_renderer->GetDevice()->CreateDeferredContext(0, m_context.GetAddressOf())));

			stringstream dbg;
			dbg << "D3D11:DeferredContext-";
			dbg << m_renderer->GetNumRenderContexts();
			SetDebugObjectName(m_context.Get(), dbg.str().c_str());
		}

		void DX11RenderContext::ContextBegin()
		{
			m_context->ClearState();

			m_renderer->InitPipeline(m_context.Get());

			//m_context->IASetInputLayout(m_renderer->m_inputlayout.Get());
			//m_context->RSSetState(m_renderer->m_rasterstate.Get());
		}

		void DX11RenderContext::ContextEnd()
		{
			m_context_commandlist.Reset();
			m_context->FinishCommandList(false, m_context_commandlist.GetAddressOf());
		}

		void DX11RenderContext::Draw(uint32 voffset, uint32 vcount)
		{
			m_context->Draw(vcount, voffset);
			m_renderer->IncrementDrawCallCounter();
		}

		void DX11RenderContext::DrawIndexed(uint32 ioffset, uint32 icount, uint32 voffset)
		{
			m_context->DrawIndexed(icount, ioffset, voffset);
			m_renderer->IncrementDrawCallCounter();
		}

		void DX11RenderContext::EnableAlphaBlending(bool b)
		{
			float factor[] = { 1, 1, 1, 1 };
			m_context->OMSetBlendState((b) ? m_renderer->m_blendState.Get() : nullptr, factor, 0xffffffff);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace C3E
{
	namespace ABI
	{
		extern "C"
		{
			IRenderApi* CreateRenderingInterface(const Graphics::Configuration& cfg)
			{
				return (IRenderApi*)(new DX11::DX11Render(cfg));
			}

			void DestroyRenderingInterface(ABI::IRenderApi* api)
			{
				auto f = dynamic_cast<DX11::DX11Render*>(api);
				if (f)
					delete f;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////