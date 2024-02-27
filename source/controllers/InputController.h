//
//  InputController.h
//
//  This input controller is primarily designed for mobile control.
//  There is support for keyboard inputs.
//
//  Author: Zhiyuan Chen
//  Version: 2/21/23
//
#ifndef __INPUT_CONTROLLER_H__
#define __INPUT_CONTROLLER_H__
#include <cugl/cugl.h>

/**
 * This class represents player input in the rocket demo.
 *
 * This input handler uses the CUGL input API.  It uses the polling API for
 * keyboard, but the callback API for touch.  This demonstrates a mix of ways
 * to handle input, and the reason for hiding it behind an abstraction like
 * this class.
 *
 * Unlike CUGL input devices, this class is not a singleton.  It must be
 * allocated before use.  However, you will notice that we do not do any
 * input initialization in the constructor.  This allows us to allocate this
 * controller as a field without using pointers. We simply add the class to the
 * header file of its owner, and delay initialization (via the method init())
 * until later. This is one of the main reasons we like to avoid initialization
 * in the constructor.
 */
class InputController {
private:
    /** Whether or not this input is active */
    bool _active;
    // KEYBOARD EMULATION
    /** Whether the up arrow key is down */
    bool  _keyUp;
    /** Whether the down arrow key is down */
    bool  _keyDown;
    /** Whether the reset key is down */
    bool  _keyReset;
    /** Whether the debug key is down */
    bool  _keyDebug;
    /** Whether the exit key is down */
    bool  _keyExit;
    /** Whether the player is  moving*/
    bool _moveEvent;

    // TOUCH SUPPORT
    /** The previous touch/mouse position */
    cugl::Vec2 _prevPos;
    /** The current touch location for the current gesture */
    cugl::Vec2 _currPos;
    /** The timestamp for the beginning of the current gesture */
    cugl::Timestamp _timestamp;

protected:
    // INPUT RESULTS
    /** Whether the reset action was chosen. */
    bool _resetPressed;
    /** Whether the debug toggle was chosen. */
    bool _debugPressed;
    /** Whether the exit action was chosen. */
    bool _exitPressed;
    /** How much did we move horizontally? */
    float _horizontal;
    /** How much did we move vertically? */
    float _vertical;
    /** The key for the touch listeners */
    Uint32 _touchKey;
    /** The finger position (for drag interface) */
    cugl::Vec2 _touchPos;
    /** Whether the finger is down */
    bool _touchDown;
    /** Whether there is an active touch press */
    bool _currDown;
    /** Whether there was an active touch press last frame*/
    bool _prevDown;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new input controller.
     *
     * This constructor does NOT do any initialzation.  It simply allocates the
     * object. This makes it safe to use this class without a pointer.
     */
    InputController(); // Don't initialize.  Allow stack based
    
    /**
     * Disposes of this input controller, releasing all listeners.
     */
    ~InputController() { dispose(); }
    
    /**
     * Deactivates this input controller, releasing all listeners.
     *
     * This method will not dispose of the input controller. It can be reused
     * once it is reinitialized.
     */
    void dispose();
    
    /**
     * Deactivates this input controller, releasing all listeners.
     *
     * This method will not dispose of the input controller. It can be reused
     * once it is reinitialized.
     */
    bool init();
    
#pragma mark -
#pragma mark Input Detection
    /**
     * Returns true if the input handler is currently active
     *
     * @return true if the input handler is currently active
     */
    bool isActive( ) const { return _active; }

    /**
     * Processes the currently cached inputs.
     *
     * This method is used to to poll the current input state.  This will poll the
     * keyboard and accelerometer.
     * 
     * This method also gathers the delta difference in the touches. Depending on 
     * the OS, we may see multiple updates of the same touch in a single animation
     * frame, so we need to accumulate all of the data together.
     */
    void update(float dt);

    /**
     * Clears any buffered inputs so that we may start fresh.
     */
    void clear();
    
    /**
     * Returns the current mouse/touch position
     *
     * @return the current mouse/touch position
     */
    const cugl::Vec2& getPosition() const {
        return _currPos;
    }
    
    void setPosition(cugl::Vec2 newPos);
    
    /**
     * Returns the previous mouse/touch position
     *
     * @return the previous mouse/touch position
     */
    const cugl::Vec2& getPreviousPosition() const {
        return _prevPos;
    }
    
#pragma mark -
#pragma mark Input Results
    
    /**
     * Returns the unit vector direction of movement
     *
     * up-vector (1,0) should represent upwards movement
     *
     * @return the unit vector direction of movement
     */
    cugl::Vec2 getMoveDirection();
    
    /**
     * Return true if the user initiated a press this frame.
     *
     * A press means that the user is pressing (button/finger) this
     * animation frame, but was not pressing during the last frame.
     *
     * @return true if the user initiated a press this frame.
     */
    bool didPress() const {
        return !_prevDown && _touchDown;
    }

    /**
     * Return true if the user initiated a release this frame.
     *
     * A release means that the user was pressing (button/finger) last
     * animation frame, but is not pressing during this frame.
     *
     * @return true if the user initiated a release this frame.
     */
    bool didRelease() const {
        return !_touchDown && _prevDown;
    }

    /**
     * Return true if the user is actively pressing this frame.
     *
     * This method only checks that a press is active or ongoing.
     * It does not care when the press was initiated.
     *
     * @return true if the user is actively pressing this frame.
     */
    bool isDown() const {
        return _touchDown;
    }
    
    /**
     * Returns true if the dodge input was triggered.
     */
    bool didDodge();
    
    /**
     * Returns the unit vector direction of movement for dodge motion
     *
     * The returned value can be anything in the event that {@link didDodge} is false.
     */
    cugl::Vec2 getDodgeDirection();
    
    /**
     * Returns true if the parry input was triggered.
     */
    bool didParry();
    
    /**
     * Returns true if the attack input was triggered
     */
    bool didAttack();
    
    
    /**
     * Returns true if the reset button was pressed.
     *
     * @return true if the reset button was pressed.
     */
    bool didReset() const { return _resetPressed; }
    
    /**
     * Returns true if the player wants to go toggle the debug mode.
     *
     * @return true if the player wants to go toggle the debug mode.
     */
    bool didDebug() const { return _debugPressed; }
    
    /**
     * Returns true if the exit button was pressed.
     *
     * @return true if the exit button was pressed.
     */
    bool didExit() const { return _exitPressed; }
    
    
#pragma mark -
#pragma mark Touch Callbacks
    /**
     * Callback for the beginning of a touch event
     *
     * @param t     The touch information
     * @param event The associated event
     */
    void touchBeganCB(const cugl::TouchEvent& event, bool focus);
    
    /**
     * Callback for the end of a touch event
     *
     * @param t     The touch information
     * @param event The associated event
     */
    void touchEndedCB(const cugl::TouchEvent& event, bool focus);
    
    /**
     * Call back to execute when the finger moves.
     *     *
     * @param event     The event with the touch information
     * @param previous  The previously reported finger location
     * @param focus     Whether this device has focus (UNUSED)
     */
    void dragCB(const cugl::TouchEvent& event, const cugl::Vec2 previous, bool focus);

};

#endif /* __INPUT_CONTROLLER_H__ */
