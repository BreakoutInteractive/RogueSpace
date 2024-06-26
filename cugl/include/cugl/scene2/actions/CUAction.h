//
//  CUAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for animation actions.  Actions are primitive
//  tweening operations on scene graph nodes.  They take an start state and an
//  end state and linearly interpolate them over a period of time.  Examples of
//  such operations include Move, Scale, Rotate, Fade, and Animate.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Sophie Huang and Walker White
//  Version: 12/12/22
//
#ifndef __CU_ACTION_H__
#define __CU_ACTION_H__

#include <cugl/math/cu_math.h>
#include <cugl/scene2/graph/CUSceneNode.h>

namespace cugl {
    /**
     * The classes to construct an 2-d scene graph.
     *
     * This namespace was chosen to future-proof the game engine. We will
     * eventually want to add 3-d scene graphs as well, and this namespace
     * will prevent any collisions with those scene graph nodes.
     */
    namespace scene2 {

/**
 * This class provides an template for an animation action template.
 *
 * An action contains only the definition of the animation. This can include
 * information about the transform to use or the duration of the animation.
 * However, it does not contain any attribute of the target. Hence, an action 
 * can be reapplied to different targets.
 *
 * By itself, an action does nothing.  It only specifies an action that may
 * take place. To use an action, it must be passed to the ActionManager.  The 
 * manager will create an action instance and animate that instance.  While an
 * action may be reused many times, an action instance corresponds to a single
 * animation.
 *
 * To define a custom action, simply create a subclass and override the uppdate
 * method.  This is the method that an action uses to update its target Node.
 */
class Action {
protected:
    /** The duration (in seconds) of the animation */
    float _duration;
    
    /** The callback function when animation completes */
    std::function<void(void)> _completionCallback = [](){};

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Action() {}
    
    /**
     * Deletes this action, disposing all resources
     */
    ~Action() {}

#pragma mark -
#pragma mark Attributes
    /**
     * Returns the duration (in seconds) of this animation
     * 
     * Changing this value in mid-animation has undefined side-effects.
     *
     * @return the duration (in seconds) of this animation
     */
    float getDuration() const { return _duration; }

    /**
     * Sets the duration (in seconds) of this animation
     *
     * Changing this value in mid-animation has undefined side-effects.
     *
     * @param time  the duration (in seconds) of this animation
     */
    void setDuration(float time) { _duration = time; }
    
#pragma mark -
#pragma mark Animation Methods
    /**
     * Returns a newly allocated copy of this Action.
     *
     * @return a newly allocated copy of this Action.
     */
    virtual std::shared_ptr<Action> clone();

    /**
     * Prepares a target for action
     *
     * The important state of the target should be allocated and stored in the given
     * state reference. The semantics of this state is action-dependent.
     *
     * @param target    The node to act on
     * @param state     The address to store the node state
     */
    virtual void start(const std::shared_ptr<SceneNode>& target, void** state) {}

    /**
     * Cleans up a target after an action
     *
     * The target state in the given address should be deallocated, just as it was
     * allocated in {@link #start}. The semantics of this state is action-dependent.
     *
     * @param target    The node to act on
     * @param state     The address to deallocate the node state
     */
    virtual void stop(const std::shared_ptr<SceneNode>& target, void** state) {}

    /**
     * Executes an (incremental) action on the given target node.
     *
     * The important state of the target is stored in the given state parameter.
     * The semantics of this state is action-dependent.
     *
     * @param target    The node to act on
     * @param state     The relevant node state
     * @param dt        The elapsed time to animate.
     */
    virtual void update(const std::shared_ptr<SceneNode>& target, void* state, float dt) {}

    /**
     * Sets the callback to be executed when the animation completes in its entirety. A partially
     * completed animation that is stoppped/removed will not execute this callback.
     * @param callback function to execute when the animation completes.
     */
    virtual void setOnCompleteCallback(std::function<void(void)> callback){
        _completionCallback = callback;
    }
    
    /**
     * Executes the callback attached to the completion event of this animation.
     */
    virtual void executeOnCompleteCallback(){
        _completionCallback();
    }
    
#pragma mark -
#pragma mark Debugging Methods
    /**
     * Returns a string representation of the action for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this action for debuggging purposes.
     */
    virtual std::string toString(bool verbose = false) const;
    
    /** Cast from an Action to a string. */
    operator std::string() const { return toString(); }
};
    }
}
#endif /* __CU_ACTION_H__ */
