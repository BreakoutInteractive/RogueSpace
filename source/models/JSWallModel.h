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

#include <cugl/physics2/CUPolygonObstacle.h>

using namespace cugl;


#pragma mark -
#pragma mark Wall Model

/**
 * A wall door in the rocket lander game.
 *
 * Note that this class does keeps track of its own assets via keys, so that
 * they can be attached later as part of the scene graph.
 */
class WallModel : public physics2::PolygonObstacle {
private:
	/** This macro disables the copy constructor (not allowed on scene graphs) */
	CU_DISALLOW_COPY_AND_ASSIGN(WallModel);

protected:
	/** The texture filmstrip for the wall */
	std::string _wallTexture;

#pragma mark -
#pragma mark Static Constructors
public:
	/**
	 * Creates a wall from (not necessarily convex) polygon
	 *
	 * The anchor point (the rotational center) of the polygon is at the
	 * center of the polygons bounding box.
	 *
	 * @param poly   The polygon vertices
	 *
	 * @return  A newly allocated Wall, with the given shape
	 */
	static std::shared_ptr<WallModel> alloc(const Poly2& poly) {
		std::shared_ptr<WallModel> result = std::make_shared<WallModel>();
		return (result->init(poly) ? result : nullptr);
	}

	/**
	 * Creates a wall from (not necessarily convex) polygon
	 *
	 * The anchor point (the rotational center) of the polygon is specified as a
	 * ratio of the bounding box.  An anchor point of (0,0) is the bottom left of
	 * the bounding box.  An anchor point of (1,1) is the top right of the bounding
	 * box.  The anchor point does not need to be contained with the bounding box.
	 *
	 * @param  poly     The polygon vertices
	 * @param  anchor   The rotational center of the polygon
	 *
	 * @return  A newly allocated Wall, with the given shape and anchor
	 */
	static std::shared_ptr<WallModel> alloc(const Poly2& poly, const Vec2& anchor) {
		std::shared_ptr<WallModel> result = std::make_shared<WallModel>();
		return (result->init(poly, anchor) ? result : nullptr);
	}


#pragma mark -
#pragma mark Animation
	/**
	 * Returns the texture (key) for this wall
	 *
	 * The value returned is not a Texture2D value.  Instead, it is a key for
	 * accessing the texture from the asset manager.
	 *
	 * @return the texture (key) for this wall
	 */
	const std::string& getTextureKey() const { return _wallTexture; }

	/**
	 * Returns the texture (key) for this wall
	 *
	 * The value returned is not a Texture2D value.  Instead, it is a key for
	 * accessing the texture from the asset manager.
	 *
	 * @param  strip    the texture (key) for this wall
	 */
	void setTextureKey(std::string strip) { _wallTexture = strip; }

#pragma mark -
#pragma mark Initializers
	/**
	 * Creates a new wall at the origin.
	 */
	WallModel(void) : PolygonObstacle() { }

	/**
	 * Destroys this wall, releasing all resources.
	 */
	virtual ~WallModel(void) {}

	/**
	 * Initializes a wall from (not necessarily convex) polygon
	 *
	 * The anchor point (the rotational center) of the polygon is at the
	 * center of the polygons bounding box.
	 *
	 * @param poly   The polygon vertices
	 *
	 * @return  true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const Poly2& poly) override { return init(poly,Vec2(0.5,0.5)); }

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
	virtual bool init(const Poly2& poly, const Vec2 anchor) override;

};

#endif /* defined(__JS_WALL_MODEL_H__) */
