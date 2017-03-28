﻿#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include "ModuleAnimation.h"
#include <MathGeoLib/include/Math/Quat.h>
#include "Engine.h"

ModuleAnimation::ModuleAnimation(bool start_enabled) : Module(start_enabled)
{
}

ModuleAnimation::~ModuleAnimation()
{
}

bool ModuleAnimation::CleanUp()
{
	AnimMap::iterator it = _animations.begin();
	for (std::pair<std::string, Anim*> element : _animations)
	{
		for (NodeAnim* channel : element.second->Channels)
		{
			for(float3* position : channel->Positions)
				RELEASE(position);

			for(Quat* rotation : channel->Rotations)
				RELEASE(rotation);

			RELEASE(channel);
		}

		RELEASE(element.second);
	}

	for (AnimInstance* animInstance : _instances)
		RELEASE(animInstance);

	_animations.clear();

	return true;
}

update_status ModuleAnimation::Update()
{
	for (AnimInstance* animInstance : _instances)
		if(animInstance != nullptr)
			animInstance->time = fmod(animInstance->time + App->DeltaTime,float(animInstance->anim->Duration));

	return UPDATE_CONTINUE;
}

void ModuleAnimation::Load(const char* name, const char* file)
{
	LOG("Loading animation %s", file);
	char filePath[256];
	sprintf_s(filePath, "%s", file);

	const aiScene* scene = aiImportFile(filePath, aiProcessPreset_TargetRealtime_MaxQuality);

	aiAnimation** animations = scene->mAnimations;

	Anim* anim = new Anim();
	anim->Duration = animations[0]->mDuration;
	anim->Channels = std::vector<NodeAnim*>(animations[0]->mNumChannels);

	for (unsigned int i = 0; i < animations[0]->mNumChannels; ++i)
	{
		aiNodeAnim* aiNodeAnim = animations[0]->mChannels[i];
		
		anim->Channels[i] = new NodeAnim();
		anim->Channels[i]->NodeName = aiNodeAnim->mNodeName.C_Str();

		for(unsigned int j = 0; j < aiNodeAnim->mNumPositionKeys; ++j)
		{
			aiVector3D position = aiNodeAnim->mPositionKeys[j].mValue;
			anim->Channels[i]->Positions.push_back(new float3(position.x, position.y, position.z));
		}

		for (unsigned int j = 0; j < aiNodeAnim->mNumRotationKeys; ++j)
		{
			aiQuaternion rotation = aiNodeAnim->mRotationKeys[j].mValue;
			anim->Channels[i]->Rotations.push_back(new Quat(rotation.x, rotation.y, rotation.z, rotation.w));
		}
	}

	_animations[name] = anim;
	aiReleaseImport(scene);
}

AnimInstanceID ModuleAnimation::Play(const char* name)
{
	AnimInstance* animInstance = new AnimInstance();
	animInstance->anim = _animations[name];
	unsigned pos;

	if(_holes.empty())
	{
		_instances.push_back(animInstance);
		pos = _instances.size() - 1;
	}
	else
	{
		pos = _holes.at(_holes.size() - 1);
		_instances.at(pos) = animInstance;
		_holes.pop_back();
	}

	return pos;
}


void ModuleAnimation::Stop(AnimInstanceID id)
{
	_holes.push_back(id);
	RELEASE(_instances[id]);
}

bool ModuleAnimation::GetTransform(AnimInstanceID id, const char* channelName, float3& position, Quat& rotation) const
{
	AnimInstance* instance = _instances[id];
	Anim* animation = _instances[id]->anim;
	NodeAnim* node = nullptr;
	for (NodeAnim* channel : _instances[id]->anim->Channels)
	{
		if (channel->NodeName == channelName)
			node = channel;
	}

	if (!node)
		return false;
	
	float posKey = float(instance->time * (node->Positions.size() - 1)) / float(animation->Duration);
	float rotKey = float(instance->time * (node->Rotations.size() - 1)) / float(animation->Duration);

	unsigned posIndex = unsigned(posKey);
	unsigned rotIndex = unsigned(rotKey);

	float posLambda = posKey - float(posIndex);
	float rotLambda = rotKey - float(rotIndex);

	if (node->Positions.size() > 1)
		position = float3::Lerp(*node->Positions[posIndex], *node->Positions[posIndex + 1], posLambda);
	else
		position = *node->Positions[posIndex];

	if(node->Rotations.size() > 1)
		rotation = InterpQuaternion(*node->Rotations[rotIndex], *node->Rotations[rotIndex + 1], rotLambda);
	else
		rotation = *node->Rotations[rotIndex];

	return true;
}

Quat ModuleAnimation::InterpQuaternion(const Quat& first, const Quat& second, float lambda)
{
	aiQuaternion result;

	float dot = first.x*second.x + first.y*second.y + first.z*second.z + first.w*second.w;

	if(dot >= 0.0f)
	{
		result.x = first.x*(1.0f - lambda) + second.x*lambda;
		result.y = first.y*(1.0f - lambda) + second.y*lambda;
		result.z = first.z*(1.0f - lambda) + second.z*lambda;
		result.w = first.w*(1.0f - lambda) + second.w*lambda;
	}
	else
	{
		result.x = first.x*(1.0f - lambda) + second.x*-lambda;
		result.y = first.y*(1.0f - lambda) + second.y*-lambda;
		result.z = first.z*(1.0f - lambda) + second.z*-lambda;
		result.w = first.w*(1.0f - lambda) + second.w*-lambda;
	}

	result.Normalize();

	return Quat(result.x, result.y, result.z, result.w);
}
