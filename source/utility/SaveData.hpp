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
        /** the level to load on start */
        int level;
        /** player current HP */
        float hp = 0;
        /** weapon index (0 for sword, 1 for bow) */
        int weapon = 0;
        int hpLvl = 0;
        int atkLvl = 0;
        int atkSpLvl = 0;
        int rangedLvl = 0;
        int defLvl = 0;
        int parryLvl = 0;
        int dashLvl = 0;
        /** whether this room has an available upgrade before the given level*/
        bool isUpgradeRoom = false;
        bool upgradeAvailable = false;
        int upgradeOpt1 = 0;
        int upgradeOpt1Level = 1;
        int upgradeOpt2 = 0;
        int upgradeOpt2Level = 1;
    };
    
    struct Preferences {
        // TODO: add other fields as needed
        /** master volume level - 0-10 */
        int vol = 5;
        /** background music volume level - 0-10 */
        int BGMvol = 10;
        /** sfx volume level - 0-10 */
        int SFXvol = 10;
        /** bow aim control mode*/
        bool inverted = true;
    };
    
private:
    /** prevents allocation of this class */
    SaveData(){}
    
    /** json cache object for player information */
    static std::shared_ptr<JsonValue> saveJson;
    /** struct cache object for player information */
    static Data dataCache;

    /** json cache object for player preferences */
    static std::shared_ptr<JsonValue> prefsJson;
    /** struct cache object for player preferences */
    static Preferences prefsCache;
    
    /**
     * @returns the given value clipped inside the given extremes
     */
    static int constraint(int value, int low, int high){
        return value > high ? high : (value < low ? low : value);
    }

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

    static bool hasPreferences();
    
    static Preferences getPreferences();
    
    /**
     * save the player's preferences
     */
    static void savePreferences(Preferences prefs);
    
};

#endif /* SaveData_hpp */
