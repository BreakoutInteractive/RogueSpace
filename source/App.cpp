#include "App.hpp"
#include "models/LevelConstants.hpp"
#include "utility/SaveData.hpp"

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
//    _assets->attach<LevelModel>(GenericLoader<LevelModel>::alloc()->getHook());
    _assets->attach<JsonValue>(JsonLoader::alloc()->getHook());
    
    // Create a "loading" screen
    _scene = State::LOAD;
    _loading.init(_assets);
    
    // Queue the other assets
    AudioEngine::start(24);
    _assets->loadDirectoryAsync("json/assets.json",nullptr);
    _assets->loadDirectoryAsync("json/scenes/gameplay.json", nullptr);
    _assets->loadDirectoryAsync("json/scenes/hud.json", nullptr);
    _assets->loadDirectoryAsync("json/scenes/pause.json", nullptr);
    _assets->loadDirectoryAsync("json/scenes/upgrades.json", nullptr);
    _assets->loadDirectoryAsync("json/scenes/title.json", nullptr);
    _assets->loadDirectoryAsync("json/animations/player.json", nullptr);
    _assets->loadDirectoryAsync("json/animations/enemy.json", nullptr);
    _assets->loadDirectoryAsync("json/assets-tileset.json", nullptr);


    Application::onStartup(); // this is required
}

void App::onShutdown() {
    _loading.dispose();
    _gameplay.dispose();
    _pause.dispose();
    _upgrades.dispose();
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

void App::update(float dt){
    if (_loading.isActive()) {
        _loading.update(0.01f);
    } else {
        _loading.dispose(); // Disables the input listeners in this mode
        _gameplay.init(_assets); // this makes GameScene active
        _pause.init(_assets);
        _upgrades.init(_assets);
        _title.init(_assets);
        // finish loading -> go to title/main menu
        _scene = State::TITLE;
        setTitleScene();
        setDeterministic(true);
    }
}

void App::preUpdate(float dt) {
    switch (_scene) {
        case LOAD:
            // only for intermediate loading screens
            break;
        case TITLE:
            updateTitleScene(dt);
            break;
        case UPGRADE:
            _upgrades.setActive(true);
            _gameplay.activateInputs(true);
            _gameplay.getRenderer().setActivated(false);
            _gameplay.preUpdate(dt);
            updateUpgradesScene(dt);
            break;
        case PAUSE:
            _pause.setActive(true);
            updatePauseScene(dt);
            break;
        case GAME:
            if(_gameplay.getRenderer().getPaused()){
                _scene = State::PAUSE;
                _gameplay.activateInputs(false);
                _gameplay.getRenderer().setActivated(false);
            } else if (_gameplay.upgradeScreenActive){
                _upgrades.setActive(false);
                _scene = State::UPGRADE;
                _upgrades.updateScene(_gameplay.getDisplayedUpgrades(), _gameplay.getAvailableUpgrades());
            }
            else{
                _gameplay.activateInputs(true);
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
        case UPGRADE:
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
        case UPGRADE:
            _gameplay.postUpdate(getFixedRemainder()/1000000.0f);
            break;
        default:
            break;
    }
}

void App::updatePauseScene(float dt) {
    _pause.update(dt);
    switch (_pause.getChoice()) {
        case PauseScene::Choice::RESTART:
            _pause.setActive(false);
            _gameplay.getRenderer().setActivated(true);
            _gameplay.restart();
            _scene = State::GAME;
            break;
        case PauseScene::Choice::RESUME:
            _pause.setActive(false);
            _gameplay.getRenderer().setActivated(true);
            _gameplay.activateInputs(true);
            _scene = State::GAME;
            break;
        case PauseScene::Choice::SETTINGS:
            _pause.setActive(false);
            break;
        case PauseScene::Choice::NONE:
            break;
    }
}

void App::updateUpgradesScene(float dt){
    _upgrades.update(dt);
    if (!_gameplay.upgradeScreenActive){
        _upgrades.setActive(false);
        _gameplay.getRenderer().setActivated(true);
        _scene = State::GAME;
    } else{ 
        switch (_upgrades.getChoice()) {
            case UpgradesScene::HEALTH:
                _upgrades.setActive(false);
                _gameplay.getRenderer().setActivated(true);
                _gameplay.updatePlayerAttributes(_upgrades._selectedUpgrade);
                _gameplay.upgradeScreenActive=false;
                _gameplay.upgradeChosen = true;
                _gameplay.setRelicActive(false);
                _scene = State::GAME;
            case UpgradesScene::Choice::UPGRADE_1:
                _upgrades.setActive(false);
                _gameplay.getRenderer().setActivated(true);
                _gameplay.updatePlayerAttributes(_upgrades._selectedUpgrade);
                _gameplay.upgradeScreenActive=false;
                _gameplay.upgradeChosen = true;
                _gameplay.setRelicActive(false);
                _scene = State::GAME;
                break;
            case UpgradesScene::Choice::UPGRADE_2:
                _upgrades.setActive(false);
                _gameplay.getRenderer().setActivated(true);
                _gameplay.updatePlayerAttributes(_upgrades._selectedUpgrade);
                _gameplay.upgradeScreenActive=false;
                _gameplay.upgradeChosen = true;
                _gameplay.setRelicActive(false);
                _scene = State::GAME;
                break;
            default:
                break;
        }
    }
}

void App::setTitleScene(){
    bool hasSave = SaveData::hasGameSave();
    CULog("previous save available: %s", (hasSave ? "true" : "false"));
    auto sceneType = hasSave ? TitleScene::SceneType::WITH_CONTINUE : TitleScene::SceneType::WITHOUT_CONTINUE;
    _title.setSceneType(sceneType);
    _title.setActive(true);
}

void App::updateTitleScene(float dt){
    auto save = SaveData::getGameSave();
    switch (_title.getChoice()){
        case TitleScene::NONE:
            break;
        case TitleScene::NEW:
            _title.setActive(false);
            _gameplay.setActive(true);
            _gameplay.restart();
            _scene = GAME; // switch to game scene
            break;
        case TitleScene::CONTINUE:
            _title.setActive(false);
            _gameplay.setActive(true);
            CULog("loading lv %d", save.level);
            _gameplay.setLevel(save);
            _scene = GAME;
            break;
        case TitleScene::SETTINGS: case TitleScene::TUTORIAL:
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
        case UPGRADE:
            _gameplay.render(_batch);
            _upgrades.render(_batch);
            break;
        case TITLE:
            _title.render(_batch);
        default:
            break;
    }
}

