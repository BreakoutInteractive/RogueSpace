//
//  Player.hpp
//  RS
//
//  Created by Zhiyuan Chen on 2/23/24.
//

#ifndef Player_hpp
#define Player_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "GameObject.hpp"
#include "Upgradeable.hpp"
#include "GameConstants.hpp"

using namespace cugl;

class Animation;

/**
 * This class represents the player's melee attack hitbox. During the timeframe between the activation of this hitbox, it records whether there is a successful hit of any opponent to allow the player to perform combo hits.
 * This is a separate component attached to the player and the player model is responsible for its physics.
 */
class PlayerHitbox : public physics2::WheelObstacle {
    
public:
    
    /** whether the hitbox hit any targets */
    bool hitFlag;
    
    /**
     * Creates a new hitbox located at the world origin
     */
    PlayerHitbox(void): physics2::WheelObstacle(), hitFlag(false) {}
    
    /**
     * Returns a new hitbox arc of the given radius.
     *
     * @param  pos      Initial position in world coordinates
     * @param  radius   The hitbox radius
     *
     * @return a new hitbox object of the given radius.
     */
    static std::shared_ptr<PlayerHitbox> alloc(const Vec2 pos, float radius) {
        std::shared_ptr<PlayerHitbox> result = std::make_shared<PlayerHitbox>();
        return (result->init(pos,radius) ? result : nullptr);
    }
    
    /**
     * Sets whether the body is enabled and clears the hit flag.
     * Any processing of the hitflag has to be done between calls to this method before the truth value is cleared.
     *
     * @param value whether the body is enabled
     */
    void setEnabled(bool value) override {
        WheelObstacle::setEnabled(value);
        hitFlag = false; // clear the flag
    }
    
};

/**
 * This class is the player object in this game.
 */
class Player : public GameObject {

protected:
#pragma mark Player Animation States

    /** previous animation (animation state tracker) */
    std::shared_ptr<Animation> _prevAnimation;
    //TODO: there are a lot of these, maybe put them in a hash table with key = animation name
    /** The animaton to use while idle */
    std::shared_ptr<Animation> _idleAnimation;
    /** The animation to use during parry startup */
    std::shared_ptr<Animation> _parryStartAnimation;
    /** The animation to use while holding the parry stance */
    std::shared_ptr<Animation> _parryStanceAnimation;
    /** The animation to use while parrying */
    std::shared_ptr<Animation> _parryAnimation;
    /** The animation to use for combo hit 1 of the melee attack */
    std::shared_ptr<Animation> _attackAnimation1;
    /** The animation to use for combo hit 2 of the melee attack */
    std::shared_ptr<Animation> _attackAnimation2;
    /** The animation to use for combo hit 3 of the melee attack */
    std::shared_ptr<Animation> _attackAnimation3;
    /** The animation to use while running */
    std::shared_ptr<Animation> _runAnimation;
    /** The animaton to use while idle and using the bow */
    std::shared_ptr<Animation> _bowIdleAnimation;
    /** The animation to use while charging the bow */
    std::shared_ptr<Animation> _chargingAnimation;
    /** The animation to use while the bow is charged */
    std::shared_ptr<Animation> _chargedAnimation;
    /** The animation to use when the bow is shot */
    std::shared_ptr<Animation> _shotAnimation;
    /** The animation to use when recovering from shooting the bow */
    std::shared_ptr<Animation> _recoveryAnimation;
    /** The animation to use while running and using the bow*/
    std::shared_ptr<Animation> _bowRunAnimation;
    
#pragma mark Effects
    /** The effect to use while performing combo melee attack */
    std::shared_ptr<Animation> _comboSwipeEffect;
    /** The effect to use while performing melee attack */
    std::shared_ptr<Animation> _swipeEffect;
    /** The effect to use while charging the bow */
    std::shared_ptr<Animation> _chargingEffect;
    /** The effect to use while the bow is charged */
    std::shared_ptr<Animation> _chargedEffect;
    /** The effect to use when the bow is shot */
    std::shared_ptr<Animation> _shotEffect;
    /** The effect to use upon successfully parrying */
    std::shared_ptr<Animation> _parryEffect;
    
#pragma mark Misc Internal State

    /** The 8 directions ranging from front and going counter clockwise until front-right*/
    Vec2 _directions[8];
    /** The direction that the player is currently facing */
    Vec2 _facingDirection;
    /** the index of the 8-cardinal directions that most closely matches the direction the player faces*/
    int _directionIndex;

    /** accumulated move buff*/
    double _moveScale;
    
    /** how long we have been dodging for */
    float _dodge;
    
#pragma mark Melee Attack State

    /**time since the last attack*/
    float _comboTimer;
    /** which step in the melee combo we are in */
    int _combo;
    /** player melee hitbox (semi-circle) */
    std::shared_ptr<PlayerHitbox> _meleeHitbox;
    
public:
#pragma mark Player States

    enum weapon { MELEE, RANGED };
    enum state {IDLE, ATTACK, CHARGING, CHARGED, SHOT, RECOVERY, PARRYSTART, PARRYSTANCE, PARRY, DODGE};
    weapon _weapon;
    state _state;
    
    float _hp;

#pragma mark Counters

    /** attack cooldown counter*/
    Counter atkCD; //TODO: possibly deprecated
    /** dodge cooldown counter*/
    Counter dodgeCD;
    /** counter that is active while the player takes damage */
    Counter hitCounter;
    
    /**
     * decrement all counters
     */
    void updateCounters();

#pragma mark Constructors

    /**
     * Creates a new player at the origin.
     */
    Player(void) : GameObject() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~Player(void) { dispose(); }
    
    /**
     * Disposes all resources and assets
     */
    void dispose();
    
    /**
     * Initializes a newly allocated player
     *
     * The player size is specified in world coordinates.
     * @param playerData  the structured json with player collision, hitbox, position data
     * @return true if the player is initialized properly, false otherwise.
     */
    virtual bool init(std::shared_ptr<JsonValue> playerData, std::shared_ptr<cugl::physics2::ObstacleWorld> world);
    
    /**
     * Returns a newly allocated player
     *
     * @param playerData the structured json with player collision, hitbox, position data
     *
     * @return a newly allocated player
     */
    static std::shared_ptr<Player> alloc(std::shared_ptr<JsonValue> playerData, std::shared_ptr<cugl::physics2::ObstacleWorld> world) {
        std::shared_ptr<Player> result = std::make_shared<Player>();
        return (result->init(playerData, world) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Player Stats Accessors
    
    /**
    * Gets the movement boost accumulated by this player.
    *
    */
    int getMoveScale();
    /**
     * @return the maximum HP of the player
     */
    int getMaxHP();

    /** damage done by melee*/
    float meleeDamage = GameConstants::PLAYER_ATK_DAMAGE;
    /** percentage of how much damage gets taken  */
    float defense = GameConstants::PLAYER_DEFENSE;
    /** size of parry window*/
    float parryWindow = 1; //unimplemented
    /** speed of melee attack  */
    float atkSpeed = 1; //unimplemented
    /** bow strength stat*/
    float bowDamage = GameConstants::PROJ_DAMAGE_P;
    /** 
     * Damage of the ranged attack. It is 0.5x the bow strength stat at 
     * minimum charge and 1.5x the bow strength stat at maximum charge. The
     * damage is linearly interpolated between these two extremes. If called
     * when the player is not in the CHARGING or CHARGED states, it returns
     * the bow damage stat.
     * @return the damage of the ranged attack
     */
    float getBowDamage();
    
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
#pragma mark -
#pragma mark Player Game State Accessors
    /**
     * @return the unit vector direction that the player is facing towards
     */
    Vec2 getFacingDir(){ return _facingDirection; }
    
    /**
     * Sets the direction that the player is currently facing
     */
    void setFacingDir(Vec2 dir);
    
    /**
     * To be explicit, player is attacking when charging a bow / having charged a bow / striking with melee / firing
     * @return whether the player is attacking, includes preparing an attack.
     */
    bool isAttacking();

    /**
     * switches the weapon of the player from melee to range or vice versa.
     */
    void swapWeapon() { _weapon = static_cast<weapon>((_weapon + 1) % 2); }
    
    /**
     * resets the combo hit to be the first hit
     */
    void resetCombo() { _combo = 1; }
    
    /**
     * advance combo by 1 (if at end of combo, this resets)
     */
    void accumulateCombo() { _combo += 1; if (_combo > 3){ _combo = 1;} }
    
    /**
     * @return whether the current melee attack is a combo hit
     */
    bool isComboStrike() const { return _combo == 3; }

#pragma mark -
#pragma mark Animation
 
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<AssetManager>& assets);

    /** Change to using the parry start animation and change state to PARRYSTART */
    void animateParryStart();
    /** Change to using the parry animation and change state to PARRY */
    void animateParry();
    /** Change to using the default (idle/running) animation and change state to IDLE */
    void animateDefault();
    /** Change to using the melee attack animation and change state to ATTACK */
    void animateAttack();
    /** Change to using the charging animation and change state to CHARGING. Also start the charging effect */
    void animateCharge();
    /** Change to using the shooting animation and change state to SHOT */
    void animateShot();
    /** Start playing the parry effect. Should only be called when the player successfully parries */
    void playParryEffect();

    /**
    * Method to call when player is hit by an attack
    * @param atkDir the normal vector of the direction of the attack that hit the player
    * @param damage how much damage the player takes
    * @param knockback_scl the factor to multiply the direction by for applying knockback
    */
    void hit(Vec2 atkDir, float damage = 1, float knockback_scl = GameConstants::KNOCKBACK);
    void drawRangeIndicator(const std::shared_ptr<SpriteBatch>& batch);
    void draw(const std::shared_ptr<SpriteBatch>& batch) override;
    void setAnimation(std::shared_ptr<Animation> animation) override;
    void updateAnimation(float dt) override;
    
    void update(float dt);
    
#pragma mark -
#pragma mark Physics
    
    /**
     * @return reference to the player melee hitbox
     */
    std::shared_ptr<PlayerHitbox> getMeleeHitbox(){ return _meleeHitbox; }
    
    /**
     * enables the melee attack hitbox
     *
     * @param angle the attack angle/direction
     */
    void enableMeleeAttack(float angle);
    
    /**
     * turns off the melee attack hitbox
     */
    void disableMeleeAttack(){ _meleeHitbox->setEnabled(false); }
    
    void addObstaclesToWorld(std::shared_ptr<physics2::ObstacleWorld> world) override;
    
    void removeObstaclesFromWorld(std::shared_ptr<physics2::ObstacleWorld> world) override;

    void setDebugNode(const std::shared_ptr<scene2::SceneNode> &debug) override;
    
    void syncPositions() override;
};

    

#endif /* Player_hpp */
