#include <cugl/assets/CUJsonLoader.h>
#include "LevelModel.h"
#include "JSLevelConstants.h"
#include "JSExitModel.h"
#include "JSCrateModel.h"
#include "JSWallModel.h"

#pragma mark -
#pragma mark Static Constructors

/**
* Creates a new, empty level.
*/
LevelModel::LevelModel(void) : Asset(),
_root(nullptr),
_world(nullptr),
_worldnode(nullptr),
_debugnode(nullptr),
_rocket(nullptr),
_goalDoor(nullptr)
{
	_bounds.size.set(1.0f, 1.0f);
}

/**
* Destroys this level, releasing all resources.
*/
LevelModel::~LevelModel(void) {
	unload();
	clearRootNode();
}


#pragma mark -
#pragma mark Drawing Methods
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
void LevelModel::setDrawScale(float value) {
	if (_rocket != nullptr) {
		_rocket->setDrawScale(value);
	}
}

/**
* Clears the root scene graph node for this level
*/
void LevelModel::clearRootNode() {
	if (_root == nullptr) {
		return;
	}
    _worldnode->removeFromParent();
	_worldnode->removeAllChildren();
    _worldnode = nullptr;
  
    _debugnode->removeFromParent();
	_debugnode->removeAllChildren();
	_debugnode = nullptr; 

	_root = nullptr;
}

/**
* Sets the scene graph node for drawing purposes.
*
* The scene graph is completely decoupled from the physics system.  The node
* does not have to be the same size as the physics body. We only guarantee
* that the node is positioned correctly according to the drawing scale.
*
* @param value  the scene graph node for drawing purposes.
*
* @retain  a reference to this scene graph node
* @release the previous scene graph node used by this object
*/
void LevelModel::setRootNode(const std::shared_ptr<scene2::SceneNode>& node) {
	if (_root != nullptr) {
		clearRootNode();
	}

	_root = node;
	_scale.set(_root->getContentSize().width/_bounds.size.width,
             _root->getContentSize().height/_bounds.size.height);

	// Create, but transfer ownership to root
	_worldnode = scene2::SceneNode::alloc();
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(Vec2::ZERO);
  
	_debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(Vec2::ZERO);
  
	_root->addChild(_worldnode);
	_root->addChild(_debugnode);

	// Add the individual elements
	std::shared_ptr<scene2::PolygonNode> poly;
	std::shared_ptr<scene2::WireNode> draw;

	if (_goalDoor != nullptr) {
        auto sprite = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(_goalDoor->getTextureKey()));
		addObstacle(_goalDoor,sprite); // Put this at the very back
	}

	for(auto it = _crates.begin(); it != _crates.end(); ++it) {
		std::shared_ptr<CrateModel> crate = *it;
		auto sprite = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(crate->getTextureKey()));
		addObstacle(crate,sprite);   // PUT SAME TEXTURES IN SAME LAYER!!!
	}

	for(auto it = _walls.begin(); it != _walls.end(); ++it) {
		std::shared_ptr<WallModel> wall = *it;
		auto sprite = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(wall->getTextureKey()),
                                                            wall->getPolygon() * _scale);
		addObstacle(wall,sprite);  // All walls share the same texture
	}
  
  if (_rocket != nullptr) {
		auto rocketNode = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(_rocket->getTextureKey()));
		_rocket->setShipNode(rocketNode, _assets);
        _rocket->setDrawScale(_scale.x);

		// Create the polygon node (empty, as the model will initialize)
		_worldnode->addChild(rocketNode);
		_rocket->setDebugScene(_debugnode);
	}
}

/**
* Toggles whether to show the debug layer of this game world.
*
* The debug layer displays wireframe outlines of the physics fixtures.
*
* @param  flag whether to show the debug layer of this game world
*/
void LevelModel::showDebug(bool flag) {
	if (_debugnode != nullptr) {
		_debugnode->setVisible(flag);
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
	// Initial geometry
	float w = json->get(WIDTH_FIELD)->asFloat();
	float h = json->get(HEIGHT_FIELD)->asFloat();
	_bounds.size.set(w, h);

	/** Create the physics world */
	_world = physics2::ObstacleWorld::alloc(getBounds(),Vec2::ZERO);

	// Parse the rocket
	if (!loadRocket(json)) {
		CUAssertLog(false, "Failed to load rocket");
		return false;
	}

	if (!loadGoalDoor(json)) {
		CUAssertLog(false, "Failed to load goal door");
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
	auto crates = json->get(CRATES_FIELD);
	if (crates != nullptr) {
		// Convert the object to an array so we can see keys and values
		int csize = (int)crates->size();
		for(int ii = 0; ii < csize; ii++) {
			loadCrate(crates->get(ii));
		}
	} else {
		CUAssertLog(false, "Failed to load crates");
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
	if (_rocket != nullptr) {
		if (_world != nullptr) {
			_world->removeObstacle(_rocket);
		}
		_rocket = nullptr;
	}
	if (_goalDoor != nullptr) {
		if (_world != nullptr) {
			_world->removeObstacle(_goalDoor);
		}
		_goalDoor = nullptr;
	}
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
	if (_world != nullptr) {
		_world->clear();
		_world = nullptr;
	}
}


#pragma mark -
#pragma mark Individual Loaders

bool LevelModel::loadRocket(const std::shared_ptr<JsonValue>& json) {
	bool success = false;
	auto rocket = json->get(ROCKET_FIELD);
	if (rocket != nullptr) {
		success = true;

		auto rockPosArray = rocket->get(POSITION_FIELD);
		success = success && rockPosArray->isArray();
		Vec2 rockPos = Vec2(rockPosArray->get(0)->asFloat(), rockPosArray->get(1)->asFloat());

		auto sizeArray = rocket->get(SIZE_FIELD);
		success = success && sizeArray->isArray();
		Vec2 rockSize = Vec2(sizeArray->get(0)->asFloat(), sizeArray->get(1)->asFloat());


		// Get the object, which is automatically retained
		_rocket = RocketModel::alloc(rockPos,(Size)rockSize);
		_rocket->setDrawScale(_scale.x);
		_rocket->setName(rocket->key());

		_rocket->setThrust(rocket->getDouble(THRUST_FIELD));
		_rocket->setDensity(rocket->getDouble(DENSITY_FIELD));
		_rocket->setFriction(rocket->getDouble(FRICTION_FIELD));
		_rocket->setRestitution(rocket->getDouble(RESTITUTION_FIELD));
		_rocket->setFixedRotation(!rocket->getBool(ROTATION_FIELD));

		std::string btype = rocket->getString(BODYTYPE_FIELD);
		if (btype == STATIC_VALUE) {
			_rocket->setBodyType(b2_staticBody);
		}

		// Set the animation nodes
		success = success && rocket->get(TEXTURE_FIELD)->isString();
		_rocket->setTextureKey(rocket->getString(TEXTURE_FIELD));

		success = success && rocket->get(MAIN_FLAMES_FIELD)->isString();
		_rocket->setBurnerStrip(RocketModel::Burner::MAIN, rocket->getString(MAIN_FLAMES_FIELD));

		success = success && rocket->get(LEFT_FLAMES_FIELD)->isString();
		_rocket->setBurnerStrip(RocketModel::Burner::LEFT, rocket->getString(LEFT_FLAMES_FIELD));

		success = success && rocket->get(RIGHT_FLAMES_FIELD)->isString();
		_rocket->setBurnerStrip(RocketModel::Burner::RIGHT, rocket->getString(RIGHT_FLAMES_FIELD));

		success = success && rocket->get(MAIN_SOUND_FIELD)->isString();
		_rocket->setBurnerSound(RocketModel::Burner::MAIN, rocket->getString(MAIN_SOUND_FIELD));

		success = success && rocket->get(LEFT_SOUND_FIELD)->isString();
		_rocket->setBurnerSound(RocketModel::Burner::LEFT, rocket->getString(LEFT_SOUND_FIELD));

		success = success && rocket->get(RIGHT_SOUND_FIELD)->isString();
		_rocket->setBurnerSound(RocketModel::Burner::RIGHT, rocket->getString(RIGHT_SOUND_FIELD));

		_rocket->setDebugColor(parseColor(rocket->getString(DEBUG_COLOR_FIELD)));

		if (success) {
			_world->addObstacle(_rocket);
		} else {
			_rocket = nullptr;
		}
	}
	return success;
}

/**
* Loads the singular exit door
*
* The exit door will will be stored in _goalDoor field and retained.
* If the exit fails to load, then _goalDoor will be nullptr.
*
* @param  reader   a JSON reader with cursor ready to read the exit
*
* @retain the exit door
* @return true if the exit door was successfully loaded
*/
bool LevelModel::loadGoalDoor(const std::shared_ptr<JsonValue>& json) {
	bool success = false;
	auto goal = json->get(GOALDOOR_FIELD);
	if (goal != nullptr) {
		success = true;

		auto posArray = goal->get(POSITION_FIELD);
		success = success && posArray->isArray();
		Vec2 goalPos = Vec2(posArray->get(0)->asFloat(), posArray->get(1)->asFloat());

		auto sizeArray = goal->get(SIZE_FIELD);
		success = success && sizeArray->isArray();
		Vec2 goalSize = Vec2(sizeArray->get(0)->asFloat(), sizeArray->get(1)->asFloat());

		// Get the object, which is automatically retained
		_goalDoor = ExitModel::alloc(goalPos,(Size)goalSize);
		_goalDoor->setName(goal->key());

		_goalDoor->setDensity(goal->getDouble(DENSITY_FIELD));
		_goalDoor->setFriction(goal->getDouble(FRICTION_FIELD));
		_goalDoor->setRestitution(goal->getDouble(RESTITUTION_FIELD));
		_goalDoor->setSensor(true);

		std::string btype = goal->getString(BODYTYPE_FIELD);
		if (btype == STATIC_VALUE) {
			_goalDoor->setBodyType(b2_staticBody);
		}

		// Set the texture value
		success = success && goal->get(TEXTURE_FIELD)->isString();
		_goalDoor->setTextureKey(goal->getString(TEXTURE_FIELD));
		_goalDoor->setDebugColor(parseColor(goal->getString(DEBUG_COLOR_FIELD)));

		if (success) {
   //   _world->addObstacle(_goalDoor);
    }
    else {
			_goalDoor = nullptr;
		}
	}
	return success;
}

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

/**
 * Adds the physics object to the physics world and loosely couples it to the scene graph
 *
 * There are two ways to link a physics object to a scene graph node on the
 * screen.  One way is to make a subclass of a physics object, like we did
 * with rocket.  The other is to use callback functions to loosely couple
 * the two.  This function is an example of the latter.
 *
 *
 * param obj    The physics object to add
 * param node   The scene graph node to attach it to
 */
void LevelModel::addObstacle(const std::shared_ptr<cugl::physics2::Obstacle>& obj,
                             const std::shared_ptr<cugl::scene2::SceneNode>& node) {
	_world->addObstacle(obj);
	obj->setDebugScene(_debugnode);

	// Position the scene graph node (enough for static objects)
	node->setPosition(obj->getPosition()*_scale);
	_worldnode->addChild(node);

	// Dynamic objects need constant updating
	if (obj->getBodyType() == b2_dynamicBody) {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
		obj->setListener([=](physics2::Obstacle* obs){
			weak->setPosition(obs->getPosition()*_scale);
			weak->setAngle(obs->getAngle());
		});
	}
}












