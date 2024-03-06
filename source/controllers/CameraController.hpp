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

/**
 A camera controller manages the behavior of a (2D orthographic) camera.
 */
class CameraController {

private:
    /** current active camera */
    std::shared_ptr<cugl::Camera> _camera;

    /** camera target */
    cugl::Vec2 _targetPosition;
    
    /** camera initial position (x,y,z) */
    cugl::Vec3 _initPosition;
    
    /** whether the camera is currently active */
    bool _active;
    
    /** camera chase speed */
    float _speed;
    
public:
    
#pragma mark -
#pragma mark Constructors and Destructors
    /**
     * initializes a new inactive controller with the given camera and default camera speed of 1.
     *
     * @param camera the camera
    */
    void init(std::shared_ptr<cugl::Camera> camera){ init(camera, 1.0f); }
    
    
    /**
     * initializes a new inactive controller with the given camera and speed.
     *
     * @param camera the camera
     * @param speed the camera movement speed
    */
    void init(std::shared_ptr<cugl::Camera> camera, float speed);

    /**
     * Destroys the controller and removes all references to resources such as the camera.
    */
    ~CameraController();

#pragma mark-
#pragma mark Attributes
    
    /**
     * @return the camera position
    */
    cugl::Vec2 getPosition(){ return _camera->getPosition();}
    
    /**
     * @return whether the camera (controller) is active.
     */
    bool isActve(){ return _active; }
    
#pragma mark -
#pragma mark Camera Handling
    
    /**
     * resets the camera to the initial position and inactivates off the controller
     */
    void reset();
    
    /**
     * sets the speed at which the camera follows its target
     */
    void setSpeed(float spd){ _speed = spd; }
    
    /**
     * moves the camera (x,y) position to the given position
     *
     * this is useful to initialize the camera to a specific entity location, map area, etc.
     */
    void setCamPosition(cugl:: Vec2 pos);

    /**
     * sets the target position of the camera (to move towards)
    */
    void setTarget(cugl::Vec2 targetPos);

    /**
     * updates the camera by the given amount of time. If the camera is in an idle state
     * (eg. inactive camera has no target or is not used for gameplay), the update will not perform any changes.
    */
    void update(float dt);
    
    // TODO: possibly camera shakes :skull

};

#endif /* __CAMERA_CONTROLLER_HPP__ */
