#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define MAIN_LOG(log_msg) std::cout << "[Main]" << log_msg << std::endl;
// #undef _DEBUG

#include "camera_manager.hpp"
#include "DirectX/directx_manager.hpp"
#include "entity.hpp"
#include "gui_manager.hpp"
#include "window_manager.hpp"
#include "world.hpp"
#include <External/imgui/imgui.h>
#include <External/imgui/imgui_impl_dx12.h>
#include <External/imgui/imgui_impl_glfw.h>
#include <cassert>
#include <dxgi.h>
#include <iostream>
#include <vector>

int main() {
	MAIN_LOG("============================= Begin Program =============================");

	std::unique_ptr<albedo::WindowManager> window_manager = std::make_unique<albedo::WindowManager>();
	MAIN_LOG("Window Manager");

	std::unique_ptr<albedo::CameraManager> camera_manager = std::make_unique<albedo::CameraManager>();
	MAIN_LOG("Camera Manager");

	std::unique_ptr<albedo::World> world = std::make_unique<albedo::World>();
	MAIN_LOG("Prepared World Datum");

	std::unique_ptr<albedo::DirectXManager> directx_manager = std::make_unique<albedo::DirectXManager>();
	MAIN_LOG("DirectX12 Manager");

	for (std::shared_ptr<albedo::Entity> entity : world->get_all_entities()) {
		entity->init_directx_contexts(directx_manager->device.Get(), directx_manager->descriptor_heap_cbv_srv.Get());
		entity->init_directx_shadow_buffer(directx_manager->shadow_render->resource_shadow.Get());
	}

	std::unique_ptr<albedo::GUIManager> gui_manager = std::make_unique<albedo::GUIManager>();
	MAIN_LOG("GUI Manager");

	MAIN_LOG("Initialized Entity to Renderer");

	while (window_manager->should_update()) {
		gui_manager->update();
		window_manager->update();
		camera_manager->update();
		world->update();

		gui_manager->render();
		directx_manager->render();
	}

	gui_manager->shutdown();
	window_manager->shutdown();

	MAIN_LOG("============================= End Program =============================");

	return 0;
}