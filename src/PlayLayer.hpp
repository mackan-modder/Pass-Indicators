// PlayLayer.hpp
#pragma once
#include "Common.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(JMPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<InputPoint> m_currentRunInputs;
        std::vector<InputPoint> m_postDeathInputs;
        LevelModData m_levelData;
        CCDrawNode* m_drawNode = nullptr;
        std::string m_fileId;
        CCLabelBMFont* m_hoverLabel = nullptr;
        CCPoint m_hoverWorldPos = CCPoint(-9999, -9999);
        bool m_showVisualizer = false;
        float m_runTimer = 0.0f;
        float m_deathTime = 0.0f;
        float m_gridCellSize = 0.1f;     
        double m_lastX = 0.0;
        double m_lastY = 0.0;             
        double m_currentVelocityX = 0.0; 
        double m_currentVelocityY = 0.0;

        ~Fields();
    };

    // Function prototypes (Declarations)
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects);
    void postUpdate(float dt);
    void resetLevel();
    void levelComplete();
    void drawVisuals();
    void addToPermanent(CCPoint pos, bool isRelease);
};