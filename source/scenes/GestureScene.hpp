//
//  GestureScene.hpp
//  RS
//
//  Created by Zhiyuan Chen on 5/17/24.
//

#ifndef GestureScene_hpp
#define GestureScene_hpp

#include <cugl/cugl.h>

using namespace cugl;

/**
 * Similar to the upgrades scene, this class represents an optional overlay over the main `GameScene`. This scene is rendered on top of the game to assist players with controls.
 */
class GestureScene : public Scene2 {
    
public:
    
    /**
     * the gesture to animate on this tutorial overlay
     */
    enum Gesture {
        MOVE,
        DASH,
        ATTACK,
        SHOOT,
        PARRY
    };
    
protected:
    
    /** the asset directory */
    std::shared_ptr<AssetManager> _assets;
    /** the selected gesture to render*/
    Gesture gesture;
    /** the dimension of the resized scene */
    /** the value to apply to `setActive` after transitions are complete */
    bool _activeFlag;
    
#pragma mark - Scene Nodes

    Scene2 _overlays;
    /** the overlay used for left sided gesture*/
    std::shared_ptr<scene2::PolygonNode> _leftOverlay;
    /** the overlay used for right sided gestures (parry, dash, attack, shoot)*/
    std::shared_ptr<scene2::PolygonNode> _rightOverlay;

    // Röguespace gestures
    std::shared_ptr<scene2::SpriteNode> _moveGesture;
    std::shared_ptr<scene2::SpriteNode> _dashGesture;
    std::shared_ptr<scene2::SpriteNode> _attackGesture;
    std::shared_ptr<scene2::SpriteNode> _shootGesture;
    std::shared_ptr<scene2::SpriteNode> _parryGesture;

    
    // Active Set (pointers to the defined options above)
    /** the currently active overlay (or none) */
    std::shared_ptr<scene2::PolygonNode> _activeOverlay;
    /** the currently active gesture node (or none) */
    std::shared_ptr<scene2::SpriteNode> _activeGesture;
    
#pragma mark - Scene Graph Animation
    
    scene2::ActionManager _actionManager;
    
    std::string FADE_IN_GESTURE_KEY = "GESTURE_FADE_IN";
    std::string FADE_IN_KEY = "FADE_IN";
    std::shared_ptr<scene2::FadeIn> _fadeInAction;
    
    std::string FADE_OUT_GESTURE_KEY = "GESTURE_FADE_OUT";
    std::string FADE_OUT_KEY = "FADE_OUT";
    std::shared_ptr<scene2::FadeOut> _fadeOutAction;

    std::string ANIMATE_KEY = "ANIMATION";
    std::shared_ptr<scene2::Animate> _animateAction;
    
public:
    
    /**
     * initializes this scene with the necessary gestures and animation actions to display them.
     */
    bool init(std::shared_ptr<AssetManager> assets);
    
    /**
     * sets the active gesture to display.
     * @note an inactive scene will still not render/update any gestures
     */
    void setGesture(Gesture g);
    
    /**
     * @param name the gesture name (one of MOVE, DASH, ATTACK, SHOOT, PARRY)
     * @return the gesture to play, matching the given name
     */
    Gesture getGestureFromName(std::string name);
    
    void reset() override;
    
    /**
     * Unlike other typical usages of setActive, this involves transitions which are not immediately finished. If turning off transitions is necessary, use `reset`.
     */
    void setActive(bool value) override;
    
    /**
     * perform updates on the gestures to process their animation forward in time (and to loop).
     */
    void update(float dt) override;
    void render(const std::shared_ptr<SpriteBatch> &batch) override;
};

#endif /* GestureScene_hpp */
