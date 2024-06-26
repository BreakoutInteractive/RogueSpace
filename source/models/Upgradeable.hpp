//
//  Upgradeable.hpp
//  RS
//
//  Created by Dasha Griffiths on 3/26/24.
//

#ifndef Upgradeable_hpp
#define Upgradeable_hpp

#include "GameConstants.hpp"
#include <cugl/cugl.h>


class Upgradeable {
        
protected:
    /** current level of stat on 0 based number scale*/
    int _currLevel;
    /** type of upgrade following upgrades enum*/
    UpgradeType _type;
    /** initial value of stat */
    std::vector<float> _tierValues;
    
public:
    Upgradeable(){
        _currLevel=0;
    }
        
    /**
     * Initializes an upgrade at level 0 and values at each tier evenly by stepAmt
     *
     * @param stepAmt  amount gained per level
     * @param initialVal  value of first level
     *
     */
    Upgradeable(float initialVal, float maxValue, UpgradeType type);
    
    /**
     * Initializes an upgrade at level 0
     *
     * @param tiers  amount gained per level
     * @param type  value of first level
     *
     */
    Upgradeable(std::vector<float> tiers, UpgradeType type);
        
    /**
     * Initializes an upgrade based on data with default values set for currentLevel and stepAmt
     *
     */
    Upgradeable(std::shared_ptr<cugl::JsonValue> data);

    /**
     * Increases the current level and updates stat
     */
    void levelUp();
    
    /**
     * Decreases the current level and updates stat
     */
    void levelDown();
    
    /**
     * Adds an increase to stat
     *
     * @param boostValue amount of increase
     */
    void boostStat(float boostPercentage);
    
    /**
     * Adds an additonal percent increase to stat
     *
     * @param debuffValue amount of decrease
     */
    void debuffStat(float debuffValue);
    
    /**
     * Resets upgrade data
     *
     */
    void resetUpgrade();
    
    /**
     * Returns current level of stat
     */
    int getCurrentLevel(){return _currLevel;}
    
    /**
     * Returns current level of stat
     */
    void setCurrentLevel(int level){_currLevel=level;}
        
    /**
     * Returns max level of stat
     */
    int getMaxLevel(){return (int)_tierValues.size()-1;}
    
    bool isMaxLevel(){ return _currLevel == getMaxLevel(); }
    
    UpgradeType getType(){return _type;}
    
    /**
     * Returns max value of stat
     */
    float getMaxValue(){return _tierValues.at(_tierValues.size()-1);}
        
    /**
     * Returns the value of stat after level up
     */
    float getNextValue(){return _tierValues.at(_currLevel+1);}
    
    /**
     * Returns current value of stat
     */
    float getCurrentValue(){return _tierValues.at(_currLevel);}
    
};

#endif /* Upgradeable_hpp */
