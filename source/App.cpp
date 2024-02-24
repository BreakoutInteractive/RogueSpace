#include "App.h"
#include "models/JSLevelConstants.h"

using namespace cugl;


#pragma mark -
#pragma mark Application State

void App::onStartup() {
    _assets = AssetManager::alloc();
    _batch  = SpriteBatch::alloc();
    
    // Start-up basic input
#ifdef CU_TOUCH_SCREEN
    Input::activate<Touchscreen>();
#else
    Input::activate<Mouse>();
#endif
    
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Sound>(SoundLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook());
    _assets->attach<LevelModel>(GenericLoader<LevelModel>::alloc()->getHook());
    
    // Create a "loading" screen
    _loaded = false;
    _loading.init(_assets);
    
    // Queue the other assets
    AudioEngine::start(24);
    _assets->loadDirectoryAsync("json/assets.json",nullptr);
    _assets->loadAsync<LevelModel>(LEVEL_ONE_KEY,LEVEL_ONE_FILE,nullptr);

    Application::onStartup(); // this is required
}

void App::onShutdown() {
    _loading.dispose();
    _gameplay.dispose();
    _assets = nullptr;
    _batch = nullptr;
    
    // Shutdown input
#ifdef CU_TOUCH_SCREEN
    Input::deactivate<Touchscreen>();
#else
    Input::deactivate<Mouse>();
#endif
    
    AudioEngine::stop();
    Application::onShutdown();  // this is required
}


void App::onSuspend() {
    AudioEngine::get()->pause();
}


void App::onResume() {
    AudioEngine::get()->resume();
}


#pragma mark -
#pragma mark Application Loop


void App::update(float timestep) {
    if (!_loaded && _loading.isActive()) {
        _loading.update(0.01f);
    } else if (!_loaded) {
        _loading.dispose(); // Disables the input listeners in this mode
        _gameplay.init(_assets); // this makes GameScene active
        _loaded = true;
        
        // Switch to deterministic mode
        setDeterministic(true);
	}
}


void App::preUpdate(float dt) {
    _gameplay.preUpdate(dt);
}


void App::fixedUpdate() {
    // Compute time to report to game scene version of fixedUpdate
    float time = getFixedStep()/1000000.0f;
    _gameplay.fixedUpdate(time);
}


void App::postUpdate(float dt) {
    // Compute time to report to game scene version of postUpdate
    float time = getFixedRemainder()/1000000.0f;
    _gameplay.postUpdate(time);
}


void App::draw() {
    if (!_loaded) {
        _loading.render(_batch);
    } else {
        _gameplay.render(_batch);
    }
}

