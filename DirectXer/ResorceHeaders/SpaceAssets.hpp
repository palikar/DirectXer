#pragma once


#define T_ATLAS_0          32769
#define T_ATLAS_1          32770
#define T_ATLAS_2          32771
#define I_EVIL_SHIP_1          1
#define I_EVIL_SHIP_2          2
#define I_EVIL_SHIP_3          3
#define I_MAIN_SHIP            4
#define I_BULLET               5
#define I_EXPLOSION            6
#define I_HEART                7
#define I_HEALTH               8
#define I_BG                   9
#define I_STATS               10
#define I_SHOOT               11
#define I_FACEBOOK            12
#define I_INSTAGRAM           13
#define Material_001_Kd_Map 32772
#define Material_001          14
#define Material_001_CB    32769
#define M_TREE_1_VB        32769
#define M_TREE_1_IB        32769
#define M_TREE_1              15
#define SimpleColor           16
#define SimpleColor_CB     32770
#define M_SUZANNE_VB       32770
#define M_SUZANNE_IB       32770
#define M_SUZANNE             17
#define T_SKY              32773
#define T_NIGHT_SKY        32774
#define T_CHECKER          32775
#define T_ROCKS_COLOR      32776
#define T_FLOOR_COLOR      32777
#define T_ROCKS_AO         32778
#define T_ROCKS_NORMAL     32779
#define F_DroidSans_24        18
#define F_DroidSansBold_24    19
#define A_SHOOT               20
#define A_EXPLODE             21


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
	20,
	21,
};

/* ------------------------------  */

static inline const char* FontsNames[] = {
	"F_DroidSans_24",
	"F_DroidSansBold_24",
};

static inline uint32 FontsIds[] = {
	18,
	19,
};

/* ------------------------------  */

static inline const char* MeshesNames[] = {
	"M_TREE_1",
	"M_SUZANNE",
};

static inline uint32 MeshesIds[] = {
	15,
	17,
};

/* ------------------------------  */

GPUResource GPUResources[] = {
	{ GPUResourceType(0), 32769, "T_ATLAS_0" },
	{ GPUResourceType(0), 32770, "T_ATLAS_1" },
	{ GPUResourceType(0), 32771, "T_ATLAS_2" },
	{ GPUResourceType(0), 32772, "Material_001_Kd_Map" },
	{ GPUResourceType(3), 32769, "Material_001_CB" },
	{ GPUResourceType(1), 32769, "M_TREE_1_VB" },
	{ GPUResourceType(2), 32769, "M_TREE_1_IB" },
	{ GPUResourceType(3), 32770, "SimpleColor_CB" },
	{ GPUResourceType(1), 32770, "M_SUZANNE_VB" },
	{ GPUResourceType(2), 32770, "M_SUZANNE_IB" },
	{ GPUResourceType(0), 32773, "T_SKY" },
	{ GPUResourceType(0), 32774, "T_NIGHT_SKY" },
	{ GPUResourceType(0), 32775, "T_CHECKER" },
	{ GPUResourceType(0), 32776, "T_ROCKS_COLOR" },
	{ GPUResourceType(0), 32777, "T_FLOOR_COLOR" },
	{ GPUResourceType(0), 32778, "T_ROCKS_AO" },
	{ GPUResourceType(0), 32779, "T_ROCKS_NORMAL" },
};

static inline AssetFile AssetFiles[] = {
	{ "resources/SpaceAssets.dbundle", 139365260 },
};

static inline size_t SpaceGameAssetFile = 0;
