#pragma once

#include "FactoryUtils.h"

CREATE_STATIC_FACTORY(EditorSubmodule)

#define REGISTER_EDITOR_SUBMODULE(ClassName) \
	static EditorSubmoduleFactory<ClassName> Factory_##ClassName;
