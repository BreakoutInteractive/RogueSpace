//
//  Upgradeable.hpp
//  RS
//
//  Created by Dasha Griffiths on 3/26/24.
//

#ifndef Upgradeable_hpp
#define Upgradeable_hpp

#include <stdio.h>
#include <cugl/cugl.h>

class Upgradeable {
        
protected:
    /** current level of stat on 0 based number scale*/
    int _currLevel;
    /** type of upgrade following upgrades enum*/
    int _type;
    /** initial value of stat */
    float _tierValues[6];
    
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
    Upgradeable(float initialVal, float maxValue, int type);
    
    /**
     * Initializes an upgrade at level 0
     *
     * @param stepAmt  amount gained per level
     * @param initialVal  value of first level
     *
     */
    Upgradeable(float tiers[6], int type);
        
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
     * Returns max level of stat
     */
    int getMaxLevel(){return 6;}
    
    int getType(){return _type;}
    
    /**
     * Returns max value of stat
     */
    float getMaxValue(){return _tierValues[5];}
        
    /**
     * Returns the value of stat after level up
     */
    float getNextValue(){return _tierValues[_currLevel+1];}
    
    /**
     * Returns current value of stat
     */
    float getCurrentValue(){return _tierValues[_currLevel];}
    
};

#endif /* Upgradeable_hpp */
