//
//  AudioController.cpp
//  RS
//

#include "AudioController.hpp"
#include <stdio.h>

using namespace cugl;

#pragma mark -
#pragma mark Constructors

    /**
     * Initializes the controller
     */
void AudioController::init(std::shared_ptr<cugl::AssetManager> assets, SaveData::Preferences p){
    _assets = assets;
    _master = p.vol / 10.0;
    _sfx = p.SFXvol / 10.0;
    _bgm = p.BGMvol / 10.0;
    CULog("%f", _bgm);
}
        
#pragma mark -
#pragma mark audio
    /**
     * Plays sound associated with collision bodies.
     *
     * @param  key1  the reference key for the sound effect
     * @param  key2  the reference key for the sound effect
     */
void AudioController::playCollisionFX(const std::string key1, const std::string key2){
    std::string uniqueKey = key1+key2;
    if (!AudioEngine::get()->isActive(uniqueKey)) {
        if(key1 == "player" && key2.substr(0,4)=="wall"){
            auto source = _assets->get<Sound>("bumpWall");
            AudioEngine::get()->play(uniqueKey, source, false, source->getVolume() * _master * _sfx);
        }
    }
}
    
    /**
     * Plays sound associated with player action.
     *
     * @param  action  the action key for the player
     */
void AudioController::playPlayerFX(const std::string action){
    if (!AudioEngine::get()->isActive(action+"player")) {
        std::shared_ptr<Sound> source; // TODO: might be bad to create a <Sound> instead of <AudioSample>
        bool loop = false;
        if (action == "attackMiss") {
            source = _assets->get<Sound>("airSlash");
        } 
        else if (action == "attackHit") {
            source = _assets->get<Sound>("playerAttack");
        }
        else if (action == "drawBow") {
            source = _assets->get<Sound>("bowDraw");
        }
        else if (action == "loopBow") {
            source = _assets->get<Sound>("bowCharge");
            loop = true;
        }
        else if (action == "shootBow") {
            source = _assets->get<Sound>("bowFire");
        }
        else {
            CULogError("player sound action not found");
        }
        AudioEngine::get()->play(action+"player", source, loop, source->getVolume() * _master * _sfx);
    }
}

/**
 * Clears sound associated with player action.
 *
 * @param  action  the action key for the player
 */
void AudioController::clearPlayerFX(const std::string action){
    if (AudioEngine::get()->isActive(action+"player")) {
        AudioEngine::get()->clear(action+"player");
    }
}
    
    /**
     * Plays sound associated with action of enemy type.
     *
     * @param  action  the action key for the enemy
     * @param  key  the reference key for the enemy type
     */
void AudioController::playEnemyFX(const std::string action, const std::string key){
    if (!AudioEngine::get()->isActive(action+key)) {
        if (action == "attackHit") {
            auto source = _assets->get<Sound>("enemyMelee");
            AudioEngine::get()->play(action+key, source, false, source->getVolume() * _master * _sfx);
        }
    }
}

/**
 * Plays sound associated with player action.
 *
 * @param  action  the action key for the player
 */
void AudioController::playUiFX(const std::string action){
    if (!AudioEngine::get()->isActive(action+"ui")) {
        if (action == "menuClick") {
            auto source = _assets->get<Sound>("menuClick");
            AudioEngine::get()->play(action+"ui", source, false, source->getVolume() * _master * _sfx);
        }
        else if (action == "upgrade") {
            auto source = _assets->get<Sound>("upgrade");
            AudioEngine::get()->play(action+"ui", source, false, source->getVolume() * _master * _sfx);
        }
    }
}
    
    /**
     * Plays music associated with scene.
     *
     * @param  key1  the reference key for the event
     */
void AudioController::playMusic(const std::string key) {
    if (!AudioEngine::get()->isActive("music")) {
        if (key == "title") {
            auto source = _assets->get<Sound>("title");
            AudioEngine::get()->play("music", source, true, source->getVolume() * _master * _bgm);
        } else if (key == "oasis") {
            auto source = _assets->get<Sound>("oasis");
            AudioEngine::get()->play("music", source, true, source->getVolume() * _master * _bgm);
        }
        _currTrack = key;
    } else if (_currTrack != key) {
        if (key == "clear") {
            AudioEngine::get()->clear("music");
        } 
        else if (key == "title") {
            AudioEngine::get()->clear("music");
        }
        else if (key == "oasis") {
            AudioEngine::get()->clear("music");
        }
        _currTrack = key;
    } else if (AudioEngine::get()->getVolume("music") != 0.5 * _master * _bgm) {
        AudioEngine::get()->setVolume("music", 0.5 * _master * _bgm);
    }
}
    
    /**
     * Pause all sound associated with game.
     */
void AudioController::PauseSound(){
    AudioEngine::get()->pause();
}
    
    /**
     * Plause all effects associated with level.
     */
void AudioController::PauseFX(){
    AudioEngine::get()->pauseEffects();
}
    
    /**
     * Resume all sound associated with game.
     *
     * @param  key1  the reference key for the event
     */
void AudioController::resumeSound(){
    AudioEngine::get()->resume();
}
    
    /**
     * Resume all effects associated with level.
     */
void AudioController::resumeFX(){
    AudioEngine::get()->resumeEffects();
};

/**
 * Change master volume
 */
void AudioController::changeMasterVolume(float t) {
    _master += t;
    if (_master < 0.0) {
        _master = 0.0;
    } else if (_master > 1.0) {
        _master = 1.0;
    }
};

/**
 * Change sfx volume
 */
void AudioController::changeSFXVolume(float t){
    _sfx += t;
    if (_sfx < 0.0) {
        _sfx = 0.0;
    } else if (_sfx > 1.0) {
        _sfx = 1.0;
    }
};

/**
 * Change master volume
 */
void AudioController::changeBGMVolume(float t){
    _bgm += t;
    if (_bgm < 0.0) {
        _bgm = 0.0;
    } else if (_bgm > 1.0) {
        _bgm = 1.0;
    }
};
