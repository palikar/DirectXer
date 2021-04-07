#pragma once


#define I_BG	0
#define I_STATS	1
#define F_DroidSans_	0
#define F_DroidSansBold_	1
#define A_SHOOT	0
#define A_EXPLODE	1
#define I_EVIL_SHIP_1	65537
#define I_EVIL_SHIP_2	131073
#define I_EVIL_SHIP_3	196609
#define I_MAIN_SHIP	262145
#define I_BULLET	327681
#define I_EXPLOSION	393217
#define I_HEART	458753
#define I_HEALTH	524289


static inline const char* ImageAssets[] = {
	"I_BG",
	"I_STATS",
};

static inline const char* FontAssets[] = {
	"F_DroidSans",
	"F_DroidSansBold",
};

static inline const char* WavAssets[] = {
	"A_SHOOT",
	"A_EXPLODE",
};

static inline const char* SpaceGameAssetFilePath = "resources/SpaceAssets.dbundle";

