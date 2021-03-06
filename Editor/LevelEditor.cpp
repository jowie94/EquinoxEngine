#include "EditorSubmodule.h"
#include "EditorUtils.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "BaseComponent.h"
#include "ModuleLevelManager.h"
#include "Level.h"
#include "BaseComponentEditor.h"

#include "IMGUI/imgui.h"
#include <unordered_map>
#include <typeindex>

namespace
{
	std::string GenerateNiceNameForComponent(const std::string& componentName)
	{
		std::string ret = componentName;
		
		for (size_t i = 1; i < ret.length(); ++i)
		{
			if (islower(ret[i - 1]) && isupper(ret[i]))
			{
				ret.insert(i, 1, ' ');  
			}
		}

		return ret;
	}
}

class LevelEditor : public EditorSubmodule
{
public:
	void Init() override;
	void Update() override;
	void CleanUp() override;

private:
	void drawProperties();
	void drawLevelHierachy();
	void drawLevelHierachy(GameObject* node);

	GameObject* _selectedGameObject = nullptr;
	std::unordered_map<std::type_index, BaseComponentEditor*> _componentEditors;

	std::shared_ptr<ModuleWindow> _moduleWindow;
	std::shared_ptr<ModuleLevelManager> _levelManager;
};

REGISTER_EDITOR_SUBMODULE(LevelEditor);

void LevelEditor::Init()
{
	for (ComponentEditorFactoryBase* editorComponent : GetComponentEditorFactoryDictionary()->GetAllFactories())
	{
		BaseComponentEditor* editor = editorComponent->Instantiate();
		_componentEditors[editor->GetComponentTypeIndex()] = editor;
	}
	LOG("Loaded %d component editors", _componentEditors.size());
	GetComponentEditorFactoryDictionary()->Clear();

	_moduleWindow = App->GetModule<ModuleWindow>();
	_levelManager = App->GetModule<ModuleLevelManager>();
}

void LevelEditor::Update()
{
	drawProperties();
	drawLevelHierachy();

	if (nullptr != _selectedGameObject)
	{
		_selectedGameObject->DrawBoundingBox();
	}
}

void LevelEditor::CleanUp()
{
	for (auto& editor : _componentEditors)
	{
		RELEASE(editor.second);
	}
	_componentEditors.clear();
}

void LevelEditor::drawProperties()
{
	int w, h;
	_moduleWindow->GetWindowSize(w, h);
	ImVec2 windowPosition(w - 400, 0);
	ImGui::SetNextWindowSize(ImVec2(400, h - 400), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(windowPosition, ImGuiSetCond_Always);
	if (ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (_selectedGameObject)
		{
			for (BaseComponent* component : _selectedGameObject->GetComponents())
			{
				std::string componentName = GenerateNiceNameForComponent(component->GetComponentName());
				ImGui::PushID(component->GetComponentName().c_str());
				if (ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlapMode))
				{
					auto it = _componentEditors.find(component->GetComponentClassId());
					if (it != _componentEditors.end())
					{
						BaseComponentEditor* editor = it->second;
						if (editor->CanBeDisabled())
						{
							ImGui::Checkbox("Enabled", &component->Enabled);
						}

						if (editor->IsRemovable())
						{
							if (editor->CanBeDisabled())
							{
								ImGui::SameLine();
							}

							ImGui::PushStyleColor(ImGuiCol_Button, ImColor(255, 0, 0));
							if (ImGui::Button("Delete Component"))
							{
								_selectedGameObject->DeleteComponent(component);
							}
							ImGui::PopStyleColor();
						}

						editor->DrawUI(component);
					}
					else
					{
						ImGui::Checkbox("Enabled", &component->Enabled); ImGui::SameLine();
						ImGui::PushStyleColor(ImGuiCol_Button, ImColor(255, 0, 0));
						if (ImGui::Button("Delete Component"))
						{
							_selectedGameObject->DeleteComponent(component);
						}
						ImGui::PopStyleColor();
					}
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::End();
}

void LevelEditor::drawLevelHierachy()
{
	int w, h;
	_moduleWindow->GetWindowSize(w, h);

	ImGui::SetNextWindowSize(ImVec2(300, h), ImGuiSetCond_Once);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

	if (ImGui::Begin("Hierachy", nullptr, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		for (GameObject* node : _levelManager->GetCurrentLevel().GetRootNode()->GetChilds())
			drawLevelHierachy(node);
	}
	ImGui::End();
}

void LevelEditor::drawLevelHierachy(GameObject* node)
{
	int flags = ImGuiTreeNodeFlags_DefaultOpen;
	if (node->GetChilds().size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (_selectedGameObject == node)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(node->Name.c_str(), flags))
	{
		if (ImGui::IsItemClicked(0))
		{
			_selectedGameObject = node;
		}

		for (GameObject* child : node->GetChilds())
		{
			drawLevelHierachy(child);
		}
		ImGui::TreePop();
	}
}
