#include <Glm.hpp>
#include <Input.hpp>
#include <Logging.hpp>
#include <Math.hpp>
#include <App.hpp>
#include <Memory.hpp>

#include "SpaceGame.hpp"

#include <imgui.h>

static uint32 BGIMAGE;
static uint32 SHIPIMAGE;
static uint32 EVIL_SHIP_1;
static uint32 EVIL_SHIP_2;
static uint32 EVIL_SHIP_3;
static uint32 SHIP_BULLET;

struct Enemy
{
	glm::vec2 Position;
	uint32 Image;
};

struct Bullet
{
	glm::vec2 Position;
};

struct GameState
{
	uint32 Score;
	glm::vec2 PlayerPosition;
	BulkVector<Enemy> Enemies;
	BulkVector<Bullet> Bulltets;
};

void SpaceGame::Init()
{
	Renderer2D.InitRenderer(Graphics, { Application->Width, Application->Height });

	Memory::EstablishTempScope(Kilobytes(2));
	ImageLibraryBuilder imagebuilder;
	imagebuilder.Init(10);
	BGIMAGE = imagebuilder.PutImage("assets/PNG/Main_Menu/BG.png");
	SHIPIMAGE = imagebuilder.PutImage("assets/PNG/Ship_Parts/Ship_Main_Icon.png");
	EVIL_SHIP_1 = imagebuilder.PutImage("assets/evil_ship_1.png");
	EVIL_SHIP_2 = imagebuilder.PutImage("assets/evil_ship_2.png");
	EVIL_SHIP_3 = imagebuilder.PutImage("assets/evil_ship_3.png");
	SHIP_BULLET = imagebuilder.PutImage("assets/ship_bullet.png");
	Renderer2D.ImageLib.Build(imagebuilder);
	Memory::EndTempScope();

	FontBuilder fontBuilder;
	Memory::EstablishTempScope(Kilobytes(1));
	fontBuilder.Init(2);
	fontBuilder.PutTypeface("fonts/DroidSans/DroidSans.ttf", 24);
	fontBuilder.PutTypeface("fonts/DroidSans/DroidSans-Bold.ttf", 24);
	Renderer2D.FontLib.Build(fontBuilder);
	Memory::EndTempScope();

	SpriteSheets.Init(5, &Renderer2D);
	SpriteSheets.PutSheet(2, { 640.0f, 470.0f }, { 8, 5 });

	uiRenderTarget = Graphics->CreateRenderTarget((uint16)Application->Width, (uint16)Application->Height, TF_RGBA);

	Graphics->SetRasterizationState(RS_NORMAL);
	
	GameState = Memory::BulkGet<struct GameState>();

	GameState->PlayerPosition = { 300.0f, Application->Height - 100.0f };
	GameState->Enemies.reserve(50);
	GameState->Bulltets.reserve(100);
		
}

void SpaceGame::Resize()
{
	Renderer2D.Params.Width = Application->Width;
	Renderer2D.Params.Height = Application->Height;
}

void SpaceGame::ControlPlayer(float dt)
{
	glm::vec2& pos = GameState->PlayerPosition;
	if (Input::gInput.IsKeyPressed(KeyCode::A))
	{
		pos.x -= 250.0f * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::D))
	{
		pos.x += 250.0f * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::W))
	{
		pos.y -= 250.0f * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::S))
	{
		pos.y += 250.0f * dt;
	}

	pos.y = clamp(pos.y, Application->Height - 130.0f, Application->Height - 70.0f);
	pos.x = clamp(pos.x, 2.0f,  Application->Width - 64.0f - 2.0f);
}

void SpaceGame::UpdateGameState(float dt)
{
	auto& bullets = GameState->Bulltets;
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](auto bullet) { return bullet.Position.y < 0.0f; }), bullets.end());
	for (auto& bullet : GameState->Bulltets)
	{
		bullet.Position.y -= 800.0f * dt;
	}
	
	ControlPlayer(dt);
	if (Input::gInput.IsKeyReleased(KeyCode::E))
	{
		float x = Random::Uniform(50.0f, Application->Width - 50.0f);
		GameState->Enemies.push_back({{x, 50.0f}, EVIL_SHIP_1});
	}

	if (Input::gInput.IsKeyReleased(KeyCode::Space))
	{
		GameState->Bulltets.push_back({GameState->PlayerPosition + glm::vec2{32.0f, -32.0f}});
	}

}

void SpaceGame::Update(float dt)
{

	UpdateGameState(dt);
	// @Note: Main Scene

	Graphics->SetDepthStencilState(DSS_2DRendering);
	Graphics->SetBlendingState(BS_PremultipliedAlpha);
	
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();


	// @Note: Draw the background as the last thing so that the least amount of framgents can get processed
	Renderer2D.BeginScene();
	Renderer2D.DrawImage(BGIMAGE, { 0.0f, 0.0f }, { Application->Width, Application->Height });
	Renderer2D.EndScene();

	Renderer2D.BeginScene();
	for (const auto& enemy : GameState->Enemies)
	{
		Renderer2D.DrawImage(enemy.Image, enemy.Position, { 64.0f, 64.0f });
	}

	for (const auto& bullet : GameState->Bulltets)
	{
		Renderer2D.DrawImage(SHIP_BULLET, bullet.Position, { 16.0f, 32.0f });
	}
	

	// Renderer2D.EndScene();
	// Renderer2D.BeginScene();
	
	Renderer2D.DrawImage(SHIPIMAGE, GameState->PlayerPosition, { 64.0f, 64.0f });
	
	
	Renderer2D.EndScene();
}

