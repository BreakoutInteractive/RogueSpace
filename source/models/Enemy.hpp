//
//  Enemy.hpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#ifndef Enemy_hpp
#define Enemy_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "GameObject.hpp"
#include "LevelModel.hpp"
#include "GameConstants.hpp"

class Animation;

/**
 *  This class represents an enemy in the game.
 */
class Enemy : public GameObject {

protected:
#pragma mark - Health Bar Assets
    /** The enemy health bar background */
    std::shared_ptr<cugl::Texture> _healthBG;
    
    /** The enemy health bar foreground */
    std::shared_ptr<cugl::Texture> _healthFG;
    
    /** how tall the healthbar should be */
    float _healthbarHeight = 5;
    /** the healthbar at 1x multiplier is the width of the idle animation */
    float _healthbarWidthMultiplier = 1;
    /** the healthbar is drawn at the top of the sprite, this adds additional vertical space*/
    float _healthbarExtraOffsetY = 3;
    
#pragma mark - Animation Assets
    /** The animation to use while idle */
    std::shared_ptr<Animation> _idleAnimation;
    /** The animation to use while walking */
    std::shared_ptr<Animation> _walkAnimation;
    /** The animation to use while attacking */
    std::shared_ptr<Animation> _attackAnimation;

#pragma mark - Animation Effects
    /** The hit effect animation when hit by the melee attack */
    std::shared_ptr<Animation> _meleeHitEffect;
    /** The hit effect animation when hit by the ranged attack */
    std::shared_ptr<Animation> _bowHitEffect;
    /** The stun effect animation */
    std::shared_ptr<Animation> _stunEffect;
    /** The death effect animation */
    std::shared_ptr<Animation> _deathEffect;
    
    /** Enemy image's pixel height */
    int _pixelHeight;
    /** Enemy's sight range */
    float _sightRange;
    /** Enemy's proximity range */
    float _proximityRange;
    /** Whether this enemy can currently see the player */
    bool _playerInSight;
    /** The location this enemy will aggro to */
    cugl::Vec2 _aggroLoc;
    /** Enemy's attack range */
    float _attackRange;
    /** Enemy's movement speed */
    float _moveSpeed;
    /** The enemy's current health */
    float _health;
    /** The damage inflicted by this enemy */
    float _damage;
    /** The enemy's maximum health */
    float _maxHealth;
    
    /** The 8 directions ranging from front and going counter clockwise until front-right*/
    cugl::Vec2 _directions[8];
    /** The current direction the enemy is facing */
    cugl::Vec2 _facingDirection;
    /** the index of the 8-cardinal directions that most closely matches the direction the enemy faces*/
    int _directionIndex;
    
    /** The enemy's default state */
    std::string _defaultState;
    /** The enemy's patrol path */
    std::vector<cugl::Vec2> _path;
    /** The enemy's goal position */
    cugl::Vec2 _goal;
    /** Whether the enemy is aligned with the level grid */
    bool _isAligned;
    /** The enemy's goal path index */
    int _pathIndex;
    
public:
#pragma mark Counters
    
    Counter _atkCD;
    
    Counter _sentryCD;

    Counter _hitCounter;
    
    /** whether this enemy has tried to drop a health pack */
    bool _dropped;
    
    enum class BehaviorState: int {
        DEFAULT = 1,
        SEEKING = 2,
        CHASING = 3,
        ATTACKING = 4,
        STUNNED = 5,
        DYING = 6
    };
    
protected:
    
    /** internal enemy state (for animation, logic and triggering events) */
    BehaviorState _state;

public:    
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    Enemy(void) : GameObject() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~Enemy(void) { dispose(); }
    
    /**
     * Disposes all resources and assets
     *
     * Any assets owned by this object will be immediately released.
     * Requires initialization before next use.
     */
    void dispose();
    
    /**
     * Initializes a new enemy with the given data
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(std::shared_ptr<JsonValue> data);
    
    
#pragma mark Static Constructors
    
    /**
     * @return a newly allocated enemy with the given data
     */
    static std::shared_ptr<Enemy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<Enemy>();
        return (result->init(data) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
    
    
    /**
     * @return the sight range applied to this enemy
     */
    const float getSightRange() const { return _sightRange; }
    
    /**
     * @return the proximity range applied to this enemy.
     */
    const float getProximityRange() const { return _proximityRange; }
    
    /**
     * Returns this enemy's aggro location.
     */
    const cugl::Vec2 getAggroLoc() const { return _aggroLoc; }
    
    /**
     * Sets this enemy's aggro location.
     */
    void setAggroLoc(cugl::Vec2 value) { _aggroLoc = value; }
    
    /**
     * Returns the attack range applied to this enemy.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @return the force applied to this player.
     */
    const float getAttackRange() const { return _attackRange; }
    
    /**
     * Gets the movement speed of this enemy.
     */
    int getMoveSpeed() const { return _moveSpeed; }
    
    /**
     * @return the enemy's damage power
     */
    float getDamage() const { return _damage; }
    
    /**
     * Gets the current health of this enemy.
     */
    float getHealth() const { return _health; }
    
    /**
     * Sets the current health of this enemy.
     */
    void setHealth(float value) { _health = value; }
    
    /**
     * Gets the maximum health of this enemy.
     */
    float getMaxHealth() const { return _maxHealth; }
    
    /**
     * Sets the maximum health of this enemy.
     */
    void setMaxHealth(float value){ _maxHealth = value; }
    
    /**
     * Gets this enemy's default state.
     */
    std::string getDefaultState() const { return _defaultState; }
    
    /**
     * Sets this enemy's default state.
     */
    void setDefaultState(std::string value) { _defaultState = value; }
    
    /**
     * Gets this enemy's patrol path.
     */
    std::vector<cugl::Vec2> getPath() const { return _path; }
    
    /**
     * Sets this enemy's patrol path.
     */
    void setPath(std::vector<cugl::Vec2> value) { _path = value; }
    
    /**
     * Gets this enemy's goal position.
     */
    cugl::Vec2 getGoal() const { return _goal; }
    
    /**
     * Sets this enemy's goal position.
     */
    void setGoal(cugl::Vec2 value) { _goal = value; }
    
    /**
     * Returns whether this enemy is aligned with the level grid
     */
    bool getAligned() const { return _isAligned; }
    
    /**
     * Sets whether this enemy is aligned with the level grid
     */
    void setAligned(bool value) { _isAligned = value; }
    
    /**
     * Gets this enemy's goal path index.
     */
    int getPathIndex() const { return _pathIndex; }
    
    /**
     * Sets this enemy's goal path index.
     */
    void setPathIndex(int value) { _pathIndex = value; }
    
    /**
     * @return the unit vector direction that the enemy is facing towards
     */
    cugl::Vec2 getFacingDir() { return _facingDirection; }
    
    /**
     * Sets the direction that the enemy is currently facing
     */
    virtual void setFacingDir(cugl::Vec2 dir);
    
    /**
     * Gets whether this enemy can currently see the player
     */
    bool getPlayerInSight() const { return _playerInSight; }
    
    /**
     * Sets whether this enemy can currently see the player
     */
    void setPlayerInSight(bool value) { _playerInSight = value; }
    
    /**
     * Returns this enemy's type
     */
    virtual std::string getType() { return ""; };
    
    /**
     * @note derived classes must implement
     * @return whether this enemy can start an attack
     */
    virtual bool canBeginNewAttack(){ return true; }
    
    /**
     * @note this allows customization for whether a given enemy must be defeated, allowing
     * enabled enemies that are simultaneously defeated.
     * @return whether this enemy has been defeated
     */
    virtual bool isDefeated(){ return !isEnabled(); }
    
#pragma mark -
#pragma mark Physics
    
    virtual void attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets);
    
    
#pragma mark -
#pragma mark Animation and State
    
    /** get current enemy behavior state */
    BehaviorState getBehaviorState() { return _state; }
    
    /** Set idle state and change to using the idle animation */
    void setIdling();
    
    /** Set moving state and change to using the walk animation */
    void setMoving();
    
    /** Set attack state and change to using the attack animation */
    void setAttacking();
    
    /** Set stunned state */
    void setStunned();
    
    /** Set default state */
    void setDefault();
    
    /** Set seeking state */
    void setSeeking();
    
    /** Set chasing state */
    void setChasing();

    /** Set dying state */
    virtual void setDying();
    
    /**
     * whether enemy is attacking
     */
    bool isAttacking(){ return _attackAnimation->isActive() && _state == BehaviorState::ATTACKING; }
    /** whether enemy is stunned */
    bool isStunned(){ return _state == BehaviorState::STUNNED; }
    /** whether enemy is dying */
    virtual bool isDying() { return _deathEffect->isActive() && _state == BehaviorState::DYING; }
    
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    virtual void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /** Change to using the default animation */
    void animateDefault();
    
    /** Change to using the walk animation */
    void animateWalk();
    
    /** Change to using the attack animation */
    void animateAttack();

    /**
     * Method to call when an enemy is hit by an attack
     * @param atkDir the normal vector of the direction of the attack that hit this enemy
     * @param ranged whether the attack that hit this enemy was a ranged attack
     * @param damage how much damage this enemy takes
     * @param knockback_scl the factor to multiply the direction by for applying knockback
     */
    virtual void hit(cugl::Vec2 atkDir, bool ranged, float damage = 1, float knockback_scl = GameConstants::KNOCKBACK);

    /**
     * Method to call when an enemy is stunned, e.g. when parried
     */
    void stun();

    virtual void drawEffect(const std::shared_ptr<cugl::SpriteBatch>& batch, const std::shared_ptr<Animation>& effect, float scale = 1);
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
    void setDrawScale(cugl::Vec2 scale) override;

    void updateAnimation(float dt) override;


#pragma mark -
#pragma mark State Update
    void updateCounters();
};

#endif /* Enemy_hpp */
