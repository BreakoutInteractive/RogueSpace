//
//  Upgradeable.cpp
//  RS
//
//  Created by Dasha Griffiths on 3/26/24.
//

#include "Upgradeable.hpp"

void Upgradeable::levelUp(){
    if (_currentLevel<_maxLevel){
        _currentLevel+=1;
        float ratio = (float)_currentLevel/_maxLevel;
        _currPercentage = ratio*_maxPercentage;
        _currValue = (float)_baseValue+_currPercentage;
    }
}

/**
 * Decreases the current level and updates
 */
void Upgradeable::levelDown(){
    if (_currentLevel>0) {
        _currentLevel-=1;
        float ratio = (float)_currentLevel/_maxLevel;
        _currPercentage = ratio*_maxPercentage;
        _currValue = _baseValue+_currPercentage;
    }
}

void Upgradeable::boostStat(float boostPercentage){
    _currPercentage = ((_currentLevel/_maxLevel)*_maxPercentage)+ boostPercentage;
    _currValue = _baseValue+_currPercentage;
    
}

void Upgradeable::debuffStat(float boostPercentage){
    _currPercentage = ((_currentLevel/_maxLevel)*_maxPercentage) - boostPercentage;
    _currValue = _baseValue+_currPercentage;
}
