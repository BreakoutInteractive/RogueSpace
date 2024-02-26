#include <cugl/assets/CUJsonLoader.h>
#include "LevelModel.h"
#include "JSLevelConstants.h"
#include "JSExitModel.h"
#include "JSCrateModel.h"
#include "JSWallModel.h"
#include "Floor.hpp"

#pragma mark -
#pragma mark Static Constructors

//the radius of a melee attack. sweeps out a semicircle with this radius (in physics coordinates) centered at the center of the player
//this is how sweeping melee attacks work in Hades
#define ATK_RADIUS 3.5f

/**
* Creates a new, empty level.
*/
LevelModel::LevelModel(void) : Asset(),
_world(nullptr),
_debugNode(nullptr)
{
	_bounds.size.set(1.0f, 1.0f);
}

/**
* Destroys this level, releasing all resources.
*/
LevelModel::~LevelModel(void) {
	unload();
    clearDebugNode();
}


#pragma mark -
#pragma mark Drawing Methods

void LevelModel::setDrawScale(Vec2 scale) {
	if (_player != nullptr) {
		_player->setDrawScale(scale);
	}
    else {
        CUAssertLog(false, "Failed to set draw scale for player");
    }
    if (_floor != nullptr) {
        _floor->setDrawScale(scale);
    }
    else {
        CUAssertLog(false, "Failed to set draw scale for floor");
    }
}

void LevelModel::render(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: draw contents manually, sorting
    _floor->draw(batch);
    _player->draw(batch);
    //we add pi/2 to the angle since the sprite is pointing down but the hitbox points right by default
    if (_atk->isEnabled()) batch->draw(_attackAnimation, Color4(255,255,255,200), Vec2(_attackAnimation->getWidth() / 2, _attackAnimation->getHeight()/2), 
                                        Vec2(ATK_RADIUS/(_attackAnimation->getWidth() / 2), ATK_RADIUS/(_attackAnimation->getHeight() / 2)) * _player->getDrawScale(),
                                        _atk->getAngle() + M_PI_2, _atk->getPosition() * _player->getDrawScale());
}

void LevelModel::clearDebugNode(){
    _debugNode->removeAllChildren();
    _debugNode = nullptr;
}

void LevelModel::setDebugNode(const std::shared_ptr<scene2::SceneNode> & node) {
	if (_debugNode != nullptr) {
		clearDebugNode();
	}

	_debugNode = node;
	_scale.set(_debugNode->getContentSize().width/_bounds.size.width,
             _debugNode->getContentSize().height/_bounds.size.height);

    _debugNode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugNode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugNode->setPosition(Vec2::ZERO);
    
    // debug node should be added once objects are initialized
    _player->setDebugScene(_debugNode);
    for (int ii = 0; ii < _crates.size(); ii++){
        _crates[ii]->setDebugScene(_debugNode);
    }
    for (int ii = 0; ii < _walls.size(); ii++){
        _walls[ii]->setDebugScene(_debugNode);
    }
}

void LevelModel::setAssets(const std::shared_ptr<AssetManager> &assets){
    _assets = assets;
    _player->loadAssets(assets);
    _floor->loadAssets(assets);
    _attackAnimation = assets->get<Texture>("atk");
}


void LevelModel::showDebug(bool flag) {
	if (_debugNode != nullptr) {
		_debugNode->setVisible(flag);
	}
}


#pragma mark -
#pragma mark Asset Loading
/**
 * Loads this game level from the source file
 *
 * This load method should NEVER access the AssetManager.  Assets are loaded in
 * parallel, not in sequence.  If an asset (like a game level) has references to
 * other assets, then these should be connected later, during scene initialization.
 *
 * @return true if successfully loaded the asset from a file
 */
bool LevelModel::preload(const std::string file) {
	std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
	return preload(reader->readJson());
}

/**
 * Loads this game level from the source file
 *
 * This load method should NEVER access the AssetManager.  Assets are loaded in
 * parallel, not in sequence.  If an asset (like a game level) has references to
 * other assets, then these should be connected later, during scene initialization.
 *
 * @return true if successfully loaded the asset from a file
 */
bool LevelModel:: preload(const std::shared_ptr<cugl::JsonValue>& json) {
	if (json == nullptr) {
		CUAssertLog(false, "Failed to load level file");
		return false;
	}
    CULog("loading");
	// Initial geometry
	float w = json->get(WIDTH_FIELD)->asFloat();
	float h = json->get(HEIGHT_FIELD)->asFloat();
	_bounds.size.set(w, h);

	/** Create the physics world */
	_world = physics2::ObstacleWorld::alloc(getBounds(),Vec2::ZERO);
    
    auto playerJson = json->get(PLAYER_FIELD);
    if (playerJson != nullptr){
        loadPlayer(playerJson);
    }
    else {
        CUAssertLog(false, "Failed to load player");
        return false;
    }

	auto walls = json->get(WALLS_FIELD);
	if (walls != nullptr) {
		// Convert the object to an array so we can see keys and values
		int wsize = (int)walls->size();
		for(int ii = 0; ii < wsize; ii++) {
			loadWall(walls->get(ii));
		}
	} else {
		CUAssertLog(false, "Failed to load walls");
		return false;
	}
//	auto crates = json->get(CRATES_FIELD);
//	if (crates != nullptr) {
//		// Convert the object to an array so we can see keys and values
//		int csize = (int)crates->size();
//		for(int ii = 0; ii < csize; ii++) {
//			loadCrate(crates->get(ii));
//		}
//	} else {
//		CUAssertLog(false, "Failed to load crates");
//		return false;
//	}
    
    // Add objects to world
    addObstacle(_player);
	addObstacle(_atk);
    for (int ii = 0; ii < _crates.size(); ii++){
        addObstacle(_crates[ii]);
    }
    for (int ii = 0; ii < _walls.size(); ii++){
        addObstacle(_walls[ii]);
    }
    
    // load visuals (floor)
    auto floor = json->get("floor");
    if (floor != nullptr){
        loadFloor(floor);
    }
    else {
        CUAssertLog(false, "Failed to load floor tiles");
        return false;
    }

	return true;
}

/**
* Unloads this game level, releasing all sources
*
* This load method should NEVER access the AssetManager.  Assets are loaded and
* unloaded in parallel, not in sequence.  If an asset (like a game level) has
* references to other assets, then these should be disconnected earlier.
*/
void LevelModel::unload() {
	for(auto it = _crates.begin(); it != _crates.end(); ++it) {
		if (_world != nullptr) {
			_world->removeObstacle((*it));
		}
    (*it) = nullptr;
	}
	_crates.clear();
	for(auto it = _walls.begin(); it != _walls.end(); ++it) {
		if (_world != nullptr) {
			_world->removeObstacle((*it));
		}
    (*it) = nullptr;
	}
	_walls.clear();
    
    _world->removeObstacle(_player);
    _player = nullptr;
    _floor = nullptr;
    
    if (_world != nullptr) {
        _world->clear();
        _world = nullptr;
    }
}


#pragma mark -
#pragma mark Individual Loaders

bool LevelModel::loadPlayer(const std::shared_ptr<JsonValue> &json){
    bool success = true;
    auto posData = json->get(POSITION_FIELD);
    success = success && posData->isArray();
    Vec2 pos = Vec2(posData->get(0)->asFloat(), posData->get(1)->asFloat());

    auto sizeArray = json->get(SIZE_FIELD);
    success = success && sizeArray->isArray();
    Vec2 size = Vec2(sizeArray->get(0)->asFloat(), sizeArray->get(1)->asFloat());


    // Get the object, which is automatically retained
    _player = Player::alloc(pos,(Size) size);
    _player->setName(json->key());

    _player->setDensity(json->getDouble(DENSITY_FIELD));
    _player->setFriction(json->getDouble(FRICTION_FIELD));
    _player->setRestitution(json->getDouble(RESTITUTION_FIELD));
    _player->setFixedRotation(!json->getBool(ROTATION_FIELD));
    _player->setDebugColor(parseColor(json->getString(DEBUG_COLOR_FIELD)));
    _player->setTextureKey(json->getString(TEXTURE_FIELD));
    _player->setParryTextureKey(json->getString(PARRY_FIELD));
    _player->setAttackTextureKey(json->getString(ATTACK_FIELD));
    _player->setDrawScale(_scale);

    std::string btype = json->getString(BODYTYPE_FIELD);
    if (btype == STATIC_VALUE) {
        _player->setBodyType(b2_staticBody);
    }

    //setup the attack for collision detection
	_atk = physics2::CapsuleObstacle::alloc(pos, Size(0, ATK_RADIUS), poly2::Capsule::HALF_REVERSE);
	_atk->setSensor(true);
	_atk->setBodyType(b2_staticBody);
    return success;
}

bool LevelModel::loadFloor(const std::shared_ptr<JsonValue> &json){
    bool success = true;
    auto sizeData = json->get(SIZE_FIELD);
    success = success && sizeData->isArray();
    Vec2 size(sizeData->get(0)->asFloat(), sizeData->get(1)->asFloat());
    std::string textureName = json->getString(TEXTURE_FIELD);
    std::vector<std::shared_ptr<Tile>> tiles;
    
    bool useGrid = json->get("use-grid")->asBool();
    if (!useGrid){
        // load all tiles as given
        auto tilesData = json->get("tiles");
        int count = (int)tilesData->size();

        for(int ii = 0; ii < count; ii++) {
            auto tileData = tilesData->get(ii);
            auto posData = tileData->get(POSITION_FIELD);
            Vec2 pos(posData->get(0)->asFloat(), posData->get(1)->asFloat());
            auto tile = Tile::alloc(pos, textureName);
            tiles.emplace_back(tile);
        }
    }
    else {
        // generate grid of tiles automatically
        auto grid = json->get("grid");
        auto startData = grid->get("start");
        Vec2 startPos(startData->get(0)->asFloat(), startData->get(1)->asFloat());
        int rows = grid->get("rows")->asInt();
        int cols = grid->get("columns")->asInt();
        for (int i = 0; i < rows; i++){
            // compute the first on row
            Vec2 firstTilePos = startPos + Vec2(size.x / 2 * i, -size.y / 4 * i );
            for (int j = 0; j < cols; j++){
                Vec2 pos = firstTilePos - Vec2(size.x / 2 * j, size.y/4 * j);
                auto tile = Tile::alloc(pos, textureName);
                tiles.emplace_back(tile);
            }
        }
    }
    
    _floor = Floor::alloc(size, tiles);
    _floor->setDrawScale(_scale);
    
    return success;
}


bool LevelModel::loadWall(const std::shared_ptr<JsonValue>& json) {
	bool success = true;

	int polysize = json->getInt(VERTICES_FIELD);
	success = success && polysize > 0;

	std::vector<float> vertices = json->get(BOUNDARY_FIELD)->asFloatArray();
	success = success && 2*polysize == vertices.size();

    Vec2* verts = reinterpret_cast<Vec2*>(&vertices[0]);
	Poly2 wall(verts,(int)vertices.size()/2);
	EarclipTriangulator triangulator;
	triangulator.set(wall.vertices);
	triangulator.calculate();
	wall.setIndices(triangulator.getTriangulation());
    triangulator.clear();
	
	// Get the object, which is automatically retained
	std::shared_ptr<WallModel> wallobj = WallModel::alloc(wall);
	wallobj->setName(json->key());

	std::string btype = json->getString(BODYTYPE_FIELD);
	if (btype == STATIC_VALUE) {
		wallobj->setBodyType(b2_staticBody);
	}

	wallobj->setDensity(json->getDouble(DENSITY_FIELD));
	wallobj->setFriction(json->getDouble(FRICTION_FIELD));
	wallobj->setRestitution(json->getDouble(RESTITUTION_FIELD));

	// Set the texture value
	success = success && json->get(TEXTURE_FIELD)->isString();
	wallobj->setTextureKey(json->getString(TEXTURE_FIELD));
	wallobj->setDebugColor(parseColor(json->getString(DEBUG_COLOR_FIELD)));

	if (success) {
		_walls.push_back(wallobj);
	} else {
		wallobj = nullptr;
	}

	vertices.clear();
	return success;
}

/**
* Loads a single crate object
*
* The crate will be retained and stored in the vector _crates.  If the
* crate fails to load, then it will not be added to _crates.
*
* @param  reader   a JSON reader with cursor ready to read the crate
*
* @retain the crate
* @return true if the crate was successfully loaded
*/
bool LevelModel::loadCrate(const std::shared_ptr<JsonValue>& json) {
	bool success = true;

	auto posArray = json->get(POSITION_FIELD);
	success = success && posArray->isArray();
	Vec2 cratePos = Vec2(posArray->get(0)->asFloat(), posArray->get(1)->asFloat());

	auto sizeArray = json->get(SIZE_FIELD);
	success = success && sizeArray->isArray();
	Vec2 crateSize = Vec2(sizeArray->get(0)->asFloat(), sizeArray->get(1)->asFloat());

	// Get the object, which is automatically retained
	std::shared_ptr<CrateModel> crate = CrateModel::alloc(cratePos,(Size)crateSize);

	// Using the key makes too many sounds
	// crate->setName(reader.getKey());
	std::string textureName = json->getString(TEXTURE_FIELD);
	crate->setName(textureName);
	std::string btype = json->getString(BODYTYPE_FIELD);
	if (btype == STATIC_VALUE) {
		crate->setBodyType(b2_staticBody);
	}

	crate->setDensity(json->getDouble(DENSITY_FIELD));
	crate->setFriction(json->getDouble(FRICTION_FIELD));
	crate->setRestitution(json->getDouble(RESTITUTION_FIELD));
	crate->setAngularDamping(json->getDouble(DAMPING_FIELD));
	crate->setAngleSnap(0);     // Snap to the nearest degree

								// Set the texture value
	success = success && json->get(TEXTURE_FIELD)->isString();
	crate->setTextureKey(json->getString(TEXTURE_FIELD));
	crate->setDebugColor(parseColor(json->getString(DEBUG_COLOR_FIELD)));

	if (success) {
		_crates.push_back(crate);
	} else {
		crate = nullptr;
	}

	return success;
}

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
Color4 LevelModel::parseColor(std::string name) {
	if (name == "yellow") {
		return Color4::YELLOW;
	} else if (name == "red") {
		return Color4::RED;
	} else if (name == "green") {
		return Color4::GREEN;
	} else if (name == "blue") {
		return Color4::BLUE;
	} else if (name == "black") {
		return Color4::BLACK;
	} else if (name == "gray") {
		return Color4::GRAY;
	}
	return Color4::WHITE;
}


void LevelModel::addObstacle(const std::shared_ptr<cugl::physics2::Obstacle>& obj) {
	_world->addObstacle(obj);
}

