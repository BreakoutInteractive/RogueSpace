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
    
    /** accumulated defense buff*/
    double _defenseUpgrade;
    /** accumulated attack buff*/
    double _atkDamage;
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
    
    /** The animaton to use while idle */
    std::shared_ptr<Animation> _idleAnimation;
    /** The animation to use while parrying */
    std::shared_ptr<Animation> _parryAnimation;
    /** The animation to use while attacking */
    std::shared_ptr<Animation> _attackAnimation;
    /** The animation to use while running */
    std::shared_ptr<Animation> _runAnimation;
    
    /** The 8 directions ranging from front and going counter clockwise until front-right*/
    cugl::Vec2 _directions[8];
    
    /** The direction that the player is currently facing */
    cugl::Vec2 _facingDirection;
    
    /** the index of the 8-cardinal directions that most closely matches the direction the player faces*/
    int _directionIndex;
    
    std::shared_ptr<Animation> _prevAnimation;
    
public:
#pragma mark -
#pragma mark Counters
    /** attack cooldown counter*/
    Counter _atkCD;
    /** parry cooldown counter */
    Counter _parryCD;
    /** dodge ooldown counter*/
    Counter _dodgeCD;
    /** counter that is active during the dodge motion*/
    Counter _dodgeDuration;
    /** counter that is active while the player takes damage */
    Counter _hitCounter;

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
     * Initializes a new player at the origin.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() { return init(cugl::Vec2::ZERO,cugl::Size::ZERO); }
    
    /**
     * Initializes a new player with the given position and unit size.
     *
     * @param  pos  Initial position in world coordinates
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const cugl::Vec2 pos) { return init(pos,cugl::Size(1,1)); }
    
    /**
     * Initializes a new player with the given position and size.
     *
     * The player size is specified in world coordinates.
     *
     * @param  pos      Initial position in world coordinates
     * @param  size       The dimensions of the box.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const cugl::Vec2 pos, const cugl::Size size);
    
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocate player at the origin.
     *
     * The player is 1 unit by 1 unit in size. The player is scaled so that
     * 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @return a newly allocate player at the origin.
     */
    static std::shared_ptr<Player> alloc() {
        std::shared_ptr<Player> result = std::make_shared<Player>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated player with the given position
     *
     * The player is 1 unit by 1 unit in size. The player is scaled so that
     * 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     *
     * @return a newly allocated player with the given position
     */
    static std::shared_ptr<Player> alloc(const cugl::Vec2& pos) {
        std::shared_ptr<Player> result = std::make_shared<Player>();
        return (result->init(pos) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated player with the given position and size
     *
     * The player size is specified in world coordinates.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The dimensions of the box.
     *
     * @return a newly allocated player with the given position
     */
    static std::shared_ptr<Player> alloc(const cugl::Vec2 pos, const cugl::Size size) {
        std::shared_ptr<Player> result = std::make_shared<Player>();
        return (result->init(pos,size) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
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
     * Gets the movement boost accumulated by this player.
     *
     */
    const int getMoveScale() const {return _moveScale;}
    
    /**
     * Increments the movement boost by a fixed amount.
     *
     */
    void upgradeMoveSpeed() {_moveScale+=(float)1/15;}
    
    /**
     * Gets the attack strength accumulated by this player.
     *
     */
    const int getAtkDamage() const {return _atkDamage;}
    
    /**
     * Increments the attack damage  by a fixed amount.
     */
    void upgradeAtkDamage() {_atkDamage+=(float)1/20;}
    
    /**
     * Increments the defense boost by a fixed amount.
     *
     */
    void upgradeDefense(){_defenseUpgrade-=(float)1/16;}

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
     * @return the maximum HP of the player;
     */
    int getMaxHP();
    

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

    /** Change to using the parry animation */
    void animateParry();
    /** Change to using the default animation */
    void animateDefault();
    /** Change to using the attack animation */
    void animateAttack();

    /**
    * Method to call when player is hit by an attack
    * @param atkDir the normal vector of the direction of the attack that hit the player
    */
    void hit(cugl::Vec2 atkDir, int damage = 1);
    
    // INHERITED
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    void setAnimation(std::shared_ptr<Animation> animation) override;
    void updateAnimation(float dt) override;
    
#pragma mark -
#pragma mark Physics

};

#endif /* Player_hpp */
