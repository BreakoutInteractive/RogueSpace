//
//  JSWallModel.cpp
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
//  Author: Walker White
//  Version: 1/20/16
//
#include "JSWallModel.hpp"
#include <cugl/cugl.h>
#include "CollisionConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Initializers

/**
 * Initializes a wall from (not necessarily convex) polygon
 *
 * The anchor point (the rotational center) of the polygon is specified as a
 * ratio of the bounding box.  An anchor point of (0,0) is the bottom left of
 * the bounding box.  An anchor point of (1,1) is the top right of the bounding
 * box.  The anchor point does not need to be contained with the bounding box.
 *
 * @param  poly     The polygon vertices
 * @param  anchor   The rotational center of the polygon
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool WallModel::init(const Poly2& poly, const Vec2 anchor) {
    auto p = std::make_shared<physics2::PolygonObstacle>();
    p->PolygonObstacle::initWithAnchor(poly,anchor);
    _collider = p;
    
    b2Filter filter;
    // this is a wall
    filter.categoryBits = CATEGORY_WALL;
    // a wall can collide with a player or an enemy
    filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
    p->setFilterData(filter);
    
	return true;
}

void WallModel::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    Vec2 pos = getPosition();
    Vec2 scl = getDrawScale();
    int size = 2;
    // batch draw(texture, color, origin, scale, angle, offset)
    batch->draw(_texture, Color4::WHITE, Vec2(_texture->getWidth()/2, 0), size * scl/_texture->getWidth(), 0, ((pos - Vec2(0,0.5f))) * scl);
}

void WallModel::loadAssets(const std::shared_ptr<AssetManager> &assets){
    if (_tall){
        _texture = assets->get<Texture>("tall-wall");
    }
    else {
        _texture = assets->get<Texture>("short-wall");
    }
}
