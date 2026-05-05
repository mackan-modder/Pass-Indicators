#include "PlayLayer.hpp"
#include <Geode/modify/CCTouchDispatcher.hpp>

using namespace geode::prelude;

/*
    Here I made my feature for showing clicks after death
    while predicting where they would have been. CCTouchDispatcher isn't
    ideal since it only works for mouse inputs and it is used for
    every touch on the screen but I didn't find another
    place to hook that would work after death. If you know a better
    way to do this, please let me know!

    I suspect that I will change this to use a normal keybind listener 
    in a future update. I just can't get a way to get to it work with a 
    vanilla keybind. I'm gonna ask the discord about this.

    I also skipped including a .hpp file here since it is so small.
*/

class $modify(CCTouchDispatcher) {
        void touches(cocos2d::CCSet* touches, 
            cocos2d::CCEvent* event, unsigned int type) {
        auto pl = static_cast<JMPlayLayer*>(PlayLayer::get());

        if (pl && pl->m_player1->m_isDead && !pl->m_isPaused && pl->m_fields->m_postDeathInputs.empty()) {
            // type 0 = Press, type 2 = Release
            if ((type == 0 || type == 2)) {
                float dtSinceDeath 
                = pl->m_fields->m_runTimer - pl->m_fields->m_deathTime;
                
                // Calculate predicted X and Y
                float predictedX 
                = pl->m_player1->getPositionX() 
                + (dtSinceDeath * pl->m_fields->m_currentVelocityX);

                float predictedY 
                = pl->m_player1->getPositionY() 
                + (dtSinceDeath * pl->m_fields->m_currentVelocityY);
                
                bool isRelease = (type == 2);
                
                pl->m_fields->m_postDeathInputs.push_back({
                    CCPoint(predictedX, predictedY), 
                    pl->m_fields->m_runTimer,
                    isRelease
                });
            }
            
        }
        CCTouchDispatcher::touches(touches, event, type);
    }
};