//
//  AudioController.hpp
//  RS
//
//  Created by Brie Smith on 3/11/24.
//

#ifndef AudioController_hpp
#define AudioController_hpp

#include <cugl/cugl.h>
#include <stdio.h>

class AudioController {
protected:
    /** The asset manager for this audio controller. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
public:
#pragma mark -
#pragma mark Constructors

    /**
     * Default Constructor
    */
    AudioController(){}

    /**
     * Initializes the controller
     */
    void init(std::shared_ptr<cugl::AssetManager> assets);

#pragma mark -
#pragma mark audio
    
//    /**
//     * Returns true if the given key is associated with an active sound
//     *
//     * @param  key  the reference key for the sound effect
//     *
//     * @return true if the key is associated with an active sound.
//     */
//    bool isActive(const std::string key) const {
//        return cugl::AudioEngine::get()->isActive(key);
//    }
    
    /**
     * Plays sound associated with collision bodies.
     *
     * @param  key1  the reference key for the sound effect
     * @param  key2  the reference key for the sound effect
     */
    void playCollisionFX(const std::string key1, const std::string key2);
    
    /**
     * Plays sound associated with player action.
     *
     * @param  action  the action key for the player
     */
    void playPlayerFX(const std::string action);
    
    /**
     * Plays sound associated with action of eney type.
     *
     * @param  action  the action key for the enemy
     * @param  key  the reference key for the enemy type
     */
    void playEnemyFX(const std::string action, const std::string key);
    
    /**
     * Plays music associated with scene.
     *
     * @param  key1  the reference key for the event
     */
    void playMusic(const std::string key);
    
    /**
     * Paise all sound associated with game.
     *
     * @param  key1  the reference key for the event
     */
    void PauseSound(); 
    
    /**
     * Plause all effects associated with level.
     */
    void PauseFX();
    
    /**
     * Resume all sound associated with game.
     *
     * @param  key1  the reference key for the event
     */
    void resumeSound();
    
    /**
     * Resume all effects associated with level.
     */
    void resumeFX();
    
};

#endif /* AudioController_hpp */
