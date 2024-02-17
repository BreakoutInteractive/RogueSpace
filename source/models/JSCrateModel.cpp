//
//  JSCrateModel.cpp
//  JSONDemo
//
//  This encapsulates all of the information for representing a crate. Since crates only
//  move under their own power, we normally would not need a separate class (because we
//  could just use an instance of BoxObstacle).  However, this changes when we do dynamic
//  level loading.
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
#include "JSCrateModel.h"
#include <cugl/scene2/graph/CUPolygonNode.h>


using namespace cugl;
using namespace cugl::scene2;

#pragma mark -
#pragma mark Initializers
/**
 * Initializes a new crate with the given position and size.
 *
 * The crate size is specified in world coordinates.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param  pos      Initial position in world coordinates
 * @param  size   	The dimensions of the crate.
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool CrateModel::init(const Vec2 pos, const Size size) {
	if (BoxObstacle::init(pos,size)) {
		std::string name("crate");
		setName(name);
		_crateTexture = "";
		return true;
	}
	return false;
}

