//
//  CollisionConstants.hpp
//  RS
//
//  Created by Dylan McIntyre on 3/10/24.
//

#ifndef CollisionConstants_h
#define CollisionConstants_h

#define CATEGORY_PLAYER              0x0001
#define CATEGORY_PLAYER_SHADOW       0x0002
#define CATEGORY_ENEMY               0x0004
#define CATEGORY_ENEMY_SHADOW        0x0008
#define CATEGORY_SHORT_WALL          0x0010
#define CATEGORY_TALL_WALL           0x0020
#define CATEGORY_ATTACK              0x0040
#define CATEGORY_PLAYER_HITBOX       0x0080
#define CATEGORY_ENEMY_HITBOX        0x0100
#define CATEGORY_PROJECTILE          0x0200
#define CATEGORY_PROJECTILE_SHADOW   0x0400
#define CATEGORY_HEALTHPACK			 0x0800
#define CATEGORY_TUTORIAL_COLLIDER   0x1000

#endif /* CollisionConstants_h */
