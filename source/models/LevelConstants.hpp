//
//  LevelConstants.h
//
#ifndef __LEVEL_CONSTANTS_H__
#define __LEVEL_CONSTANTS_H__

/** The global fields of the level model */
#define WIDTH_FIELD         "width"
#define HEIGHT_FIELD        "height"
#define PLAYER_FIELD        "player"
#define ENEMY_FIELD         "enemy"

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
#define DEBUG_COLOR_FIELD   "debugcolor"
#define DEBUG_OPACITY_FIELD "debugopacity"

#pragma mark -
#pragma mark Data Categories

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
#define CLASS_RANGEDLIZARD  "RangedLizard"
#define CLASS_CASTER        "Caster"
#define CLASS_SLIME         "Slime"
#define CLASS_TANK			"TankEnemy"
#define CLASS_BOSS          "Boss"
#define CLASS_RELIC         "Relic"
#define CLASS_COLLIDER      "Collider"
#define CLASS_HITBOX        "Hitbox"
#define CLASS_TUTORIAL_REGION      "TutorialRegion"

#pragma mark -
#pragma mark Enemy Stat Fields

#define ENEMY_HP_FIELD      "hp"
#define ENEMY_DMG_FIELD     "damage"


#pragma mark -
#pragma mark Parsing Attributes

#define TILE_LAYER          "tilelayer"
#define GROUP_LAYER         "group"
#define OBJECT_LAYER        "objectgroup"
#define LAYERS_KEY          "layers"
#define TYPE                "type"
#define VISIBLE             "visible"
#define FIRSTGID            "firstgid"

#endif /* defined(__LEVEL_CONSTANTS_H__) */
