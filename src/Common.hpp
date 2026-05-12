#pragma once
#include <Geode/Geode.hpp>
#include <matjson.hpp>
#include <set>

using namespace geode::prelude;

struct GridPos {
    int x;
    int y;
    bool isRelease;
    bool operator<(const GridPos& other) const {
        return std::tie(y, isRelease, x) < std::tie(other.y, other.isRelease, other.x);
    }
};

struct InputPoint {
    CCPoint pos;
    float timestamp;
    bool isRelease;
};

struct LevelModData {
    std::set<GridPos> permanentInputs; 
    bool addRunInputsToPermanent = true; //This leveldata isn't used yet.
};

template <>
struct matjson::Serialize<GridPos> {
    static matjson::Value toJson(GridPos const& value) {
        return matjson::makeObject({
            { "x", value.x },
            { "y", value.y },
            { "r", value.isRelease }
        });
    }

    static geode::Result<GridPos> fromJson(matjson::Value const& value) {
        GEODE_UNWRAP_INTO(int x, value["x"].asInt());
        GEODE_UNWRAP_INTO(int y, value["y"].asInt());
        GEODE_UNWRAP_INTO(bool r, value["r"].asBool());

        return geode::Ok(GridPos { x, y, r });
    }
};

template <>
struct matjson::Serialize<LevelModData> {
    static matjson::Value toJson(LevelModData const& value) {
        return matjson::makeObject({
            { "inputs", value.permanentInputs },
            { "autoAdd", value.addRunInputsToPermanent }
        });
    }
    static geode::Result<LevelModData> fromJson(matjson::Value const& value) {
        LevelModData data;
        if (value.contains("inputs")) 
            data.permanentInputs
            = value["inputs"].as<std::set<GridPos>>().unwrapOrDefault();
        if (value.contains("autoAdd")) 
            data.addRunInputsToPermanent 
            = value["autoAdd"].asBool().unwrapOrDefault();
        return geode::Ok(data);
    }
};