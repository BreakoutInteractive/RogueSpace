#include "InputController.hpp"

using namespace cugl;

#pragma mark Input Constants

/** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for toggling the debug display */
#define DEBUG_KEY KeyCode::Q
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE

/** How far we must swipe (in pixels) in any direction for a dodge gesture*/
const int DODGE_SWIPE_LENGTH = 175;
/** the maximum amount of milliseconds for a motion swipe to be considered a dodge*/
const int DODGE_SWIPE_TIME = 200;
/** How far we must swipe in any direction for a movement gesture i*/
const int MOVE_SWIPE_LENGTH = 100;
/** the minimum amount of milliseconds for a press to be considered a hold*/
const u_long HOLD_TIME = 300;
/** the maximum change in position for a hold to not be considered a drag/swipe*/
const int HOLD_POS_DELTA = 50;
/** the maximum number of milliseconds for a tap to be considered */
const u_long TAP_TIME = 150;
/** the maximum amount of milliseconds between the end of the first tap and the start of the second tap*/
const u_long DOUBLE_TAP_TIME_GAP = 200;
/** the distance needed to pull the joystick backwards to fire the projectile */
const float DRAG_DISTANCE = 100;


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
    if (_keyAttack) _keyAttackDir = (mouse->pointerPosition());
    _keyAttackDown = mouse->buttonDown().hasLeft();
    _keyAttackReleased = mouse->buttonReleased().hasLeft();
    _keyParry = mouse->buttonPressed().hasRight();
    _keyDodge = keys->keyDown(KeyCode::SPACE);
    _keySwap = keys->keyPressed(KeyCode::LEFT_SHIFT);
    
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
    _attackDown = _keyAttackDown;
    _attackReleased = _keyAttackReleased;
    _dodgePressed = _keyDodge;
    _parryPressed = _keyParry;
    _swapPressed = _keySwap;
    
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
    _keySwap = false;
#endif
}

void InputController::clear() {
    // clear abstraction data
    _resetPressed = false;
    _debugPressed = false;
    _exitPressed  = false;
    _dodgePressed = false;
    _attackPressed = false;
    _attackReleased = false;
    _parryPressed = false;
    _swapPressed = false;
    _moveDir.setZero();
    _dodgeDir.setZero();
    _attackDir.setZero();
    
    // clear internal data
    _keyDodge = false;
    _keyParry = false;
    _keyAttack = false;
    _keyAttackReleased = false;
    _keyReset = false;
    _keyDebug = false;
    _keyExit = false;
    _keySwap = false;
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
    _dodgeDir.set(_keyDodgeDir).normalize();
    #endif
    return _dodgeDir;
}

Vec2 InputController::getAttackDirection(cugl::Vec2 facingDir){
    if (mode == Mode::MELEE){
        _attackDir.set(facingDir).normalize();
    }
    else {
        _attackDir.set(_keyAttackDir).normalize();
    }
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
    data.touchMoved = false;
    
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
//        CULog("%llu, %f", elapsed, changeInPosition);
        
        // DODGE: swipe in any direction
        if (changeInPosition >= DODGE_SWIPE_LENGTH && elapsed <= DODGE_SWIPE_TIME){
            _keyDodgeDir.set(swipeDir.x, -swipeDir.y);
            _keyDodge = true;
        }
        
        // MELEE ATTACK: quick tap
        if (mode == Mode::MELEE && elapsed <= TAP_TIME && changeInPosition <= HOLD_POS_DELTA && !_combatGesture.touchMoved){
            _keyAttack = true;
        }
        
        // PARRY (hold option)
        if (scheme == ControlOption::HOLD_PARRY){
            if (mode == Mode::MELEE && changeInPosition < HOLD_POS_DELTA && elapsed >= HOLD_TIME && !_combatGesture.touchMoved){
                _keyParry = true;
            }
        }
        _combatGesture.active = false;
    }
    
    if (_motionGesture.active && _motionGesture.touchID == event.touch){
        Vec2 swipeDir = touchPos - _motionGesture.initialPos;
        float changeInPosition = swipeDir.length();
        auto elapsed = event.timestamp.ellapsedMillis(_motionGesture.timestamp);
        
        // FIRE (SHOOTING MODE)
        if (mode == Mode::RANGE && _combatGesture.active && !_combatGesture.touchMoved){
            if (changeInPosition >= DRAG_DISTANCE){
                _keyAttackDir.set(-swipeDir.x, swipeDir.y); // the signs are correct
                _keyAttack = true;
            }
        }
        
        // double tap weapon swap
        if (elapsed <= TAP_TIME && changeInPosition <= HOLD_POS_DELTA){
            TapData& tap = _motionGesture.tap;
            if (tap.count == 0){
                tap.timestamp = event.timestamp;
                tap.pos = _motionGesture.initialPos;
                tap.count = 1;
            }
            else {
                // second tap, check the difference in time
                // check difference in position between second tap and first tap
                auto curStartTime = _motionGesture.timestamp;
                auto prevEndTime = tap.timestamp;
                auto curStartPos = _motionGesture.curPos;
                auto prevStartPos = tap.pos;
                auto timeDiff = curStartTime.ellapsedMillis(prevEndTime);
                auto posDiff = (curStartPos - prevStartPos).length();
                // CULog("DTAP data: %llu, %f", timeDiff, posDiff);
                if (timeDiff <= DOUBLE_TAP_TIME_GAP && posDiff <= HOLD_POS_DELTA){
                    _keySwap = true;
                    mode = mode == Mode::MELEE ? Mode::RANGE : Mode::MELEE;
                }
                tap.count = 0; // clear the tap data (it is now processed)
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
    
    // check if we ever moved the combat touch (this disables all attacks)
    if (_combatGesture.active && _combatGesture.touchID == event.touch
        && !_combatGesture.touchMoved){
        Vec2 swipeDir = touchPos - _combatGesture.initialPos;
        float changeInPosition = swipeDir.length();
        auto elapsed = event.timestamp.ellapsedMillis(_combatGesture.timestamp);
        if (changeInPosition > HOLD_POS_DELTA){
            _combatGesture.touchMoved = true;
        }
    }
    
    
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
