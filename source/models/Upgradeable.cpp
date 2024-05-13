//
//  Upgradeable.cpp
//  RS
//
//  Created by Dasha Griffiths on 3/26/24.
//

#include "Upgradeable.hpp"

Upgradeable::Upgradeable(float initialVal, float maxValue, UpgradeType type){
    _currLevel = 0;
    _type = type;
    _tierValues.push_back(initialVal);
    float stepAmt = (maxValue-initialVal)/5;
    for (int idx = 1; idx < 6; idx++) {
        _tierValues.push_back(_tierValues.at(idx-1)+stepAmt);
    }
}
Upgradeable::Upgradeable(std::vector<float> tiers, UpgradeType type){
    _currLevel = 0;
    _type = type;
    _tierValues = tiers;
}

void Upgradeable::levelUp(){
    if (_currLevel<_tierValues.size()-1){
        _currLevel+=1;
    }
}

void Upgradeable::levelDown(){
    if (_currLevel>0) {
        _currLevel-=1;
    }
}

void Upgradeable::resetUpgrade(){
    _currLevel = 0;
}
