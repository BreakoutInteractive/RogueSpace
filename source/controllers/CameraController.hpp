//
//  CameraController.hpp
//
//  This controller is primarily responsible for moving the camera
//  to follow a given target (represented by a location)
//
//  Author: Zhiyuan Chen
//  Version: 3/3/24
//

#ifndef __CAMERA_CONTROLLER_HPP__
#define __CAMERA_CONTROLLER_HPP__
#include <cugl/cugl.h>

class CameraController {

private:
    /** current active camera */
    std::shared_ptr<cugl::Camera> _camera;

    /** camera target */
    cugl::Vec2 _targetPosition;
    
public:
    
#pragma mark -
#pragma mark Constructors and Destructors
    /**
     * creates a new controller with the given camera
    */
    CameraController(std::shared_ptr<cugl::Camera> camera);

    /**
     * Destroys the controller and removes all references to resources such as the camera.
    */
    ~CameraController();

#pragma mark -
#pragma mark Camera Handling

    /**
     * @return the camera position
    */
    cugl::Vec2 getPosition(){ return _camera->getPosition();}

    /**
     * sets the target position of the camera (to move towards)
    */
    void setTarget(cugl::Vec2 targetPos);

    /**
     * updates the camera by the given amount of time. If the camera is in an idle state
     * (eg. camera has no target or not used for gameplay), the update will not perform any changes.
    */
    void update(float dt);

};

#endif /* __CAMERA_CONTROLLER_HPP__ */