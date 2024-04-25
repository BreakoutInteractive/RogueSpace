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
#include "LevelGrid.hpp"
#include "Wall.hpp"

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
*
* This class is a subclass of Asset so that we can use it with a GenericLoader. As with
* all assets, this class SHOULD NOT make any references to AssetManager in the load/unload 
* methods. Assets should be treated as if they load in parallel, not in sequence.  Therefore,
* it is unsafe to assume that one asset loads before another.  If this level needs to connect 
* to other assets (sound, images, etc.) this should take place after asset loading, such as 
* during scene graph initialization.
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

    std::vector<std::shared_ptr<Projectile>> _projectiles;
    
    /** list of all moving game objects */
    std::vector<std::shared_ptr<GameObject>> _dynamicObjects;

    std::shared_ptr<physics2::WheelObstacle> _atk;
    
    std::vector<std::shared_ptr<TileLayer>> _tileLayers;
    
    /** Reference to all the walls */
    std::vector<std::shared_ptr<Wall>> _walls;
    
    /** Reference to all energy walls*/
    std::vector<std::shared_ptr<EnergyWall>> _energyWalls;

    /** Reference to all custom boundaries (box2d obstacles) */
    std::vector<std::shared_ptr<physics2::Obstacle>> _boundaries;
    
    /** The AssetManager for the game mode */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** Reference to the debug root of the scene graph */
    std::shared_ptr<scene2::SceneNode> _debugNode;

    std::shared_ptr<cugl::Texture> _attackAnimation;
    std::shared_ptr<Animation> _playerAttack;
    
    std::shared_ptr<LevelGrid> _grid;
    
    /** whether the player is exiting the level*/
    bool _exiting;
    
    /** the random number generator */
    std::mt19937 generator;
    /** the random number distribution (uniform distribution) */
    std::uniform_real_distribution<double> distribution;

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
     * Loads a single collision object
     *
     * @param  reader   a JSON reader with cursor ready to read the custom boundary
     *
     * @return true if the collision object was successfully loaded
     */
    bool loadBoundary(const std::shared_ptr<JsonValue>& json);

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
     * @return the player's attack semi-sphere
     */
    const std::shared_ptr<physics2::WheelObstacle> getAttack() { return _atk; }

    /**
     * @return the enemies in this game level
     */
    const std::vector<std::shared_ptr<Enemy>> getEnemies() { return _enemies; }

    /**
     * @return the walls in this game level
     */
    const std::vector<std::shared_ptr<Wall>> getWalls() { return _walls; }
    
    /**
     * @return the energy walls in this game level
     */
    const std::vector<std::shared_ptr<EnergyWall>> getEnergyWalls() { return _energyWalls; }

    const std::shared_ptr<Animation> getPlayerAtk() { return _playerAttack; }

    /** add a projectile to this level */
    void addProjectile(std::shared_ptr<Projectile> p);
    /** remove the given projectile from this level and from the physics world (if present) */
    void delProjectile(std::shared_ptr<Projectile> p);
    const std::vector<std::shared_ptr<Projectile>> getProjectiles() { return _projectiles; }

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
     */
    void render(const std::shared_ptr<cugl::SpriteBatch>& batch);


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
