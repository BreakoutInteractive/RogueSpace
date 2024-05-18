//
//  DummyEnemy.hpp
//  RS
//
//  Created by Zhiyuan Chen on 5/18/24.
//

#ifndef DummyEnemy_hpp
#define DummyEnemy_hpp

#include "Enemy.hpp"

class DummyEnemy : public Enemy {
protected:
    
    std::shared_ptr<cugl::Texture> _texture;
    
    /** the size of the visible object (including transparent region) expressed in game units */
    cugl::Vec2 _size;
    
    /** json data reference for this tile (contains information about texture, sizing, position, etc) */
    std::shared_ptr<JsonValue> _jsonData;
    
public:
    
    
#pragma mark Constructors
    
    virtual bool init(std::shared_ptr<JsonValue> data) override;
    
    static std::shared_ptr<DummyEnemy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<DummyEnemy>();
        return (result->init(data) ? result : nullptr);
    }
    
#pragma mark Asset Loading
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
    void loadAssets(const std::shared_ptr<cugl::AssetManager> &assets) override;
    
#pragma mark Enemy Functionality
    
    std::string getType() override { return "dummy enemy"; };
    
    /**
     * this enemy does not require player to defeat so should be treated as an already defeated mob.
     */
    bool isDefeated() override { return true; }
    
    /**
     * this enemy is effectively constantly dying
     */
    bool isDying() override { return true; }
    
    /**
     * this enemy does not take any hits
     */
    void hit(cugl::Vec2 atkDir, bool ranged, float damage = 1, float knockback_scl = GameConstants::KNOCKBACK) override {
        return;
    }
    
    void updateAnimation(float dt) override {
        return;
    }
    
};

#endif /* DummyEnemy_hpp */
