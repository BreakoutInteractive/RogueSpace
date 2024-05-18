//
//  SaveData.cpp
//  RS
//
//  Created by Zhiyuan Chen on 5/10/24.
//

#include "SaveData.hpp"
#include <cugl/cugl.h>

#define SAVEFILE    "save.json"
#define PREFS       "prefs.json"
#define VERSION     "v1.0"

using namespace cugl;

SaveData::Data SaveData::dataCache;
std::shared_ptr<JsonValue> SaveData::saveJson = nullptr;
SaveData::Preferences SaveData::prefsCache;
std::shared_ptr<JsonValue> SaveData::prefsJson = nullptr;

bool SaveData::hasGameSave(){
    auto path = Application::get()->getSaveDirectory() + SAVEFILE;
    auto reader = JsonReader::alloc(path);
    if (reader == nullptr){
        // no such file
        saveJson = JsonValue::allocObject();
        return false;
    }
    std::shared_ptr<JsonValue> json = reader->readJson();
    saveJson = json;
    float hp = json->getFloat("hp", 0);
    if (hp == 0){
        // player died, so start fresh
        reader->close();
        return false;
    }
    dataCache.hp = hp;
    dataCache.level = json->getInt("level", 1);
    dataCache.weapon = json->getInt("weapon");
    dataCache.hpLvl = constraint(json->getInt("hpLvl"), 0, 5);
    dataCache.atkLvl = constraint(json->getInt("atkLvl"), 0, 5);
    dataCache.atkSpLvl = constraint(json->getInt("atkSpLvl"), 0, 5);
    dataCache.rangedLvl = constraint(json->getInt("rangedLvl"), 0, 5);
    dataCache.defLvl = constraint(json->getInt("defLvl"), 0, 5);
    dataCache.parryLvl = constraint(json->getInt("parryLvl"), 0, 5);
    dataCache.dashLvl = constraint(json->getInt("dashLvl"), 0, 5);
    dataCache.isUpgradeRoom = json->getBool("up", false);
    dataCache.upgradeAvailable = json->getBool("available", false);
    dataCache.upgradeOpt1 = json->getInt("opt1", 0);
    dataCache.upgradeOpt2 = json->getInt("opt2", 0);
    dataCache.upgradeOpt1Level = constraint(json->getInt("opt1level"), 0, 5);
    dataCache.upgradeOpt2Level = constraint(json->getInt("opt2level"), 0, 5);
    reader->close();
    return true;
}

SaveData::Data SaveData::getGameSave(){
    // make sure to update the cache on new saves + when checking whether there is a save.
    return SaveData::dataCache;
}

void SaveData::makeSave(Data data){
    auto json = JsonValue::allocObject();
    json->appendChild("level", JsonValue::alloc((long) data.level));
    json->appendChild("weapon", JsonValue::alloc((long)data.weapon));
    json->appendChild("hp", JsonValue::alloc(data.hp));
    json->appendChild("hpLvl", JsonValue::alloc((long)data.hpLvl));
    json->appendChild("atkLvl", JsonValue::alloc((long)data.atkLvl));
    json->appendChild("atkSpLvl", JsonValue::alloc((long)data.atkSpLvl));
    json->appendChild("rangedLvl", JsonValue::alloc((long)data.rangedLvl));
    json->appendChild("defLvl", JsonValue::alloc((long)data.defLvl));
    json->appendChild("parryLvl", JsonValue::alloc((long)data.parryLvl));
    json->appendChild("dashLvl", JsonValue::alloc((long)data.dashLvl));
    // upgrade room info
    json->appendChild("up", JsonValue::alloc((data.isUpgradeRoom)));
    json->appendChild("available", JsonValue::alloc(data.upgradeAvailable));
    json->appendChild("opt1", JsonValue::alloc((long)data.upgradeOpt1));
    json->appendChild("opt1level", JsonValue::alloc((long)data.upgradeOpt1Level));
    json->appendChild("opt2", JsonValue::alloc((long)data.upgradeOpt2));
    json->appendChild("opt2level", JsonValue::alloc((long)data.upgradeOpt2Level));
    
    auto path = Application::get()->getSaveDirectory() + SAVEFILE;
    //CULog("savefile %s", std::string(path).c_str());
    auto writer = JsonWriter::alloc(path);
    writer->writeJson(json);
    writer->flush();
    writer->close();
    saveJson = json;
    dataCache = data;
}

void SaveData::removeSave(){
    auto json = JsonValue::allocObject();
    auto path = Application::get()->getSaveDirectory() + SAVEFILE;
    auto writer = JsonWriter::alloc(path);
    writer->writeJson(json);
    writer->flush();
    writer->close();
    dataCache = SaveData::Data();
}

bool SaveData::hasPreferences() {
    auto path = Application::get()->getSaveDirectory() + PREFS;
    auto reader = JsonReader::alloc(path);
    if (reader == nullptr) {
        // no such file
        prefsJson = JsonValue::allocObject();
        return false;
    }
    std::shared_ptr<JsonValue> json = reader->readJson();
    prefsJson = json;
    
    prefsCache.vol = json->getInt("volume", 5);
    prefsCache.BGMvol = json->getInt("music", 10);
    prefsCache.SFXvol = json->getInt("sfx", 10);
    prefsCache.inverted = json->getBool("inverted", true);
    reader->close();
    return true;
}

SaveData::Preferences SaveData::getPreferences() {
    return prefsCache;
}

void SaveData::savePreferences(Preferences prefs) {
    auto json = JsonValue::allocObject();
    json->appendChild("volume", JsonValue::alloc((long)prefs.vol));
    json->appendChild("music", JsonValue::alloc((long)prefs.BGMvol));
    json->appendChild("sfx", JsonValue::alloc((long)prefs.SFXvol));
    json->appendChild("inverted", JsonValue::alloc(prefs.inverted));
    auto path = Application::get()->getSaveDirectory() + PREFS;
    //CULog("prefsfile %s", std::string(path).c_str());
    auto writer = JsonWriter::alloc(path);
    writer->writeJson(json);
    writer->flush();
    writer->close();
    prefsJson = json;
    prefsCache = prefs;
}
