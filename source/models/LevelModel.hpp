//
//  LevelModel.hpp
//
//  This module provides a level that can be loaded from a JSON file.  It reads the JSON
//  file and creates the objects in the level.
//
//  This class is a subclass of Asset so that we can use a GenericLoader to manage the
//  the loading.  Even though this class defines a load method, there is the question of
//  when the method is called. The GenericLoader allows this object to be loaded in a
//  separate thread along with all of the other assets.
//
//  Author: Walker White and Anthony Perello
//  Version: 3/12/17
//
#ifndef __LEVEL_MODEL_H__
#define __LEVEL_MODEL_H__
#include <cugl/cugl.h>
#include <cugl/physics2/CUObstacleWorld.h>
#include <vector>
#include <cugl/assets/CUAsset.h>
#include <cugl/io/CUJsonReader.h>
#include "../components/Animation.hpp"
#include "Projectile.hpp"
#include "HealthPack.hpp"
#include "LevelGrid.hpp"
#include "Wall.hpp"
#include "Relic.hpp"

using namespace cugl;

/** Forward references to the various classes used by this level */
class TileLayer;
class Player;
class Enemy;
class MeleeEnemy;
class MeleeLizard;
class RangedEnemy;
class RangedLizard;
class MageAlien;
class GameObject;
class Projectile;

#pragma mark -
#pragma mark Level Model
/**
* Class that represents a dynamically loaded level in the game
*/
class LevelModel {
protected:

    /** The bounds of this level in physics coordinates */
    Rect _bounds;
    /** The dimension of the view (forming a subregion of the bounds) */
    Size _viewBounds;
    /** The level drawing scale (difference between physics and drawing coordinates) */
    Vec2 _scale;

    // Physics objects for the game
    /** The physics word; part of the model (though listeners elsewhere) */
    std::shared_ptr<physics2::ObstacleWorld> _world;
    
    /** Reference to the player object */
    std::shared_ptr<Player> _player;
    
    /** list of enemy references */
    std::vector<std::shared_ptr<Enemy>> _enemies;
    /** list of all projectiles */
    std::vector<std::shared_ptr<Projectile>> _projectiles;
    /** list of all health packs */
    std::vector<std::shared_ptr<HealthPack>> _healthpacks;
    
    /** list of all moving game objects */
    std::vector<std::shared_ptr<GameObject>> _dynamicObjects;
    
    /** reference to all tile layers*/
    std::vector<std::shared_ptr<TileLayer>> _tileLayers;
    /** Reference to all the walls */
    std::vector<std::shared_ptr<Wall>> _walls;
    /** reference to the relic object*/
    std::shared_ptr<Relic> _relic;
    
    /** Reference to all energy walls*/
    std::vector<std::shared_ptr<EnergyWall>> _energyWalls;
    
    /** Reference to all custom tutorial region collisions */
    std::vector<std::shared_ptr<TutorialCollision>> _tutorialCollisions;
    
    /** The AssetManager for the game mode */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** Reference to the debug root of the scene graph */
    std::shared_ptr<scene2::SceneNode> _debugNode;
    
    std::shared_ptr<LevelGrid> _grid;
    
    /** whether the player is exiting the level*/
    bool _exiting;
    
    /** the random number generator */
    std::mt19937 generator;
    /** the random number distribution (uniform distribution) */
    std::uniform_real_distribution<double> distribution;
    
    /** the name of the soundtrack to play for this level */
    std::string _musicName;

#pragma mark Internal Helper Methods
    
    /**
     * depending on the component `class`property, calls the approriate loader function.
     * If the component is a list of components (`Random` and `Collection` classes), the loading will be called on each subcomponent.
     */
    bool loadGameComponent(const std::shared_ptr<JsonValue> constants, const std::shared_ptr<JsonValue>& json);
    
    /**
     * Loads the player object
     *
     * The player will will be stored in `_player`  field and retained.
     * If the player fails to load, then `_player` will be nullptr.
     *
     * @param  json   a JSON reader with cursor ready to read the player
     *
     * @retain the player
     * @return true if the player was successfully loaded
     */
    bool loadPlayer(const std::shared_ptr<JsonValue> constants, const std::shared_ptr<JsonValue> &json);
    
    /**
     * Loads the enemy object
     *
     * The enemy will be stored in the  `_enemies` field and retained.
     *
     * @param constants : the constants associated with enemies
     * @param json the dynamic data from the map editor for this enemy
     */
    bool loadEnemy(const std::shared_ptr<JsonValue> constants, const std::shared_ptr<JsonValue>& json);
    
    
    /**
     * Loads a tile layer
     *
     * @param  json   a JSON reader with cursor ready to read the tile layer data
     *
     * @retain the floor tiles
     * @return true if the  tiles were successfully loaded
     */
    bool loadTileLayer(const std::shared_ptr<JsonValue>& json);
    

    /**
     * Loads a single wall object
     *
     * The wall will be retained and stored in the vector _walls.  If the
     * wall fails to load, then it will not be added to _walls.
     *
     * @param  reader   a JSON reader with cursor ready to read the wall
     *
     * @retain the wall
     * @return true if the wall was successfully loaded
     */
    bool loadWall(const std::shared_ptr<JsonValue>& json);
    
    /**
     * Loads a single collision object for tutorial activation
     * @return true if the collision object was successfully loaded
     */
    bool loadTutorialCollisions(const std::shared_ptr<JsonValue>& json);
    
    bool loadRelic(const std::shared_ptr<JsonValue>& json);

    /**
     * Converts the string to a color
     *
     * Right now we only support the following colors: yellow, red, blur, green,
     * black, and grey.
     *
     * @param  name the color name
     *
     * @return the color for the string
     */
    Color4 parseColor(std::string name);

    /**
     * Adds the physics object to the physics world
     *
     * param obj    The physics object to add
     */
    void addObstacle(const std::shared_ptr<cugl::physics2::Obstacle>& obj);

public:
#pragma mark Model Access
    
    const std::string getMusicName(){ return _musicName; }
    
    /**
     * @return whether player has completed the level and is exiting the barriers
     */
    const bool isCompleted(){ return _exiting; }
    
    /**
     * set whether the level has been completed by the player.
     */
    const void setCompleted(bool value){ _exiting = value; }
    
    /**
     * @return the static obstacle grid for this level
     */
    const std::shared_ptr<LevelGrid> getGrid() { return _grid; }

    /**
     * Returns the Obstacle world in this game level 
     *
     * @return the obstacle world in this game level
     */
    const std::shared_ptr<physics2::ObstacleWorld>& getWorld() { return _world; }
    
    /**
     * @return the player in this game level
     */
    const std::shared_ptr<Player> getPlayer() {return _player; }
    
    /**
     * @return the relic in this game level
     */
    const std::shared_ptr<Relic> getRelic() {return _relic; }
    
    /**
     * @return the enemies in this game level
     */
    const std::vector<std::shared_ptr<Enemy>>& getEnemies() { return _enemies; }

    /**
     * @return the walls in this game level
     */
    const std::vector<std::shared_ptr<Wall>>& getWalls() { return _walls; }
    
    /**
     * @return the energy walls in this game level
     */
    const std::vector<std::shared_ptr<EnergyWall>>& getEnergyWalls() { return _energyWalls; }

    /** add a projectile to this level */
    void addProjectile(std::shared_ptr<Projectile> p);
    /** remove the given projectile from this level and from the physics world (if present) */
    void delProjectile(std::shared_ptr<Projectile> p);
    /** add a health pack to this level */
    void addHealthPack(std::shared_ptr<HealthPack> h);
    /** remove the given health pack from this level and from the physics world (if present) */
    void delHealthPack(std::shared_ptr<HealthPack> h);
    
    /**
     * @return reference to list of all projectiles
     */
    const std::vector<std::shared_ptr<Projectile>>& getProjectiles() { return _projectiles; }
    /**
     * @return reference to list of all health packs
     */
    const std::vector<std::shared_ptr<HealthPack>>& getHealthPacks() { return _healthpacks; }
    
    /**
     * @note WARNING: temporarily excludes the list of projectiles.
     * @return reference to all dynamic objects
     */
    const std::vector<std::shared_ptr<GameObject>>& getDynamicObjects() {
        return _dynamicObjects;
    }
    
    /**
     * @return the reference to all colliders that activates tutorial gestures
     */
    const std::vector<std::shared_ptr<TutorialCollision>>& getTutorialCollisions() { return _tutorialCollisions; }

#pragma mark Physics Attributes
    /** 
     * Returns the bounds of this level in physics coordinates
     *
     * @return the bounds of this level in physics coordinates
     */
    const Rect& getBounds() const   { return _bounds; }
    
    /**
     * @return the size of the desired view in physics coordinates
     */
    const Size& getViewBounds() const { return _viewBounds; }


#pragma mark Drawing Methods
    /**
     * Returns the drawing scale for this game level
     *
     * The drawing scale is the number of pixels to draw before Box2D unit. Because
     * mass is a function of area in Box2D, we typically want the physics objects
     * to be small.  So we decouple that scale from the physics object.  However,
     * we must track the scale difference to communicate with the scene graph.
     *
     * We allow for the scaling factor to be non-uniform.
     *
     * @return the drawing scale for this game level
     */
    const Vec2& getDrawScale() const { return _scale; }

    /**
     * Sets the drawing scale for this game level
     *
     * The drawing scale is the number of pixels to draw before Box2D unit. Because
     * mass is a function of area in Box2D, we typically want the physics objects
     * to be small.  So we decouple that scale from the physics object.  However,
     * we must track the scale difference to communicate with the scene graph.
     *
     * We allow for the scaling factor to be non-uniform.
     *
     * @param value  the drawing scale for this game level
     */
    void setDrawScale(Vec2 value);

    void clearDebugNode();

    /**
     * Sets the debug scene graph node for debugging purposes.
     *
     * @param node  the scene graph node for drawing purposes.
     *
     * @retain  a reference to this scene graph node
     * @release the previous scene graph node used by this object
     */
    void setDebugNode(const std::shared_ptr<scene2::SceneNode>& node);

    /**
     * Sets the loaded assets for this game level and populate all models with their needed assets.
     *
     * @param assets the loaded assets for this game level
     */
    void setAssets(const std::shared_ptr<AssetManager>& assets);

    /**
     * Toggles whether to show the debug layer of this game world.
     *
     * The debug layer displays wireframe outlines of the physics fixtures.
     *
     * @param  flag whether to show the debug layer of this game world
     */
    void showDebug(bool flag);
    
    
    /**
     * Render the level container using the sprite batch. The container includes all elements
     * of this level which includes entities and environment.
     *
     * @param batch the spritebatch
     * @param camRect a rectangle defining the camera's viewport where position is shifted to the left corner as opposed to center of the viewport.
     */
    void render(const std::shared_ptr<cugl::SpriteBatch>& batch, Rect camRect);


#pragma mark -
#pragma mark Static Constructors

    /**
     * Loads this game level from a JsonValue containing all data from a source Json file.
     * @param json the json loaded from the source file to use when loading this game level
     *
     * @return true if successfully loaded the asset from the input JsonValue
     */
    bool init(const std::shared_ptr<cugl::JsonValue>& json, std::shared_ptr<JsonValue> parsedJson);

    /**
     * Creates a new game level with the given source json.
     *
     * This method does NOT load the level. You must call the load() method to do that.
     * This method returns false if file does not exist.
     *
     * @return  an autoreleased level file
     */
    static std::shared_ptr<LevelModel> alloc(std::shared_ptr<JsonValue> json, std::shared_ptr<JsonValue> parsedJson) {
        std::shared_ptr<LevelModel> result = std::make_shared<LevelModel>();
        return (result->init(json, parsedJson) ? result : nullptr);
    }
    
    /**
     * Unloads this game level, releasing all sources
     */
    void unload();

    /**
     * Creates a new, empty level.
     */
    LevelModel(void);

    /**
     * Destroys this level, releasing all resources.
     */
    virtual ~LevelModel(void);
};

#endif /* __LEVEL_MODEL_H__ */
