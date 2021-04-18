#pragma once

#if defined(DXER_EXAMPLE_SCENES)

#include <Games/ExampleScenes.hpp>
using GameClass = ExampleScenes;

#elif defined(DXER_SPACE_GAME)

#include <Games/SpaceGame.hpp>
using GameClass = SpaceGame;

#else
struct DummyGame
{
	void Init(){}
	void PostInit(){}
	void Update(float){}
	void Resize(){}

	void* Graphics;
	void* Application;
};
using GameClass = DummyGame;

#endif
