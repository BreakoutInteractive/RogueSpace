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

using namespace cugl;

/**
 A camera controller manages the behavior of a (2D orthographic) camera.
 */
class CameraController {
    
public:
    /**
     * Camera Configurations for Initialization
     */
    struct CameraConfig {
        /** camera chase speed */
        float speed = 0;
        /** camera acceleration */
        float acceleration = 0;
        /** camera minimum speed */
        float minSpeed = 0;
        /** camera maximum speed */
        float maxSpeed = INFINITY;
        /** camera zoom */
        float zoom = 1;
        /** camera zoom out speed */
        float zoomSpeed = 0;
        /** minimum zoom out */
        float minZoom = 1;
        /** maximum zoom out */
        float maxZoom = INFINITY;
    };

private:
    /** current active camera */
    std::shared_ptr<OrthographicCamera> _camera;
    /** camera target */
    Vec2 _targetPosition;
    /** camera initial position (x,y,z) */
    Vec3 _initPosition;
    /** whether the camera is currently active */
    bool _active;
    /** camera configurations */
    CameraConfig _config;
    /** default camera chase speed */
    float _defaultSpeed;
    /** default camera zoom*/
    float _defaultZoom;
public:
    
#pragma mark -
#pragma mark Constructors and Destructors
    /**
     * initializes a new inactive controller with the given camera and default camera speed of 0
     *
     * @param camera the camera
    */
    void init(std::shared_ptr<Camera> camera){ init(camera, CameraConfig()); }
    
    /**
     * initializes a new inactive controller with the given camera and speed.
     *
     * @param camera the camera
     * @param speed the camera movement speed
    */
    void init(std::shared_ptr<Camera> camera, CameraConfig config);

    /**
     * Destroys the controller and removes all references to resources such as the camera.
    */
    ~CameraController();

#pragma mark-
#pragma mark Attributes
    
    /**
     * @return the camera position
    */
    Vec2 getPosition(){ return _camera->getPosition();}
    
    /**
     * @return camera current acceleration
     */
    float getAcceleration(){ return _config.acceleration; }
    
    /**
     * @return camera current speed
     */
    float getSpeed(){ return _config.speed; }
    
    /**
     * @return whether the camera (controller) is active.
     */
    bool isActve(){ return _active; }
    
#pragma mark -
#pragma mark Camera Handling
    
    /**
     * resets the camera to the initial position and inactivates the controller
     */
    void reset();
    
    /**
     * sets the rate at which the camera zooms out
     */
    void setZoomSpeed(float spd){ _config.zoomSpeed = spd; }
    
    /**
     * sets the rate at which the camera accelerates
     */
    void setAcceleration(float acc){ _config.acceleration = acc; }
    
    /**
     * moves the camera (x,y) position to the given position
     *
     * this is useful to initialize the camera to a specific entity location, map area, etc.
     */
    void setCamPosition(Vec2 pos);

    /**
     * sets the target position of the camera (to move towards)
    */
    void setTarget(Vec2 targetPos);

    /**
     * updates the camera by the given amount of time. If the camera is in an idle state
     * (eg. inactive camera has no target or is not used for gameplay), the update will not perform any changes.
    */
    void update(float dt);

};

#endif /* __CAMERA_CONTROLLER_HPP__ */
