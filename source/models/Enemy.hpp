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

/**
 * This class represents an enemy in the game.
 */
class Enemy : public cugl::physics2::BoxObstacle {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(Enemy);

protected:

    /** The force applied to the player for general movement purposes */
    cugl::Vec2 _force;

    /** The texture key for the player*/
    std::string _textureKey;
    
    /** Cache object for transforming the force according the object angle */
    cugl::Mat4 _affine;
    
    cugl::Vec2 _drawScale;
    
    /** The player texture*/
    std::shared_ptr<cugl::Texture> _texture;

    cugl::Color4 _tint;

    Counter _hitCounter;
    
    std::shared_ptr<cugl::physics2::WheelObstacle> _attack;
    
    /** Enemy's sight range */
    float _range;
    
    /** The enemy's current health */
    int _health;
    
    /** The current direction the enemy is facing */
    cugl::Vec2 _facingDirection;
    
    /** The enemy's default state */
    std::string _defaultState;
    
    /** The enemy's patrol path */
    std::vector<cugl::Vec2> _path;
    
public:
#pragma mark Counters
    
    Counter _atkLength;
    
    Counter _atkCD;
    
    Counter _stunCD;
    
    Counter _sentryCD;
    
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    Enemy(void) : BoxObstacle(), _drawScale(1.0f, 1.0f) { }
    
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
     * Initializes a new player with the given position and size.
     *
     * The player size is specified in world coordinates.
     *
     * @param  pos      Initial position in world coordinates
     * @param  size       The dimensions of the box.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const cugl::Vec2 pos, const cugl::Size size) override;
    
    
#pragma mark Static Constructors
    
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
    static std::shared_ptr<Enemy> alloc(const cugl::Vec2 pos, const cugl::Size size) {
        auto result = std::make_shared<Enemy>();
        return (result->init(pos,size) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
    /**
     * Returns the sight range applied to this player.
     *
     * Remember to modify the input values by the thrust amount before assigning
     * the value to force.
     *
     * @return the force applied to this player.
     */
    const float getRange() const { return _range; }
    
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
     * Gets the current health of this enemy.
     */
    int getHealth() const { return _health; }
    
    /**
     * Sets the current health of this enemy.
     */
    void setHealth(int value) { _health = value; }
    
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
     * Gets this enemy's patrol path.
     */
    std::vector<cugl::Vec2> getPath() const { return _path; }
    
    /**
     * Sets this enemy's patrol path.
     */
    void setPath(std::vector<cugl::Vec2> value) { _path = value; }
    
    /**
     * @return the unit vector direction that the enemy is facing towards
     */
    cugl::Vec2 getFacingDir() { return _facingDirection; }
    
    /**
     * Sets the direction that the enemy is currently facing
     */
    void setFacingDir(cugl::Vec2 dir) { _facingDirection = dir; };
    
#pragma mark -
#pragma mark Animation
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
 
    /**
    * Returns the texture (key) for this player
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @return the texture (key) for this player
    */
    const std::string& getTextureKey() const { return _textureKey; }

    /**
    * Returns the texture (key) for this player
    *
    * The value returned is not a Texture2D value.  Instead, it is a key for
    * accessing the texture from the asset manager.
    *
    * @param  strip    the texture (key) for this player
    */
    void setTextureKey(const std::string& key) { _textureKey = key; }
    
    /**
     * Sets the ratio of the player sprite to the physics body
     *
     * The player needs this value to convert correctly between the physics
     * coordinates and the drawing screen coordinates.  Otherwise it will
     * interpret one Box2D unit as one pixel.
     *
     * All physics scaling must be uniform.  Rotation does weird things when
     * attempting to scale physics by a non-uniform factor.
     *
     * @param scale The ratio of the player sprite to the physics body
     */
    void setDrawScale(cugl::Vec2 scale);
    
    /**
     * Returns the ratio of the player sprite to the physics body
     *
     * The player needs this value to convert correctly between the physics
     * coordinates and the drawing screen coordinates.  Otherwise it will
     * interpret one Box2D unit as one pixel.
     *
     * All physics scaling must be uniform.  Rotation does weird things when
     * attempting to scale physics by a non-uniform factor.
     *
     * @return the ratio of the player sprite to the physics body
     */
    cugl::Vec2 getDrawScale() const { return _drawScale; }
    
    
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets);

    /**
     * Method to call when an enemy is hit by an attack
     */
    void hit();
    
    
#pragma mark -
#pragma mark Physics
    /**
     * Applies the force to the body of this player
     *
     * This method should be called after the force attribute is set.
     */
    void applyForce();

#pragma mark -
#pragma mark State Update
    void updateCounters();
};

#endif /* Enemy_hpp */
