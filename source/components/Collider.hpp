//
//  Collider.hpp
//  RS
//
//  Created by Zhiyuan Chen on 4/24/24.
//

#ifndef Collider_hpp
#define Collider_hpp


#include <cugl/cugl.h>

using namespace cugl;

class Collider {
    
public:
#pragma mark Shaped Box2D bodies
    
    /**
     * construct a polygon collider with the given data and assign the collider with the given type, sensor, and name.
     *
     * {
     *  shape : "polygon"
     *  vertices : float array
     *  x : float
     *  y : float
     * }
     */
    static std::shared_ptr<physics2::Obstacle> makePolygon(std::shared_ptr<JsonValue> polyColliderData, b2BodyType type, std::string name, bool is_sensor = false);
    
    /**
     * construct a box collider with the given data and assign the collider with the given type, sensor, and name.
     *
     * {
     *  shape : "box"
     *  width : float
     *  height : float
     *  x : float
     *  y : float
     * }
     *
     */
    static std::shared_ptr<physics2::Obstacle> makeBox(std::shared_ptr<JsonValue> boxColliderData, b2BodyType type, std::string name, bool is_sensor = false);
    
    
    /**
     * construct a generic collider with the given data and assign the collider with the given type, sensor, and name. The correct shaped collider will be based on the shape property of the collider data. The function will fail if the shape is not identified.
     *
     * {
     *  shape : string,
     *  ... <other data> ...
     * }
     */
    static std::shared_ptr<physics2::Obstacle> makeCollider(std::shared_ptr<JsonValue> colliderData, b2BodyType type, std::string name, bool is_sensor = false);
};

#endif /* Collider_hpp */
