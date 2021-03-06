#include "Globals.h"
#include "Engine.h"
#include "ModuleRender.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "SDL/include/SDL.h"
#include <windows.h>
#include "GL/glew.h"
#include <gl/GL.h>
#include "Plane.h"
#include "ModuleAnimation.h"
#include "CoordinateArrows.h"
#include "Level.h"
#include "TransformComponent.h"
#include "ModuleCameraManager.h"

struct ShaderProgram;

ModuleRender::ModuleRender()
{
}

// Destructor
ModuleRender::~ModuleRender()
{}

// Called before render is available
bool ModuleRender::Init()
{
	_moduleWindow = App->GetModule<ModuleWindow>();
	_moduleInput = App->GetModule<ModuleInput>();
	_cameraManager = App->GetModule<ModuleCameraManager>();
	return true;
}

bool ModuleRender::Start()
{
	LOG("Creating Renderer context");
	bool ret = true;
	context = SDL_GL_CreateContext(_moduleWindow->window);

	if (context == nullptr)
	{
		LOG("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		GLenum err = glewInit();

		if (err != GLEW_OK)
		{
			LOG("Error initialising GLEW: %s", glewGetErrorString(err));
			return false;
		}

		LOG("Using Glew %s", glewGetString(GLEW_VERSION));
		LOG("Vendor: %s", glGetString(GL_VENDOR));
		LOG("Renderer: %s", glGetString(GL_RENDERER));
		LOG("OpenGL version supported %s", glGetString(GL_VERSION));
		LOG("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		glClearColor(0, 0, 0, 1.f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		int w, h;
		_moduleWindow->GetWindowSize(w, h);
		CameraComponent* camera = _cameraManager->GetMainCamera();
		if (nullptr != camera)
		{
			camera->SetAspectRatio(float(w) / float(h));
		}

		Quat rotation_plane = Quat::FromEulerXYZ(DEG2RAD(0.f), DEG2RAD(0.f), DEG2RAD(0.f));
		objects.push_back(new ::Plane(float3(0, 0.f, -5.f), rotation_plane, 60));

		objects.push_back(new CoordinateArrows());

		SetVSync(-1);
	}
	return ret;
}

update_status ModuleRender::PreUpdate(float DeltaTime)
{	
	
	CameraComponent* camera = _cameraManager->GetMainCamera();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(camera->GetProjectionMatrix());

	if (_moduleInput->GetWindowEvent(WE_RESIZE))
	{
		int w, h;
		_moduleWindow->GetWindowSize(w, h);
		camera->SetAspectRatio(float(w) / float(h));
		glViewport(0, 0, w, h);
	}

	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(camera->GetViewMatrix());

	return UPDATE_CONTINUE;
}

// Called every draw update
update_status ModuleRender::Update(float DeltaTime)
{
	bool ret = true;

	for (std::list<Primitive*>::iterator it = objects.begin(); it != objects.end(); ++it)
		(*it)->Draw();

	return ret ? UPDATE_CONTINUE : UPDATE_ERROR;
}

update_status ModuleRender::PostUpdate(float DeltaTime)
{
	SDL_GL_SwapWindow(_moduleWindow->window);

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRender::CleanUp()
{
	LOG("Destroying renderer");

	//Destroy window
	if (context != nullptr)
	{
		SDL_GL_DeleteContext(context);
	}

	for (std::list<Primitive*>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->CleanUp();
		RELEASE(*it);
	}

	return true;
}

using PFNWGLSWAPINTERVALFARPROC = BOOL(APIENTRY *)(int);
void ModuleRender::SetVSync(int interval) const
{
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALFARPROC>(wglGetProcAddress("wglSwapIntervalEXT"));

	if (wglSwapIntervalEXT)
	{
		if (wglSwapIntervalEXT(interval))
		{
			LOG("VSync changed");
		}
		else
		{
			LOG("VSync change failed");
		}
	}
	else
	{
		LOG("VSync change unsupported");
	}
}

