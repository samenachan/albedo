﻿#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Source\Sample3DSceneRenderer.h"

//画面に Direct3D の内容をレンダリングします。
namespace Arabesque
{
	class Main
	{
	public:
		Main();
		void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Update();
		bool Render();

		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

	private:
		// TODO: これを独自のコンテンツ レンダラーで置き換えます。
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

		// ループ タイマーをレンダリングしています。
		DX::StepTimer m_timer;
	};
}