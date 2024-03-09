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
/** the minimum amount of milliseconds for a press to be considered a hold*/
#define HOLD_TIME 300
/** the maximum amount of milliseconds for a motion swipe to be considered a dodge*/
#define DODGE_SWIPE_TIME 200
/** horizontal constraint for a swipe to qualify for upwards or downwards */
#define VSWIPE_HORIZONTAL_DIFF 250
/** the maximum amount of milliseconds between the end of the first tap and the start of the second tap*/
#define DOUBLE_TAP_TIME_GAP 200


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
    _resetPressed = false;
    _debugPressed = false;
    _exitPressed  = false;
    _dodgePressed = false;
    _attackPressed = false;
    _parryPressed = false;
    _motionGesture.active = false;
    _combatGesture.active = false;
    _settingsGesture.active = false;
}

#pragma mark -
#pragma mark Results

Vec2 InputController::getDodgeDirection(cugl::Vec2 facingDir){
    if (scheme == ControlOption::UNIFIED){
        return facingDir;
    }
    // OMNI
    return _dodgeDir;
}

Vec2 InputController::getAttackDirection(cugl::Vec2 facingDir){
    if (scheme == ControlOption::UNIFIED){
        return facingDir;
    }
    // OMNI
    return _attackDir;
}

#pragma mark -
#pragma mark Helper Functions

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

/**
 * @return true if the touch is applied on the settings gesture
 */
bool settingsPosConstraint(Vec2 touchPos){
    Size s = Application::get()->getDisplaySize();
    return s.width/3 < touchPos.x && touchPos.x < 2 * s.width/3;
}

void InputController::readEndGestureOminidirectional(GestureData& _motionGesture, GestureData& _combatGesture, const cugl::TouchEvent& event){
    Vec2 touchPos = event.position;
    
    // Dodge = quick release of motion gesture
    if (_motionGesture.active && event.touch == _motionGesture.touchID){
        _motionGesture.active = false;
        if (event.timestamp.ellapsedMillis(_motionGesture.timestamp) <= DODGE_SWIPE_TIME){
            // quick swipe
            Vec2 swipeDir = touchPos - _motionGesture.initialPos;
            float swipeLength = swipeDir.length();
            if (swipeLength >= EVENT_SWIPE_LENGTH){
                _keyDodgeDir.set(swipeDir.x, -swipeDir.y); //negate y because screen origin is different from game origin.
                _keyDodge = true;
            }
        }
        // slowly letting go of the joystick always implies no movement
        _keyMoveDir = Vec2::ZERO;
    }
    if (_combatGesture.active && event.touch == _combatGesture.touchID){
        _combatGesture.active = false;
        Vec2 gestureMotion = touchPos - _combatGesture.initialPos;
        float changeInPosition = gestureMotion.length();
        auto elapsed = event.timestamp.ellapsedMillis(_combatGesture.timestamp);
        if (changeInPosition < EVENT_SWIPE_LENGTH && elapsed >= HOLD_TIME){
            _keyParry = true;
        }
        if (changeInPosition >= EVENT_SWIPE_LENGTH && elapsed < HOLD_TIME){
            _keyAttack = true;
            _keyAttackDir.set(gestureMotion.x, -gestureMotion.y);
        }
        
        // TODO: user-testing: make sure a tap and a swipe isn't being recognized as being too similar
        bool is_tap_event = elapsed <= HOLD_TIME && changeInPosition < EVENT_SWIPE_LENGTH;
        TapData& tap = _combatGesture.tap;
        if (is_tap_event){
            if (tap.expired){
                // first tap, so we should renew the data so to anticipate a second tap.
                tap.expired = false;
                tap.timestamp = event.timestamp;
                tap.pos = _combatGesture.initialPos;
            }
            else {
                // second tap
                // check the difference in time
                auto curStartTime = _combatGesture.timestamp;
                auto prevEndTime = tap.timestamp;
                auto timeDiff = curStartTime.ellapsedMillis(prevEndTime);
                // TODO: figure out whether the two taps need to be as close as possible location-wise. that is quite interesting.
                if (timeDiff <= DOUBLE_TAP_TIME_GAP){
                    // double tap performed
                    _keySwap = true;
                    
                    _keyDodge = true;
                }
                tap.expired = true; // clear the tap data.
            }
        }
        else {
            // this release was not a tap (some form of a "hold" or quick swipe)
            tap.expired = true; // clear the tap data.
        }
    }
}

void InputController::readEndGestureUnified(GestureData &_motionGesture, GestureData &_combatGesture, const cugl::TouchEvent &event){
    Vec2 touchPos = event.position;
    
    if (_combatGesture.active && event.touch == _combatGesture.touchID){
        _combatGesture.active = false;
        Vec2 gestureMotion = touchPos - _combatGesture.initialPos;
        float changeInPosition = gestureMotion.length();
        auto elapsed = event.timestamp.ellapsedMillis(_combatGesture.timestamp);

        if (changeInPosition >= EVENT_SWIPE_LENGTH){
            // up
            if (gestureMotion.y < 0){
                _keyDodge = true;
                return;
            }
            // down
            if (gestureMotion.y > 0){
                _keyParry = true;
                return;
            }
        }
        bool is_tap_event = elapsed <= HOLD_TIME && changeInPosition <= 20;
        if (is_tap_event){
            _keyAttack = true;
        }
        else if (elapsed >= HOLD_TIME && changeInPosition <= 20){
            // HOLD DOWN to change weapon ? to charge ? to do boosted shots?
            _keySwap = true; // TODO: chosen weapon swap for now.
        }
    }
    
    if (_motionGesture.active && event.touch == _motionGesture.touchID){
        _keyMoveDir.setZero();
        _motionGesture.active = false;
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
    if (!data.tap.expired){
        auto prevEndTime = data.tap.timestamp;
        auto timeDiff = event.timestamp.ellapsedMillis(prevEndTime);
        if (timeDiff > DOUBLE_TAP_TIME_GAP){
            data.tap.expired = true;
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
    
    if (settingsPosConstraint(touchPos) && !_settingsGesture.active){
        initGestureDataFromEvent(_settingsGesture, event);
    }
}
 

void InputController::touchEndedCB(const cugl::TouchEvent& event, bool focus) {
    if (!_active){
        return;;
    }

    if (scheme == ControlOption::UNIFIED){
        readEndGestureUnified(_motionGesture, _combatGesture, event);
    }
    else if (scheme == ControlOption::OMNIDIRECTIONAL){
        readEndGestureOminidirectional(_motionGesture, _combatGesture, event);
    }
    
    
    // TODO: this is temporary setting on screen
    if (_settingsGesture.active && event.touch == _settingsGesture.touchID){
        _settingsGesture.active = false;
        Vec2 gestureMotion = event.position - _settingsGesture.initialPos;
        float changeInPosition = gestureMotion.length();
        auto elapsed = event.timestamp.ellapsedMillis(_settingsGesture.timestamp);
        
        if (changeInPosition >= EVENT_SWIPE_LENGTH && gestureMotion.y < 0){
            // this is a swipe up, change controls.
            scheme = (scheme == ControlOption::UNIFIED) ? ControlOption::OMNIDIRECTIONAL : ControlOption::UNIFIED;
        }
        bool is_tap_event = elapsed <= HOLD_TIME && changeInPosition <= 20;
        TapData& tap = _settingsGesture.tap;
        if (is_tap_event){
            if (tap.expired){
                // first tap, so we should renew the data so to anticipate a second tap.
                tap.expired = false;
                tap.timestamp = event.timestamp;
                tap.pos = _settingsGesture.initialPos;
            }
            else {
                // second tap, check the difference in time
                auto curStartTime = _settingsGesture.timestamp;
                auto prevEndTime = tap.timestamp;
                auto timeDiff = curStartTime.ellapsedMillis(prevEndTime);
                if (timeDiff <= DOUBLE_TAP_TIME_GAP){
                    reversedGestures = !reversedGestures; // double tap performed
                }
                tap.expired = true; // clear the tap data.
            }
        }
        else { 
            tap.expired = true; // clear the tap data.
        }
    }
}

void InputController::touchMotionCB(const cugl::TouchEvent& event, const Vec2 previous, bool focus) {
    if (!_active){
        return;;
    }
    // update positions in gestures (important for dodge and move)
    Vec2 touchPos = event.position;
    
    if (_motionGesture.active && _motionGesture.touchID == event.touch){
        _motionGesture.prevPos = previous;
        _motionGesture.curPos = event.position;
        // check if finger dragged enough to initiate movement
        Vec2 swipeDir = touchPos - _motionGesture.initialPos;
        float swipeLength = swipeDir.length();
        if (swipeLength >= EVENT_SWIPE_LENGTH){
            // update movement based on the direction vector
            _keyMoveDir.set(swipeDir.x, -swipeDir.y); //negate y because screen origin is different from game origin.
        }
    }
    
    if (scheme == ControlOption::UNIFIED){
        if (_combatGesture.active && _combatGesture.touchID == event.touch){
            // reinforce swipes to be vertical for dodge and parry
            // inactivate the touch if deviates too much from initial point of contact
            float deltaX = touchPos.x - _combatGesture.initialPos.x;
            if (abs(deltaX) >= VSWIPE_HORIZONTAL_DIFF){
                _combatGesture.active = false;
            }
        }
    }
}
