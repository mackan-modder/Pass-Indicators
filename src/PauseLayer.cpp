#include "PauseLayer.hpp"
#include "PlayLayer.hpp"
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

void JMPauseLayer::customSetup() {
    PauseLayer::customSetup();
    
    auto pl = static_cast<JMPlayLayer*>(PlayLayer::get());
    if (pl) {
        
        // Draw visuals immediately if player is dead or visualizer is toggled
        if (pl->m_player1->m_isDead || pl->m_fields->m_showVisualizer) {
            pl->drawVisuals();
        }
    }

    // Schedule the mouse-tracking update loop
    this->schedule(schedule_selector(JMPauseLayer::onUpdate), 0.f);
}

void JMPauseLayer::onUpdate(float dt) {
    auto pl = static_cast<JMPlayLayer*>(PlayLayer::get());
    if (!pl || !pl->m_objectLayer) return;

    // Convert raw screen mouse pos to level world space
    auto rawMousePos = getMousePos();
    auto worldPos = pl->m_objectLayer->convertToNodeSpace(rawMousePos);
    

    // Only force a redraw if the mouse actually moved
    if (pl->m_fields->m_hoverWorldPos != worldPos 
        && (pl->m_player1->m_isDead || pl->m_fields->m_showVisualizer)) {
        pl->m_fields->m_shouldRedraw = true;
        pl->m_fields->m_hoverWorldPos = worldPos;
        pl->drawVisuals(); 
    }
}

void JMPauseLayer::onResume(cocos2d::CCObject* sender) {
    auto pl = static_cast<JMPlayLayer*>(PlayLayer::get());
    if (pl) {
        pl->m_fields->m_hoverWorldPos = CCPoint(-9999, -9999);
        
        // Clear visuals if resuming the level
        if (!pl->m_player1->m_isDead && !pl->m_fields->m_showVisualizer 
            && pl->m_fields->m_drawNode) {
            pl->m_fields->m_drawNode->clear();
        } else {
            pl->drawVisuals();
        }
    }
    PauseLayer::onResume(sender);
}