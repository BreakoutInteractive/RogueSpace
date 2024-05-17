//
//  GameConstants.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/18/24.
//

#include <cugl/cugl.h>
#include "GameConstants.hpp"

#pragma mark -
#pragma mark Game Effects
    
float GameConstants::TRANSITION_FADE_IN_TIME = 2.0f;
float GameConstants::TRANSITION_FADE_OUT_TIME = 1.0f;
int GameConstants::HIT_PAUSE_FRAMES = 0; //disabled as it feels like lag. Unsure how to fix

#pragma mark -
#pragma mark Camera

float GameConstants::GAME_CAMERA_SPEED = 2.5f;
float GameConstants::GAME_CAMERA_MAX_SPEED = 40.f;
float GameConstants::GAME_CAMERA_ACCEL = 30.0f;
float GameConstants::GAME_CAMERA_DECEL = -100.0f;
float GameConstants::GAME_CAMERA_MAX_ZOOM_OUT = 1.3f;
float GameConstants::GAME_CAMERA_ZOOM_SPEED = 0.7f;

#pragma mark -
#pragma mark Enemy

int GameConstants::ENEMY_IFRAME = 16;

int GameConstants::ENEMY_ATK_COOLDOWN = 62; //previously 50- changed so EN attacks are less spammy

int GameConstants::ENEMY_SENTRY_COOLDOWN = 32; //previously 74

float GameConstants::ENEMY_SIGHT_RANGE = 9;

float GameConstants::ENEMY_PROXIMITY_RANGE = 4;

float GameConstants::ENEMY_MELEE_ATK_RANGE = 1.6f; // previously 1.7

float GameConstants::ENEMY_RANGED_ATK_RANGE = 6.8; //previously 7

float GameConstants::ENEMY_MELEE_ATK_SPEED = 1.44f;

float GameConstants::ENEMY_RANGED_ATK_SPEED = 1.08f;

float GameConstants::ENEMY_MOVE_SPEED = 2.3;

float GameConstants::PROJ_SPEED_E = 5.4f; // previously 5.5

float GameConstants::PROJ_RADIUS_LIZARD = 0.32f; //previously 0.3

float GameConstants::PROJ_RADIUS_MAGE = 0.26f; //previously 0.4

float GameConstants::PROJ_DIST_E = 8.0f; //previously 9.0

float GameConstants::TANK_ENEMY_DR = 0.1f;

float GameConstants::STUN_DMG_BONUS = 2.34f; // previously 1.6

#pragma mark -
#pragma mark Slime Constants

int GameConstants::EXPLODE_TIMER = 60;
// note radius needs to be larger than proximity otherwise player won't necessarily take damage
float GameConstants::EXPLODE_RADIUS = 2.3f;
float GameConstants::EXPLODE_PROX_RANGE = 1.8f;

#pragma mark -
#pragma mark Boss Constants

float GameConstants::STORM_CHARGE_TIME = 1.5f;

int GameConstants::STORM_TIMER = 300;

float GameConstants::STORM_RADIUS = 3.0f;

#pragma mark -
#pragma mark Player Combat

float GameConstants::PLAYER_MAX_HP = 2.8; // must match level 0 of HP

// Requires min <= max, can be any range. Interpolation of [min,max] will be used based on charged time.
// If user has completed charge, they get the max multiplier.
// If their bow dmg is 2 and max multiplier is 2, they deal a damage of 4 on full charge.
// If min multiplier is 0.2, they deal a damage of 0.2 * 2 = 0.4 on instant fire.
// Between instant fire and complete charge, the damage would be linear between 0.4 and 4 in the example.
float GameConstants::MIN_PROJ_DMG_MULTIPLIER = 0.3f;
float GameConstants::MAX_PROJ_DMG_MULTIPLIER = 2.4f;
float GameConstants::CHARGE_TIME = 0.66f;
float GameConstants::PLAYER_PROJ_KNOCKBACK = 1.4f;

float GameConstants::PLAYER_MELEE_ATK_RANGE = 1.36f; // previously 1.4
float GameConstants::COMBO_TIME = 0.8f;
float GameConstants::PLAYER_PARRY_TIME = 0.6f; //previously 0.5, do not confuse with the stun time.

float GameConstants::KNOCKBACK = 1.6f;
float GameConstants::KNOCKBACK_PWR_ATK = 4.7f; // previously 4.0

#pragma mark -
#pragma mark Player

int GameConstants::PLAYER_IFRAME = 16;  // dead constant, cannot be removed until knockback fix.
int GameConstants::PLAYER_INCOMING_KB_FRAMES = 5; // dead constant, same issue as above

int GameConstants::PLAYER_STAMINA = 240;
float GameConstants::PLAYER_DODGE_TIME = 1/6.0f;
float GameConstants::PLAYER_DODGE_SPEED = 14.0f;

float GameConstants::PLAYER_MOVE_SPEED = 4.7; //previously 4.6
float GameConstants::PLAYER_ATK_MOVE_SPEED = 1.9f; //previously 1.8

float GameConstants::PROJ_SPEED_P = 9.0f;
float GameConstants::PROJ_SIZE_P_HALF = 1.0f;
float GameConstants::PROJ_DIST_P = 6.8f;

float GameConstants::HEALTHPACK_DROP_RATE = 34.0f;
float GameConstants::HEALTHPACK_HEAL_AMT = 0.25f;

#pragma mark - Warning
// Do not adjust these, will need actual non-parameter adjustments, no one number fits all.
float GameConstants::PROJ_SHADOW_SCALE = 0.25f;
