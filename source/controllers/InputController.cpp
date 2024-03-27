#include "InputController.hpp"

using namespace cugl;

#pragma mark Input Constants

/** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for toggling the debug display */
#define DEBUG_KEY KeyCode::Q
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE

/** How far we must swipe in any direction for a dodge gesture i*/
#define DODGE_SWIPE_LENGTH 175
/** How far we must swipe in any direction for a movement gesture i*/
#define MOVE_SWIPE_LENGTH 100
/** the minimum amount of milliseconds for a press to be considered a hold*/
#define HOLD_TIME 300
/** the maximum change in position for a hold to not be considered a drag/swipe*/
#define HOLD_IN_PLACE_CONSTRAINT 50
/** the maximum number of milliseconds for a tap to be considered */
#define TAP_TIME 150
/** the maximum amount of milliseconds for a motion swipe to be considered a dodge*/
#define DODGE_SWIPE_TIME 200
/** horizontal constraint for a swipe to qualify for upwards or downwards */
#define VSWIPE_HORIZONTAL_DIFF 250
/** the maximum amount of milliseconds between the end of the first tap and the start of the second tap*/
#define DOUBLE_TAP_TIME_GAP 200


#pragma mark -
#pragma mark Input Controller


InputController::InputController() :
_active(false)
{
     clear();
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
    reversedGestures = false;
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
    
    defenseUpgrade = keys->keyReleased(KeyCode::NUM_1);
    atkUpgrade = keys->keyReleased(KeyCode::NUM_2);
    moveUpgrade = keys->keyReleased(KeyCode::NUM_3);
        
    // Directional controls
    float _horizontal = 0.0f;
    if (right) _horizontal += 1.0f;
    if (left) _horizontal -= 1.0f;
    float _vertical = 0.0f;
    if (up) _vertical += 1.0f;
    if (down) _vertical -= 1.0f;
    
    if (_horizontal == 1){
        _keyMoveDir.set(1,0).rotate(_vertical * 0.464f); // 0.464f is the 26 deg angle. depends on art.
    }
    else if (_horizontal == -1){
        _keyMoveDir.set(-1,0).rotate(-_vertical * 0.464f);
    }
    else {
        _keyMoveDir.set(_horizontal, _vertical);
    }

#else
    // MOBILE CONTROLS
    _keyReset = false;
    _keyExit = false;
    
    // The actual updates to movement/dodge directions are all during callbacks
    // this is why there are keys for movement, dodge, parry and attack.
    // the update loop reads these values at the time of the function call
    // these key values can change right after input.update() when a listener event gets triggered
    // this is probably the redundant but safe way to handle inputs without losing any past inputs.
    
#endif
    // Update the abstraction layer (note that the abstraction layer should not be updated outside of this update call)
    _resetPressed = _keyReset;
    _debugPressed = _keyDebug;
    _exitPressed  = _keyExit;
    _attackPressed = _keyAttack;
    _dodgePressed = _keyDodge;
    _parryPressed = _keyParry;
    
    _moveDir.set(_keyMoveDir).normalize();
    _dodgeDir.set(_keyDodgeDir).normalize();
    _attackDir.set(_keyAttackDir).normalize();


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
    // clear abstraction data
    _resetPressed = false;
    _debugPressed = false;
    _exitPressed  = false;
    _dodgePressed = false;
    _attackPressed = false;
    _parryPressed = false;
    _moveDir.setZero();
    _dodgeDir.setZero();
    _attackDir.setZero();
    
    // clear internal data
    _keyDodge = false;
    _keyParry = false;
    _keyAttack = false;
    _keyReset = false;
    _keyDebug = false;
    _keyExit = false;
    _keyMoveDir.setZero();
    _keyDodgeDir.setZero();
    _keyAttackDir.setZero();
    
    // reset gestures
    _motionGesture.active = false;
    _combatGesture.active = false;
}

#pragma mark -
#pragma mark Results

Vec2 InputController::getDodgeDirection(cugl::Vec2 facingDir){
    #ifndef CU_TOUCH_SCREEN
        _dodgeDir.set(facingDir).normalize();
    #else
    #endif
    return _dodgeDir;
}

Vec2 InputController::getAttackDirection(cugl::Vec2 facingDir){
    _attackDir.set(facingDir).normalize();
    return _attackDir;
}

#pragma mark -
#pragma mark Private Helper Functions

/**
 * @return true if the touch is applied on the motion gesture
 */
bool motionPosConstraint(Vec2 touchPos, bool reversed){
    Size s = Application::get()->getDisplaySize();
    if (reversed){
        return touchPos.x >= 2* s.width/3;
    }
    else {
        return touchPos.x < s.width/3;
    }
}

/**
 * @return true if the touch is applied on the combat gesture
 */
bool combatPosConstraint(Vec2 touchPos, bool reversed){
    Size s = Application::get()->getDisplaySize();
    if (reversed){
        return touchPos.x < s.width/3;
    }
    else {
        return touchPos.x >= 2* s.width/3;
    }
}

#pragma mark -
#pragma mark Touch Callbacks

void InputController::initGestureDataFromEvent(GestureData& data, const cugl::TouchEvent &event){
    data.active = true;
    data.timestamp = event.timestamp;
    data.curPos = event.position;
    data.initialPos = data.curPos;
    data.touchID = event.touch;
    
    // if previous tap is active but the start of this touch is too far apart from the end of the first tap, clear the previous tap.
    // if we do not do this, then tap orders such as 1-tap 1-tap 2-tap will not be read
    // as a double tap since the second 1-tap is processed as a failed second tap of a double tap, and hence 2-tap becomes the first tap.
    if (data.tap.count > 0){
        auto prevEndTime = data.tap.timestamp;
        auto timeDiff = event.timestamp.ellapsedMillis(prevEndTime);
        if (timeDiff > DOUBLE_TAP_TIME_GAP){
            data.tap.count = 0;
        }
    }
}

void InputController::touchBeganCB(const cugl::TouchEvent& event, bool focus) {
    if (!_active){
        return;;
    }
    Vec2 touchPos = event.position;
    // default: combat = right sided, motion = left
    if (combatPosConstraint(touchPos, reversedGestures) && !_combatGesture.active){
        initGestureDataFromEvent(_combatGesture, event);
    }
    else if (motionPosConstraint(touchPos, reversedGestures) && !_motionGesture.active){
        initGestureDataFromEvent(_motionGesture, event);
    }
}
 

void InputController::touchEndedCB(const cugl::TouchEvent& event, bool focus) {
    if (!_active){
        return;;
    }
    
    Vec2 touchPos = event.position;
    
    if (_combatGesture.active && _combatGesture.touchID == event.touch){
        Vec2 swipeDir = touchPos - _combatGesture.initialPos;
        float changeInPosition = swipeDir.length();
        auto elapsed = event.timestamp.ellapsedMillis(_combatGesture.timestamp);
        
        // DODGE: swipe in any direction
        if (changeInPosition >= DODGE_SWIPE_LENGTH && elapsed <= DODGE_SWIPE_TIME){
            _keyDodgeDir.set(swipeDir.x, -swipeDir.y);
            _keyDodge = true;
        }
        
        // ATTACK: quick tap
        // CULog("%llu, %f", elapsed, changeInPosition);
        if (elapsed <= TAP_TIME && changeInPosition <= HOLD_IN_PLACE_CONSTRAINT){
            _keyAttack = true;
        }
        
        // PARRY (hold option)
        if (scheme == ControlOption::HOLD_PARRY){
            if (changeInPosition < HOLD_IN_PLACE_CONSTRAINT && elapsed >= HOLD_TIME){
                _keyParry = true;
            }
        }
        _combatGesture.active = false;
    }
    
    if (_motionGesture.active && _motionGesture.touchID == event.touch){
        Vec2 swipeDir = touchPos - _motionGesture.initialPos;
        float changeInPosition = swipeDir.length();
        auto elapsed = event.timestamp.ellapsedMillis(_motionGesture.timestamp);
        // PARRY (double tap option)
        if (scheme == ControlOption::DOUBLE_TAP_PARRY){
            TapData& tap = _motionGesture.tap;
            if (elapsed <= TAP_TIME && changeInPosition <= HOLD_IN_PLACE_CONSTRAINT){
                if (tap.count == 0){
                    tap.timestamp = event.timestamp;
                    tap.pos = _motionGesture.initialPos;
                    tap.count = 1;
                }
                else {
                    // second tap, check the difference in time
                    auto curStartTime = _motionGesture.timestamp;
                    auto prevEndTime = tap.timestamp;
                    auto timeDiff = curStartTime.ellapsedMillis(prevEndTime);
                    if (timeDiff <= DOUBLE_TAP_TIME_GAP){
                        _keyParry = true;
                    }
                    tap.count = 0; // clear the tap data (it is now processed)
                }
            }
        }
        
        // letting go of the joystick always implies no movement
        _keyMoveDir = Vec2::ZERO;
        _motionGesture.active = false;
    }
}

void InputController::touchMotionCB(const cugl::TouchEvent& event, const Vec2 previous, bool focus) {
    if (!_active){
        return;;
    }
    Vec2 touchPos = event.position;
    
    if (_motionGesture.active && _motionGesture.touchID == event.touch){
        _motionGesture.prevPos = previous;
        _motionGesture.curPos = event.position;
        // check if finger dragged enough to initiate movement
        Vec2 swipeDir = touchPos - _motionGesture.initialPos;
        float swipeLength = swipeDir.length();
        if (swipeLength >= MOVE_SWIPE_LENGTH){
            // update movement based on the direction vector
            //negate y because screen origin is different from game origin.
            _keyMoveDir.set(swipeDir.x, -swipeDir.y);
        }
        else {
            _keyMoveDir.setZero();  // stop moving
        }
    }
}
