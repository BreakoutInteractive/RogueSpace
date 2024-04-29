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

int GameConstants::ENEMY_ATK_COOLDOWN = 120;

int GameConstants::ENEMY_STUN_COOLDOWN = 60;

int GameConstants::ENEMY_SENTRY_COOLDOWN = 120;

float GameConstants::ENEMY_SIGHT_RANGE = 8;

float GameConstants::ENEMY_PROXIMITY_RANGE = 5;

float GameConstants::ENEMY_MELEE_ATK_RANGE = 1.5f; // previously 2

float GameConstants::ENEMY_RANGED_ATK_RANGE = 6;

float GameConstants::ENEMY_MOVE_SPEED = 2.3;

float GameConstants::PROJ_SPEED_E = 10.0f;

float GameConstants::PROJ_RADIUS_LIZARD = 1.0f;

float GameConstants::PROJ_RADIUS_MAGE = 0.75f;

float GameConstants::PROJ_TIME_E = 1.0f;

float GameConstants::PARRY_ENEMY_DR = 0.1f;


#pragma mark -
#pragma mark Player

int GameConstants::PLAYER_IFRAME = 16;

int GameConstants::PLAYER_ATTACK_COOLDOWN =  16;

float GameConstants::PLAYER_PARRY_TIME = 0.5f;

int GameConstants::PLAYER_DODGE_COOLDOWN = 60;
float GameConstants::PLAYER_DODGE_TIME = 1/6.0f;
float GameConstants::PLAYER_DODGE_SPEED = 20.0f;

float GameConstants::PLAYER_MAX_HP = 3;

float GameConstants::PLAYER_MOVE_SPEED = 4.5; // previously 5
float GameConstants::PLAYER_ATK_MOVE_SPEED = 0.5f;

float GameConstants::PLAYER_DEFENSE = 1;

float GameConstants::PLAYER_ATK_DAMAGE = 1;

float GameConstants::PLAYER_MELEE_ATK_RANGE = 1.3f; // previously 2

float GameConstants::PROJ_SPEED_P = 10.0f;

float GameConstants::PROJ_SIZE_P_HALF = 1.0f;

float GameConstants::PROJ_TIME_P = 1.0f;

float GameConstants::CHARGE_TIME = 0.3f;

float GameConstants::COMBO_TIME = 1.0f; // TO TEST

float GameConstants::KNOCKBACK = 4.0f; // previously 10

float GameConstants::KNOCKBACK_PWR_ATK = 8.0f; // previously 20

float GameConstants::PROJ_SHADOW_SCALE = 0.25f;
