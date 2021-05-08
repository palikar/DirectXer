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
#define I_SHOOT              11
#define I_FACEBOOK           12
#define I_INSTAGRAM          13
#define Kd_Map            32772
#define Material_001         14
#define M_TREE_1             15
#define T_SKY             32773
#define T_CHECKER         32774
#define T_ROCKS_COLOR     32775
#define T_FLOOR_COLOR     32776
#define T_ROCKS_AO        32777
#define T_ROCKS_NORMAL    32778
#define F_DroidSans_24       16
#define F_DroidSansBold_24   17
#define A_SHOOT              18
#define A_EXPLODE            19


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
	"I_SHOOT",
	"I_FACEBOOK",
	"I_INSTAGRAM",
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
	11,
	12,
	13,
};

/* ------------------------------  */

static inline const char* WavsNames[] = {
	"A_SHOOT",
	"A_EXPLODE",
};

static inline uint32 WavsIds[] = {
	18,
	19,
};

/* ------------------------------  */

static inline const char* FontsNames[] = {
	"F_DroidSans_24",
	"F_DroidSansBold_24",
};

static inline uint32 FontsIds[] = {
	16,
	17,
};

/* ------------------------------  */

static inline const char* MeshesNames[] = {
	"M_TREE_1",
};

static inline uint32 MeshesIds[] = {
	15,
};

/* ------------------------------  */

static inline AssetFile AssetFiles[] = {
	{ "resources/SpaceAssets.dbundle", 114124656 },
};

static inline size_t SpaceGameAssetFile = 0;
