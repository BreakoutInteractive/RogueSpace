//
//  InputController.h
//
//  This input controller is primarily designed for mobile control.
//  There is support for keyboard inputs.
//
//  Author: Zhiyuan Chen
//  Version: 3/5/24
//
#ifndef __INPUT_CONTROLLER_H__
#define __INPUT_CONTROLLER_H__
#include <cugl/cugl.h>

using namespace cugl;

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
#pragma mark Internal Types
    
    /**
     * When a tap is finished, the data can be saved to denote whether the next tap initiates a double tap, triple tap, etc.
     */
    struct TapData {
        /** the number of valid taps associated with a given touch*/
        int count = 0;
        /** the position of the last tap */
        Vec2 pos;
        /** the time when the last tap was completed */
        Timestamp timestamp;
        
        std::string toString() const {
            return ("taps " + std::to_string(count)) + pos.toString() + " ";
        }
    };
    
    /**
     * A gesture is defined by the starting position of a touch event,
     * A gesture officially finishes when the touch is released. An inactive gesture can be safely discarded.
     */
    struct GestureData {
        /** whether the touch event is still active*/
        bool active = false;
        /** where on the screen did the touch start */
        Vec2 initialPos;
        /** where on the screen is the touch currently*/
        Vec2 curPos;
        /** where on the screen was the touch last time before it moved*/
        Vec2 prevPos;
        /** the time when the touch was initiated */
        Timestamp timestamp;
        /** the identifier of this touch event */
        TouchID touchID;
        /** the data associated with this gesture's tap motion */
        TapData tap;
    };
    
    /**
     * initialize gesture data based on event timestamp, position, id, etc.
     */
    void initGestureDataFromEvent(GestureData& data, const TouchEvent& event);
    
#pragma mark -
#pragma mark Touch Callbacks
    /**
     * Callback for the beginning of a touch event
     *
     * @param t     The touch information
     * @param event The associated event
     */
    void touchBeganCB(const TouchEvent& event, bool focus);
    
    /**
     * Callback for the end of a touch event
     *
     * @param t     The touch information
     * @param event The associated event
     */
    void touchEndedCB(const TouchEvent& event, bool focus);
    
    /**
     * Call back to execute when the finger moves.
     *
     * @param event     The event with the touch information
     * @param previous  The previously reported finger location
     * @param focus     Whether this device has focus (UNUSED)
     */
    void touchMotionCB(const TouchEvent& event, const Vec2 previous, bool focus);
    
    /** a function to execute to determine whether to skip the touch input (begin touch)*/
    std::function<bool(Vec2)> preprocesser;
    
#pragma mark -
#pragma mark Internal Data
    
    // this section can be modified between calls to update()
    
    /** Whether or not this input is active */
    bool _active;
    /** Whether the dodge  key is down */
    bool  _keyDodge;
    /** Whether the parry key was pressed */
    bool  _keyParry;
    /** Whether the parry key is held down */
    bool _keyParryDown;
    /** Whether the parry key was released*/
    bool _keyParryReleased;
    /** Whether the attack key was pressed */
    bool _keyAttack;
    /** Whether the attack key is held down*/
    bool _keyAttackDown;
    /** Whether the attack key was released*/
    bool _keyAttackReleased;
    /** Whether the reset key was pressed */
    bool  _keyReset;
    /** Whether the debug key was pressed */
    bool  _keyDebug;
    /** Whether the exit key was pressed */
    bool  _keyExit;
    /** Whether the weapon swap key was pressed */
    bool  _keySwap;
    /** a vector cache representing the intended direction of movement*/
    Vec2 _keyMoveDir;
    /** a vector cache representing the intended direction of dodge*/
    Vec2 _keyDodgeDir;
    /** a vector cache representing the intended direction of attack*/
    Vec2 _keyAttackDir;

    // TOUCH SUPPORT
    
    /** The listener key associated with the touchscreen */
    uint32_t _touchKey;
    
    /** gesture data for controlling motion */
    GestureData _motionGesture;
    /** gesture data for controlling combat */
    GestureData _combatGesture;
    /** the time (seconds) in which combat gesture is active */
    float _combatGestureActiveTime;
    /** whether the combat gesture has been on hold */
    bool _combatGestureHeld;
    /** whether the gestures have positions swapped. */
    bool reversedGestures;
    /** whether the range controls are active */
    bool rangedMode;

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
    /** Whether the attack action was down*/
    bool _attackDown;
    /** Whether the attack action was released*/
    bool _attackReleased;
    /** Whether the parry action was chosen */
    bool _parryPressed;
    /** Whether the parry action was down*/
    bool _parryDown;
    /** Whether the parry action was released*/
    bool _parryReleased;
    /** Whether the weapon swap action was chosen */
    bool _swapPressed;
    /** unit direction of the attack*/
    Vec2 _attackDir;
    /** unit vector direction of movement */
    Vec2 _moveDir;
    /** unit vector direction of dodge motion */
    Vec2 _dodgeDir;
    
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
     * @param preprocessor a function determining whether the new touch inputs received has already been processed.
     * @return true if the controller was initialized successfully
     */
    bool init(std::function<bool(Vec2)> preprocessor);
    
#pragma mark -
#pragma mark Input Detection
    /**
     * Returns true if the input handler is currently active
     *
     * @return true if the input handler is currently active
     */
    bool isActive( ) const { return _active; }
    
    /**
     * sets whether this input controller is actively reading inputs.
     */
    void setActive(bool value) {
        _active = value;
    }
    
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
    
    void activateRangeControls(){ rangedMode = true; }
    void activateMeleeControls(){ rangedMode = false; }
    /**
     * swaps the active control mode
     */
    void swapControlMode() { rangedMode = !rangedMode;}
    
#pragma mark -
#pragma mark Input Results
    
    /**
     * Returns the unit vector direction of movement
     *
     * up-vector (1,0) should represent upwards movement
     *
     * @return the unit vector direction of movement
     */
    Vec2 getMoveDirection(){ return _moveDir; }
    
    /**
     * Returns true if the dodge input was triggered.
     */
    bool didDodge() const { return _dodgePressed; }
    
    /**
     * Returns the unit vector direction of movement for dodge motion
     *
     * The returned value can be anything in the event that `didDodge` is false.
     */
    Vec2 getDodgeDirection(Vec2 facingDir);
    
    /**
     * Returns true if the parry input was triggered (the player started a parry).
     */
    bool didParry() const { return _parryPressed; }
    /**
     * Returns true if the parry input is down (the player held the stance).
     */
    bool didStance() const { return _parryDown; }
    /**
     * Returns true if the parry input was released (the player parried).
     */
    bool didParryRelease() const { return _parryReleased; }
    
    /**
     * Returns true if the attack input was triggered
     */
    bool didAttack() const { return _attackPressed; }
    /**
     * Returns true if the attack input is down (the player charged a ranged attack)
     */
    bool didCharge() const { return _attackDown; }
    /**
     * Returns true if the attack input was released (the player shot a ranged attack)
     */
    bool didShoot() const { return _attackReleased; }
    
    /**
     * Returns the vector direction of attack (i.e. the position of the mouse)
     *
     * The returned value can be anything in the event that `didAttack` is false.
     */
    Vec2 getAttackDirection(Vec2 facingDir);
    
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

    /**
     * Returns true if the swap button was pressed.
     *
     * @return true if the swap button was pressed.
     */
    bool didSwap() const { return _swapPressed; }
    
#pragma mark -
#pragma mark Input Results (Mobile Only)
    
    /**
     * @return whether there is touch event associated with the motion gesture
     */
    bool isMotionActive() const { return _motionGesture.active; }
    
    /**
     * The returned value can be anything in the event that `isMotionActive` is false.
     * @return the starting location of the touch event associated with the motion gesture
     */
    Vec2 getInitTouchLocation() const { return _motionGesture.initialPos;}
    
    /**
     * The returned value can be anything in the event that `isMotionActive` is false.
     * @return the current location of the touch event associated with the motion gesture
     */
    Vec2 getTouchLocation() const {return _motionGesture.curPos; }
    
    /**
     * @return whether there is touch event associated with the combat gesture
     */
    bool isCombatActive() const { return _combatGesture.active; }
    
    /**
     * The returned value can be anything in the event that `isCombatActive` is false.
     * @return the starting location of the touch event associated with the combat gesture
     */
    Vec2 getInitCombatLocation() const { return _combatGesture.initialPos;}
    
    /**
     * The returned value can be anything in the event that `isCombatActive` is false.
     * @return the current location of the touch event associated with the combat gesture
     */
    Vec2 getCombatTouchLocation() const {return _combatGesture.curPos; }

};

#endif /* __INPUT_CONTROLLER_H__ */
