#include "PlayLayer.hpp"


using namespace geode::prelude;


$on_game(Loaded) {
    listenForKeybindSettingPresses("visualizer-toggle", 
        [](Keybind const& keybind, bool down, bool repeat, double timestamp){

        if (down && !repeat) {
            auto pl = static_cast<JMPlayLayer*>(PlayLayer::get());
            if (pl) {
                pl->m_fields->m_showVisualizer = !pl->m_fields->m_showVisualizer;
                
                if (pl->m_isPaused && !pl->m_fields->m_showVisualizer 
                    && pl->m_fields->m_drawNode) {
                    pl->m_fields->m_drawNode->clear();
                } 
            }
        }
    });
}