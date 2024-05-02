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
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(Enemy);

protected:

    /** The force applied to the player for general movement purposes */
    cugl::Vec2 _force;

    /** The texture key for the enemy*/
    std::string _enemyTextureKey;
    
    /** The texture key for the walk animation*/
    std::string _walkTextureKey;
    
    /** The enemy texture*/
    std::shared_ptr<cugl::Texture> _enemyTexture;
    
    /** The enemy health bar background */
    std::shared_ptr<cugl::Texture> _healthBG;
    
    /** The enemy health bar foreground */
    std::shared_ptr<cugl::Texture> _healthFG;
    
    /** The animation to use while idle */
    std::shared_ptr<Animation> _idleAnimation;
    
    /** The animation to use while walking */
    std::shared_ptr<Animation> _walkAnimation;
    
    /** The animation to use while attacking */
    std::shared_ptr<Animation> _attackAnimation;
    /** The animation of the hitbox while attacking */
    std::shared_ptr<Animation> _hitboxAnimation;
    
    /** The animation to use while stunned */
    std::shared_ptr<Animation> _stunAnimation;

    /** The hit effect animation */
    std::shared_ptr<Animation> _hitEffect;
    /** The stun effect animation */
    std::shared_ptr<Animation> _stunEffect;
    
    std::shared_ptr<cugl::physics2::WheelObstacle> _attack;
    
    /** Enemy's sight range */
    float _sightRange;
    
    /** Enemy's proximity range */
    float _proximityRange;
    
    /** Whether this enemy can currently see the player */
    bool _playerInSight;
    
    /** The player's last seen location */
    cugl::Vec2 _playerLoc;
    
    /** Enemy's attack range */
    float _attackRange;
    
    /** Enemy's movement speed */
    float _moveSpeed;
    
    /** The enemy's current health */
    float _health;
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
    
    /** Whether the enemy is currently in its default state */
    bool _isDefault;
    
    /** Whether the enemy is aiming its ranged attack */
    bool _isAiming;
    
    /** Whether the enemy's ranged attack is charged */
    bool _isCharged;
    
    /** The enemy's patrol path */
    std::vector<cugl::Vec2> _path;
    
    /** The enemy's goal position */
    cugl::Vec2 _goal;
    
    /** Whether the enemy is aligned with the level grid */
    bool _isAligned;
    
    /** The enemy's goal path index */
    int _pathIndex;
    
    std::shared_ptr<Animation> _animation;
    
public:
#pragma mark Counters
    
    Counter _atkCD;
    
    Counter _stunCD; //todo: if stun animation exists, remove counter.
    
    Counter _sentryCD;

    Counter _hitCounter;
    
    enum class EnemyState : int {
        IDLE = 1,
        MOVING = 2,
        ATTACKING = 3,
        STUNNED = 4
    };
    
private:
    
    /** internal enemy state (for animation and triggering events) */
    EnemyState _state;

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
     * TODO: document this propertly
     *
     * @return a newly allocated player with the given position
     */
    static std::shared_ptr<Enemy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<Enemy>();
        return (result->init(data) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
    /**
     * Returns the sight range applied to this enemy.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @return the force applied to this player.
     */
    const float getSightRange() const { return _sightRange; }
    
    /**
     * Returns the proximity range applied to this enemy.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @return the force applied to this player.
     */
    const float getProximityRange() const { return _proximityRange; }
    
    /**
     * Returns this enemy's last known location of the player.
     */
    const cugl::Vec2 getPlayerLoc() const { return _playerLoc; }
    
    /**
     * Sets this enemy's last known location of the player.
     */
    void setPlayerLoc(cugl::Vec2 value) { _playerLoc = value; }
    
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
     * Returns the force applied to this player.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @return the force applied to this player.
     */
    const cugl::Vec2 getForce() const { return _force; }

    /**
     * Sets the force applied to this player.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @param value  the force applied to this player.
     */
    void setForce(const cugl::Vec2 value) { _force.set(value); }

    /**
     * Returns the x-component of the force applied to this player.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @return the x-component of the force applied to this player.
     */
    float getFX() const { return _force.x; }
    
    /**
     * Sets the x-component of the force applied to this player.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @param value the x-component of the force applied to this player.
     */
    void setFX(float value) { _force.x = value; }
    
    /**
     * Returns the y-component of the force applied to this player.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @return the y-component of the force applied to this player.
     */
    float getFY() const { return _force.y; }
    
    /**
     * Sets the x-component of the force applied to this player.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @param value the x-component of the force applied to this player.
     */
    void setFY(float value) { _force.y = value; }
    
    /**
     * Gets the movement speed of this enemy.
     */
    int getMoveSpeed() const { return _moveSpeed; }
    
    /**
     * Gets the current health of this enemy.
     */
    float getHealth() const { return _health; }
    
    /**
     * Sets the current health of this enemy.
     */
    void setHealth(float value) { _health = value; }
    
    /**
     * Sets the maximum health of this enemy.
     */
    void setMaxHealth(float value){ _maxHealth = value; }
    
    /**
     * Gets this enemy's attack hitbox.
     */
    std::shared_ptr<cugl::physics2::WheelObstacle> getAttack() const { return _attack; }
    
    /**
     * Sets this enemy's attack hitbox.
     */
    void setAttack(std::shared_ptr<cugl::physics2::WheelObstacle> value) { _attack = value; }
    
    /**
     * Gets this enemy's default state.
     */
    std::string getDefaultState() const { return _defaultState; }
    
    /**
     * Sets this enemy's default state.
     */
    void setDefaultState(std::string value) { _defaultState = value; }
    
    /**
     * Gets whether this enemy is in its default state
     */
    bool isDefault() const { return _isDefault; }
    
    /**
     * Sets whether this enemy is in its default state
     */
    void setDefault(bool value) { _isDefault = value; }
    
    /**
     * Gets whether this enemy's ranged attack is charged
     */
    bool getCharged() const { return _isCharged; }
    
    /**
     * Sets whether this enemy's ranged attack is charged
     */
    void setCharged(bool value) { _isCharged = value; }
    
    /**
     * Gets whether this enemy is aiming its ranged attack
     */
    bool getAiming() const { return _isAiming; }
    
    /**
     * Sets whether this enemy is aiming its ranged attack
     */
    void setAiming(bool value) { _isAiming = value; }
    
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
    
    
#pragma mark -
#pragma mark Physics
    
    virtual void attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets);
    
    
#pragma mark -
#pragma mark Animation and State

    void setHitboxAnimation(std::shared_ptr<Animation> animation) { _hitboxAnimation = animation; }
    std::shared_ptr<Animation> getHitboxAnimation() const { return _hitboxAnimation; }
    
    /** current enemy state  */
    EnemyState getState() { return _state;}
    
    /** Set idle state and change to using the idle animation */
    void setIdling();
    
    /** Set moving state and change to using the walk animation */
    void setMoving();
    
    /** Set attack state and change to using the attack animation */
    void setAttacking();
    
    /** Set stunned state and change to using the stunned animation */
    void setStunned();
    
    /**
     * whether enemy is attacking
     */
    bool isAttacking(){ return _state == EnemyState::ATTACKING; }
    /** whether enemy is stunned */
    bool isStunned(){ return _state == EnemyState::STUNNED; }
    
 
    /**
    * Returns the texture (key) for this player
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @return the texture (key) for this player
    */
    const std::string& getTextureKey() const { return _enemyTextureKey; }

    /**
    * Returns the texture (key) for this player
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @param  strip    the texture (key) for this player
    */
    void setTextureKey(const std::string& key) { _enemyTextureKey = key; }    
    
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
     * @param damage how much damage this enemy takes
     * @param knockback_scl the factor to multiply the direction by for applying knockback
     */
    virtual void hit(cugl::Vec2 atkDir, float damage = 1, float knockback_scl = GameConstants::KNOCKBACK);

    /**
     * Method to call when an enemy is stunned, e.g. when parried
     */
    void stun();
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
    void setDrawScale(cugl::Vec2 scale) override;

    void updateAnimation(float dt) override;
    
    
#pragma mark -
#pragma mark Physics
//    /**
//     * Applies the force to the body of this player
//     *
//     * This method should be called after the force attribute is set.
//     */
//    void applyForce();

#pragma mark -
#pragma mark State Update
    void updateCounters();
};

#endif /* Enemy_hpp */
