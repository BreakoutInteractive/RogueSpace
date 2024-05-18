//
//  AudioController.hpp
//  RS
//

#ifndef AudioController_hpp
#define AudioController_hpp

#include <cugl/cugl.h>
#include <stdio.h>
#include "../utility/SaveData.hpp"

class AudioController {
protected:
    /** The asset manager for this audio controller. */
    static std::shared_ptr<cugl::AssetManager> _assets;
    
    static std::string _currTrack;
    static bool _looping;
    
    static float _master;
    static float _sfx;
    static float _bgm;
    
    /**
     * Default Constructor
    */
    AudioController(){}
    
public:
#pragma mark -
#pragma mark Constructors

    /**
     * Initializes the controller
     */
    static void init(std::shared_ptr<cugl::AssetManager> assets, SaveData::Preferences p);
    
    /**
     * @note should only be called once and remove reference to assets.
     */
    static void dispose(){ _assets = nullptr; }

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
     * @return get current track name
     */
    static std::string getCurrTrack() { return _currTrack; }
    
    /**
     * Change master volume
     *
     * @param t  the proportion to change by
     */
    static void changeMasterVolume(float t);
    
    /**
     * Change SFX volume
     *
     * @param t  the proportion to change by
     */
    static void changeSFXVolume(float t);
    
    /**
     * Change music volume
     *
     * @param t  the proportion to change by
     */
    static void changeBGMVolume(float t);
    
    /**
     * Plays sound associated with collision bodies.
     *
     * @param  key1  the reference key for the sound effect
     * @param  key2  the reference key for the sound effect
     */
    static void playCollisionFX(const std::string key1, const std::string key2);
    
    /**
     * Plays sound associated with player action.
     *
     * @param  action  the action key for the player
     */
    static void playPlayerFX(const std::string action);
    
    /**
     * Plays sound associated with player action.
     *
     * @param  action  the action key for the player
     */
    static void clearPlayerFX(const std::string action);
    
    /**
     * Plays sound associated with action of eney type.
     *
     * @param  action  the action key for the enemy
     * @param  key  the reference key for the enemy type
     */
    static void playEnemyFX(const std::string action, const std::string key);
    
    /**
     * Plays sound associated when enemy takes damage
     *
     * @param  action  the action key for the enemy
     * @param  key  the reference key for the enemy type
     */
    static void playDamagedEnemy(const std::string enemyType, const std::string key);
    
    /**
     * Plays sound associated when enemy takes aggro
     *
     * @param  action  the action key for the enemy
     * @param  key  the reference key for the enemy type
     */
    static void playAggroEnemy(const std::string enemyType);
    
    /**
     * Plays sound associated with projectile impact of enemy type.
     */
    
    static void onEnemyProjImpact(const std::string enemyType);
    
    /**
     * Plays sound associated with UI actions.
     *
     * @param  action  the action key for the enemy
     * @param  key  the reference key for the enemy type
     */
    static void playUiFX(const std::string action);
    
    /**
     * Plays music associated with scene.
     *
     * @param  key1  the reference key for the event
     */
    static void updateMusic(const std::string key, float fade);
    
    /**
     * Pause all sound associated with game.
     *
     * @param  key1  the reference key for the event
     */
    static void PauseSound();
    
    /**
     * Plause all effects associated with level.
     */
    static void PauseFX();
    
    /**
     * Resume all sound associated with game.
     *
     * @param  key1  the reference key for the event
     */
    static void resumeSound();
    
    /**
     * Resume all effects associated with level.
     */
    static void resumeFX();
    
};

#endif /* AudioController_hpp */
