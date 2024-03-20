//
//  GameConstants.hpp
//  RS
//
//  Created by Zhiyuan Chen on 3/18/24.
//

#ifndef GameConstants_hpp
#define GameConstants_hpp

#include <cugl/cugl.h>

/**
 Wrapper class to host many game constants used by the game entities.
 */
class GameConstants {
    
public:

#pragma mark -
#pragma mark Enemy Constants
    // should be at least the player's attack duration so that it can't get hit twice by the same attack
    /** the number of invincibility frames for enemy to avoid getting multiple hits from the same attack*/
    static int ENEMY_IFRAME;
    /**the number of frames we wait before allowing another attack*/
    static int ENEMY_ATK_COOLDOWN;
    /**the number of frames an enemy will be stunned*/
    static int ENEMY_STUN_COOLDOWN;
    /**the number of frames a sentry will wait before rotating**/
    static int ENEMY_SENTRY_COOLDOWN;
    /**the sight range of an enemy**/
    static float ENEMY_SIGHT_RANGE;
    /** the proximity range of an enemy*/
    static float ENEMY_PROXIMITY_RANGE;
    /**the melee attack range of an enemy**/
    static float ENEMY_MELEE_ATK_RANGE;
    /**the default movement speed for an enemy**/
    static float ENEMY_MOVE_SPEED;

#pragma mark -
#pragma mark Player Constants
    
    //should be at least the enemy's attack time so that we can't get hit twice by the same attack
    /** the number of invincibility frames for player to avoid getting multiple hits from the same attack*/
    static int PLAYER_IFRAME;
    
    /**
     the number of frames we wait before allowing another attack, also currently the length of the attack
    @note this should be deprecated because attack duration is set by player attack animation speed, unless we want explicit attack cooldown.
     */
    static int PLAYER_ATTACK_COOLDOWN;
    /**the number of frames we wait before allowing another parry, also currently the length of the parry*/
    static int PLAYER_PARRY_COOLDOWN;
    /**the number of frames we wait before allowing another dodge*/
    static int PLAYER_DODGE_COOLDOWN;
    /** the number of frames for the dodge effect to last*/
    static float PLAYER_DODGE_DURATION;
    /** player initial HP */
    static int PLAYER_MAX_HP;
    /** player melee attack range
     
     the radius of a melee attack. sweeps out a semicircle with this radius (in physics coordinates) centered at the center of the player.
     
     this is how sweeping melee attacks work in Hades
     */
    static float PLAYER_MELEE_ATK_RANGE;
};


#endif /* GameConstants_hpp */
