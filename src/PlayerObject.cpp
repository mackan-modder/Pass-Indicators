#include "PlayerObject.hpp"
#include "PlayLayer.hpp"

using namespace geode::prelude;

void JMPlayerObject::playerDestroyed(bool p0) {
    PlayerObject::playerDestroyed(p0);
    
    auto pl = static_cast<JMPlayLayer*>(PlayLayer::get());
    if (!pl) return;

    pl->m_fields->m_deathTime = pl->m_fields->m_runTimer;
    
    // Check if we should track survival inputs
    if (pl->m_fields->m_levelData.addRunInputsToPermanent 
        && Mod::get()->getSettingValue<bool>("tracking-toggle")) {

        auto const& inputs = pl->m_fields->m_currentRunInputs;
        float threshold = Mod::get()->getSettingValue<double>("slider");
        
        if (inputs.size() > 2) { 
            for (size_t i = 0; i < inputs.size() - 2; ++i) { 
                auto const& input = inputs[i];
                
                // Add to permanent data if they survived the threshold before death
                if (pl->m_fields->m_deathTime - input.timestamp >= threshold) { 
                    pl->addToPermanent(input.pos, input.isRelease);
                }
            }
        }
    }
    pl->m_fields->m_shouldRedraw = true;
    pl->drawVisuals();
}

bool JMPlayerObject::pushButton(PlayerButton btn) {
    if (btn == PlayerButton::Jump) recordInput(false);
    return PlayerObject::pushButton(btn); 
}

bool JMPlayerObject::releaseButton(PlayerButton btn) {
    if (btn == PlayerButton::Jump) recordInput(true);
    return PlayerObject::releaseButton(btn); 
}

void JMPlayerObject::recordInput(bool isRelease) {
    auto pl = PlayLayer::get();
    if (!pl || pl->m_player1->m_isDead || pl->m_isPaused) return;
    
    auto fields = static_cast<JMPlayLayer*>(pl)->m_fields.self();
    fields->m_currentRunInputs.push_back({
        this->getPosition(), 
        fields->m_runTimer, 
        isRelease
    });
}