#pragma once


#define T_ATLAS_0         32769
#define T_ATLAS_1         32770
#define T_ATLAS_2         32771
#define I_EVIL_SHIP_1         1
#define I_EVIL_SHIP_2         2
#define I_EVIL_SHIP_3         3
#define I_MAIN_SHIP           4
#define I_BULLET              5
#define I_EXPLOSION           6
#define I_HEART               7
#define I_HEALTH              8
#define I_BG                  9
#define I_STATS              10
#define T_CHECKER         32772
#define T_ROCKS_COLOR     32773
#define T_FLOOR_COLOR     32774
#define T_ROCKS_AO        32775
#define T_ROCKS_NORMAL    32776
#define F_DroidSans_24       11
#define F_DroidSansBold_24   12
#define A_SHOOT              13
#define A_EXPLODE            14


static inline const char* ImagesNames[] = {
	"I_EVIL_SHIP_1",
	"I_EVIL_SHIP_2",
	"I_EVIL_SHIP_3",
	"I_MAIN_SHIP",
	"I_BULLET",
	"I_EXPLOSION",
	"I_HEART",
	"I_HEALTH",
	"I_BG",
	"I_STATS",
};

static inline uint32 ImagesIds[] = {
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
};

/* ------------------------------  */

static inline const char* WavsNames[] = {
	"A_SHOOT",
	"A_EXPLODE",
};

static inline uint32 WavsIds[] = {
	13,
	14,
};

/* ------------------------------  */

static inline const char* FontsNames[] = {
	"F_DroidSans_24",
	"F_DroidSansBold_24",
};

static inline uint32 FontsIds[] = {
	11,
	12,
};

/* ------------------------------  */

static inline AssetFile AssetFiles[] = {
	{ "resources/SpaceAssets.dbundle", 88914776 },
};

static inline size_t SpaceGameAssetFile = 0;
