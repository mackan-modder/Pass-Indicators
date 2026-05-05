#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(JMPauseLayer, PauseLayer) {
    void customSetup();
    void onResume(cocos2d::CCObject* sender);
    void onUpdate(float dt);
};