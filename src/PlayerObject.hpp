#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(JMPlayerObject, PlayerObject) {
    // Hooks
    void playerDestroyed(bool p0);
    bool pushButton(PlayerButton btn);
    bool releaseButton(PlayerButton btn);

    // Custom helper
    void recordInput(bool isRelease);
};