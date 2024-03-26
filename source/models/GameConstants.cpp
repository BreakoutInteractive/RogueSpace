//
//  GameConstants.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/18/24.
//

#include <cugl/cugl.h>
#include "GameConstants.hpp"

#pragma mark -
#pragma mark Enemy

int GameConstants::ENEMY_IFRAME = 16;

int GameConstants::ENEMY_ATK_COOLDOWN = 120;

int GameConstants::ENEMY_STUN_COOLDOWN = 60;

int GameConstants::ENEMY_SENTRY_COOLDOWN = 120;

float GameConstants::ENEMY_SIGHT_RANGE = 8;

float GameConstants::ENEMY_MELEE_ATK_RANGE = 4;

float GameConstants::ENEMY_MOVE_SPEED = 2;

float GameConstants::PROJ_SPEED_E = 10.0f;

float GameConstants::PROJ_SIZE_E_HALF = 1.5f;

float GameConstants::PROJ_TIME_E = 1.0f;


#pragma mark -
#pragma mark Player

int GameConstants::PLAYER_IFRAME = 16;

int GameConstants::PLAYER_ATTACK_COOLDOWN =  16;

int GameConstants::PLAYER_PARRY_COOLDOWN = 6;

int GameConstants::PLAYER_DODGE_COOLDOWN = 60;

float GameConstants::PLAYER_DODGE_DURATION = 10;

int GameConstants::PLAYER_MAX_HP = 3;

float GameConstants::PLAYER_MELEE_ATK_RANGE = 4.0f;

float GameConstants::PROJ_SPEED_P = 10.0f;

float GameConstants::PROJ_SIZE_P_HALF = 1.5f;

float GameConstants::PROJ_TIME_P = 1.0f;