//
//  SaveData.hpp
//  RS
//
//  Created by Zhiyuan Chen on 5/10/24.
//

#ifndef SaveData_hpp
#define SaveData_hpp

#include <cugl/cugl.h>

using namespace cugl;

/**
 a singleton class handling
 */
class SaveData {
    
public:
    
    /**
     * player data (with attributes default to 0)
     */
    struct Data {
        // TODO: add other fields as needed
        /** the level to load on start */
        int level;
        /** player current HP */
        float hp = 0;
        /** weapon index (0 for sword, 1 for bow) */
        int weapon = 0;
        int hpLvl = 0;
        int atkLvl = 0;
        int rangedLvl = 0;
        int defLvl = 0;
        int parryLvl = 0;
        int dashLvl = 0;
    };
    
    struct Preferences {
        // TODO: add other fields as needed
        /** background music volume */
        float BGMvol;
    };
    
private:
    /** prevents allocation of this class */
    SaveData(){}
    
    /** json cache object for player information */
    static std::shared_ptr<JsonValue> saveJson;
    /** struct cache object for player information */
    static Data dataCache;

public:
#pragma mark - Game Related Saves
    
    /**
     * @return whether there is a previous save of the game on the user's device
     */
    static bool hasGameSave();
    
    /**
     * @return save data
     */
    static Data getGameSave();
    
    /**
     * save the player's stats and upgrade levels
     */
    static void makeSave(Data data);
    
    /** deletes the previous] save of the Game */
    static void removeSave();
    
#pragma mark - Settings Related Saves
    
    static Preferences getPreferences();
    
    static void setPreferences(Preferences prefs);
    
};

#endif /* SaveData_hpp */
