//
//  SoundController.hpp
//
//  This controller is primarily responsible for handling audio-related features.
//
//  Author: Zhiyuan Chen
//  Version: 3/3/24
//

#ifndef __SOUND_CONTROLLER_HPP__
#define __SOUND_CONTROLLER_HPP__
#include <cugl/cugl.h>

class SoundController {

private:
    
    /** all sound effects*/
    std::unordered_map<std::string, std::shared_ptr<cugl::Sound>> _sounds;

    // TODO: does sound include music? 
    // TODO: add other attributes
    
public:
    
#pragma mark -
#pragma mark Constructors and Destructors

    /**
     * Destroys the controller and removes all references to resources (sounds, music, etc).
    */
    ~SoundController();

    /**
     * @return a pointer to a sound controller (to be owned by various other controllers to play SFX/BGM)
    */
    static std::shared_ptr<SoundController> alloc() {
        return std::make_shared<SoundController>();
    }

#pragma mark -
#pragma mark SFX Handling

    void setVolume();

    void playSFX(std::string name);

    // TODO: add other methods to interface

};

#endif /* __SOUND_CONTROLLER_HPP__ */