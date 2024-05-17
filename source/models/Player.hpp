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
#include "../components/Hitbox.hpp"

using namespace cugl;

class Animation;

/**
 * This class is the player object in this game.
 */
class Player : public GameObject {
    
public:
    enum Weapon { MELEE, RANGED };
    enum State {IDLE, ATTACK, CHARGING, CHARGED, SHOT, RECOVERY, PARRYSTART, PARRYSTANCE, PARRY, DODGE, DYING, DEAD};

protected:
#pragma mark -
#pragma mark Player Animation Assets

    /** previous animation (animation state tracker) */
    std::shared_ptr<Animation> _prevAnimation;
    
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
    /** The effect to use when hit */
    std::shared_ptr<Animation> _hitEffect;
    /** The effect to use when dead */
    std::shared_ptr<Animation> _deathEffect;
    
#pragma mark -
#pragma mark Player Internal State
    /** The 8 directions ranging from front and going counter clockwise until front-right*/
    Vec2 _directions[8];
    /** The direction that the player is currently facing */
    Vec2 _facingDirection;
    /** the index of the 8-cardinal directions that most closely matches the direction the player faces*/
    int _directionIndex;
    
    /** how long we have been dodging for */
    float _dodgeDuration;
    /** how long before the next melee attack is enabled*/
    float _attackActiveCooldown;
    /** the current accumulated stamina */
    float _stamina;
    /** player health */
    float _hp = GameConstants::PLAYER_MAX_HP;
    
    /** player's active weapon */
    Weapon _weapon;
    /** player state */
    State _state;

    /** counter that is active while the player takes damage */
    Counter _iframeCounter;
    /** decrement all counters */
    void updateCounters();
    
#pragma mark Player Stats
    /** player's maximum health*/
    Upgradeable _maxHP;
    /** damage done by melee */
    Upgradeable _meleeDamage;
    /** passive damage reduction */
    Upgradeable _damageReduction;
    /** block-mode damage reduction */
    Upgradeable _blockReduction;
    /** duration of parry stun */
    Upgradeable _stunWindow;
    /** bow strength stat */
    Upgradeable _bowDamage;
    /** number of repeated dodges available */
    Upgradeable _dodgeCount;
    /** delay between each melee attack */
    Upgradeable _attackCooldown;
    
#pragma mark Melee Attack

    /** time since the last attack*/
    float _comboTimer;
    /** which step in the melee combo we are in */
    int _combo;
    /** player melee hitbox (semi-circle) */
    std::shared_ptr<SemiCircleHitbox> _meleeHitbox;

public:
#pragma mark -
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
    virtual bool init(std::shared_ptr<JsonValue> playerData, std::shared_ptr<JsonValue> upgradesJson);
    
    /**
     * Returns a newly allocated player
     *
     * @param playerData the structured json with player collision, hitbox, position data
     *
     * @return a newly allocated player
     */
    static std::shared_ptr<Player> alloc(std::shared_ptr<JsonValue> playerData, std::shared_ptr<JsonValue> upgradeJson) {
        std::shared_ptr<Player> result = std::make_shared<Player>();
        return (result->init(playerData, upgradeJson) ? result : nullptr);
    }

#pragma mark -
    
#pragma mark Player Stats
    /** @return the maximum HP of the player */
    float getMaxHP(){ return _maxHP.getCurrentValue(); }
    Upgradeable getHPUpgrade(){ return _maxHP; }
    void setMaxHPLevel(int level){
        CUAssertLog(level >= 0 && level <= 5, "level must be within [0,5]");
        _maxHP.setCurrentLevel(level);
    }
    
    /** @return player current HP */
    float getHP() { return _hp; }
    void setHP(float hp){
        CUAssertLog(hp >= 0, "hp cannot be negative");
        CUAssertLog(hp <= _maxHP.getCurrentValue(), "hp cannot be greater than max hp");
        _hp = hp;
    }
    
    /** @return player passive damage reduction */
    float getDamageReduction(){ return _damageReduction.getCurrentValue(); }
    Upgradeable getDamageReductionUpgrade(){ return _damageReduction; }
    void setArmorLevel(int level){
        CUAssertLog(level >= 0 && level <= 5, "level must be within [0,5]");
        _damageReduction.setCurrentLevel(level);
    }
    
    /** @return player passive damage reduction */
    float getBlockingDamageReduction(){ return _blockReduction.getCurrentValue(); }
    Upgradeable getBlockingUpgrade(){ return _blockReduction; }
    void setBlockLevel(int level){
        CUAssertLog(level >= 0 && level <= 5, "level must be within [0,5]");
        _blockReduction.setCurrentLevel(level);
    }
    
    /** @return number of allowed consecutive dodges */
    int getDodgeCounts(){ return _dodgeCount.getCurrentValue();}
    Upgradeable getDodgeUpgrade(){ return _dodgeCount; }
    void setDodgeLevel(int level){
        CUAssertLog(level >= 0 && level <= 5, "level must be within [0,5]");
        _dodgeCount.setCurrentLevel(level);
    }
    
    /** @return the damage dealt by the player's sword */
    float getMeleeDamage(){ return _meleeDamage.getCurrentValue();}
    Upgradeable getMeleeUpgrade(){ return _meleeDamage; }
    void setMeleeLevel(int level){
        CUAssertLog(level >= 0 && level <= 5, "level must be within [0,5]");
        _meleeDamage.setCurrentLevel(level);
    }
    
    float getMeleeSpeed(){ return _attackCooldown.getCurrentValue();}
    Upgradeable getMeleeSpeedUpgrade(){ return _attackCooldown; }
    void setMeleeSpeedLevel(int level){
        CUAssertLog(level >= 0 && level <= 5, "level must be within [0,5]");
        _attackCooldown.setCurrentLevel(level);
    }
    
    /**
     * Damage of the ranged attack. It is 0.5x the bow strength stat at
     * minimum charge and 1.5x the bow strength stat at maximum charge. The
     * damage is linearly interpolated between these two extremes. If called
     * when the player is not in the CHARGING or CHARGED states, it returns
     * the bow damage stat.
     * @return the damage of the ranged attack
     */
    float getBowDamage();
    Upgradeable getBowUpgrade(){ return _bowDamage; }
    /** sets the bow damage stat */
    void setBowLevel(int level ){
        CUAssertLog(level >= 0 && level <= 5, "level must be within [0,5]");
        _bowDamage.setCurrentLevel(level);
    }
    
    float getStunWindow() { return _stunWindow.getCurrentValue(); }
    Upgradeable getStunUpgrade() { return _stunWindow; }
    void setStunLevel(int level){ _stunWindow.setCurrentLevel(level); }

    
    int getIframes(){ return _iframeCounter.getCount(); }
    
    /** @return whether the player can start a new melee attack (implies no cooldown active and weapon is sword)*/
    bool canMeleeAttack(){ return _weapon == MELEE && _attackActiveCooldown == 0;}
    /** resets the melee attack cooldown  */
    void resetAttackCooldown(){ _attackActiveCooldown = _attackCooldown.getCurrentValue(); }
    
    /** @return player stamina (integer count) */
    float getStamina(){ return _stamina; }
    /** @return whether this player has enough stamina to activate dodge */
    bool canDodge(){ return _stamina >= 1.0/_dodgeCount.getCurrentValue() * GameConstants::PLAYER_STAMINA; }
    /**
     * reduces the player stamina by the equivalence of one single dodge
     * @pre `canDodge()` must return true.
     */
    void reduceStamina(){ _stamina -= 1.0/_dodgeCount.getCurrentValue() * GameConstants::PLAYER_STAMINA; }
    
    
#pragma mark Player Animation State
    /** @return the unit vector direction that the player is facing towards */
    Vec2 getFacingDir(){ return _facingDirection; }
    /** Sets the direction that the player is currently facing */
    void setFacingDir(Vec2 dir);
    
    bool isIdle(){ return _state == IDLE; }
    bool isMeleeAttacking(){ return _state == ATTACK; }
    bool isRecovering(){ return _state == RECOVERY; }
    bool isParrying(){ return _state == PARRY; }
    bool isDodging(){ return _state == DODGE; }
    void setDodging(){ _state = DODGE; }
    
    /**
     * To be explicit, player is attacking when charging a bow / having charged a bow / striking with melee / firing
     * @return whether the player is attacking, includes preparing an attack.
     */
    bool isAttacking(){
        return _state == CHARGING || _state == CHARGED || _state == SHOT || _state == ATTACK;
    }
    
    /** @return whether the player has started to get into parry stance or in a parry stance BEFORE releasing the parry */
    bool isBlocking(){ return _state == PARRYSTART || _state == PARRYSTANCE; }
    
    /** @return whether the player is charging or have charged the bow */
    bool isRangedAttackActive(){ return _state == CHARGING || _state == CHARGED; }
    /** @return whether the bow is fully charged */
    bool isCharged(){ return _state == CHARGED; }

#pragma mark Player Combat State
    
    Weapon getWeapon(){ return _weapon; }
    void setWeapon(Weapon weapon){ _weapon = weapon; }
    
    /** switches the weapon of the player from melee to range or vice versa.  */
    void swapWeapon() { _weapon = static_cast<Weapon>((_weapon + 1) % 2); }
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
#pragma mark Animation Functions
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
    /** Play the death effect and change state to DYING */
    void animateDying();
    /** Start playing the parry effect. Should only be called when the player successfully parries */
    void playParryEffect();

    /**
    * Method to call when player is hit by an attack
    * @param atkDir the normal vector of the direction of the attack that hit the player
    * @param damage how much damage the player takes
    * @param knockback_scl the factor to multiply the direction by for applying knockback
    */
    void hit(Vec2 atkDir, float damage = 1, float knockback_scl = GameConstants::KNOCKBACK);
    void drawRangeIndicator(const std::shared_ptr<SpriteBatch>& batch, const std::shared_ptr<cugl::physics2::ObstacleWorld>& world);
    void drawEffect(const std::shared_ptr<SpriteBatch>& batch, const std::shared_ptr<Animation>& effect, float ang = 0, float scale = 1);
    void draw(const std::shared_ptr<SpriteBatch>& batch) override;
    void setAnimation(std::shared_ptr<Animation> animation) override;
    void updateAnimation(float dt) override;
    
    /**
     * updates internal player states (eg. timers) and switches player out of time-dependent states (eg. dodging).
     */
    void update(float dt);
    
#pragma mark -
#pragma mark Physics
    
    /**
     * @return reference to the player melee hitbox
     */
    std::shared_ptr<SemiCircleHitbox> getMeleeHitbox(){ return _meleeHitbox; }
    
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
