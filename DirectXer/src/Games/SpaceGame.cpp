#include <Glm.hpp>
#include <Math.hpp>
#include <Input.hpp>
#include <Logging.hpp>
#include <Math.hpp>
#include <App.hpp>
#include <Memory.hpp>
#include <Assets.hpp>
#include <Timing.hpp>

#include <Timing.hpp>

#include "SpaceGame.hpp"
#include <SpaceAssets.hpp>

#include <imgui.h>

static uint32 EXPLOSION_SPRITE;

static float EXPLOSION_MAX_TIME = 1.0f;

struct Enemy
{
	glm::vec2 Position;
	uint32 Image;
	bool Dead;
	float XVector;
};

struct Bullet
{
	glm::vec2 Position;
};

struct Animation
{
	uint32 Sheet;
	glm::vec2 Position;
	uint32 MaxIndex;
	uint32 CurrentIndex;
	float Time;
};

struct GameState
{
	BulkVector<Enemy> Enemies;
	BulkVector<Bullet> Bulltets;
	BulkVector<Animation> Animations;
	uint32 Score;
	uint32 SpawndedEnemies;
	glm::vec2 PlayerPosition;
	float EnemySpwaner;
	float Time;
	uint16 Wave;
	
};

void SpaceGame::Init()
{
	DxProfileCode(DxTimedBlock(Phase_Init, "Game initialization"));
	Renderer2D.InitRenderer(Graphics, { Application->Width, Application->Height });

	Memory::EstablishTempScope(Megabytes(4));
	AssetBuildingContext masterBuilder{&Renderer2D.ImageLib, &Renderer2D.FontLib, &AudioEngine, Graphics};
	AssetStore::LoadAssetFile(AssetFiles[SpaceGameAssetFile], masterBuilder);
	Memory::EndTempScope();	

	const uint32 maxSpritesCount = 10;
	SpriteSheets.Init(maxSpritesCount, &Renderer2D);
	EXPLOSION_SPRITE = SpriteSheets.PutSheet(I_EXPLOSION, { 960.0f, 384.0f }, { 5, 2 });

	GameState = Memory::BulkGet<struct GameState>();
	GameState->PlayerPosition = { 300.0f, Application->Height - 100.0f };
	GameState->Enemies.reserve(50);
	GameState->Bulltets.reserve(100);
	GameState->EnemySpwaner = 0.0f;
	GameState->SpawndedEnemies = 0;
	GameState->Time = 0.0f;
	GameState->Score = 0;
	GameState->Wave = 1;

	Graphics->SetRasterizationState(RS_NORMAL);
}

void SpaceGame::PostInit()
{
	Application->Window->Resize(680, 900);
}

void SpaceGame::Resize()
{
	Renderer2D.Params.Width = Application->Width;
	Renderer2D.Params.Height = Application->Height;
}

void SpaceGame::ControlPlayer(float dt)
{
	glm::vec2& pos = GameState->PlayerPosition;
	const float playerSped = 350.0f;

	auto lThumb = Input::gInput.GetLeftThumb();
	
	if (Input::gInput.IsKeyPressed(KeyCode::A) || lThumb.x < 0.0f)
	{
		pos.x -= playerSped * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::D) || lThumb.x > 0.0f)
	{
		pos.x += playerSped * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::W) || lThumb.y > 0.0f)
	{
		pos.y -= playerSped * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::S) || lThumb.y < 0.0f)
	{
		pos.y += playerSped * dt;
	}

	pos.y = clamp(pos.y, Application->Height - 130.0f, Application->Height - 70.0f);
	pos.x = clamp(pos.x, 2.0f,  Application->Width - 64.0f - 2.0f);

	if (Input::gInput.IsKeyReleased(KeyCode::Space) || Input::gInput.IsJoystickButtonReleased(GAMEPAD_A))
	{
		GameState->Bulltets.push_back({GameState->PlayerPosition + glm::vec2{32.0f, -32.0f}});
		AudioEngine.Play(A_SHOOT, 0.25f);
	}
}

void SpaceGame::CleanUpDead()
{
	auto& bullets = GameState->Bulltets;
	auto& enemies = GameState->Enemies;
	auto& animations = GameState->Animations;
	
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](auto bullet) { return bullet.Position.y < 0.0f; }), bullets.end());
	enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](auto enemy) { return enemy.Dead; }), enemies.end());

	animations.erase(std::remove_if(animations.begin(), animations.end(), [](auto animation) {
		return animation.Time > EXPLOSION_MAX_TIME;
	}), animations.end());
}

void SpaceGame::UpdateGameState(float dt)
{
	OPTICK_EVENT();
	
	CleanUpDead();
	GameState->Time += dt;
	
	auto& bullets = GameState->Bulltets;
	auto& enemies = GameState->Enemies;
	auto& animations = GameState->Animations;

	const float bulletSpeed = 800.0f;
	const float enemySpeed = 80.0f;
	
	for (auto& bullet : GameState->Bulltets)
	{
		bullet.Position.y -= bulletSpeed * dt;
	}

	for (auto& bullet : GameState->Bulltets)
	{
		Rectangle2D bulletRect{bullet.Position, { 16.0f, 32.0f }};
		
		for (auto& enemy : GameState->Enemies)
		{
			Rectangle2D enemyRect{enemy.Position, { 64.0f, 64.0f }};
			
			if(IntersectRects(bulletRect, enemyRect))
			{
				Animation newAnimation;
				newAnimation.Time = 0.0f;
				newAnimation.Sheet = EXPLOSION_SPRITE;
				newAnimation.Position = enemy.Position;
				newAnimation.MaxIndex = 6;
				newAnimation.CurrentIndex = 0;
				GameState->Animations.push_back(newAnimation);
				enemy.Dead = true;
				bullet.Position.y = -3.0f;
				AudioEngine.Play(A_EXPLODE, 0.5f);
				GameState->Score += 1;
				break;
			}
			
		}

	}

	for (auto& animation : animations)
	{
		animation.Time += 2.5f * dt;
		animation.CurrentIndex = uint32(roundf(animation.MaxIndex * (animation.Time / EXPLOSION_MAX_TIME)));
		animation.CurrentIndex = animation.CurrentIndex >= animation.MaxIndex ? animation.MaxIndex : animation.CurrentIndex; 
	}

	GameState->EnemySpwaner += dt;
	if (GameState->EnemySpwaner > Random::Uniform(2.0f, 4.0f) && GameState->SpawndedEnemies < 15)
	{
		GameState->SpawndedEnemies += 1;
		float x = Random::Uniform(50.0f, Application->Width - 50.0f);
		Enemy newEnemy{{x, -2.0f}, I_EVIL_SHIP_1, false};
		newEnemy.XVector = Random::Uniform(0.0f, 1.0f) < 0.5f ? 50.0f : -50.0f;
		GameState->Enemies.push_back(newEnemy);
		GameState->EnemySpwaner = 0.0f;
	}
	
	for(auto& enemy : enemies)
	{
		enemy.Position.y += enemySpeed * dt;
		enemy.Position.x += enemy.XVector * dt;
		if(enemy.Position.y > Application->Height)
		{
			enemy.Dead = true;
			continue;
		}
		
		if(enemy.Position.x < 0.0f || enemy.Position.x > Application->Width - 64.0f)
		{
			enemy.XVector = -enemy.XVector;
			continue;
		}
		
		enemy.XVector = Random::Uniform() < 0.02f ? -enemy.XVector : enemy.XVector;
	}
	
	ControlPlayer(dt);

}

void SpaceGame::Render(float dt)
{
	OPTICK_EVENT();
		
	Graphics->SetDepthStencilState(DSS_2DRendering);
	Graphics->SetBlendingState(BS_PremultipliedAlpha);
	
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();


	// @Note: Draw the background as the last thing so that the least amount of framgents can get processed
	Renderer2D.BeginScene();
	Renderer2D.DrawImage(I_BG, { 0.0f, 0.0f }, { Application->Width, Application->Height });
	Renderer2D.EndScene();

	Renderer2D.BeginScene();
	for (const auto& enemy : GameState->Enemies)
	{
		Renderer2D.DrawImage(enemy.Image, enemy.Position, { 64.0f, 64.0f });
	}

	for (const auto& bullet : GameState->Bulltets)
	{
		Renderer2D.DrawImage(I_BULLET, bullet.Position, { 16.0f, 32.0f });
	}

	for (const auto& anim : GameState->Animations)
	{
		SpriteSheets.DrawSprite(anim.Sheet, anim.CurrentIndex, anim.Position, { 64.0f, 64.0f });
		
	}

	Renderer2D.DrawImage(I_MAIN_SHIP, GameState->PlayerPosition, { 64.0f, 64.0f });
	Renderer2D.EndScene();
	
	Renderer2D.BeginScene();
	Renderer2D.DrawImage(I_STATS, {20.0f, 20.0f}, { Application->Width - 40.0f, 32.0f });

	auto time = (int)roundf(GameState->Time);
	Renderer2D.DrawText(Formater.Format("Wave: {}", GameState->Wave), {42.0f, 44.0f}, F_DroidSansBold_24);
	Renderer2D.DrawText(Formater.Format("Score: {}", GameState->Score), {260.0f, 44.0f}, F_DroidSansBold_24);
	Renderer2D.DrawText(Formater.Format("Time: {}:{}", time / 60, time % 60), {460.0f, 44.0f}, F_DroidSansBold_24);
	Renderer2D.DrawImage(I_HEALTH, {10.0f, Application->Height - 32.0f - 20.0f}, { Application->Width / 3.5f, 32.0f});
	
	Renderer2D.EndScene();
}

void SpaceGame::Update(float dt)
{
	OPTICK_EVENT();
	UpdateGameState(dt);
	// @Note: Main Scene
	Render(dt);
}

