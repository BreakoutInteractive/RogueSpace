//
//  AudioController.cpp
//  RS
//
//  Created by Brie Smith on 3/11/24.
//

#include "AudioController.hpp"
#include <stdio.h>

using namespace cugl;

#pragma mark -
#pragma mark Constructors

    /**
     * Initializes the controller
     */
void AudioController::init(std::shared_ptr<cugl::AssetManager> assets){
    _assets = assets;
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
            AudioEngine::get()->play(uniqueKey, source, false, source->getVolume());
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
        if(action == "attackHit"){
            auto source = _assets->get<Sound>("playerAttack");
            AudioEngine::get()->play(action+"player", source, false, source->getVolume());
        }
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
        if(action == "attackHit"){
            auto source = _assets->get<Sound>("enemyAttack");
            AudioEngine::get()->play(action+key, source, false, source->getVolume());
        }
    }
}
    
    /**
     * Plays music associated with scene.
     *
     * @param  key1  the reference key for the event
     */
void AudioController::playMusic(const std::string key){
        
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
