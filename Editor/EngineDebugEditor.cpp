#pragma once

#include "EditorSubmodule.h"
#include "IMGUI/imgui.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "EditorUtils.h"
#include "ModuleLevelManager.h"
#include "Level.h"

namespace
{
	void DrawQuadtreeNodeAABB(const QuadtreeNode& node)
	{
		DrawBoundingBox(node.GetBox());

		for(QuadtreeNode* child : node.GetChilds())
		{
			DrawQuadtreeNodeAABB(*child);
		}
	}
}

class EngineDebugEditor : public EditorSubmodule
{
public:

	void Init() override;
	void Update() override;

private:
	bool _wireframe = false;
	bool _drawQuadtree = false;
	bool _drawHierachy = false;

	std::shared_ptr<ModuleWindow> _moduleWindow;
	std::shared_ptr<ModuleLevelManager> _levelManager;
};

REGISTER_EDITOR_SUBMODULE(EngineDebugEditor);


void EngineDebugEditor::Init()
{
	_moduleWindow = App->GetModule<ModuleWindow>();
	_levelManager = App->GetModule<ModuleLevelManager>();
}

void EngineDebugEditor::Update()
{
	int w, h;
	_moduleWindow->GetWindowSize(w, h);

	ImVec2 windowPosition(301, h - 100);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(windowPosition, ImGuiSetCond_Always);
	if (ImGui::Begin("Engine Debug", nullptr, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{

		bool wireframe = _wireframe;
		ImGui::Checkbox("Wireframe mode", &wireframe);

		if (wireframe != _wireframe)
		{
			_wireframe = wireframe;
			wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		ImGui::Checkbox("Draw hierachy", &_drawHierachy);

		ImGui::Checkbox("Draw quadtree", &_drawQuadtree);
	}
	ImGui::End();

	const Level& currentLevel = _levelManager->GetCurrentLevel();
	if (_drawQuadtree)
	{
		DrawQuadtreeNodeAABB(currentLevel.GetQuadtree().GetRootNode());
	}

	if (_drawHierachy)
	{
		const GameObject* root = currentLevel.GetRootNode();

		root->DrawHierachy();
	}
}
