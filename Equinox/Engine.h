#ifndef __APPLICATION_CPP__
#define __APPLICATION_CPP__

#include<list>
#include "Globals.h"
#include "Module.h"

class ModuleMaterialManager;
class ModuleMeshManager;
class ModuleStats;
class ModuleRender;
class ModuleWindow;
class ModuleTextures;
class ModuleInput;
class ModuleAudio;
class ModuleCollision;
class ModuleLevelManager;
class ModuleTimer;
class ModuleEditorCamera;
class ModuleEditor;
class ModuleSettings;
class ModuleLighting;
class ModuleAnimation;

// Game modules ---

class Engine
{
public:

	enum State
	{
		CREATION,
		START,
		UPDATE,
		FINISH,
		EXIT
	};

	enum class UpdateState
	{
		Playing,
		Stopped
	};

	Engine();
	~Engine();

	UpdateState GetUpdateState() const;
	void SetUpdateState(const UpdateState state);
	bool IsPaused() const;
	void SetPaused(bool paused);

	int Loop();

	bool Init();
	update_status Update();
	bool CleanUp();

public:
	ModuleMeshManager* meshManager;
	ModuleMaterialManager* materialManager;
	ModuleRender* renderer;
	ModuleWindow* window;
	ModuleTextures* textures;
	ModuleInput* input;
	ModuleAudio* audio;
	ModuleCollision* collision;
	ModuleTimer* timer;
	ModuleEditorCamera* editorCamera;
	ModuleEditor* editor;
	ModuleSettings* settings;
	ModuleLighting* lighting;
	ModuleAnimation* animator;
	ModuleStats* stats;

	// Game modules ---
	ModuleLevelManager* level_manager;

	float DeltaTime;

private:
	State state;
	UpdateState _updateState = UpdateState::Playing;
	bool _isPaused = false;

	std::list<Module*> modules;

	float _timeFromLastFrame = 0;
};

extern Engine* App;

#endif // __APPLICATION_CPP__