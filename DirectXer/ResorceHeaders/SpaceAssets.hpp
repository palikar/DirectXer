#pragma once


#define I_BG	0
#define I_STATS	1
#define F_DroidSans_24 	 0
#define F_DroidSansBold_24 	 1
#define A_SHOOT	0
#define A_EXPLODE	1
#define I_EVIL_SHIP_1	65536
#define I_EVIL_SHIP_2	65537
#define I_EVIL_SHIP_3	65538
#define I_MAIN_SHIP	65539
#define I_BULLET	65540
#define I_EXPLOSION	65541
#define I_HEART	65542
#define I_HEALTH	65543


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

