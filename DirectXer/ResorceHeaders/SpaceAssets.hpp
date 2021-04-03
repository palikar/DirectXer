#pragma once


#define I_BG	0
#define I_SHIP	1
#define I_ENEMY_1	2
#define I_ENEMY_2	3
#define I_ENEMY_3	4
#define i_BULLET	5
#define i_EXPLODE	6
#define I_STATS	7
#define i_HEALTH	8
#define I_HEART	9
#define F_DroidSans	0
#define F_DroidSansBold	1
#define A_SHOOT	0
#define A_EXPLODE	1


static inline const char* ImageAssets[] = {
	"I_BG",
	"I_SHIP",
	"I_ENEMY_1",
	"I_ENEMY_2",
	"I_ENEMY_3",
	"i_BULLET",
	"i_EXPLODE",
	"I_STATS",
	"i_HEALTH",
	"I_HEART",
};

static inline const char* FontAssets[] = {
	"F_DroidSans",
	"F_DroidSansBold",
};

static inline const char* WavAssets[] = {
	"A_SHOOT",
	"A_EXPLODE",
};

static inline const char* SpaceGameAssetFilePath = "resources/SpaceAssets.dx1";

