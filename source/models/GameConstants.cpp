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
int GameConstants::HIT_PAUSE_FRAMES = 8;

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

float GameConstants::ENEMY_STUN_DURATION = 1.25f; //previously 1.4

int GameConstants::ENEMY_SENTRY_COOLDOWN = 75; //previously 120

float GameConstants::ENEMY_SIGHT_RANGE = 10;

float GameConstants::ENEMY_PROXIMITY_RANGE = 5;

float GameConstants::ENEMY_MELEE_ATK_RANGE = 1.7f; // previously 1.6

float GameConstants::ENEMY_RANGED_ATK_RANGE = 7;

float GameConstants::ENEMY_MELEE_ATK_SPEED = 1.125f;

float GameConstants::ENEMY_RANGED_ATK_SPEED = 0.84375f;

float GameConstants::ENEMY_MOVE_SPEED = 2.3;

float GameConstants::PROJ_SPEED_E = 5.5f; // previously 6.0

float GameConstants::PROJ_RADIUS_LIZARD = 0.3f; //previously 0.4

float GameConstants::PROJ_RADIUS_MAGE = 0.4f; //previously 0.65

float GameConstants::PROJ_DIST_E = 9.0f;

float GameConstants::PROJ_DIST_E_SQ = 81.0f;

float GameConstants::TANK_ENEMY_DR = 0.1f;

float GameConstants::STUN_DMG_BONUS = 1.6f; // previously 1.5

#pragma mark -
#pragma mark Player Upgradeable Stat Constants

float GameConstants::PLAYER_MAX_HP = 3;

float GameConstants::PLAYER_ATK_DAMAGE = 1.0f;

float GameConstants::PLAYER_BOW_DAMAGE = 1.0f;

float GameConstants::PLAYER_PASSIVE_REDUCTION = 0; // initially no defense

float GameConstants::PLAYER_BLOCKING_REDUCTION = 0.1; // initially 10% damage reduction while blocking

float GameConstants::PLAYER_DODGE_COUNT = 1.0;    // could technically be a floating point to imply something like you have 1/3 dash energy remaining if dash count per full-charge is 1.5

float GameConstants::PLAYER_STUN_DURATION = 1.0f;

float GameConstants::PLAYER_ATTACK_COOLDOWN = 0; // disabled cooldown

#pragma mark -
#pragma mark Player

int GameConstants::PLAYER_IFRAME = 16;
int GameConstants::PLAYER_INCOMING_KB_FRAMES = 5;

float GameConstants::PLAYER_PARRY_TIME = 0.5f;

int GameConstants::PLAYER_STAMINA = 240; // should be a multiple of 3,4,5,6 to accomodate for evenly divisible by number of dashes
float GameConstants::PLAYER_DODGE_TIME = 1/6.0f;
float GameConstants::PLAYER_DODGE_SPEED = 14.0f;



float GameConstants::PLAYER_MOVE_SPEED = 4.7; //previously 4.6
float GameConstants::PLAYER_ATK_MOVE_SPEED = 1.8f; //previously 0.5



float GameConstants::PLAYER_MELEE_ATK_RANGE = 1.35f; // previously 1.4

float GameConstants::PROJ_SPEED_P = 9.0f;

float GameConstants::PROJ_SIZE_P_HALF = 1.0f;

float GameConstants::PROJ_DIST_P_SQ = 100.0f;

float GameConstants::PROJ_DIST_P = 7.0f;

float GameConstants::CHARGE_TIME = 0.3f;

float GameConstants::COMBO_TIME = 1.0f;

float GameConstants::HEALTHPACK_DROP_RATE = 34.0f;

float GameConstants::HEALTHPACK_HEAL_AMT = 0.25f;

float GameConstants::KNOCKBACK = 1.6f;

float GameConstants::KNOCKBACK_PWR_ATK = 4.7f; // previously 4.0

float GameConstants::PROJ_SHADOW_SCALE = 0.25f;
