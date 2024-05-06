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
    /** maximum level stat can reach on 0 based number scale */
    int _maxLevel;
    /** initial value of stat */
    float _initialValue;
    /** current value of stat */
    float _currValue;
    /** maximum value stat can reach */
    float _maxValue;
    
private:
    /** amount gained between levels */
    float _stepAmt;

public:
    Upgradeable(){
        _maxLevel=0;
        _currLevel=0;
        _initialValue=0;
        _currValue=0;
        _maxValue=0;
        _stepAmt=0;
    }
    
    /**
     * Initializes an upgrade at level 0 and increments evenly by level up to max level
     *
     * @param maxLevel maximum amount of levels on 0 based number scale
     * @param maxValue maximum value the stat can reach
     * @param baseValue starting value of stat
     * @param type  category of stat
     *
     */
    Upgradeable(int maxLevel, float maxValue, float baseValue);
    
    /**
     * Initializes an upgrade at level 0 and increments by stepAmt up to max level
     *
     * @param maxLevel maximum amount of levels on 0 based number scale
     * @param maxValue maximum value the stat can reach
     * @param baseValue starting value of stat
     * @param type  category of stat
     * @param stepAmt  category of stat
     *
     */
    Upgradeable(int maxLevel, float maxValue, float baseValue, float stepAmt);

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
    
    void resetUpgrade();
    
    /**
     * Returns current level of stat
     */
    int getCurrentLevel(){return _currLevel;}
        
    /**
     * Returns max level of stat
     */
    int getMaxLevel(){return _maxLevel;}
    
    /**
     * Returns max level of stat
     */
    float getMaxValue(){return _maxValue;}
    
    /**
     * Returns current percentage of stat in terms of percentage until its maximum Value is reached
     */
    float getCurrentPercentage(){return _currValue/_maxValue;}
    
    /**
     * Returns the value of stat after level up
     */
    float getNextValue(){return _currValue+_stepAmt;}
    
    /**
     * Returns current value of stat
     */
    float getCurrentValue(){return _currValue;}
    
};

#endif /* Upgradeable_hpp */
