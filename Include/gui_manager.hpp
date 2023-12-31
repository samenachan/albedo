#include "DirectX/directx_constant.hpp"
#include "DirectX/directx_imgui_render.hpp"
#include "DirectX/directx_manager.hpp"
#include "camera_manager.hpp"
#include "entity.hpp"
#include "system_variables.hpp"
#include "window_manager.hpp"
#include "world.hpp"
#include <External/GLFW/glfw3.h>
#include <External/imgui/imgui.h>
#include <External/imgui/imgui_impl_dx12.h>
#include <External/imgui/imgui_impl_glfw.h>
#include <External/imgui/imgui_internal.h>
#include <d3d12.h>
#include <iostream>
#include <string>
#include <vector>

namespace albedo {
	class GUIManager {
	public:
		GUIManager() { construct(); }

		void construct() {
			init_imgui();
			init_imgui_glfw(albedo::WindowManager::window_ptr);
			init_imgui_directX(albedo::DirectXManager::device.Get(), System::num_frames_in_fright,
							   albedo::DirectXImGuiRender::descriptor_heap_imgui.Get());
		}
		void update() {
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			create_dock();
			create_panel_1();
			create_panel_2();
			create_panel_3();
		}
		void render() { ImGui::Render(); }
		void shutdown() {
			ImGui_ImplDX12_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

	private:
		int selected_id = 0;

		void init_imgui() {
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			ImGui::StyleColorsLight();
		}
		void init_imgui_glfw(GLFWwindow* window) { ImGui_ImplGlfw_InitForOther(window, true); }
		void init_imgui_directX(ID3D12Device* device, UINT num_frames, ID3D12DescriptorHeap* heap_srv) {
			ImGui_ImplDX12_Init(device, num_frames, DXGI_FORMAT_R8G8B8A8_UNORM, heap_srv,
								heap_srv->GetCPUDescriptorHandleForHeapStart(),
								heap_srv->GetGPUDescriptorHandleForHeapStart());
		}

		void create_dock() {
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoBackground;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_NoDocking;
			ImGui::Begin("Dock Base Window", nullptr, window_flags);

			ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
			dock_flags |= ImGuiDockNodeFlags_NoWindowMenuButton;
			ImGui::DockSpace(ImGui::GetID("Dock Base"), ImVec2(0, 0), dock_flags);

			ImGui::End();
			ImGui::PopStyleVar();
		}
		void create_panel_1() {
			ImGuiWindowFlags panel_1_flags = ImGuiWindowFlags_NoNav;
			ImGui::Begin("World", nullptr, panel_1_flags);

			ImGui::Text("fps: %.1f", ImGui::GetIO().Framerate);
			ImGui::ColorEdit3("BG", System::bg_color);

			if (ImGui::TreeNode("Camera")) {
				ImGui::DragFloat3("VPos", albedo::CameraManager::camera_position, 0.1f, -30.0f, 30.0f, "%.2f");
				ImGui::DragFloat3("Look", albedo::CameraManager::camera_lookat, 0.1f, -10.0f, 10.0f, "%.2f");
				ImGui::DragFloat3("VUp", albedo::CameraManager::camera_up, 0.01f, -1.0f, 1.0f, "%.2f");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Projection")) {
				ImGui::DragFloat("FOV", &albedo::CameraManager::projection_FOV, 1.f, 0.0f, 360.0f, "%.2f");
				ImGui::DragFloat("Near", &albedo::CameraManager::projection_near, 0.1f, 0.0f, 100.0f, "%.2f");
				ImGui::DragFloat("Far", &albedo::CameraManager::projection_far, 1.f, 0.0f, 2000.0f, "%.2f");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Light")) {
				ImGui::DragFloat3("LPos", System::light_position, 0.1f, -30.0f, 30.0f, "%.2f");
				ImGui::ColorEdit4("LAmb", System::light_ambient);
				ImGui::SliderFloat("LInt", &System::light_intensity, 0.f, 5.f, "%.2f");
				ImGui::Checkbox("Shadow Mapping", &System::is_enabled_shadow_mapping);
				ImGui::SliderFloat("Shadow Bias", &System::shadow_mapping_bias, 0.f, 0.0002f, "%.6f");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Shadow Map")) {
				ImGui::Checkbox("Shadow Mapping", &albedo::System::is_enabled_shadow_mapping);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Anti Aliasing")) {
				if (ImGui::Checkbox("MSAA(Forward)", &System::is_enabled_msaa)) {
					for (std::shared_ptr<albedo::Entity> object : albedo::World::get_entities()) {
						object->change_render_pipeline();
					}
					albedo::World::get_skydome_entity()->change_render_pipeline();
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Postprocess")) {
				ImGui::Checkbox("Postprocessing", &albedo::System::is_enabled_postprocess);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Skydome")) {
				ImGui::Checkbox("Display Skydome", &albedo::System::is_enabled_skydome);
				ImGui::TreePop();
			}

			ImGui::End();
		}
		void create_panel_2() {
			ImGui::Begin("Entity");

			ImGuiTableFlags object_table_flags = ImGuiTableFlags_Resizable;
			object_table_flags |= ImGuiTableFlags_Reorderable;
			object_table_flags |= ImGuiTableFlags_Hideable;
			object_table_flags |= ImGuiTableFlags_Sortable;
			object_table_flags |= ImGuiTableFlags_SortMulti;
			object_table_flags |= ImGuiTableFlags_RowBg;
			object_table_flags |= ImGuiTableFlags_Borders;
			object_table_flags |= ImGuiTableFlags_NoBordersInBody;
			object_table_flags |= ImGuiTableFlags_ScrollX;
			object_table_flags |= ImGuiTableFlags_ScrollY;
			object_table_flags |= ImGuiTableFlags_SizingFixedFit;
			if (ImGui::BeginTable("Entity Table", 2, object_table_flags, ImVec2(0.0f, 10.f * 7.f), 0.0f)) {
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();
				std::vector<std::shared_ptr<albedo::Entity>> entities = albedo::World::get_entities();
				for (int i = 0; i < static_cast<int>(entities.size()); i++) {
					char			id_label[32];
					albedo::Entity* object = entities[i].get();
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					sprintf(id_label, "%d", i);
					bool is_selected = selected_id == i;
					if (ImGui::Selectable(id_label, is_selected, ImGuiSelectableFlags_SpanAllColumns,
										  ImVec2(0.0f, 0.f))) {
						selected_id = i;
					}
					ImGui::TableNextColumn();
					ImGui::Text("%s", object->name.c_str());
				}
				ImGui::EndTable();
			}

			ImGui::End();
		}
		void create_panel_3() {
			ImGui::Begin("Detail");

			albedo::Entity* entity = albedo::World::get_entities()[selected_id].get();

			ImGui::SeparatorText("Transform");
			ImGui::DragFloat3("Pos", (float*)&entity->position, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Rot", (float*)&entity->rotation, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Scl", (float*)&entity->scale, 0.1f, -10.0f, 10.0f, "%.2f");

			ImGui::SeparatorText("Shader");
			const char* shader_items[]		= {"Color", "Phong", "Skydome"};
			int			shader_item_current = (int)entity->shader_component->type;
			if (ImGui::Combo("Shaders", &shader_item_current, shader_items, IM_ARRAYSIZE(shader_items))) {
				const albedo::ShaderComponent::ShaderType shader_type =
					(albedo::ShaderComponent::ShaderType)shader_item_current;
				entity->change_shader(shader_type);
			}

			ImGui::SeparatorText("Material");
			const char* tex_items[]		 = {"Mono", "Checker Board", "Image"};
			int			tex_item_current = (int)entity->map_component->texture_type;
			if (ImGui::Combo("Tex", &tex_item_current, tex_items, IM_ARRAYSIZE(tex_items))) {
				const MapComponent::TextureType texture_type = (MapComponent::TextureType)tex_item_current;
				entity->set_texture_type(texture_type);
			}
			/*
			if (ImGui::ColorEdit3("TCol", object->map_component->map_color)) {
				const MapComponent::TextureType texture_type = (MapComponent::TextureType)tex_item_current;
				object->set_texture_type(texture_type);
			}*/

			ImGui::SliderFloat("Spec", &entity->specular_power, 0.1f, 1000.f, "%.2f");

			ImGui::End();
		}
	};
} // namespace albedo