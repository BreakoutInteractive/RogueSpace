//
//  Upgradeable.cpp
//  RS
//
//  Created by Dasha Griffiths on 3/26/24.
//

#include "Upgradeable.hpp"


Upgradeable::Upgradeable(int maxLevel, float maxValue, float baseValue){
    _maxLevel = maxLevel;
    _currLevel = 0;
    _maxValue = maxValue;
    _initialValue = baseValue;
    _currValue = baseValue;
    _stepAmt = (_maxValue-_currValue)/_maxLevel;
}

void Upgradeable::levelUp(){
    if (_currLevel<_maxLevel){
        _currLevel+=1;
        _currValue += _stepAmt;
    }
}

void Upgradeable::levelDown(){
    if (_currLevel>0) {
        _currLevel-=1;
        _currValue -= _stepAmt;
    }
}

void Upgradeable::resetUpgrade(){
    _currValue = _initialValue;
    _currLevel = 1;
}


void Upgradeable::boostStat(float boostValue){
    _currValue += boostValue;
    
}

void Upgradeable::debuffStat(float boostValue){
    _currValue -= boostValue;
}
