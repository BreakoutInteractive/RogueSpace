#include <cugl/cugl.h>
#include "CameraController.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors and Destructors

void CameraController::init(std::shared_ptr<cugl::Camera> camera, float speed){
    _camera = camera;
    _active = false;
    _initPosition = camera->getPosition();
    _speed = speed;
}

CameraController::~CameraController(){
    _camera = nullptr;
}

#pragma mark -
#pragma mark Camera Handling

void CameraController::reset(){
    _camera->setPosition(_initPosition);
    _active = false;
}

void CameraController::setCamPosition(cugl::Vec2 pos){
    Vec3 camPos = _camera->getPosition();
    camPos.set(pos.x, pos.y, camPos.z);
    _camera->setPosition(camPos);
    _camera->update();
}

void CameraController::setTarget(Vec2 targetPos){
    _targetPosition.set(targetPos);
    _active = true;
}

void CameraController::update(float dt){
    if (_active){
        Vec3 cam3Dpos = _camera->getPosition();
        Vec2 camPos(cam3Dpos.x, cam3Dpos.y);
        camPos += (_targetPosition - camPos) * _speed * dt;
        _camera->setPosition(Vec3(camPos.x, camPos.y, cam3Dpos.z));
        _camera->update(); // must call in order to change combined matrix
    }
}
