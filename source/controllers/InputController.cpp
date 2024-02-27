#include "InputController.h"

using namespace cugl;

#pragma mark Input Constants

/** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for toggling the debug display */
#define DEBUG_KEY KeyCode::D
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE

/** How fast a double click must be in milliseconds */
#define EVENT_DOUBLE_CLICK  400
/** How far we must swipe left or right for a gesture */
#define EVENT_SWIPE_LENGTH  100
/** How far we must drag  for movement */
#define EVENT_DRAG_LENGTH  500
/** How fast we must swipe left or right for a gesture */
#define EVENT_SWIPE_TIME   1000
/** How far we must turn the tablet for the accelerometer to register */
#define EVENT_ACCEL_THRESH  M_PI/10.0f
/** The key for the event handlers */
#define LISTENER_KEY        1


#pragma mark -
#pragma mark Input Controller
/**
 * Creates a new input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
InputController::InputController() :
_active(false),
_resetPressed(false),
_debugPressed(false),
_exitPressed(false),
_keyUp(false),
_keyDown(false),
_keyReset(false),
_keyDebug(false),
_keyExit(false),
_touchDown(false),
_prevDown(false),
_moveEvent(false),
_horizontal(0.0f),
_vertical(0.0f) {
}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void InputController::dispose() {
    if (_active) {
#ifndef CU_TOUCH_SCREEN
        Input::deactivate<Keyboard>();
#else
        Input::deactivate<Accelerometer>();
        Touchscreen* touch = Input::get<Touchscreen>();
        touch->removeBeginListener(LISTENER_KEY);
        touch->removeEndListener(LISTENER_KEY);
        touch->removeMotionListener(LISTENER_KEY);
#endif
        _active = false;
    }
}

/**
 * Initializes the input control for the given drawing scale.
 *
 * This method works like a proper constructor, initializing the input
 * controller and allocating memory.  However, it still does not activate
 * the listeners.  You must call start() do that.
 *
 * @return true if the controller was initialized successfully
 */
bool InputController::init() {
    _timestamp.mark();
    bool success = true;
    
    // Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    success = Input::activate<Keyboard>();
#else
    success = Input::activate<Accelerometer>();
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        _touchKey = touch->acquireKey();
        touch->addBeginListener(_touchKey,[=](const cugl::TouchEvent& event, bool focus) { //LISTENER_KEY
            this->touchBeganCB(event,focus);
        });
        touch->addEndListener(_touchKey, [=](const cugl::TouchEvent& event, bool focus) {
            this->touchEndedCB(event,focus);
        });
        touch->addMotionListener(_touchKey, [=](const cugl::TouchEvent& event, cugl::Vec2 fingerPos, bool focus) {
            this->dragCB(event,fingerPos,focus);
        });
    }
#endif
    _active = success;
    return success;
}

#pragma mark -
#pragma mark Input Detection

/**
 * Processes the currently cached inputs.
 *
 * This method is used to to poll the current input state.  This will poll the
 * keyboad and accelerometer.
 *
 * This method also gathers the delta difference in the touches. Depending on
 * the OS, we may see multiple updates of the same touch in a single animation
 * frame, so we need to accumulate all of the data together.
 */
void InputController::update(float dt) {
    int left = false;
    int rght = false;
    int up   = false;
    int down = false;

#ifndef CU_TOUCH_SCREEN
    // DESKTOP CONTROLS
    Keyboard* keys = Input::get<Keyboard>();

    // Map "keyboard" events to the current frame boundary
    _keyReset  = keys->keyPressed(RESET_KEY);
    _keyDebug  = keys->keyPressed(DEBUG_KEY);
    _keyExit   = keys->keyPressed(EXIT_KEY);
    
    left = keys->keyDown(KeyCode::ARROW_LEFT);
    rght = keys->keyDown(KeyCode::ARROW_RIGHT);
    up   = keys->keyDown(KeyCode::ARROW_UP);
    down = keys->keyDown(KeyCode::ARROW_DOWN);
#else
    // MOBILE CONTROLS
    Vec3 acc = Input::get<Accelerometer>()->getAcceleration();
    
    // Accelerometer axis depends on location of the home button
    float pitch;
    if (Display::get()->getDefaultOrientation() == Display::Orientation::LANDSCAPE) {
        pitch = atan2(-acc.x, sqrt(acc.y*acc.y + acc.z*acc.z));
    } else {
        pitch = atan2(acc.y, sqrt(acc.x*acc.x + acc.z*acc.z));
    }
    
    // Check if we turned left or right
    left |= (pitch > EVENT_ACCEL_THRESH);
    rght |= (pitch < -EVENT_ACCEL_THRESH);
    up   |= _keyUp;
    
    _prevDown = _currDown;
    _currDown = _touchDown;
    _prevPos = _currPos;
    _currPos = _touchPos;
#endif

    _resetPressed = _keyReset;
    _debugPressed = _keyDebug;
    _exitPressed  = _keyExit;
    
    // Directional controls
    _horizontal = 0.0f;
    if (rght) {
        _horizontal += 1.0f;
    }
    if (left) {
        _horizontal -= 1.0f;
    }

    _vertical = 0.0f;
    if (up) {
        _vertical += 1.0f;
    }
    if (down) {
        _vertical -= 1.0f;
    }


// If it does not support keyboard, we must reset "virtual" keyboard
#ifdef CU_TOUCH_SCREEN
    _keyDebug = false;
    _keyReset = false;
    _keyDebug = false;
#endif
}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void InputController::clear() {
    _resetPressed = false;
    _debugPressed = false;
    _exitPressed  = false;
    
    _horizontal = 0.0f;
    _vertical   = 0.0f;
    
    _prevPos = Vec2::ZERO;
    _timestamp.mark();
}

void InputController::setPosition(cugl::Vec2 newPos){
    _prevPos = _currPos;
    _currPos = newPos;
}


#pragma mark -
#pragma mark Input Results

Vec2 InputController::getMoveDirection() {
    // TODO: return results from mobile input
    Vec2 direction;
    #ifndef CU_TOUCH_SCREEN
    // DESKTOP CONTROLS
    // desktop cannot freely move in all directions
    direction.set(_horizontal, _vertical);
    #else
        // MOBILE
    #endif
    return direction.normalize();
    
    // TODO: complete all other functions from h file

}

#pragma mark -
#pragma mark Touch Callbacks
/**
 * Callback for the beginning of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchBeganCB(const cugl::TouchEvent& event, bool focus) {
    // All touches correspond to key up
//    _keyUp = true;
    if (!_touchDown){
        _touchDown = true;
        _touchPos = event.position;
    }
     
    // Update the touch location for later gestures
    _timestamp = event.timestamp;
//    _prevPos = event.position;
}
 
/**
 * Callback for the end of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchEndedCB(const cugl::TouchEvent& event, bool focus) {
    // Gesture has ended.  Give it meaning.
    if (!_touchDown){
        _touchDown = false;
    }
    Vec2 diff = event.position-_prevPos;
    bool fast = (event.timestamp.ellapsedMillis(_timestamp) < EVENT_SWIPE_TIME);
    _keyReset = fast && diff.x < -EVENT_SWIPE_LENGTH;
    _keyExit  = fast && diff.x > EVENT_SWIPE_LENGTH;
    _keyDebug = fast && diff.y > EVENT_SWIPE_LENGTH;
    _keyUp = false;
}

void InputController::dragCB(const cugl::TouchEvent& event, const Vec2 previous, bool focus) {
    if (_touchDown) {
        _touchPos = event.position;
    }
}
