//
//  GameConstants.hpp
//  RS
//
//  Created by Zhiyuan Chen on 3/18/24.
//

#ifndef GameConstants_hpp
#define GameConstants_hpp

/** classification of the different upgrade types*/
enum UpgradeType {SWORD, PARRY, ATK_SPEED, DASH, BOW, HEALTH, SHIELD, BLOCK};


#include <cugl/cugl.h>

/**
 Wrapper class to host many game constants used by the game entities.
 */
class GameConstants {
    
public:
    
#pragma mark -
#pragma mark Game Effects
    
    /** duration (seconds) for fade-in  between levels */
    static float TRANSITION_FADE_IN_TIME;
    /** duration (seconds) for fade-out  between levels */
    static float TRANSITION_FADE_OUT_TIME;
    /** the number of frames the critical hit-lag lasts */
    static int HIT_PAUSE_FRAMES;
    
#pragma mark -
#pragma mark Game Camera Constants
    
    /** starting camera speed */
    static float GAME_CAMERA_SPEED;
    /** how fast the camera can move when accelerating */
    static float GAME_CAMERA_MAX_SPEED;
    /** camera acceleration */
    static float GAME_CAMERA_ACCEL;
    /** camera deceleration */
    static float GAME_CAMERA_DECEL;
    /** the maximum zoom out */
    static float GAME_CAMERA_MAX_ZOOM_OUT;
    /** the speed at which the camera zooms out */
    static float GAME_CAMERA_ZOOM_SPEED;

#pragma mark -
#pragma mark Enemy Constants
    /** the number of invincibility frames for enemy to avoid getting multiple hits from the same attack*/
    static int ENEMY_IFRAME;
    /**the number of frames we wait before allowing another attack*/
    static int ENEMY_ATK_COOLDOWN;
    /**the number of frames a sentry will wait before rotating**/
    static int ENEMY_SENTRY_COOLDOWN;
    /**the sight range of an enemy**/
    static float ENEMY_SIGHT_RANGE;
    /** the proximity range of an enemy*/
    static float ENEMY_PROXIMITY_RANGE;
    /**the melee attack range of an enemy**/
    static float ENEMY_MELEE_ATK_RANGE;
    /**the ranged attack range of an enemy*/
    static float ENEMY_RANGED_ATK_RANGE;
    /**the melee attack speed of an enemy**/
    static float ENEMY_MELEE_ATK_SPEED;
    /**the ranged attack speed of an enemy*/
    static float ENEMY_RANGED_ATK_SPEED;
    /**the default movement speed for an enemy**/
    static float ENEMY_MOVE_SPEED;
    /** the speed of an enemy projectile */
    static float PROJ_SPEED_E;
    /** the radius of a lizard enemy projectile */
    static float PROJ_RADIUS_LIZARD;
    /** the radius of a mage enemy projectile */
    static float PROJ_RADIUS_MAGE;
    /** the radius of a boss enemy projectile */
    static float PROJ_RADIUS_BOSS;
    /** the distance the enemy projectile travels, not including the explosion animation */
    static float PROJ_DIST_E;
    /** the fraction of damage the tank enemy takes when not stunned */
    static float TANK_ENEMY_DR;
    /** the bonus damage multiplier applied to stunned enemies */
    static float STUN_DMG_BONUS;

#pragma mark -
#pragma mark Slime Constants
    /** the number of game frames before explosion for slime */
    static int EXPLODE_TIMER;
    /** radius of explosion */
    static float EXPLODE_RADIUS;
    /** range of detection from slime */
    static float EXPLODE_PROX_RANGE;
    
#pragma mark -
#pragma mark Boss Constants
    /** the number of seconds the storm takes to charge */
    static float STORM_CHARGE_TIME;
    /** the number of game frames the storm will last */
    static int STORM_TIMER;
    /** radius of storm */
    static float STORM_RADIUS;
    
#pragma mark -
#pragma mark Player Combat
    
    /** player initial HP */
    static float PLAYER_MAX_HP;

    /** the multiplier value applied to the bow damage stat when firing a projectile instantly (weakest) */
    static float MIN_PROJ_DMG_MULTIPLIER;
    /** the multiplier value applied to the bow damage stat when projectile is FULLY charged (strongest) */
    static float MAX_PROJ_DMG_MULTIPLIER;
    /** how many seconds it takes for the player to charge the ranged attack */
    static float CHARGE_TIME;
    /** the knockback dealt to the enemies when projectile is fully charged*/
    static float PLAYER_PROJ_KNOCKBACK;
    
    /** player melee attack range, radius of semicircle sweep */
    static float PLAYER_MELEE_ATK_RANGE;
    /** the maximum time before restting the combo */
    static float COMBO_TIME;
    /** the damage multiplier applied to the last hit of the combo */
    static float COMBO_DMG_MUL;
    /** the length of the parry animation */
    static float PLAYER_PARRY_TIME;
    
    
#pragma mark -
#pragma mark Remaining Player Constants
    /** the length of the stamina bar, also the number of frames needed to fully recharge the dodge meter (when not dodging) */
    static int PLAYER_STAMINA;
    
    /** the number of invincibility frames for player*/
    static int PLAYER_IFRAME;
    /** the number of frames to receive enemy knockback (must be strictly less than or equal to number of iframes) */
    static int PLAYER_INCOMING_KB_FRAMES;


    /** the number of seconds for the dodge effect to last*/
    static float PLAYER_DODGE_TIME;
    /** the speed of a dodging player*/
    static float PLAYER_DODGE_SPEED;
    /** player initial movement speed */
    static float PLAYER_MOVE_SPEED;
    /** player speed while melee attacking */
    static float PLAYER_ATK_MOVE_SPEED;
    

    /** the speed of a player projectile */
    static float PROJ_SPEED_P;
    /** half of the size of a player projectile */
    static float PROJ_SIZE_P_HALF;
    /** the distance the player projectile travels, not including the explosion animation */
    static float PROJ_DIST_P;


    /** The chance (out of 100) for a health pack to drop when defeating an enemy.
     *  If the value is <0, the health pack will never drop. If it is >=99, it will always drop.
     */
    static float HEALTHPACK_DROP_RATE;
    /** the proportion of the player's health that is restored when picking up a health pack */
    static float HEALTHPACK_HEAL_AMT;

#pragma mark -
#pragma mark Universal Constants
    /** default knockback multiplier of an attack - applies to player attacks, enemy attacks, and projectiles */
    static float KNOCKBACK;
    /** knockback multiplier of a power attack (combo hit 3) */
    static float KNOCKBACK_PWR_ATK;
    /**the scale of a projectile's shadow relative to its size*/
    static float PROJ_SHADOW_SCALE;
};


#endif /* GameConstants_hpp */
