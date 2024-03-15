//
//  JSWallModel.h
//  JSONDemo
//
//  This encapsulates all of the information for representing a wall. Since wall never
//  move, we normally would not need a separate class (because we could just use an instance
//  of PolygonObstacle).  However, this changes when we do dynamic level loading.
//
//  The problem is that we need to initialize an physics object when a level is loaded,
//  but NOT attach any assets yet.  That is because the physics object is itself an asset
//  and it is not safe to assume the order of asset loading.  So instead, we just store
//  the keys of the assets in this class, and they attach the assets later when we
//  initialize the scene graph.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White and Anthony Perello
//  Version: 3/12/17
//
#ifndef __JS_WALL_MODEL_H__
#define __JS_WALL_MODEL_H__

#include <cugl/cugl.h>
#include "GameObject.hpp"

using namespace cugl;


#pragma mark -
#pragma mark Wall Model

/**
 * dummy wall
 */
class WallModel : public GameObject {
private:
	/** This macro disables the copy constructor (not allowed on scene graphs) */
	CU_DISALLOW_COPY_AND_ASSIGN(WallModel);

protected:
	
    bool _tall;
    
    std::shared_ptr<Texture> _texture;

public:
#pragma mark -
#pragma mark Animation

    //void Player::loadAssets(const std::shared_ptr<AssetManager> &assets){

    void loadAssets(const std::shared_ptr<AssetManager> &assets);
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new wall whose position is the given center.
	 */
	WallModel(bool tall) : GameObject(), _tall(tall) { }
    
    bool init(const Poly2& poly, const Vec2 anchor);

	/**
	 * Destroys this wall, releasing all resources.
	 */
	virtual ~WallModel(void) {}

};

#endif /* defined(__JS_WALL_MODEL_H__) */
