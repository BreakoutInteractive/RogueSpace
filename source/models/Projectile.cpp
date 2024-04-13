#include "Projectile.hpp"

bool Projectile::playerInit(Vec2 pos, int damage, const std::shared_ptr<AssetManager>& assets) {
	//init fields
	_enabled = true;
	_position = pos;
	_tint = Color4::WHITE;
	_drawScale.set(1.0f, 1.0f);
	_damage = damage;
	_state = FLYING;

	//init hitbox
	std::vector<Vec2> v;
	//halve the height because of iso perspective
	v.push_back(pos+Vec2(0, GameConstants::PROJ_SIZE_P_HALF/2));
	v.push_back(pos+Vec2(-GameConstants::PROJ_SIZE_P_HALF, 0));
	v.push_back(pos+Vec2(0, -GameConstants::PROJ_SIZE_P_HALF/2));
	v.push_back(pos+Vec2(GameConstants::PROJ_SIZE_P_HALF, 0));
    EarclipTriangulator et;
    et.set(v);
	et.calculate();
	Poly2 poly = et.getPolygon();
	std::shared_ptr<physics2::PolygonObstacle> obs = physics2::PolygonObstacle::allocWithAnchor(poly, Vec2(0.5f,0.5f));
	obs->setName("player-projectile-collider");
	obs->setPosition(pos);
	b2Filter filter;
	//projectiles are attacks. they can hit enemies and are destroyed on contact with a wall.
	filter.categoryBits = CATEGORY_PROJECTILE;
	filter.maskBits = CATEGORY_ENEMY | CATEGORY_ENEMY_HITBOX | CATEGORY_WALL;
	obs->setFilterData(filter);
	obs->setEnabled(true);
	obs->setAwake(true);
	//obs->setBodyType(b2_kinematicBody);
	//might need this depending on projectile speed
	// obs->setBullet(true);
	_collider = obs;
	std::shared_ptr<Texture> t = assets->get<Texture>("player-projectile");
	//TODO: modify this to use the right frames
	_flyingAnimation = Animation::alloc(SpriteSheet::alloc(t,4,4), 0.125f, true, 9, 11); //0.125 because 3 frames/24 fps = 1/8 seconds
	_explodingAnimation = Animation::alloc(SpriteSheet::alloc(t, 4, 4), 0.25f, false);
	setFlying();
	return true;
}

bool Projectile::lizardInit(Vec2 pos, int damage, const std::shared_ptr<AssetManager>& assets) {
	//TODO: For now, the enemy and player have the same projectile. Change the enemy projectile once we have the final one

	//init fields
	_enabled = true;
	_position = pos;
	_tint = Color4::WHITE;
	_drawScale.set(1.0f, 1.0f);
	_damage = damage;
	_state = FLYING;

	//init hitbox
	//TODO: modify shape and size
	std::shared_ptr<physics2::WheelObstacle> obs = physics2::WheelObstacle::alloc(pos, GameConstants::PROJ_RADIUS_LIZARD);
	obs->setName("lizard-projectile-collider");
	obs->setPosition(pos);
	b2Filter filter;
	//projectiles are attacks. they can hit players and are destroyed on contact with a wall.
	filter.categoryBits = CATEGORY_PROJECTILE;
	filter.maskBits = CATEGORY_PLAYER | CATEGORY_PLAYER_HITBOX | CATEGORY_WALL;
	obs->setFilterData(filter);
	obs->setEnabled(true);
	obs->setAwake(true);
	obs->setBodyType(b2_kinematicBody);
	//might need this depending on projectile speed
	// obs->setBullet(true);
	_collider = obs;
	std::shared_ptr<Texture> t = assets->get<Texture>("lizard-projectile");
	//TODO: modify this to use the right frames
	_flyingAnimation = Animation::alloc(SpriteSheet::alloc(t, 3, 5), 0.5f, true, 5, 14); 
	_explodingAnimation = Animation::alloc(SpriteSheet::alloc(t, 3, 5), 0.000001f, false); //make time really small because there is no explosion effect
	setFlying();
	return true;
}

bool Projectile::mageInit(Vec2 pos, int damage, const std::shared_ptr<AssetManager>& assets) {
	//TODO: For now, the enemy and player have the same projectile. Change the enemy projectile once we have the final one

	//init fields
	_enabled = true;
	_position = pos;
	_tint = Color4::WHITE;
	_drawScale.set(1.0f, 1.0f);
	_damage = damage;
	_state = FLYING;

	//init hitbox
	//TODO: modify shape and size
	std::shared_ptr<physics2::WheelObstacle> obs = physics2::WheelObstacle::alloc(pos, GameConstants::PROJ_RADIUS_MAGE);
	obs->setName("mage-projectile-collider");
	obs->setPosition(pos);
	b2Filter filter;
	//projectiles are attacks. they can hit players and are destroyed on contact with a wall.
	filter.categoryBits = CATEGORY_PROJECTILE;
	filter.maskBits = CATEGORY_PLAYER | CATEGORY_PLAYER_HITBOX | CATEGORY_WALL;
	obs->setFilterData(filter);
	obs->setEnabled(true);
	obs->setAwake(true);
	obs->setBodyType(b2_kinematicBody);
	//might need this depending on projectile speed
	// obs->setBullet(true);
	_collider = obs;
	std::shared_ptr<Texture> t = assets->get<Texture>("mage-projectile");
	//TODO: modify this to use the right frames
	_flyingAnimation = Animation::alloc(SpriteSheet::alloc(t, 3, 7), 7.0f / 24.0f, true, 14, 20); //24fps
	_explodingAnimation = Animation::alloc(SpriteSheet::alloc(t, 3, 7), 0.000001f, false); //make time really small because there is no explosion effect
	setFlying();
	return true;
}

void Projectile::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
	if (_currAnimation->isActive()) {
		auto spriteSheet = _currAnimation->getSpriteSheet();
		Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, spriteSheet->getFrameSize().height / 2);
		Affine2 transform = Affine2::createRotation(_collider->getAngle());
		float d = 1;
		if ((_collider->getFilterData().maskBits & CATEGORY_PLAYER) == CATEGORY_PLAYER)
			d = 32;
		else if ((_collider->getFilterData().maskBits & CATEGORY_ENEMY) == CATEGORY_ENEMY)
			d = 48;
		transform.scale(_drawScale/d);
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

        // impossible path
        return false;
	}
    else if (_state == EXPLODING) {return !_currAnimation->isActive();}
    else return false;
    
}

void Projectile::dispose() { 
	_currAnimation = nullptr; 
	_collider = nullptr; 
	setEnabled(false);
}
