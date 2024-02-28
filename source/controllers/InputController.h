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
#pragma mark -
#pragma mark Internal Input Raw Data
    
    // this section can be modified between calls to update()
    
    /** Whether or not this input is active */
    bool _active;
    // KEYBOARD EMULATION
    /** Whether the dodge  key is down */
    bool  _keyDodge;
    /** Whether the parry key is down */
    bool  _keyParry;
    /** Whether the attack key is down*/
    bool _keyAttack;
    /** Whether the reset key is down */
    bool  _keyReset;
    /** Whether the debug key is down */
    bool  _keyDebug;
    /** Whether the exit key is down */
    bool  _keyExit;
    /** a vector cache representing the intended direction of movement*/
    cugl::Vec2 _keyMoveDir;
    /** a vector cache representing the intended direction of dodge*/
    cugl::Vec2 _keyDodgeDir;

    // TOUCH SUPPORT
    /** The timestamp for the beginning of the current gesture */
    cugl::Timestamp _timestamp;
    /** The listener key associated with the touchscreen */
    uint32_t _touchKey;

    /**
     * A gesture is defined by the starting position of a touch event,
     * A gesture officially finishes when the touch is released. An inactive gesture can be safely discarded.
     */
    struct GestureData {
        /** whether the touch event is still active*/
        bool active = false;
        /** where on the screen did the touch start */
        cugl::Vec2 initialPos;
        /** where on the screen is the touch currently*/
        cugl::Vec2 curPos;
        /** where on the screen was the touch last time before it moved*/
        cugl::Vec2 prevPos;
        /** the time when the touch was initiated */
        cugl::Timestamp timestamp;
        /** the identifier of this touch event */
        cugl::TouchID touchID;
    };
    
    /**
     * initialize gesture data based on event timestamp, position, id, etc.
     */
    void initGestureDataFromEvent(GestureData& data, const cugl::TouchEvent& event);
    
    /** gesture data on the left hand side of the landscape screen*/
    GestureData _leftGesture;
    /** gesture data on the right hand side of the landscape screen*/
    GestureData _rightGesture;

protected:
#pragma mark -
#pragma mark Input Results (Abstraction Layer)
    
    /** Whether the reset action was chosen. */
    bool _resetPressed;
    /** Whether the debug toggle was chosen. */
    bool _debugPressed;
    /** Whether the exit action was chosen. */
    bool _exitPressed;
    /** Whether the dodge action was chosen */
    bool _dodgePressed;
    /** Whether the attack action was chosen*/
    bool _attackPressed;
    /** Whether the parry action was chosen */
    bool _parryPressed;
    /** TODO: this should not be necessary; which direction did we attack*/
    cugl::Vec2 _attackDir;
    /** unit vector direction of movement */
    cugl::Vec2 _moveDir;
    /** unit vector direction of dodge motion */
    cugl::Vec2 _dodgeDir;
    
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
     * Initializes the input control and activates input listeners (touch/keyboard).
     *
     * This method works like a proper constructor, initializing the input
     * controller and allocating memory.
     *
     * @return true if the controller was initialized successfully
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
    
#pragma mark -
#pragma mark Input Results
    
    /**
     * Returns the unit vector direction of movement
     *
     * up-vector (1,0) should represent upwards movement
     *
     * @return the unit vector direction of movement
     */
    cugl::Vec2 getMoveDirection(){ return _moveDir; }
    
    /**
     * Returns true if the dodge input was triggered.
     */
    bool didDodge() const { return _dodgePressed; }
    
    /**
     * Returns the unit vector direction of movement for dodge motion
     *
     * The returned value can be anything in the event that {@link didDodge} is false.
     */
    cugl::Vec2 getDodgeDirection(){ return _dodgeDir; }
    
    /**
     * Returns true if the parry input was triggered.
     */
    bool didParry() const { return _parryPressed; }
    
    /**
     * Returns true if the attack input was triggered
     */
    bool didAttack() const { return _attackPressed; }
    
    /**
     * Returns the vector direction of attack (i.e. the position of the mouse)
     *
     * The returned value can be anything in the event that {@link didAttack} is false.
     */
    cugl::Vec2 getAttackDirection() const { return _attackDir; }
    
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
    void touchMotionCB(const cugl::TouchEvent& event, const cugl::Vec2 previous, bool focus);

};

#endif /* __INPUT_CONTROLLER_H__ */
