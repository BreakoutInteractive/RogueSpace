#include <cugl/cugl.h>
#include "CameraController.hpp"

using namespace cugl;

// TODO: add controller settings (speed, etc)

#pragma mark -
#pragma mark Constructors and Destructors
CameraController::CameraController(std::shared_ptr<cugl::Camera> camera){
    // TODO: implement constructor

}

CameraController::~CameraController(){
    // TODO: implement destructor
}

#pragma mark -
#pragma mark Camera Handling

void CameraController::setTarget(Vec2 targetPos){
    // TODO: update target
}

void CameraController::update(float dt){
    // TODO: update camera
}