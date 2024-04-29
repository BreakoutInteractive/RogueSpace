#include <cugl/cugl.h>
#include "CameraController.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors and Destructors

void CameraController::init(std::shared_ptr<cugl::Camera> camera, CameraConfig config){
    CUAssertLog(_config.maxSpeed > _config.minSpeed, "maximum speed must exceed minimum speed");
    CUAssertLog(_config.maxZoom > _config.minZoom, "maximum zoom must exceed minimum zoom");
    CUAssertLog(_config.speed >= 0 && _config.zoomSpeed >= 0, "camera speeds must be nonnegative");
    _camera = std::dynamic_pointer_cast<OrthographicCamera>(camera);
    _config = config;
    _defaultZoom = _camera->getZoom();
    _defaultSpeed = _config.speed;
}

CameraController::~CameraController(){
    _camera = nullptr;
}

#pragma mark -
#pragma mark Camera Handling

void CameraController::reset(){
    _camera->setPosition(_initPosition);
    _active = false;
    _config.speed = _defaultSpeed;
    _config.zoom = _defaultZoom;
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
        float zoom = _config.zoom + _config.zoomSpeed * dt;
        _config.zoom = std::max(std::min(zoom, _config.maxZoom), _config.minZoom);
        Vec3 cam3DPos = _camera->getPosition();
        _camera->setZoom(1/_config.zoom);
        _camera->setPosition(cam3DPos);
        _config.speed += _config.acceleration * dt;
        _config.speed = std::max(std::min(_config.speed, _config.maxSpeed), _config.minSpeed);
        Vec2 camPos = Vec2(cam3DPos.x, cam3DPos.y);
        camPos += (_targetPosition - camPos) * _config.speed * dt;
        _camera->setPosition(Vec3(camPos.x, camPos.y, cam3DPos.z));
        _camera->update();
    }
}

