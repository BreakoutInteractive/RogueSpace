//
//  LevelConstants.h
//
#ifndef __LEVEL_CONSTANTS_H__
#define __LEVEL_CONSTANTS_H__

/** The global fields of the level model */
#define WIDTH_FIELD         "width"
#define HEIGHT_FIELD        "height"
#define GRAVITY_FIELD       "gravity"
#define PLAYER_FIELD        "player"
#define ENEMY_FIELD         "enemy"
#define GOALDOOR_FIELD      "exit"
#define CRATES_FIELD        "crates"
#define WALLS_FIELD         "walls"

/** The physics fields for each object */
#define POSITION_FIELD      "pos"
#define SIZE_FIELD          "size"
#define BODYTYPE_FIELD      "bodytype"
#define DENSITY_FIELD       "density"
#define FRICTION_FIELD      "friction"
#define RESTITUTION_FIELD   "restitution"
#define DAMPING_FIELD       "damping"
#define ROTATION_FIELD      "rotation"
#define STATIC_VALUE        "static"

/** The drawing fields for each object */
#define TEXTURE_FIELD       "texture"
#define PARRY_FIELD			"parry"
#define ATTACK_FIELD		"attack"
#define DEBUG_COLOR_FIELD   "debugcolor"
#define DEBUG_OPACITY_FIELD "debugopacity"

/** The wall specific fields */
#define WALL_FIELD          "walls"
#define BOUNDARY_FIELD      "boundaries"

/** The key for our loaded level */
#define LEVEL_KEY           "level"

/** Level Data Format*/
#define CONTENTS_FIELD      "contents"
#define CDF_FIELD           "cdf"
#define MAP_FIELD           "map"

/** Level Data Classes */
#define CLASS               "class"
#define CLASS_WALL          "Wall"
#define CLASS_PLAYER        "Player"
#define CLASS_ENEMY         "Enemy"
#define CLASS_RANDOM        "Random"
#define CLASS_COLLECTION    "Collection"
#define CLASS_TILELAYER     "TileLayer"
#define CLASS_LIZARD        "Lizard"
#define CLASS_CASTER        "Caster"
#define CLASS_PARRY			"Parry"

#endif /* defined(__LEVEL_CONSTANTS_H__) */
