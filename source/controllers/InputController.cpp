#include "InputController.hpp"

using namespace cugl;

#pragma mark Input Constants

/** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for toggling the debug display */
#define DEBUG_KEY KeyCode::Q
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE
/** How far we must swipe in any direction for a movement gesture i*/
#define EVENT_SWIPE_LENGTH 175
/** How long in milliseconds must we hold for a parry*/
#define PARRY_HOLD_TIME 300


#pragma mark -
#pragma mark Input Controller


InputController::InputController() :
_active(false),
_resetPressed(false),
_debugPressed(false),
_exitPressed(false),
_keyDodge(false),
_keyParry(false),
_keyReset(false),
_keyDebug(false),
_keyExit(false) {
}

void InputController::dispose() {
    if (_active) {
#ifndef CU_TOUCH_SCREEN
        // DESKTOP
        Input::deactivate<Keyboard>();
#else
        // MOBILE
        Touchscreen* touch = Input::get<Touchscreen>();
        touch->removeBeginListener(_touchKey);
        touch->removeEndListener(_touchKey);
        touch->removeMotionListener(_touchKey);
#endif
        _active = false;
    }
}

bool InputController::init() {
    _timestamp.mark();
    bool success = true;
    
    // Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    success = Input::activate<Keyboard>() && Input::activate<Mouse>();
#else
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        // setup MOBILE touch listeners
        _touchKey = touch->acquireKey();
        touch->addBeginListener(_touchKey,[=](const cugl::TouchEvent& event, bool focus){
            this->touchBeganCB(event,focus);
        });
        touch->addEndListener(_touchKey, [=](const cugl::TouchEvent& event, bool focus) {
            this->touchEndedCB(event,focus);
        });
        touch->addMotionListener(_touchKey, [=](const cugl::TouchEvent& event, cugl::Vec2 previous, bool focus) {
            this->touchMotionCB(event,previous,focus);
        });
    }
    success = touch != nullptr;
#endif
    _active = success;
    clear();
    return success;
}

#pragma mark -
#pragma mark Input Detection

void InputController::update(float dt) {

#ifndef CU_TOUCH_SCREEN
    // DESKTOP CONTROLS
    Keyboard* keys = Input::get<Keyboard>();
    Mouse* mouse = Input::get<Mouse>();

    // Map "keyboard" events to the current frame boundary
    _keyReset  = keys->keyPressed(RESET_KEY);
    _keyDebug  = keys->keyPressed(DEBUG_KEY);
    _keyExit   = keys->keyPressed(EXIT_KEY);

    //attack on left click, parry on right click
    _keyAttack = mouse->buttonPressed().hasLeft();
    if (_keyAttack) _attackDir = (mouse->pointerPosition());
    _keyParry = mouse->buttonPressed().hasRight();
    _keyDodge = keys->keyDown(KeyCode::SPACE);
    
    // move with WASD
    bool left = keys->keyDown(KeyCode::A);
    bool right = keys->keyDown(KeyCode::D);
    bool up   = keys->keyDown(KeyCode::W);
    bool down = keys->keyDown(KeyCode::S);
    
    // Directional controls
    float _horizontal = 0.0f;
    if (right) _horizontal += 1.0f;
    if (left) _horizontal -= 1.0f;
    float _vertical = 0.0f;
    if (up) _vertical += 1.0f;
    if (down) _vertical -= 1.0f;
    
    _keyMoveDir.set(_horizontal, _vertical);
    _keyDodgeDir.set(_keyMoveDir);    // dodges ("dash") in the direction of movement

#else
    // MOBILE CONTROLS
    // TODO: in callbacks map the following to something (for when it is true)
    _keyReset = false; // need to map something
    _keyDebug = false;
    _keyExit = false;
    
    // The actual updates to movement/dodge directions are all during callbacks
    // this is why there are keys for movement, dodge, parry and attack.
    // the update loop reads these values at the time of the function call
    // these key values can change right after input.update() when a listener event gets triggered
    // this is probably the redundant but safe way to handle inputs without losing any past inputs.
    
#endif
    // Unite the two platforms
    // Update the abstraction layer (note that the abstraction layer should not be updated outside of this update call)
    _resetPressed = _keyReset;
    _debugPressed = _keyDebug;
    _exitPressed  = _keyExit;
    _attackPressed = _keyAttack;
    _dodgePressed = _keyDodge;
    _parryPressed = _keyParry;
    
    _moveDir.set(_keyMoveDir).normalize();
    _dodgeDir.set(_keyDodgeDir).normalize();


// If it does not support keyboard, we must reset "virtual" keyboard
#ifdef CU_TOUCH_SCREEN
    _keyDebug = false;
    _keyReset = false;
    _keyDebug = false;
    _keyParry = false;
    _keyAttack = false;
    _keyDodge = false;
#endif
}

void InputController::clear() {
    _resetPressed = false;
    _debugPressed = false;
    _exitPressed  = false;
    _dodgePressed = false;
    _attackPressed = false;
    _parryPressed = false;
    _leftGesture.active = false;
    _rightGesture.active = false;
    _timestamp.mark();
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
    if (!_active){
        return;;
    }
    // check where the touch was initiated
    // TODO: this does not work the same way when you rotate the phone 180 (maybe use Display instead)
    Size s = Application::get()->getDisplaySize();
    Vec2 touchPos = event.position;
    if (touchPos.x >= s.width/2 && !_rightGesture.active){
        // right sided
        initGestureDataFromEvent(_rightGesture, event);
    }
    else if (touchPos.x < s.width/2 && !_leftGesture.active){
        // left sided
        initGestureDataFromEvent(_leftGesture, event);
    }
    
}

void InputController::initGestureDataFromEvent(GestureData& data, const cugl::TouchEvent &event){
    data.active = true;
    data.timestamp = event.timestamp;
    data.curPos = event.position;
    data.initialPos = data.curPos;
    data.touchID = event.touch;
}
 
/**
 * Callback for the end of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchEndedCB(const cugl::TouchEvent& event, bool focus) {
    if (!_active){
        return;;
    }
    // parry occurs on release (longer tap)
    // attack occurs on release (short tap)
    // it is required that the touch has not moved much because otherwise it becomes
    // a player may have tap + hold + swipe (to dodge) but on release, this should not fire an attack/parry event
    Vec2 touchPos = event.position;
    Size s = Application::get()->getDisplaySize();
    if (touchPos.x >= s.width/2 && _rightGesture.active && event.touch == _rightGesture.touchID){
        // since the right-sided event did not result in a dodge,
        // this implies an insufficient movement of touch (or none)
        // so we can safely use the timestamp to judge whether it is an attack or parry
        auto elapsed = event.timestamp.ellapsedMillis(_rightGesture.timestamp);
        if (elapsed >= PARRY_HOLD_TIME){
            _keyParry = true;
        }
        else {
            _keyAttack = true;
        }
        _rightGesture.active = false;
    }
    
    // handle release on left-side (for now, this can happen if the user just don't want to move
    if (_leftGesture.active && event.touch == _leftGesture.touchID){
        _leftGesture.active = false;
        _keyMoveDir = Vec2::ZERO;
    }
}

void InputController::touchMotionCB(const cugl::TouchEvent& event, const Vec2 previous, bool focus) {
    if (!_active){
        return;;
    }
    // update positions in gestures (important for dodge and move)
    Vec2 touchPos = event.position;
    Size s = Application::get()->getDisplaySize();
    if (touchPos.x >= s.width/2 && _rightGesture.active && _rightGesture.touchID == event.touch){
        // right sided motion
        _rightGesture.prevPos = previous;
        _rightGesture.curPos = event.position;
        
        // check if finger dragged enough to initiate dodge
        Vec2 swipeDir = touchPos - _rightGesture.initialPos;
        float swipeLength = swipeDir.length();
        if (swipeLength >= EVENT_SWIPE_LENGTH){
            // update movement based on the direction vector
            _keyDodge = true;
            _keyDodgeDir.set(swipeDir.x, -swipeDir.y); //negate y because screen origin is different from game origin.
            // now this touch is "freed" even if it still touches the screen
            // every new dodge requires a tap + hold and moving
            _rightGesture.active = false;
        }
        
    }
    else if (touchPos.x < s.width/2 && _leftGesture.active && _leftGesture.touchID == event.touch){
        // left sided motion
        _leftGesture.prevPos = previous;
        _leftGesture.curPos = event.position;
        // check if finger dragged enough to initiate movement
        Vec2 swipeDir = touchPos - _leftGesture.initialPos;
        float swipeLength = swipeDir.length();
        if (swipeLength >= EVENT_SWIPE_LENGTH){
            // update movement based on the direction vector
            _keyMoveDir.set(swipeDir.x, -swipeDir.y); //negate y because screen origin is different from game origin.
        }
    }
    else if (touchPos.x >= s.width/2 && _leftGesture.active && _leftGesture.touchID == event.touch){
        // case where gesture starts left but ends on right, fixes "infinite" player moement
        _leftGesture.active = false;
        _keyMoveDir = Vec2::ZERO;
    }
        
}
