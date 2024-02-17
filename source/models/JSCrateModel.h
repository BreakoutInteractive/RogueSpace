//
//  JSCrateModel.h
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
//  Author: Walker White and Anthony Perello
//  Version: 3/12/17
//
#ifndef __JS_CRATE_MODEL_H__
#define __JS_CRATE_MODEL_H__

#include <cugl/physics2/CUBoxObstacle.h>

using namespace cugl;


#pragma mark -
#pragma mark Crate Model

/**
* A crate in the rocket lander game.
*
* Note that this class does keeps track of its own assets via keys, so that
* they can be attached later as part of the scene graph.
*/
class CrateModel : public physics2::BoxObstacle {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(CrateModel);

protected:
    /** The texture for the crate */
    std::string _crateTexture;

#pragma mark -
#pragma mark Static Constructors
public:
    /**
     * Creates a new crate at the origin.
     *
     * The crate is 1 unit by 1 unit in size. The crate is scaled so that
     * 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @return  A newly allocated Crate, at the origin
     */
    static std::shared_ptr<CrateModel> alloc() {
        std::shared_ptr<CrateModel> result = std::make_shared<CrateModel>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Creates a new crate with the given position
     *
     * The crate is 1 unit by 1 unit in size. The crate is scaled so that
     * 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos  Initial position in world coordinates
     *
     * @return  A newly allocated Crate, at the given position
     */
    static std::shared_ptr<CrateModel> alloc(const Vec2& pos) {
        std::shared_ptr<CrateModel> result = std::make_shared<CrateModel>();
        return (result->init(pos) ? result : nullptr);
    }

    /**
     * Creates a new crate with the given position and size.
     *
     * The crate size is specified in world coordinates.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos      Initial position in world coordinates
     * @param  size       The dimensions of the crate.
     *
     * @return  A newly allocated Crate, at the given position, with the given size
     */
    static std::shared_ptr<CrateModel> alloc(const Vec2& pos, const Size& size)  {
        std::shared_ptr<CrateModel> result = std::make_shared<CrateModel>();
        return (result->init(pos, size) ? result : nullptr);
    }


#pragma mark -
#pragma mark Animation
    /**
     * Returns the texture (key) for this crate
     *
     * The value returned is not a Texture2D value.  Instead, it is a key for
     * accessing the texture from the asset manager.
     *
     * @return the texture (key) for this crate
     */
    const std::string& getTextureKey() const { return _crateTexture; }

    /**
     * Sets the texture (key) for this crate
     *
     * The value returned is not a Texture2D value.  Instead, it is a key for
     * accessing the texture from the asset manager.
     *
     * @param  strip    the texture (key) for this crate
     */
    void setTextureKey(std::string strip) { _crateTexture = strip; }

#pragma mark -
#pragma mark Initializers
    /*
     * Creates a new crate at the origin.
     */
    CrateModel(void) : BoxObstacle() { }

    /**
     * Destroys this crate, releasing all resources.
     */
    virtual ~CrateModel(void) {}

    /**
     * Initializes a new crate at the origin.
     *
     * The crate is 1 unit by 1 unit in size. The crate is scaled so that
     * 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() override { return init(Vec2::ZERO,Size::ZERO); }

    /**
     * Initializes a new crate with the given position
     *
     * The crate is 1 unit by 1 unit in size. The crate is scaled so that
     * 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos  Initial position in world coordinates
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2 pos) override { return init(pos,Size(1,1)); }

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
     * @param  size       The dimensions of the crate.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2 pos, const Size size) override;
};

#endif /* defined(__JS_CRATE_MODEL_H__) */
