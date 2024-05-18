//
//  Collider.cpp
//  RS
//
//  Created by Zhiyuan Chen on 4/24/24.
//

#include "Collider.hpp"


std::shared_ptr<physics2::Obstacle> Collider::makePolygon(std::shared_ptr<JsonValue> colliderData, b2BodyType type, std::string name, bool is_sensor){
    Vec2 colliderPos(colliderData->getFloat("x"), colliderData->getFloat("y"));
    std::vector<float> vertices = colliderData->get("vertices")->asFloatArray();
    CUAssertLog(vertices.size() >= 2 && vertices.size() % 2 == 0, "polygon colliders should be specified in even number of edge vertices");
    Vec2* verts = reinterpret_cast<Vec2*>(&vertices[0]);
    Poly2 polygon(verts,(int)vertices.size()/2);
    EarclipTriangulator triangulator;
    triangulator.set(polygon.vertices);
    triangulator.calculate();
    polygon.setIndices(triangulator.getTriangulation());
    auto collider = std::make_shared<physics2::PolygonObstacle>();
    collider->init(polygon, colliderPos);
    collider->setBodyType(type);
    collider->setSensor(is_sensor);
    collider->setName(name);
    return collider;
}

std::shared_ptr<physics2::Obstacle> Collider::makeBox(std::shared_ptr<JsonValue> colliderData, b2BodyType type, std::string name, bool is_sensor){
    Size size(colliderData->getFloat("width"), colliderData->getFloat("height"));
    Vec2 pos(colliderData->getFloat("x"), colliderData->getFloat("y"));
    auto box = physics2::BoxObstacle::alloc(pos, size);
    box->setBodyType(type);
    box->setSensor(is_sensor);
    box->setName(name);
    return box;
}

std::shared_ptr<physics2::Obstacle> Collider::makeWheel(std::shared_ptr<JsonValue> wheelColliderData, b2BodyType type, std::string name, bool is_sensor){
    float radius = wheelColliderData->getFloat("radius") * 2 / 3;
    Vec2 pos(wheelColliderData->getFloat("x"), wheelColliderData->getFloat("y"));
    auto wheel = physics2::WheelObstacle::alloc(pos, radius);
    wheel->setBodyType(type);
    wheel->setSensor(is_sensor);
    wheel->setName(name);
    return wheel;
}

std::shared_ptr<physics2::Obstacle> Collider::makeCollider(std::shared_ptr<JsonValue> colliderData, b2BodyType type, std::string name, bool is_sensor){
    std::string colliderShape = colliderData->getString("shape");
    if (colliderShape == "polygon"){
        return makePolygon(colliderData, type, name, is_sensor);
    }
    if (colliderShape == "box"){
        return makeBox(colliderData, type, name, is_sensor);
    }
    if (colliderShape == "circle"){
        return makeWheel(colliderData, type, name, is_sensor);
    }
    CULogError("unsupported shape or missing shape information");
    return nullptr;
}
