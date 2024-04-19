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

class Animation;

/**
 * This class is the player object in this game.
 */
class Player : public GameObject {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(Player);

protected:

    /** The force applied to the player for general movement purposes */
    cugl::Vec2 _force;

    /** accumulated move buff*/
    double _moveScale;

    /** The texture key for the player*/
    std::string _playerTextureKey;
    /** The texture key for the parry animation */
    std::string _parryTextureKey;
    /** The texture key for the attack animation */
    std::string _attackTextureKey;
    
    //TODO: come up with a system that is similar to that of Unity's AnimationController, avoid field-member-blow-up
    /** The player texture*/
    std::shared_ptr<cugl::Texture> _playerTexture;
    
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
    /** The effect to use while charging the bow */
    std::shared_ptr<Animation> _chargingEffect;
    /** The effect to use while the bow is charged */
    std::shared_ptr<Animation> _chargedEffect;
    /** The effect to use when the bow is shot */
    std::shared_ptr<Animation> _shotEffect;
    /** The effect to use upon successfully parrying */
    std::shared_ptr<Animation> _parryEffect;
    
    /** The 8 directions ranging from front and going counter clockwise until front-right*/
    cugl::Vec2 _directions[8];
    
    /** The direction that the player is currently facing */
    cugl::Vec2 _facingDirection;
    
    /** the index of the 8-cardinal directions that most closely matches the direction the player faces*/
    int _directionIndex;
    
    std::shared_ptr<Animation> _prevAnimation;

    /** how long we have been dodging for */
    float _dodge;

    /**time since the last attack*/
    float _comboTimer;

    /** which step in the melee combo we are in */
    int _combo;
    
public:
#pragma mark -
    enum weapon { MELEE, RANGED };
    //TODO: modify more stuff (in particular, animation) to use states. Add hit and knockback states
    enum state {IDLE, ATTACK, CHARGING, CHARGED, SHOT, RECOVERY, PARRYSTART, PARRYSTANCE, PARRY, DODGE};
    weapon _weapon;
    state _state;
#pragma mark Counters
    /** attack cooldown counter*/
    Counter _atkCD;
    /** parry cooldown counter */
    Counter _parryCD;
    /** dodge cooldown counter*/
    Counter _dodgeCD;
    /** counter that is active during the dodge motion*/
    Counter _dodgeDuration;
    /** counter that is active while the player takes damage */
    Counter _hitCounter;
    /** defense upgrade*/
    std::shared_ptr<Upgradeable> defense;
    /** attack upgrade*/
    std::shared_ptr<Upgradeable> attack;
        
    std::vector<std::shared_ptr<Upgradeable>> attributes;

    float _hp;
    
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
     *
     * Any assets owned by this object will be immediately released.
     * Requires initialization before next use.
     */
    void dispose();
    
    
    /**
     * Initializes a newly allocated player
     *
     * The player size is specified in world coordinates.
     *
     * @param playerData  the structured json with player collision, hitbox, position data
     *
     * @return true if the player is initialized properly, false otherwise.
     */
    virtual bool init(std::shared_ptr<JsonValue> playerData);
    
    
#pragma mark Static Constructors
    
    /**
     * Returns a newly allocated player
     *
     * @param playerData the structured json with player collision, hitbox, position data
     *
     * @return a newly allocated player
     */
    static std::shared_ptr<Player> alloc(std::shared_ptr<JsonValue> playerData) {
        std::shared_ptr<Player> result = std::make_shared<Player>();
        return (result->init(playerData) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
    
    /**
    * Gets the movement boost accumulated by this player.
    *
    */
    int getMoveScale();
    
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
    
    std::vector<std::shared_ptr<Upgradeable>> getPlayerAttributes() {return attributes;}
    
    /**
     * Gets the attack strength accumulated by this player.
     */
    const int getAtkDamage() {return attack->getCurrentValue();}
    
    /**
     * Gets the attack strength accumulated by this player.
     */
    const int getDefense() {return defense->getCurrentValue();}
    
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
     * @return the unit vector direction that the player is facing towards
     */
    cugl::Vec2 getFacingDir(){ return _facingDirection; }
    
    /**
     * Sets the direction that the player is currently facing
     */
    void setFacingDir(cugl::Vec2 dir);
    
    /**
     * @return the maximum HP of the player
     */
    int getMaxHP();

    /**
     * @return which hit of the combo the player is on
     */
    int getCombo() const { return _combo; }

    bool isAttacking();

    void swapWeapon() { _weapon = static_cast<weapon>((_weapon + 1) % 2); }

#pragma mark -
#pragma mark Animation
 
    /**
    * Returns the idle texture (key) for this player
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @return the texture (key) for this player
    */
    const std::string& getTextureKey() const { return _playerTextureKey; }

    /**
    * Returns the idle texture (key) for this player
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @param  strip    the texture (key) for this player
    */
    void setTextureKey(const std::string& key) { _playerTextureKey = key; }

    /**
    * Returns the texture (key) for this player's parry animation
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @return the texture (key) for this player's parry animation
    */
    const std::string& getParryTextureKey() const { return _parryTextureKey; }

    /**
    * Returns the texture (key) for this player's parry animation
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @param  strip    the texture (key) for this player's parry animation
    */
    void setParryTextureKey(const std::string& key) { _parryTextureKey = key; }
    /**
    * Returns the texture (key) for this player's attack animation
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @return the texture (key) for this player's attack animation
    */
    const std::string& getAttackTextureKey() const { return _attackTextureKey; }

    /**
    * Returns the texture (key) for this player's attack animation
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @param  strip    the texture (key) for this player's attack animation
    */
    void setAttackTextureKey(const std::string& key) { _attackTextureKey = key; }
    
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets);

    /** Change to using the parry start animation and change state to PARRYSTART */
    void animateParryStart();
    /** Change to using the parry animation and change state to PARRY */
    void animateParry();
    /** Change to using the default (idle) animation and change state to IDLE */
    void animateDefault();
    /** Change to using the melee attack animation and change state to ATTACK */
    void animateAttack();
    /** Change to using the charging animation and change state to CHARGE. Also start the charging effect */
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
    void hit(cugl::Vec2 atkDir, int damage = 1, float knockback_scl = GameConstants::KNOCKBACK);
    
    // INHERITED
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    void setAnimation(std::shared_ptr<Animation> animation) override;
    void updateAnimation(float dt) override;
    
    void update(float dt);
#pragma mark -
#pragma mark Physics

};

#endif /* Player_hpp */
