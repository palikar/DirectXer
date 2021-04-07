#pragma once

#include <Geometry.hpp>
#include <Graphics.hpp>
#include <Camera.hpp>
#include <TextureCatalog.hpp>
#include <GeometryUtils.hpp>
#include <Materials.hpp>
#include <Lighting.hpp>
#include <2DRendering.hpp>
#include <Memory.hpp>
#include <Random.hpp>
#include <Audio.hpp>


class App;
struct GameState;
class SpaceGame
{
public:
	void Init();
	void PostInit();
	void Update(float dt);
	void Resize();

	Graphics* Graphics;
	App* Application;
	
	SpriteSheetHolder SpriteSheets;
	Renderer2D Renderer2D;
	RTObject uiRenderTarget;

	AudioPlayer AudioEngine;

	TempFormater Formater;

	GameState* GameState;

	// @Note: These will be used only by the concreate game
	void UpdateGameState(float dt);
	void ControlPlayer(float dt);
	void CleanUpDead();
};
