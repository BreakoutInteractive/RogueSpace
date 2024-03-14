#include "App.hpp"
#include "models/LevelConstants.hpp"

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
    _scene = State::LOAD;
    _loading.init(_assets);
    
    // Queue the other assets
    AudioEngine::start(24);
    _assets->loadDirectoryAsync("json/assets.json",nullptr);
    _assets->loadDirectoryAsync("json/scenes.json", nullptr);
    _assets->loadAsync<LevelModel>(LEVEL_ONE_KEY,LEVEL_ONE_FILE,nullptr);
    // Switch to deterministic mode
    setDeterministic(true);


    Application::onStartup(); // this is required
}

void App::onShutdown() {
    _loading.dispose();
    _gameplay.dispose();
    _pause.dispose();
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

void App::preUpdate(float dt) {
    switch (_scene) {
        case LOAD:
            updateLoadingScene(dt);
            break;
        case MENU:
            break;
        case PAUSE:
            _pause.setActive(true);
            _renderer->configurePauseButton(true);
            updatePauseScene(dt);
            break;
        case GAME:
            if(_renderer->getPaused()){
                _scene = State::PAUSE;
                _gameplay.clearInputs();
            }else{
                _gameplay.preUpdate(dt);
            }
            break;
    }
}


void App::fixedUpdate() {
    switch (_scene) {
        case GAME:
            // Compute time to report to game scene version of fixedUpdate
            _gameplay.fixedUpdate(getFixedStep()/1000000.0f);
            break;
        default:
            break;
    }
}


void App::postUpdate(float dt) {
    switch (_scene) {
        case GAME:
            // Compute time to report to game scene version of postUpdate
            _gameplay.postUpdate(getFixedRemainder()/1000000.0f);
            break;
        default:
            break;
    }
}
    

void App::updateLoadingScene(float dt) {
    if (_loading.isActive()) {
        _loading.update(dt);
    } else {
        _loading.dispose(); // Disables the input listeners in this mode
        _gameplay.init(_assets); // this makes GameScene active
        _renderer = _gameplay.getRenderer();
        _pause.init(_assets);
        _scene = State::GAME;
    }
}

void App::updatePauseScene(float timestep) {
    _pause.update(timestep);
    switch (_pause.getChoice()) {
        case PauseScene::Choice::RESTART:
            _pause.setActive(false);
            _renderer->configurePauseButton(false);
            _gameplay.restart();
            _scene = State::GAME;
            break;
        case PauseScene::Choice::GAME:
            _pause.setActive(false);
            _renderer->configurePauseButton(false);
            _scene = State::GAME;
            break;
        case PauseScene::Choice::NONE:
            break;
    }
}


void App::draw() {
    switch (_scene) {
        case LOAD:
            _loading.render(_batch);
            break;
        case PAUSE:
            _gameplay.render(_batch);
            _pause.render(_batch);
            break;
        case GAME:
            _gameplay.render(_batch);
            break;
        default:
            break;
    }
}

