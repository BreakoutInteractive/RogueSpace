#include "Projectile.hpp"

bool Projectile::playerInit(Vec2 pos, const std::shared_ptr<AssetManager>& assets) {
	//init fields
	_enabled = true;
	_position = pos;
	_tint = Color4::WHITE;
	_drawScale.set(1.0f, 1.0f);

	//init hitbox
	std::vector<Vec2> v;
	v.push_back(pos+Vec2(0, GameConstants::PROJ_SIZE_P_HALF));
	v.push_back(pos+Vec2(-GameConstants::PROJ_SIZE_P_HALF, 0));
	v.push_back(pos+Vec2(0, -GameConstants::PROJ_SIZE_P_HALF));
	v.push_back(pos+Vec2(GameConstants::PROJ_SIZE_P_HALF, 0));
	EarclipTriangulator et = EarclipTriangulator::EarclipTriangulator(v);
	et.calculate();
	Poly2 poly = et.getPolygon();
	std::shared_ptr<physics2::PolygonObstacle> obs = physics2::PolygonObstacle::allocWithAnchor(poly, Vec2(0.5f,0.5f));
	obs->setName("projectile-collider");
	obs->setPosition(pos);
	b2Filter filter;
	//projectiles are attacks. they can hit enemies and are destroyed on contact with a wall.
	filter.categoryBits = CATEGORY_ATTACK;
	filter.maskBits = CATEGORY_ENEMY | CATEGORY_WALL;
	obs->setFilterData(filter);
	obs->setEnabled(true);
	obs->setAwake(true);
	obs->setBodyType(b2_kinematicBody);
	//might need this depending on projectile speed
	// obs->setBullet(true);
	_collider = obs;
	std::shared_ptr<Texture> t = assets->get<Texture>("player-projectile");
	//TODO: modify this to use the right frames
	_flyingAnimation = Animation::alloc(SpriteSheet::alloc(t,4,4), 0.125f, true, 9, 11); //0.125 because 3 frames/24 fps = 1/8 seconds
	_explodingAnimation = Animation::alloc(SpriteSheet::alloc(t, 4, 4), 0.25f, false, 12, 15);
	setFlying();
	return true;
}

bool Projectile::enemyInit(Vec2 pos, const std::shared_ptr<AssetManager>& assets) {
	//TODO: For now, the enemy and player have the same projectile. Change the enemy projectile once we have the final one

	//init fields
	_enabled = true;
	_position = pos;
	_tint = Color4::WHITE;
	_drawScale.set(1.0f, 1.0f);

	//init hitbox
	//TODO: modify shape and size
	std::vector<Vec2> v;
	v.push_back(pos + Vec2(0, GameConstants::PROJ_SIZE_E_HALF));
	v.push_back(pos + Vec2(-GameConstants::PROJ_SIZE_E_HALF, 0));
	v.push_back(pos + Vec2(0, -GameConstants::PROJ_SIZE_E_HALF));
	v.push_back(pos + Vec2(GameConstants::PROJ_SIZE_E_HALF, 0));
	EarclipTriangulator et = EarclipTriangulator::EarclipTriangulator(v);
	et.calculate();
	Poly2 poly = et.getPolygon();
	std::shared_ptr<physics2::PolygonObstacle> obs = physics2::PolygonObstacle::allocWithAnchor(poly, Vec2(0.5f, 0.5f));
	obs->setName("projectile-collider");
	obs->setPosition(pos);
	b2Filter filter;
	//projectiles are attacks. they can hit players and are destroyed on contact with a wall.
	filter.categoryBits = CATEGORY_ATTACK;
	filter.maskBits = CATEGORY_PLAYER | CATEGORY_WALL;
	obs->setFilterData(filter);
	obs->setEnabled(true);
	obs->setAwake(true);
	obs->setBodyType(b2_kinematicBody);
	//might need this depending on projectile speed
	// obs->setBullet(true);
	_collider = obs;
	std::shared_ptr<Texture> t = assets->get<Texture>("player-projectile");
	//TODO: modify this to use the right frames
	_flyingAnimation = Animation::alloc(SpriteSheet::alloc(t, 4, 4), 0.125f, true, 9, 11); //0.125 because 3 frames/24 fps = 1/8 seconds
	_explodingAnimation = Animation::alloc(SpriteSheet::alloc(t, 4, 4), 0.25f, false, 12, 15);
	setFlying();
	return true;
}

void Projectile::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
	if (_currAnimation->isActive()) {
		auto spriteSheet = _currAnimation->getSpriteSheet();
		Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, spriteSheet->getFrameSize().height / 2);
		Affine2 transform = Affine2::createRotation(_collider->getAngle());
		transform.scale(_drawScale/32);
		transform.translate(getPosition() * _drawScale);
		spriteSheet->draw(batch, _tint, origin, transform);
	}
}

bool Projectile::isCompleted() {
	if (_state == FLYING) {
		if ((_collider->getFilterData().maskBits & CATEGORY_PLAYER) == CATEGORY_PLAYER)
			//if this can hit the player, it belongs to an enemy
			return _currAnimation->elapsed() >= GameConstants::PROJ_TIME_E;
		else if ((_collider->getFilterData().maskBits & CATEGORY_ENEMY) == CATEGORY_ENEMY)
			//if this can hit enemies, it belongs to the player
			return _currAnimation->elapsed() >= GameConstants::PROJ_TIME_P;
	}
	else if (_state == EXPLODING) return !_currAnimation->isActive();
}

void Projectile::dispose() { 
	_currAnimation = nullptr; 
	_collider = nullptr; 
	setEnabled(false);
}