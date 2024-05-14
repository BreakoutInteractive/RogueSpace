//
//  ExplodingAlien.hpp
//  RS
//

#ifndef ExplodingAlien_hpp
#define ExplodingAlien_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "Enemy.hpp"
#include "GameObject.hpp"
#include "../components/Animation.hpp"
#include "../components/Hitbox.hpp"

/**
 *  This class represents an explosive enemy. Upon death or aggression, the enemy will attack and die.
 */
class ExplodingAlien : public Enemy {

protected:
    /** The animation to use while idle */
    std::shared_ptr<Animation> _idleAnimationWhite;
    
    /** The animation to use while walking */
    std::shared_ptr<Animation> _walkAnimationWhite;
    
    /** The physics object used by this enemy's melee attack */
    std::shared_ptr<Hitbox> _attack;
    
    /** timer before explosion effect */
    Counter _windupCD;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    ExplodingAlien(void) : Enemy() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~ExplodingAlien(void) { dispose(); }
    
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
    virtual bool init(std::shared_ptr<JsonValue> data) override;
    
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
    static std::shared_ptr<ExplodingAlien> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<ExplodingAlien>();
        return (result->init(data) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
    
    std::string getType() override { return "exploding alien"; }
    
#pragma mark -
#pragma mark Physics
    
    void attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) override;
    
    std::shared_ptr<Hitbox> getAttack(){ return _attack; }
    
    void addObstaclesToWorld(std::shared_ptr<physics2::ObstacleWorld> world) override {
        GameObject::addObstaclesToWorld(world);
        world->addObstacle(_attack);
        _attack->setEnabled(false);
    }
    
    void removeObstaclesFromWorld(std::shared_ptr<physics2::ObstacleWorld> world) override{
        GameObject::removeObstaclesFromWorld(world);
        world->removeObstacle(_attack);
    }
    
    void setDebugNode(const std::shared_ptr<scene2::SceneNode> &debug) override {
        GameObject::setDebugNode(debug);
        _attack->setDebugScene(debug);
        _attack->setDebugColor(Color4::WHITE);
    }
    
#pragma mark -
#pragma mark Animation and State
    
    bool canBeginNewAttack() override {
        return !isAttacking();
    }
    
    /**
     * an explosive enemy can attack at all times when in proximity but an explicit countdown is required.
     * @return whether this enemy can start an explosion
     */
    bool canExplode(){ return _windupCD.isZero(); }
    
    /**
     * starts the attack animation on death, if the attack animation has not yet started.
     * The attack will follow up with an explosion.
     */
    void setDying() override;
    
    /**
     * increments or decrements explosion timer based on value
     * @param down true for decrement, false for increment.
     */
    void updateWindup(bool down);
    
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets) override;
    
    /**
     * Sets the direction that the enemy is currently facing
     */
    void setFacingDir(cugl::Vec2 dir) override;
    
    void updateAnimation(float dt) override;
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
};

#endif /* ExplodingAlien_hpp */
