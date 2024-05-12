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
    dataCache.hpLvl = json->getInt("hpLvl");
    dataCache.atkLvl = json->getInt("atkLvl");
    dataCache.rangedLvl = json->getInt("rangedLvl");
    dataCache.defLvl = json->getInt("defLvl");
    dataCache.parryLvl = json->getInt("parryLvl");
    dataCache.dashLvl = json->getInt("dashLvl");
    //TODO: maybe do post-processing to constraint within [0,5] for each level
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
    json->appendChild("hpLvl", JsonValue::alloc((long)data.weapon));
    json->appendChild("atkLvl", JsonValue::alloc((long)data.weapon));
    json->appendChild("rangedLvl", JsonValue::alloc((long)data.weapon));
    json->appendChild("defLvl", JsonValue::alloc((long)data.weapon));
    json->appendChild("parryLvl", JsonValue::alloc((long)data.weapon));
    json->appendChild("dashLvl", JsonValue::alloc((long)data.weapon));
    auto path = Application::get()->getSaveDirectory() + SAVEFILE;
    CULog("savefile %s", std::string(path).c_str());
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
    CULog("prefsfile %s", std::string(path).c_str());
    auto writer = JsonWriter::alloc(path);
    writer->writeJson(json);
    writer->flush();
    writer->close();
    prefsJson = json;
    prefsCache = prefs;
}