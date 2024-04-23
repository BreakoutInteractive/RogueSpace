#include <cugl/assets/CUJsonLoader.h>
#include "LevelModel.hpp"
#include "LevelConstants.hpp"
#include "Wall.hpp"
#include "TileLayer.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "MeleeEnemy.hpp"
#include "MeleeLizard.hpp"
#include "RangedEnemy.hpp"
#include "RangedLizard.hpp"
#include "MageAlien.hpp"
#include "../utility/LevelParser.hpp"
#include "GameObject.hpp"
#include "CollisionConstants.hpp"
#include "GameConstants.hpp"
#include <random>

#pragma mark -
#pragma mark Static Constructors

using namespace cugl;

/**
* Creates a new, empty level.
*/
LevelModel::LevelModel(void):
_world(nullptr),
_debugNode(nullptr),
_exiting(false)
{
	_bounds.size.set(1.0f, 1.0f);
    generator = std::mt19937(std::random_device()());
    distribution = std::uniform_real_distribution<double>(0.0, 1.0);
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
    for (int ii = 0; ii < _tileLayers.size(); ii++){
        _tileLayers[ii]->setDrawScale(scale);
    }
    
    for (int ii = 0; ii < _walls.size(); ii++){
        _walls[ii]->setDrawScale(scale);
    }
    
    for (int ii = 0; ii < _enemies.size(); ii++){
        _enemies[ii]->setDrawScale(scale);
    }
    for (int ii = 0; ii < _projectiles.size(); ii++) _projectiles[ii]->setDrawScale(scale);
}

void LevelModel::render(const std::shared_ptr<cugl::SpriteBatch>& batch){
    for (int ii = 0; ii < _tileLayers.size(); ii++){
        _tileLayers[ii]->draw(batch);
    }
    
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
//        if (_enemies[ii]->isEnabled()) {
//            batch->draw(_attackAnimation, Color4(255,255,255,64), (Vec2)_attackAnimation->getSize() / 2, _enemies[ii]->getSightRange()/((Vec2)_attackAnimation->getSize()/2) * _scale,
//                        _enemies[ii]->getFacingDir().getAngle() + M_PI_2, _enemies[ii]->getPosition() * _scale);
//        }
    }

    for (int ii = 0; ii < _projectiles.size(); ii++) _projectiles[ii]->draw(batch);
    
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
    }
    for (int ii = 0; ii < _projectiles.size(); ii++) {
        _projectiles[ii]->getCollider()->getDebugNode()->setVisible(_projectiles[ii]->isEnabled());
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
    _debugNode->setVisible(false);
    node->addChild(_debugNode);
    
    // debug node should be added once objects are initialized
    _player->setDebugNode(_debugNode);
    _atk->setDebugScene(_debugNode);
    _atk->setDebugColor(Color4::RED);

    for (int ii = 0; ii < _enemies.size(); ii++){
        _enemies[ii]->getAttack()->setDebugScene(_debugNode);
        _enemies[ii]->getAttack()->setDebugColor(Color4::RED);
        _enemies[ii]->getColliderShadow()->setDebugColor(Color4::BLUE);
        _enemies[ii]->setDebugNode(_debugNode);
    }
    
    for (int ii = 0; ii < _walls.size(); ii++){
        _walls[ii]->setDebugNode(_debugNode);
        _walls[ii]->getCollider()->setDebugColor(Color4::WHITE);
    }
    
    for (int ii = 0; ii < _boundaries.size(); ii++){
        _boundaries[ii]->setDebugScene(_debugNode);
        _boundaries[ii]->setDebugColor(Color4::WHITE);
    }

    for (int ii = 0; ii < _projectiles.size(); ii++) {
        _projectiles[ii]->setDebugNode(_debugNode);
        _projectiles[ii]->getCollider()->setDebugColor(Color4::RED);
        _projectiles[ii]->getColliderShadow()->setDebugColor(Color4::BLUE);
    }
}

void LevelModel::setAssets(const std::shared_ptr<AssetManager> &assets){
    _assets = assets;
    _player->loadAssets(assets);
    for (int ii = 0; ii < _tileLayers.size(); ii++){
        _tileLayers[ii]->loadAssets(assets);
    }
    
    for (int ii = 0; ii < _walls.size(); ii++){
        _walls[ii]->loadAssets(assets);
    }

    _attackAnimation = assets->get<Texture>("atk");
    std::shared_ptr<Texture> t = assets->get<Texture>("player-swipe");
    std::shared_ptr<SpriteSheet> s = SpriteSheet::alloc(t, 2, 3);
    _playerAttack = Animation::alloc(s, 0.25f, false); //0.25 seconds is approximately the previous length of the attack (16 frames at 60 fps)
    std::shared_ptr<Texture> t2 = assets->get<Texture>("enemy-swipe");
    std::shared_ptr<SpriteSheet> s2 = SpriteSheet::alloc(t2, 2, 3);

    for (int ii = 0; ii < _enemies.size(); ii++){
        _enemies[ii]->loadAssets(assets);
        _enemies[ii]->setHitboxAnimation(Animation::alloc(s2, 0.375f, false)); //0.25 seconds is approximately the previous length of the attack (16 frames at 60 fps);
    }
}


void LevelModel::showDebug(bool flag) {
	if (_debugNode != nullptr) {
		_debugNode->setVisible(flag);
	}
}


#pragma mark -
#pragma mark Level Loading

bool LevelModel::init(const std::shared_ptr<JsonValue>& constants, std::shared_ptr<JsonValue> parsedJson) {
	if (constants == nullptr) {
		CUAssertLog(false, "Failed to load constants file");
		return false;
	}
    if (parsedJson == nullptr){
        CUAssertLog(false, "Failed to load level file");
        return false;
    }
	// Initial geometry
	float w = parsedJson->get(WIDTH_FIELD)->asFloat();
	float h = parsedJson->get(HEIGHT_FIELD)->asFloat();
	_bounds.size.set(w, h);
    float vw = constants->get("view-width")->asFloat();
    float vh = constants->get("view-height")->asFloat();
    _viewBounds.set(vw, vh);

	/** Create the physics world */
	_world = physics2::ObstacleWorld::alloc(getBounds(),Vec2::ZERO);
    
    /** Create the grid for pathfinding around static obstacles */
    std::shared_ptr<JsonValue> gridData = parsedJson->get("grid");
    int gridWidth = gridData->getInt("width");
    int gridHeight = gridData->getInt("height");
    auto gridOrigin = gridData->get("origin")->asFloatArray();
    _grid = std::make_shared<LevelGrid>(gridWidth, gridHeight, Vec2(gridOrigin[0], gridOrigin[1]));
    
    
    // load the map
    std::vector<std::shared_ptr<JsonValue>> layers = parsedJson->get(MAP_FIELD)->children();
    for (std::shared_ptr<JsonValue>& layer : layers){
        loadGameComponent(constants, layer);
    }
    CUAssertLog(_player != nullptr, "No player could be generated for the given map, either no player is added to the map or player is being randomized!");
    CUAssertLog(_enemies.size() > 0, "No enemies could be found/generated!");
    CUAssertLog(_tileLayers.size() > 0, "no tile layers found!");

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
    
    for (int ii = 0; ii < _walls.size(); ii++){
        _walls[ii]->addObstaclesToWorld(_world);
        _dynamicObjects.push_back(_walls[ii]); // TODO: need to separate out walls to reduce sorting overhead;
    }
    
    for (int ii = 0; ii < _boundaries.size(); ii++){
        addObstacle(_boundaries[ii]);
    }
	return true;
}


void LevelModel::unload() {
    if (_world != nullptr) {
        for(auto it = _enemies.begin(); it != _enemies.end(); ++it) {
            (*it)->removeObstaclesFromWorld(_world);
        }
        for(auto it = _walls.begin(); it != _walls.end(); ++it) {
            (*it)->removeObstaclesFromWorld(_world);
        }
        for(auto it = _boundaries.begin(); it != _boundaries.end(); ++it) {
            _world->removeObstacle(*it);
        }
    }
	_enemies.clear();
	_walls.clear();
    _energyWalls.clear();
    
    _player->removeObstaclesFromWorld(_world);
    _player = nullptr;
    
    if (_world != nullptr) {
        _world->clear();
        _world = nullptr;
    }
    
    _dynamicObjects.clear();
    _tileLayers.clear();
}


#pragma mark -
#pragma mark Individual Loaders

bool LevelModel::loadGameComponent(const std::shared_ptr<JsonValue> constants, const std::shared_ptr<JsonValue> &json){
    bool success = true;
    std::string objectClass = json->getString(CLASS);
    if (objectClass == CLASS_COLLECTION){
        auto contents = json->get(CONTENTS_FIELD)->children();
        for (std::shared_ptr<JsonValue>& childJson : contents){
            if (!loadGameComponent(constants, childJson)){
                return false;
            }
        }
    }
    else if (objectClass == CLASS_RANDOM){
        auto cdf = json->get(CDF_FIELD)->asFloatArray();
        if (cdf.size() > 0){
            float probability = distribution(generator);
            for (int i = 0; i < cdf.size(); i++){
                if (cdf[i] >= probability){
                    // get i-th component, load it
                    loadGameComponent(constants, json->get(CONTENTS_FIELD)->get(i));
                    break;
                }
            }
        }
    }
    else if (objectClass == CLASS_WALL){
        loadWall(json);
    }
    else if (objectClass == CLASS_ENEMY){
        loadEnemy(constants->get(ENEMY_FIELD), json);
    }
    else if (objectClass == CLASS_PLAYER){
        loadPlayer(constants->get(PLAYER_FIELD), json);
    }
    else if (objectClass == CLASS_TILELAYER){
        loadTileLayer(json); // this changes the grid data structure by marking walkable tiles
    }
    else {
        CUAssertLog(false, "unrecognized data type to load");
    }
    return success;
}

bool LevelModel::loadPlayer(const std::shared_ptr<JsonValue> constants, const std::shared_ptr<JsonValue> &json){
    bool success = true;

    // Get the object, which is automatically retained
    _player = Player::alloc(json);
    auto playerCollider = _player->getCollider();
    playerCollider->setDensity(constants->getDouble(DENSITY_FIELD));
    playerCollider->setFriction(constants->getDouble(FRICTION_FIELD));
    playerCollider->setRestitution(constants->getDouble(RESTITUTION_FIELD));
    playerCollider->setFixedRotation(!constants->getBool(ROTATION_FIELD));
    playerCollider->setDebugColor(parseColor(constants->getString(DEBUG_COLOR_FIELD)));
    _player->setTextureKey(constants->getString(TEXTURE_FIELD)); //idle spritesheet
    _player->setParryTextureKey(constants->getString(PARRY_FIELD));
    _player->setAttackTextureKey(constants->getString(ATTACK_FIELD));
    _player->setDrawScale(_scale);

    std::string btype = constants->getString(BODYTYPE_FIELD);
    if (btype == STATIC_VALUE) {
        playerCollider->setBodyType(b2_staticBody);
    }

    //setup the attack for collision detection
    Vec2 pos(json->getFloat("x"), json->getFloat("y"));
	_atk = physics2::WheelObstacle::alloc(pos, GameConstants::PLAYER_MELEE_ATK_RANGE);
	_atk->setSensor(true);
	_atk->setBodyType(b2_dynamicBody);
    b2Filter filter;
    // this is an attack and, since it is the player's, can collide with enemies
    filter.categoryBits = CATEGORY_ATTACK;
    filter.maskBits = CATEGORY_ENEMY | CATEGORY_ENEMY_HITBOX;
    _atk->setFilterData(filter);
    return success;
}

bool LevelModel::loadEnemy(const std::shared_ptr<JsonValue> constants, const std::shared_ptr<JsonValue> &json){
    std::shared_ptr<Enemy> enemy;
    std::string enemyType = json->getString("type");
    if (enemyType == "melee-lizard") {
        enemy = MeleeLizard::alloc(json);
    }
    else if (enemyType == "ranged-lizard") {
        enemy = RangedLizard::alloc(json);
    }
    else if (enemyType == "caster") {
        enemy = MageAlien::alloc(json);
    }
    CUAssertLog(enemy != nullptr, "enemy type %s is not allowed", enemyType.c_str());
    auto enemyCollider = enemy->getCollider();
    enemyCollider->setName("enemy-" + std::to_string(_enemies.size()));
    enemyCollider->setDensity(constants->getDouble(DENSITY_FIELD));
    enemyCollider->setFriction(constants->getDouble(FRICTION_FIELD));
    enemyCollider->setRestitution(constants->getDouble(RESTITUTION_FIELD));
    enemyCollider->setFixedRotation(!constants->getBool(ROTATION_FIELD));
    enemyCollider->setDebugColor(parseColor(constants->getString(DEBUG_COLOR_FIELD)));
    
    enemy->setHealth(json->getInt("health"));
    enemy->setDefaultState(json->getString("defaultstate"));
    std::vector<Vec2> path;
    std::vector<float> vertices = json->get("path")->asFloatArray();
    Vec2* verts = reinterpret_cast<Vec2*>(&vertices[0]);
    auto numPoints = json->get("path")->size() / 2;
    for (int j = 0; j < numPoints ; j++) {
        path.push_back(verts[j]);
    }
    enemy->setPath(path);
    if (enemy->getDefaultState() == "patrol") {
        enemy->setGoal(enemy->getPath()[0]);
        enemy->setPathIndex(0);
    }
    std::string btype = constants->getString(BODYTYPE_FIELD);
    if (btype == STATIC_VALUE) {
        enemyCollider->setBodyType(b2_staticBody);
    }
    _enemies.push_back(enemy);
    
    // attack setup
    b2Filter filter;
    auto attack = physics2::WheelObstacle::alloc(enemyCollider->getPosition(), GameConstants::ENEMY_MELEE_ATK_RANGE);
    attack->setSensor(true);
    attack->setName("enemy-attack");
    attack->setBodyType(b2_dynamicBody);
    // this is an attack
    filter.categoryBits = CATEGORY_ATTACK;
    // since it is an enemy's attack, it can collide with the player
    filter.maskBits = CATEGORY_PLAYER | CATEGORY_PLAYER_HITBOX;
    attack->setFilterData(filter);
    enemy->setAttack(attack);
    return true;
}

bool LevelModel::loadTileLayer(const std::shared_ptr<JsonValue> &json){
    auto tileArray = json->get("tiles")->children();
    std::shared_ptr<TileLayer> tileLayer = TileLayer::alloc();
    for (int idx = 0; idx < tileArray.size(); idx++){
        std::shared_ptr<Tile> tile = Tile::alloc(tileArray[idx]);
        tileLayer->addTile(tile);
        _grid->setNode(_grid->worldToTile(tile->getPosition()), 1); // 1 means walkable for now
    }
    _tileLayers.push_back(tileLayer);
    return true;
}


bool LevelModel::loadWall(const std::shared_ptr<JsonValue>& json) {
	bool success = true;

    std::shared_ptr<JsonValue> colliderData = json->get("collider");
    Vec2 obstaclePosition(colliderData->getFloat("x"), colliderData->getFloat("y"));
	std::vector<float> vertices = colliderData->get("vertices")->asFloatArray();
	success = vertices.size() >= 2 && vertices.size() % 2 == 0;
    
    Vec2* verts = reinterpret_cast<Vec2*>(&vertices[0]);
	Poly2 polygon(verts,(int)vertices.size()/2);
	EarclipTriangulator triangulator;
	triangulator.set(polygon.vertices);
	triangulator.calculate();
	polygon.setIndices(triangulator.getTriangulation());
    triangulator.clear();
	
	// Get the object, which is automatically retained
    if (success){
        if (json->getString("type") == "Energy"){
            std::shared_ptr<EnergyWall> wallobj = std::make_shared<EnergyWall>(json, polygon, obstaclePosition);
            _grid->setNode(_grid->worldToTile(wallobj->getPosition()), 0); // 0 means non-walkable for now
            _energyWalls.push_back(wallobj);
            _walls.push_back(wallobj);
        }
        else {
            std::shared_ptr<Wall> wallobj = std::make_shared<Wall>(json, polygon, obstaclePosition);
            _grid->setNode(_grid->worldToTile(wallobj->getPosition()), 0); // 0 means non-walkable for now
            _walls.push_back(wallobj);
        }
    }
	return success;
}

bool LevelModel::loadBoundary(const std::shared_ptr<JsonValue>& json) {
    bool success = true;
    
    Vec2 pos(json->getFloat("x"), json->getFloat("y"));
    std::vector<float> vertices = json->get("vertices")->asFloatArray();
    success = vertices.size() >= 2 && vertices.size() % 2 == 0;
    
    Vec2* verts = reinterpret_cast<Vec2*>(&vertices[0]);
    Poly2 polygon(verts,(int)vertices.size()/2);
    EarclipTriangulator triangulator;
    triangulator.set(polygon.vertices);
    triangulator.calculate();
    polygon.setIndices(triangulator.getTriangulation());
    triangulator.clear();
    
    // Get the object, which is automatically retained
    if (success){
        auto p = std::make_shared<physics2::PolygonObstacle>();
        p->PolygonObstacle::init(polygon, pos);
        b2Filter filter;
        // this is a wall
        filter.categoryBits = CATEGORY_WALL;
        // a wall can collide with a player or an enemy
        filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
        p->setFilterData(filter);
        _boundaries.push_back(p);
    }
    return success;
}


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

void LevelModel::addProjectile(std::shared_ptr<Projectile> p) {
    _projectiles.push_back(p);
    _dynamicObjects.push_back(p);
    p->addObstaclesToWorld(_world);
    p->setDebugNode(_debugNode);
    p->getCollider()->setDebugColor(Color4::RED);
    p->getColliderShadow()->setDebugColor(Color4::BLUE);
}

void LevelModel::delProjectile(std::shared_ptr<Projectile> p) {
    for (auto it = _projectiles.begin(); it != _projectiles.end(); ++it) {
        if ((*it) == p) {
            p->removeObstaclesFromWorld(_world);
            p->dispose();
            _projectiles.erase(it);
            return;
        }
    }
}
