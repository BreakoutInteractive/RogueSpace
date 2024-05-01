//
//  TransitionScene.hpp
//  RS
//
//  Created by Zhiyuan Chen on 4/30/24.
//

#ifndef TransitionScene_hpp
#define TransitionScene_hpp

#include <cugl/cugl.h>

using namespace cugl;

/**
 * A transition scene is an overlay played on top of a given screen which is rendered beneath.
 */
class TransitionScene : public cugl::Scene2 {
public:

protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;

    /** the rectangle overlay on top of the screen*/
    std::shared_ptr<scene2::SceneNode> _overlay;
    
    /** the color to start fading from*/
    Color4 _startColor;
    /** the color to fade into */
    Color4 _fadeInColor;
    /** the color to fade out to*/
    Color4 _fadeOutColor;
    /** seconds to complete fade in */
    float _fadeInDuration;
    /** time since start of fading in */
    float _fadeInElapsed;
    /** callback function to execute on fade in completion*/
    std::function<void()> _fadeInCallback;
    /** seconds to complete fade out */
    float _fadeOutDuration;
    /** time since start of fading out */
    float _fadeOutElapsed;
    /** callback function to execute on fade out completion*/
    std::function<void()> _fadeOutCallback;
    
    /** the current state of the */
    enum State { FADEIN, FADEOUT};
    
    State _transitionState;
    
    /** reset the transition to start at the beginning */
    void resetTransition();
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new transition scene with default white fade-in and fade out.
     */
    TransitionScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    ~TransitionScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * initializes a fade-in transition from clear to white and then a fade out from white to clear.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

#pragma mark
#pragma mark Scene Functionality
    /**
     * Sets whether the scene is currently active.
     *
     * if the scene is set to active, the transition starts playing and can be updated.
     *
     * if the scene is deactivated, the transition resets and stops.
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;
    
    /**
     * sets the fade-in effect duration and color
     * @param duration length (seconds) for fade in to last
     * @param color fade-in color
     */
    void setFadeIn(float duration, Color4 color = Color4::WHITE);
    
    /**
     * sets the fade-out effect duration and color
     * @param duration length (seconds) for fade out to last
     * @param color fade-out color
     */
    void setFadeOut(float duration, Color4 color = Color4::WHITE);
    
    /** sets the callback function to execute when fade in is completed */
    void setFadeInCallBack(std::function<void ()> callback){
        _fadeInCallback = callback;
    }
    
    /** sets the callback function to execute when fade out is completed */
    void setFadeOutCallBack(std::function<void ()> callback){
        _fadeOutCallback = callback;
    }
    
    /** delete all previously assigned callback functions */
    void resetCallbacks();
    
    /** advances the transition by `dt` seconds. */
    void update(float dt) override;

};


#endif /* TransitionScene_hpp */
