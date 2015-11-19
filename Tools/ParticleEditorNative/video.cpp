
#include <particles/instances.h>
#include <particles/render.h>
#include <graphics/device.h>
#include <temple/aasrenderer.h>
#include <atlcomcli.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi")

using namespace particles;

#include "api.h"
#include "video.h"

static float GetTotalLifetime(const PartSysPtr& sys, bool& permanent) {
	auto result = 0.0f;

	for (const auto& emitter : *sys) {
		auto spec = emitter->GetSpec();

		auto lifetime = spec->GetDelay();

		if (emitter->GetSpec()->IsPermanent()) {
			auto maxParticlesReachedIn = spec->GetMaxParticles() / (float)spec->GetParticleRate();
			lifetime += maxParticlesReachedIn + spec->GetParticleLifespan();
			permanent = true;
		} else {
			lifetime += spec->GetLifespan() + spec->GetParticleLifespan();
		}

		if (lifetime > result) {
			result = lifetime;
		}
	}

	return result;
}

bool ParticleSystem_RenderVideo(TempleDll *dll, D3DCOLOR background, const wchar_t* outputFile, int fps) {

	auto orgSys = dll->partSys.get();
	auto d3dDevice = dll->renderingDevice.GetDevice();

	// The assumption is that the screen BB of the part sys encompasses the entire system
	// so we use that to render it to a video file
	auto screenBounds = orgSys->GetScreenBounds();

	auto w = (int)abs(screenBounds.right - screenBounds.left) + 10;
	auto h = (int)abs(screenBounds.bottom - screenBounds.top) + 10;
	// Needs to be divisible by 2 for h264
	if (w % 2)
		w++;
	if (h % 2)
		h++;

	const auto scale = 1.0f;
	w *= 1;
	h *= 1;

	auto encoder(VideoEncoder::Create(outputFile));
	encoder->Init(w, h, fps);

	// Create a clone here to not influence the original system
	auto sys = std::make_shared<PartSys>(orgSys->GetSpec());

	// Save the old render target
	CComPtr<IDirect3DSurface9> orgRenderTarget;
	CComPtr<IDirect3DSurface9> orgDepthSurface;
	d3dDevice->GetRenderTarget(0, &orgRenderTarget);
	d3dDevice->GetDepthStencilSurface(&orgDepthSurface);

	// Create a render target
	CComPtr<IDirect3DSurface9> depthSurface;
	CComPtr<IDirect3DSurface9> renderTarget;
	auto result = d3dDevice->CreateRenderTarget(w, h, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &renderTarget, nullptr);
	if (!SUCCEEDED(result)) {
		return false;
	}

	result = d3dDevice->CreateDepthStencilSurface(w, h, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &depthSurface, nullptr);
	if (!SUCCEEDED(result)) {
		return false;
	}

	CComPtr<IDirect3DSurface9> sysMemSurface;
	result = d3dDevice->CreateOffscreenPlainSurface(w, h, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &sysMemSurface, nullptr);
	if (!SUCCEEDED(result)) {
		return false;
	}

	auto timeStepSec = 1.0f / fps;
	auto elapsed = 0.0f;
	bool permanent;
	auto totalTime = GetTotalLifetime(sys, permanent);

	if (permanent) {
		sys->Simulate(5.0f);
	}
	
	d3dDevice->SetRenderTarget(0, renderTarget);
	d3dDevice->SetDepthStencilSurface(depthSurface);

	dll->renderingDevice.GetCamera().SetScreenWidth((float)w, (float)h);
	dll->renderingDevice.GetCamera().CenterOn(0, 0, 0);

	int frameId = 0;

	while (elapsed < totalTime) {
		sys->Simulate(timeStepSec);
		elapsed += timeStepSec;

		d3dDevice->BeginScene();
		result = d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);
		if (!SUCCEEDED(result)) {
			return false;
		}

		for (auto& emitter : *sys) {
			auto& renderer = dll->renderManager.GetRenderer(emitter->GetSpec()->GetParticleType());
			renderer.Render(*emitter);
		}

		d3dDevice->EndScene();
		d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

		// Copy from video mem to system mem
		result = d3dDevice->GetRenderTargetData(renderTarget, sysMemSurface);
		if (!SUCCEEDED(result)) {
			return false;
		}

#ifdef WRITE_FRAME_BMPS
		wchar_t frameFile[MAX_PATH];
		wcsncpy(frameFile, outputFile, wcslen(outputFile));
		PathRemoveFileSpec(frameFile);
		PathAppend(frameFile, fmt::format(L"output{:04}.bmp", frameId++).c_str());

		result = D3DXSaveSurfaceToFile(frameFile, D3DXIFF_BMP, sysMemSurface, nullptr, nullptr);
#endif

		D3DLOCKED_RECT locked;
		result = sysMemSurface->LockRect(&locked, nullptr, 0);
		if (!SUCCEEDED(result)) {
			return false;
		}
		encoder->WriteFrame((uint8_t*) locked.pBits, locked.Pitch);
		sysMemSurface->UnlockRect();


	}

	encoder->Finish();

	// Restore the org render target
	d3dDevice->SetRenderTarget(0, orgRenderTarget);
	d3dDevice->SetDepthStencilSurface(orgDepthSurface);

	return true;

}
