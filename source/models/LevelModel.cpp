#include <cugl/assets/CUJsonLoader.h>
#include "LevelModel.hpp"
#include "LevelConstants.hpp"
#include "JSWallModel.hpp"
#include "Floor.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "MeleeEnemy.hpp"
#include "RangedEnemy.hpp"
#include "../utility/LevelParser.hpp"
#include "GameObject.hpp"
#include "CollisionConstants.hpp"
#include "GameConstants.hpp"

#pragma mark -
#pragma mark Static Constructors

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
    _scale = scale;
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
    
    for (int ii = 0; ii < _enemies.size(); ii++){
        _enemies[ii]->setDrawScale(scale);
    }
}

void LevelModel::render(const std::shared_ptr<cugl::SpriteBatch>& batch){
    _floor->draw(batch);
    
    // sort elements to be drawn
    std::sort(_dynamicObjects.begin(), _dynamicObjects.end(),
        [](const std::shared_ptr<GameObject>& a, const std::shared_ptr<GameObject>& b) {
            return (*a) < (*b);
        });
    for (auto it = _dynamicObjects.begin(); it != _dynamicObjects.end(); it++){
        if ((*it)->isEnabled()){
            (*it)->draw(batch);
        }
    }
    
    for (int ii = 0; ii < _enemies.size(); ii++){
        auto enemyAtk = _enemies[ii]->getAttack();
        //only draw the effect when the enemy's attack hitbox is enabled (when swinging the knife)
        if (enemyAtk->isEnabled()) {
            auto sheet = _enemies[ii]->getHitboxAnimation()->getSpriteSheet();
            Affine2 atkTrans = Affine2::createScale( GameConstants::ENEMY_MELEE_ATK_RANGE/ ((Vec2)sheet->getFrameSize() / 2) * _scale);
            atkTrans.rotate(enemyAtk->getAngle() - M_PI_2);
            atkTrans.translate(enemyAtk->getPosition() * _scale);
            sheet->draw(batch, Color4::WHITE, Vec2(sheet->getFrameSize().getIWidth() / 2, 0), atkTrans);
            /*batch->draw(_attackAnimation, Color4(255,255,255,200), (Vec2)_attackAnimation->getSize() / 2, ATK_RADIUS/((Vec2)_attackAnimation->getSize()/2) * _scale,
                _enemies[ii]->getAttack()->getAngle() + M_PI_2, _enemies[ii]->getAttack()->getPosition() * _scale);*/
        }
        if (_enemies[ii]->isEnabled()) {
            batch->draw(_attackAnimation, Color4(255,255,255,64), (Vec2)_attackAnimation->getSize() / 2, _enemies[ii]->getSightRange()/((Vec2)_attackAnimation->getSize()/2) * _scale,
                        _enemies[ii]->getFacingDir().getAngle() + M_PI_2, _enemies[ii]->getPosition() * _scale);
        }
    }
    
    
    if (_playerAttack->isActive()){
        auto sheet = _playerAttack->getSpriteSheet();
        Affine2 atkTrans = Affine2::createScale(GameConstants::PLAYER_MELEE_ATK_RANGE / ((Vec2)sheet->getFrameSize() / 2) * _scale);
        //we subtract pi/2 from the angle since the animation is pointing up but the hitbox points right by default
        atkTrans.rotate(_atk->getAngle() - M_PI_2);
        atkTrans.translate(_atk->getPosition() * _scale);
        sheet->draw(batch, Color4::WHITE, Vec2(sheet->getFrameSize().getIWidth() / 2, 0), atkTrans);
        //batch->draw(_attackAnimation, Color4(255,255,255,200), (Vec2)_attackAnimation->getSize() / 2, ATK_RADIUS/((Vec2)_attackAnimation->getSize()/2) * _scale,
        //    _atk->getAngle() + M_PI_2, _atk->getPosition() * _scale);
    }
    
    
    // make sure debug node is hidden when not active
    _atk->getDebugNode()->setVisible(_atk->isEnabled());
    
    for (int ii = 0; ii < _enemies.size(); ii++){
        _enemies[ii]->getAttack()->getDebugNode()->setVisible(_enemies[ii]->getAttack()->isEnabled());
        _enemies[ii]->getCollider()->getDebugNode()->setVisible(_enemies[ii]->isEnabled());
        _enemies[ii]->getColliderShadow()->getDebugNode()->setVisible(_enemies[ii]->isEnabled());
    }

}

void LevelModel::clearDebugNode(){
    if (_debugNode != nullptr){
        _debugNode->removeAllChildren();
        _debugNode = nullptr;
    }
}

void LevelModel::setDebugNode(const std::shared_ptr<scene2::SceneNode> & node) {
	if (_debugNode != nullptr) {
		clearDebugNode();
	}
    
	_debugNode = scene2::SceneNode::alloc();
	//_scale.set(node->getContentSize().width/_viewBounds.width,
             //node->getContentSize().height/_viewBounds.height);

    _debugNode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugNode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugNode->setPosition(Vec2::ZERO);
    node->addChild(_debugNode);
    
    // debug node should be added once objects are initialized
    _player->getCollider()->setDebugScene(_debugNode);
    _player->getColliderShadow()->setDebugScene(_debugNode);
    _player->getColliderShadow()->setDebugColor(Color4::RED);
    _atk->setDebugScene(_debugNode);
    _atk->setDebugColor(Color4::RED);

    for (int ii = 0; ii < _enemies.size(); ii++){
        auto enemyCollider = _enemies[ii]->getCollider();
        enemyCollider->setDebugScene(_debugNode);
        _enemies[ii]->getAttack()->setDebugScene(_debugNode);
        _enemies[ii]->getAttack()->setDebugColor(Color4::RED);
        _enemies[ii]->getColliderShadow()->setDebugScene(_debugNode);
        _enemies[ii]->getColliderShadow()->setDebugColor(Color4::BLUE);
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
    std::shared_ptr<Texture> t = assets->get<Texture>("player-swipe");
    std::shared_ptr<SpriteSheet> s = SpriteSheet::alloc(t, 2, 3);
    _playerAttack = Animation::alloc(s, 0.25f, false); //0.25 seconds is approximately the previous length of the attack (16 frames at 60 fps)
    std::shared_ptr<Texture> t2 = assets->get<Texture>("enemy-swipe");
    std::shared_ptr<SpriteSheet> s2 = SpriteSheet::alloc(t2, 2, 3);

    for (int ii = 0; ii < _enemies.size(); ii++){
        _enemies[ii]->loadAssets(assets);
        _enemies[ii]->setHitboxAnimation(Animation::alloc(s2, 0.25f, false)); //0.25 seconds is approximately the previous length of the attack (16 frames at 60 fps);
    }
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
    LevelParser ls = LevelParser();
    std::shared_ptr<JsonValue> newParse = ls.preload("json/test_room.json");
//    CULog(newParse->toString().c_str());
    
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
    tempJSON = reader->readJson();
    return preload(newParse);
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
    float vw = json->get("view-width")->asFloat();
    float vh = json->get("view-height")->asFloat();
    _viewBounds.set(vw, vh);

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

//	auto walls = json->get(WALLS_FIELD);
//	if (walls != nullptr) {
//		// Convert the object to an array so we can see keys and values
//		int wsize = (int)walls->size();
//		for(int ii = 0; ii < wsize; ii++) {
//			loadWall(walls->get(ii));
//		}
//	} else {
//		CUAssertLog(false, "Failed to load walls");
//		return false;
//	}
    
    auto enemiesJson = json->get("enemies");
    if (enemiesJson != nullptr){
        loadEnemies(enemiesJson);
    }
    else {
        CUAssertLog(false, "Failed to load enemies");
        return false;
    }

    // Add objects to world
    _player->addObstaclesToWorld(_world);
	addObstacle(_atk);
    _atk->setEnabled(false); // turn off the attack semisphere
    _dynamicObjects.push_back(_player); // add the player to sorting layer
    
    for (int ii = 0; ii < _enemies.size(); ii++){
        _enemies[ii]->addObstaclesToWorld(_world);
        addObstacle(_enemies[ii]->getAttack());
        _enemies[ii]->getAttack()->setEnabled(false);
        
        _dynamicObjects.push_back(_enemies[ii]); // add the enemies to sorting layer
    }
//    for (int ii = 0; ii < _walls.size(); ii++){
//        addObstacle(_walls[ii]);
//    }
    
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


void LevelModel::unload() {
    if (_world != nullptr) {
        for(auto it = _enemies.begin(); it != _enemies.end(); ++it) {
            (*it)->removeObstaclesFromWorld(_world);
        }
        
        for(auto it = _walls.begin(); it != _walls.end(); ++it) {
                _world->removeObstacle((*it));
        }
    }
	_enemies.clear();
	_walls.clear();
    
    _player->removeObstaclesFromWorld(_world);
    
    _player = nullptr;
    _floor = nullptr;
    if (_world != nullptr) {
        _world->clear();
        _world = nullptr;
    }
    
    _dynamicObjects.clear();
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
    auto playerCollider = _player->getCollider();
    playerCollider->setDensity(json->getDouble(DENSITY_FIELD));
    playerCollider->setFriction(json->getDouble(FRICTION_FIELD));
    playerCollider->setRestitution(json->getDouble(RESTITUTION_FIELD));
    playerCollider->setFixedRotation(!json->getBool(ROTATION_FIELD));
    playerCollider->setDebugColor(parseColor(json->getString(DEBUG_COLOR_FIELD)));
    _player->setTextureKey(json->getString(TEXTURE_FIELD)); //idle spritesheet
    _player->setParryTextureKey(json->getString(PARRY_FIELD));
    _player->setAttackTextureKey(json->getString(ATTACK_FIELD));
    _player->setDrawScale(_scale);

    std::string btype = json->getString(BODYTYPE_FIELD);
    if (btype == STATIC_VALUE) {
        playerCollider->setBodyType(b2_staticBody);
    }

    //setup the attack for collision detection
	_atk = physics2::WheelObstacle::alloc(pos, GameConstants::PLAYER_MELEE_ATK_RANGE);
	_atk->setSensor(true);
	_atk->setBodyType(b2_dynamicBody);
    b2Filter filter;
    // this is an attack and can collide with a player or an enemy
    filter.categoryBits = CATEGORY_ATTACK;
    filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
    _atk->setFilterData(filter);
    return success;
}

bool LevelModel::loadEnemies(const std::shared_ptr<JsonValue> &data){
    int count = (int) data->size();
    for (int ii = 0; ii < count; ii++){
        auto json = data->get(ii);
        auto posData = json->get(POSITION_FIELD);
        auto sizeArray = json->get(SIZE_FIELD);
        Vec2 pos(posData->get(0)->asFloat(), posData->get(1)->asFloat());
        Size size(sizeArray->get(0)->asFloat(), sizeArray->get(1)->asFloat());
        auto enemy = Enemy::alloc(pos, size);
        auto enemyCollider = enemy->getCollider();
        enemyCollider->setName("enemy-" + std::to_string(ii));
        enemyCollider->setDensity(json->getDouble(DENSITY_FIELD));
        enemyCollider->setFriction(json->getDouble(FRICTION_FIELD));
        enemyCollider->setRestitution(json->getDouble(RESTITUTION_FIELD));
        enemyCollider->setFixedRotation(!json->getBool(ROTATION_FIELD));
        enemyCollider->setDebugColor(parseColor(json->getString(DEBUG_COLOR_FIELD)));
        
        enemy->setTextureKey(json->getString(TEXTURE_FIELD));
        enemy->setHealth(json->getInt("health"));
        enemy->setDefaultState(json->getString("defaultstate"));
        std::vector<Vec2> path;
        auto pathData = json->get("path");
        for (int j = 0; j < pathData->size(); j++) {
            Vec2 node(pathData->get(j)->get(0)->asFloat(), pathData->get(j)->get(1)->asFloat());
            path.push_back(node);
        }
        enemy->setPath(path);
        if (enemy->getDefaultState() == "patrol") {
            enemy->setGoal(enemy->getPath()[0]);
            enemy->setPathIndex(0);
        }
        std::string btype = json->getString(BODYTYPE_FIELD);
        if (btype == STATIC_VALUE) {
            enemyCollider->setBodyType(b2_staticBody);
        }
        _enemies.push_back(enemy);
        
        // attack setup
        b2Filter filter;
        auto attack = physics2::WheelObstacle::alloc(pos, GameConstants::ENEMY_MELEE_ATK_RANGE);
        attack->setSensor(true);
        attack->setBodyType(b2_dynamicBody);
        // this is an attack
        filter.categoryBits = CATEGORY_ATTACK;
        // an attack can collide with a player or an enemy
        filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
        attack->setFilterData(filter);
        enemy->setAttack(attack);
    }
    return true;
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
        
        std::string layers[] = {"bottom-right", "bottom-left", "floor"};
        for (int i = 0; i < json->get("layers")->asInt(); i++) {
            // TODO: convert information about the layer kind into
            std::shared_ptr<JsonValue> layerData = json->get("tiles")->get(layers[i]);
            for (int j = 0; j < (int) layerData->get("size")->asInt(); j++) {
                std::vector<float> posData = layerData->get("tiles")->get(j)->asFloatArray();
                Vec2 pos(posData[0], posData[1]);
                auto tile = Tile::alloc(pos, textureName, layers[i]);
                tiles.emplace_back(tile);
            }
        }
    }
    else {
        // generate grid of tiles automatically
        // deprecated?
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
                auto tile = Tile::alloc(pos, textureName, "");
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
    
    b2Filter filter;
    // this is a wall
    filter.categoryBits = CATEGORY_WALL;
    // a wall can collide with a player or an enemy
    filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
    wallobj->setFilterData(filter);

	if (success) {
		_walls.push_back(wallobj);
	} else {
		wallobj = nullptr;
	}

	vertices.clear();
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

