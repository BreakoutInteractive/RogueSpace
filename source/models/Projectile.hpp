#ifndef Projectile_hpp
#define Projectile_hpp

#include <cugl/cugl.h>
#include "GameObject.hpp"
#include "CollisionConstants.hpp"
#include "GameConstants.hpp"
using namespace cugl;
class Projectile : public GameObject {
private:
	bool playerInit(Vec2 pos, float damage, bool charged, float ang, const std::shared_ptr<AssetManager>& assets);
	bool lizardInit(Vec2 pos, float damage, float ang, const std::shared_ptr<AssetManager>& assets);
	bool mageInit(Vec2 pos, float damage, float ang, const std::shared_ptr<AssetManager>& assets);
    bool bossInit(Vec2 pos, float damage, float ang, const std::shared_ptr<AssetManager>& assets);
	std::shared_ptr<Animation> _flyingAnimation;
	std::shared_ptr<Animation> _explodingAnimation;
	enum state { FLYING, EXPLODING };
	state _state;
	float _damage;
	Vec2 _initPos;
    /**
     * whether the projectile was spawn from maximum charge
     * @note this will be true for enemies.
     */
    bool _isFullyCharged;
public:

	/**
	 * Creates a new player projectile.
	 *
	 * @param pos The position at which to spawn the projectile. This should be the player's position
	 * @param damage The damage this projectile deals to enemies
	 * @param ang The angle this projectile is facing
	 * @param assets The asset manager containing the player projectile assets
	 */
	static std::shared_ptr<Projectile> playerAlloc(Vec2 pos, float damage, bool fullCharged, float ang, const std::shared_ptr<AssetManager>& assets) {
		std::shared_ptr<Projectile> result = std::make_shared<Projectile>();
		return (result->playerInit(pos, damage, fullCharged, ang, assets) ? result : nullptr);
	}
	/**
	 * Creates a new lizard enemy projectile.
	 *
	 * @param pos The position at which to spawn the projectile. This should be the enemy's position
	 * @param damage The damage this projectile deals to the player
	 * @param ang The angle this projectile is facing
	 * @param assets The asset manager containing the enemy projectile assets
	 */
	static std::shared_ptr<Projectile> lizardAlloc(Vec2 pos, float damage, float ang, const std::shared_ptr<AssetManager>& assets) {
		std::shared_ptr<Projectile> result = std::make_shared<Projectile>();
		return (result->lizardInit(pos, damage, ang, assets) ? result : nullptr);
	}
	/**
	 * Creates a new mage enemy projectile.
	 *
	 * @param pos The position at which to spawn the projectile. This should be the enemy's position
	 * @param damage The damage this projectile deals to the player
	 * @param ang The angle this projectile is facing
	 * @param assets The asset manager containing the enemy projectile assets
	 */
	static std::shared_ptr<Projectile> mageAlloc(Vec2 pos, float damage, float ang, const std::shared_ptr<AssetManager>& assets) {
		std::shared_ptr<Projectile> result = std::make_shared<Projectile>();
		return (result->mageInit(pos, damage, ang, assets) ? result : nullptr);
	}
    
    /**
     * Creates a new boss enemy projectile.
     *
     * @param pos The position at which to spawn the projectile. This should be the enemy's position
     * @param damage The damage this projectile deals to the player
     * @param ang The angle this projectile is facing
     * @param assets The asset manager containing the enemy projectile assets
     */
    static std::shared_ptr<Projectile> bossAlloc(Vec2 pos, float damage, float ang, const std::shared_ptr<AssetManager>& assets) {
        std::shared_ptr<Projectile> result = std::make_shared<Projectile>();
        return (result->bossInit(pos, damage, ang, assets) ? result : nullptr);
    }

	/** Returns whether this projectile has completed its lifespan and should be destroyed 
	 * TODO: recycle the object instead of destroying it 
	 */
	bool isCompleted();

	void setFlying() {
		_state = FLYING;
		_currAnimation = _flyingAnimation;
		_currAnimation->reset();
	}
	/** start the explosion animation and deactivate collisions */
	void setExploding() { 
		_state = EXPLODING;
		_currAnimation = _explodingAnimation;
		_currAnimation->reset();
		_collider->setLinearVelocity(Vec2::ZERO);
	}

	bool isExploding() { return _state == EXPLODING; }
    
    /**
     * @return whether this projectile is fully charged (at full power) or not
     */
    bool isFullyCharged() { return _isFullyCharged; }

	void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

	void addObstaclesToWorld(std::shared_ptr<physics2::ObstacleWorld> world) override;
	void syncPositions() override;

	const float getDamage() { return _damage; }

	void setAngle(float ang) { 
		_collider->setAngle(ang);
		_colliderShadow->setAngle(ang); 
	}
	void setVelocity(Vec2 vel) { _collider->setLinearVelocity(vel); }

	/**
	 * Destroys this projectile, releasing all resources.
	 */
	virtual ~Projectile(void) { dispose(); }

	/**
	 * Disposes all resources and assets
	 *
	 * Any assets owned by this object will be immediately released.
	 * Requires initialization before next use.
	 */
	void dispose();
};

#endif /* Projectile_hpp */
