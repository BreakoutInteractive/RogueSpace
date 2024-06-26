#include "LoadingScene.hpp"

using namespace cugl;

/** The ID for the button listener */
#define LISTENER_ID 1

/** This is the size of the active portion of the screen */
#define SCENE_WIDTH 1024
#define SCENE_HEIGHT 576

#pragma mark -
#pragma mark Constructors

bool LoadingScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    Size dimen = computeActiveSize();
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // IMMEDIATELY load the splash screen assets
    _assets = assets;
    _assets->loadDirectory("json/loading.json");
    auto layer = assets->get<scene2::SceneNode>("load");
    layer->setContentSize(dimen);
    layer->doLayout(); // This rearranges the children to fit the screen
    
    _planetNode = std::dynamic_pointer_cast<scene2::SpriteNode>(_assets->get<scene2::SceneNode>("load_globe"));
    _planetEffect = Animation::alloc(SpriteSheet::alloc(_assets->get<Texture>("globe"), 2, 7), 1.0f, true);
    _planetEffect->start();
    
    Application::get()->setClearColor(Color4(192,192,192,255));
    addChild(layer);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LoadingScene::dispose() {
    _planetNode = nullptr;
    _planetEffect = nullptr;
    _assets = nullptr;
    _progress = 0.0f;
    removeAllChildren();
}


#pragma mark -
#pragma mark Progress Monitoring
/**
 * The method called to update the game mode.
 *
 * This method updates the progress bar amount.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LoadingScene::update(float progress) {
    if (_progress < 1) {
        _progress = _assets->progress();
        _planetEffect->update(progress);
        _planetNode->setFrame(_planetEffect->getFrame());
        _planetNode->setVisible(true);
        if (_progress >= 1) {
            _progress = 1.0f;
            _planetNode->setVisible(false);
            _planetEffect->reset();
            this->_active = false;
        }
    }
}


/**
 * Returns the active screen size of this scene.
 *
 * This method is for graceful handling of different aspect
 * ratios
 */
Size LoadingScene::computeActiveSize() const {
    Size dimen = Application::get()->getDisplaySize();
    float ratio1 = dimen.width/dimen.height;
    float ratio2 = ((float)SCENE_WIDTH)/((float)SCENE_HEIGHT);
    if (ratio1 < ratio2) {
        dimen *= SCENE_WIDTH/dimen.width;
    } else {
        dimen *= SCENE_HEIGHT/dimen.height;
    }
    return dimen;
}
