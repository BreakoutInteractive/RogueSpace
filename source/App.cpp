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
    _assets->loadDirectoryAsync("json/scenes/settings.json", nullptr);
    _assets->loadDirectoryAsync("json/scenes/death.json", nullptr);
    _assets->loadDirectoryAsync("json/animations/player.json", nullptr);
    _assets->loadDirectoryAsync("json/animations/enemy.json", nullptr);
    _assets->loadDirectoryAsync("json/assets-tileset.json", nullptr);


    Application::onStartup(); // this is required
}

void App::onShutdown() {
    _loading.dispose();
    _gameplay.dispose();
    _pause.dispose();
    _settings.dispose();
    _title.dispose();
    _death.dispose();
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
        _settings.init(_assets);
        _title.init(_assets);
        _death.init(_assets);
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
        case PAUSE:
            _pause.setActive(true);
            updatePauseScene(dt);
            break;
        case SETTINGS:
            _settings.setActive(true);
            updateSettingsScene(dt);
            break;
        case GAME:
            if (_gameplay.getExitCode() == GameScene::ExitCode::DEATH){
                _scene = State::DEATH;
                _gameplay.setActive(false);
                _death.setActive(true);
            }
            else if(_gameplay.getRenderer().getPaused()){
                _scene = State::PAUSE;
                _pause.setLabels(_gameplay.getPlayerLevels());
                _gameplay.setActive(false);
            }
            else{
                _gameplay.setActive(true);
                _gameplay.preUpdate(dt);
            }
            break;
        case DEATH:
            updateDeathScene(dt);
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

void App::updatePauseScene(float dt) {
    _pause.update(dt);
    switch (_pause.getChoice()) {
        case PauseScene::Choice::BACK:
            _pause.setActive(false);
            switch (_gamePrevScene) {
                case TITLE:
                    setTitleScene();
                    break;
                default:
                    break;
                }
            break;
        case PauseScene::Choice::RESUME:
            _pause.setActive(false);
            _gameplay.getRenderer().setActivated(true);
            _gameplay.activateInputs(true);
            _scene = State::GAME;
            break;
        case PauseScene::Choice::SETTINGS:
            _pause.setActive(false);
            _settings.setActive(true);
            _scene = SETTINGS;
            _prevScene = PAUSE;
            break;
        case PauseScene::Choice::NONE:
            break;
    }
}

void App::setTitleScene(){
    _scene = TITLE;
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
            _gamePrevScene = TITLE;
            break;
        case TitleScene::CONTINUE:
            _title.setActive(false);
            _gameplay.setActive(true);
            CULog("loading lv %d", save.level);
            _gameplay.setLevel(save);
            _scene = GAME;
            _gamePrevScene = TITLE;
            break;
        case TitleScene::SETTINGS:
            _title.setActive(false);
            _settings.setActive(true);
            _scene = SETTINGS; // switch to settings scene
            _prevScene = TITLE;
            break;
        case TitleScene::TUTORIAL:
            break;
    }
}

void App::updateSettingsScene(float dt) {
    _settings.update(dt);
    switch (_settings.getChoice()) {
    case SettingsScene::Choice::CLOSE:
        _settings.setActive(false);
        switch (_prevScene) {
        case PAUSE:
            _pause.setActive(true);
            _scene = PAUSE; // switch to pause scene
            break;
        case TITLE:
            setTitleScene();
            _scene = TITLE;
            break;
        default: //should never be here since you can only access settings from pause and title scenes
            break;
        }
        break;
    //TODO: control volume when music/sfx are implemented
    case SettingsScene::Choice::VOLUP:
        break;
    case SettingsScene::Choice::VOLDOWN:
        break;
    case SettingsScene::Choice::SFXUP:
        break;
    case SettingsScene::Choice::SFXDOWN:
        break;
    case SettingsScene::Choice::MUSICUP:
        break;
    case SettingsScene::Choice::MUSICDOWN:
        break;
    case SettingsScene::Choice::INVERT:
        _gameplay.getInput().setInverted(!_gameplay.getInput().getInverted());
        break;
    case SettingsScene::Choice::NONE:
        break;
    }
}

void App::updateDeathScene(float dt){
    _death.update(dt);
    switch(_death.getChoice()){
        case DeathScene::NONE:
            break;
        case DeathScene::RESTART:
            _death.setActive(false);
            _gameplay.setActive(true);
            _gameplay.restart();
            _scene = GAME;
            break;
        case DeathScene::MAIN_MENU:
            _death.setActive(false);
            setTitleScene();
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
        case TITLE:
            _title.render(_batch);
            break;
        case SETTINGS:
            switch (_prevScene) {
            case PAUSE:
                _gameplay.render(_batch);
                _pause.render(_batch);
                break;
            case TITLE:
                _title.render(_batch);
                break;
            default: //should never be here since you can only access settings from pause and title scenes
                break;
            }
            _settings.render(_batch);
            break;
        case DEATH:
            _gameplay.render(_batch);
            _death.render(_batch);
    }
}

