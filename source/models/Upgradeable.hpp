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

class Upgradeable { //map 1 to upgrades of type attack, map 2 to upgrades of type defense
        
protected:
    int _currentLevel;
    
    int _maxLevel;
    
    float _maxPercentage;
    
    float _currPercentage;
    
    float _baseValue;
    
    float _currValue;

public:
    Upgradeable(){
        _maxLevel=0;
        _currentLevel=0;
        _currPercentage=0;
        _maxPercentage=0;
        _baseValue=0;
        _currValue=0;
    }
    
    /**
     * Initializes an upgrade at level 0 and increments evenly by level up to max level
     *
     * @param maxLevel maximum amount of levels
     * @param maxPercentage maximum amount of increase possible on scale of 0-1
     * @param baseValue starting value of stat
     */
    Upgradeable(int maxLevel, float maxPercentage, int baseValue):
    _maxLevel(maxLevel), _currentLevel(0), _currPercentage(0), _maxPercentage(maxPercentage),
    _baseValue(baseValue), _currValue(baseValue){
            assert(maxLevel > 0);
        }
    
    /**
     * Initializes an upgrade at currLevel  and increments evenly by level up to max level
     *
     * @param maxLevel maximum amount of levels
     * @param maxPercentage maximum amount of increase possible on scale of 0-1
     * @param baseValue starting value of stat
     * @param currLevel starting level of sta
     */
    Upgradeable(int maxLevel, int currLevel, float maxPercentage, int baseValue):
    _maxLevel(maxLevel), _currentLevel(currLevel), _currPercentage(0), _maxPercentage(maxPercentage),
    _baseValue(baseValue), _currValue(baseValue){
            assert(maxLevel > 0);
            assert(currLevel >= 0);
        }
    
    /**
     * Increases the current level and updates
     */
    void levelUp();
    
    /**
     * Decreases the current level and updates
     */
    void levelDown();
    
    /**
     * Adds an additonaal percent increase to stat
     *
     * @param boostPercentage amount of percent increase on scale of 0-1
     */
    void boostStat(float boostPercentage);
    
    /**
     * Adds an additonaal percent increase to stat
     *
     * @param boostPercentage amount of percent decrease on scale of 0-1
     */
    void debuffStat(float boostPercentage);
    
    /**
     * Returns current level of stat
     */
    int getCurrentLevel(){return _currentLevel;}
    
    /**
     * Returns max level of stat
     */
    int getMaxLevel(){return _maxLevel;}
    
    float getBaseValue(){return _baseValue;}
    float getMaxPercentage(){return _maxPercentage;}
    
    /**
     * Returns current percentage of stat
     */
    float getCurrentPercentage(){return _currPercentage;}
    
    /**
     * Returns current value of stat
     */
    float getCurrentValue(){return _currValue;}
    
};

#endif /* Upgradeable_hpp */
