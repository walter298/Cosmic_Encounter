#include "HomeEditor.h"

#include "../data_util/DataStructures.h" //ScopeExit

nv::editor::HomeEditor::HomeEditor(Instance& instance) 
	: m_screenWidth{ instance.getScreenWidth() }, m_screenHeight{ instance.getScreenHeight() }
{
}

nv::editor::EditorDest nv::editor::HomeEditor::imguiRender() noexcept {
	static const ImVec2 winPos{ static_cast<float>(m_screenWidth) / 2, static_cast<float>(m_screenHeight) / 2 };
	static constexpr ImVec2 btnSize{ 183.0f, 80.0f };
	static constexpr ImVec2 winSize = buttonList(btnSize, 2);

	ImGui::SetNextWindowPos(winPos);
	ImGui::SetNextWindowSize(winSize);
	ImGui::Begin("Options");
	ScopeExit exit{ &ImGui::End };
	if (ImGui::Button("Sprite Editor", btnSize)) {
		return EditorDest::Sprite;
	}
	if (ImGui::Button("Scene Editor", btnSize)) {
		return EditorDest::Scene;
	}
	return EditorDest::None;
}
