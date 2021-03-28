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


class App;
struct GameState;
class SpaceGame
{
public:
	void Init();
	void Update(float dt);
	void Resize();

	Graphics* Graphics;
	App* Application;
	
	SpriteSheetHolder SpriteSheets;
	Renderer2D Renderer2D;
	RTObject uiRenderTarget;

	GameState* GameState;

private:
	void UpdateGameState(float dt);
	void ControlPlayer(float dt);
};
