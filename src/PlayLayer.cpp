#include "PlayLayer.hpp"
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

JMPlayLayer::Fields::~Fields() {
    if (m_fileId.empty()) return;

    auto allData = Mod::get()->getSavedValue<std::map<std::string, LevelModData>>("level-data-map");
    
    allData[m_fileId] = m_levelData;
    
    Mod::get()->setSavedValue("level-data-map", allData);
}

// HOOKS 
bool JMPlayLayer::init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    // Get the ID
    int id = (level->m_levelType == GJLevelType::Saved || level->m_levelID > 0) 
        ? level->m_levelID.value() 
        : EditorIDs::getID(level);
    
    m_fields->m_fileId = std::to_string(id);

    auto allData = Mod::get()->getSavedValue<std::map<std::string, LevelModData>>("level-data-map");
    
    if (allData.contains(m_fields->m_fileId)) {
        m_fields->m_levelData = allData[m_fields->m_fileId];
    }

    m_fields->m_drawNode = CCDrawNode::create();
    // m_fields->m_drawNode->setID("mackan.passIndicators"_spr);
    if (m_objectLayer) m_objectLayer->addChild(m_fields->m_drawNode, 999);

    return true;
}

void JMPlayLayer::postUpdate(float dt) {
    PlayLayer::postUpdate(dt);
    
    if (m_player1 && !m_player1->m_isDead) {

        /*
            This here is my roundabout way of getting the velocity 
            and keeping it when the player is dead and not moving. 
            This is used for my predictive click position feature.
        */

        double currentX = m_player1->getPositionX();
        double currentY = m_player1->getPositionY();
        if (dt > 0) {
            m_fields->m_currentVelocityX 
            = (currentX - m_fields->m_lastX) / dt;

            m_fields->m_currentVelocityY 
            = (currentY - m_fields->m_lastY) / dt;
        }
        m_fields->m_lastX = currentX;
        m_fields->m_lastY = currentY;
    }

    m_fields->m_runTimer += dt;


    if (m_fields->m_drawNode) {
        if ((m_player1->m_isDead) || m_fields->m_showVisualizer) {
            this->drawVisuals();
        } else {
            m_fields->m_drawNode->clear();
        }
    }
}

void JMPlayLayer::resetLevel() {
    PlayLayer::resetLevel();
    m_fields->m_currentRunInputs.clear();
    m_fields->m_postDeathInputs.clear();
    m_fields->m_runTimer = 0.0f;
}

// My custom functions.

void JMPlayLayer::addToPermanent(CCPoint pos, bool isRelease) {
    int gx 
    = static_cast<int>(std::round(pos.x / m_fields->m_gridCellSize));

    int gy 
    = static_cast<int>(std::round(pos.y / m_fields->m_gridCellSize));

    GridPos g = { gx, gy, isRelease };
    
    
    auto& permInputs 
    = m_fields->m_levelData.permanentInputs;

    const int maxGapCells = 150; 
    // 150 is half a block in-game. Maybe I'll change it in the future.
    auto it = permInputs.lower_bound(g);

    /*
        Below is my logic for connecting lines. 
        If there are points to the right or left 
        of the point we added(with the exact same y-position) 
        we connect them. This should save major time when collecting data.
    */
    
    // Checking to the left
    if (it!= permInputs.begin()) {
        auto prev = std::prev(it);
        if (prev->y == gy && prev->isRelease == isRelease) {
            int dist = gx - prev->x;
            if (dist > 1 && dist <= maxGapCells) {
                for (int i = prev->x + 1; i < gx; ++i) {
                    permInputs.insert({i, gy, isRelease});
                }
            }
        }
    }
    
    // Checking to the right
    auto itNext = permInputs.upper_bound(g);
    if (itNext!= permInputs.end()) {
        if (itNext->y == gy && itNext->isRelease == isRelease) {
            int dist = itNext->x - gx;
            if (dist > 1 && dist <= maxGapCells) {
                for (int i = gx + 1; i < itNext->x; ++i) {
                    permInputs.insert({i, gy, isRelease});
                }
            }
        }
    }
    permInputs.insert(g);
}

void JMPlayLayer::drawVisuals() {
    if (!m_fields->m_drawNode) return;
    m_fields->m_drawNode->clear();

    if (m_fields->m_hoverLabel) m_fields->m_hoverLabel->setVisible(false);
    

    
    auto cPermPress = ccc4FFromccc4B(
        Mod::get()->getSettingValue<ccColor4B>("colorPermPress"));

    auto cPermRelease = ccc4FFromccc4B(
        Mod::get()->getSettingValue<ccColor4B>("colorPermRelease"));

    auto cMarkPress = ccc4FFromccc4B(
        Mod::get()->getSettingValue<ccColor4B>("colorMarkerPress"));

    auto cMarkRelease = ccc4FFromccc4B(
        Mod::get()->getSettingValue<ccColor4B>("colorMarkerRelease"));



    // Fetching settings
    bool showIndicatorGrid 
    = Mod::get()->getSettingValue<bool>("death-effect-toggle");

    bool showMarkers 
    = Mod::get()->getSettingValue<bool>("ClickMarker-toggle");
    bool showPredict 
    = Mod::get()->getSettingValue<bool>("ClickMarkerPredict-toggle");
    
    auto indicatorSize 
    = Mod::get()->getSettingValue<float>("slider-size-Perm");
    auto markerSize 
    = Mod::get()->getSettingValue<float>("slider-size-Mark");

    auto& permInputs = m_fields->m_levelData.permanentInputs;

    float pX = m_player1->getPositionX();

    /*
        Here is my massive drawing logic. 
        We do two passes for the indicator grid
        (to add the black background first) 
        and after that we draw the click markers.
    */

    auto it = permInputs.begin();
    if (showIndicatorGrid) {
        // Pass 1: Black Border Layer
        while (it != permInputs.end()) {
            auto start = it;
            auto current = it;
            auto next = std::next(it);

            while (next != permInputs.end() &&
                next->y == start->y &&
                next->isRelease == start->isRelease &&
                next->x == current->x + 1) {
                current = next;
                next = std::next(next);
            }

            float x1 
            = static_cast<float>(start->x) * m_fields->m_gridCellSize;

            float x2 
            = static_cast<float>(current->x + 1) * m_fields->m_gridCellSize;

            float y 
            = static_cast<float>(start->y) * m_fields->m_gridCellSize;

            float h 
            = m_fields->m_gridCellSize / 2.0f*indicatorSize;

            if (x2 < pX - 600.0f || x1 > pX + 600.0f) { 
                it = next; continue; 
            }

            if (start->x == current->x) {
                m_fields->m_drawNode->drawDot(
                CCPoint(x1, y),
                m_fields->m_gridCellSize * 12.0f*indicatorSize, 
                ccc4f(0, 0, 0, 1)); 
            } else {
                float p = 0.5f;
                CCPoint borderVerts[4] = {
                    {x1 - p, y - h - p}, {x1 - p, y + h + p},
                    {x2 + p, y + h + p}, {x2 + p, y - h - p}
                };
                m_fields->m_drawNode->drawPolygon(
                borderVerts, 4,
                ccc4f(0, 0, 0, 1), 
                0, ccc4f(0, 0, 0, 0));
            }
            it = next;
        }

        // Pass 2: Indicator Grid

        it = permInputs.begin();
        while (it != permInputs.end()) {
            auto start = it;
            auto current = it;
            auto next = std::next(it);

            while (next != permInputs.end() &&
                next->y == start->y &&
                next->isRelease == start->isRelease &&
                next->x == current->x + 1) {
                current = next;
                next = std::next(next);
            }

            float x1 
            = static_cast<float>(start->x) * m_fields->m_gridCellSize;

            float x2 
            = static_cast<float>(current->x + 1) * m_fields->m_gridCellSize;

            float y 
            = static_cast<float>(start->y) * m_fields->m_gridCellSize;

            float h = m_fields->m_gridCellSize / 2.0f*indicatorSize;

            if (x2 < pX - 600.0f || x1 > pX + 600.0f) {
                it = next; continue; 
            }

            // Paused Hover Logic
            bool isHovered = false;
            if (m_isPaused 
                && (m_player1->m_isDead || m_fields->m_showVisualizer)) {
                float hitBuffer = 20.0f * h; 
                auto mPos = m_fields->m_hoverWorldPos;
                
                if (start->x == current->x) {
                    if (mPos.x >= x1 - hitBuffer 
                        && mPos.x <= x1 + hitBuffer &&
                        mPos.y >= y - hitBuffer 
                        && mPos.y <= y + hitBuffer) {
                        isHovered = true;
                    }
                } else {
                    if (mPos.x >= x1 - hitBuffer 
                        && mPos.x <= x2 + hitBuffer 
                        && mPos.y >= y - h - hitBuffer 
                        && mPos.y <= y + h + hitBuffer) {
                        isHovered = true;
                    }
                }
            }

            // Changes the color of the line to yellow if hovered.
            ccColor4F baseColor = start->isRelease 
            ? cPermRelease : cPermPress;
            if (isHovered) baseColor 
            = ccc4f(1.0f, 1.0f, 0.0f, 1.0f); 


            // Drawing
            if (start->x == current->x) {
                // This is for dots
                m_fields->m_drawNode->drawDot(CCPoint(x1, y)
                , m_fields->m_gridCellSize * 5.0f*indicatorSize
                , baseColor);
            } else {
                // This is for lines(I wanted exact edges)
                CCPoint innerVerts[4] = {
                    {x1, y - h}, {x1, y + h},
                    {x2, y + h}, {x2, y - h}
                };
                m_fields->m_drawNode->drawPolygon(innerVerts, 4, 
                    baseColor, 0.0f, 
                    ccc4f(0, 0, 0, 0));
    
                // For lines that are hovered we add the info text
                if (isHovered) {
                    float xWidth = x2 - x1;

                    float velocity 
                    = static_cast<float>(m_fields->m_currentVelocityX);

                    float timingMs 
                    = (velocity > 0) ? (xWidth / velocity) * 1000.0f : 0.0f;

                    float timingMsImprecision 
                    = (velocity > 0) 
                    ? (m_fields->m_gridCellSize / velocity) * 1000.0f : 0.0f;

                    float frames240 = timingMs * 0.24f;
                    float frames240Imprecision 
                    = timingMsImprecision * 0.24f;

                    std::string labelStr = fmt::format(
                        "Width: {:.2f}\n{:.2f}+-{:.2f}"
                        " ms\n{:.2f}+-{:.2f} frames", 
                        xWidth, timingMs, 
                        timingMsImprecision, 
                        frames240, frames240Imprecision
                    );

                    if (!m_fields->m_hoverLabel) {
                        m_fields->m_hoverLabel 
                        = CCLabelBMFont::create(labelStr.c_str(), 
                        "bigFont.fnt");

                        m_fields->m_hoverLabel->setScale(0.25f);

                        m_fields->m_hoverLabel->setAlignment(
                            kCCTextAlignmentLeft);
                        m_fields->m_drawNode->addChild(
                            m_fields->m_hoverLabel);
                    }

                    m_fields->m_hoverLabel->setString(
                        labelStr.c_str());

                    m_fields->m_hoverLabel->setVisible(true);

                    m_fields->m_hoverLabel->setPosition(
                        { x1 - 5.0f, y });
                    m_fields->m_hoverLabel->setAnchorPoint(
                    {1.0f, 0.5f});
                }
            }
            it = next;
        }
    }

    // Click Markers
    if (showMarkers){
        for (auto const& input : m_fields->m_currentRunInputs) {
            m_fields->m_drawNode->drawDot(input.pos, 
                0.90f*markerSize, 
                ccc4f(0.0f, 0.0f, 0.0f, 0.5f));

            ccColor4F color = input.isRelease ? cMarkRelease : cMarkPress;
            m_fields->m_drawNode->drawDot(input.pos, 
                0.45f*markerSize, color);
        }
    }

    // Post Death Click Markers
    if (showPredict){
        for (auto const& input : m_fields->m_postDeathInputs) {
            m_fields->m_drawNode->drawDot(input.pos, 
                0.90f*markerSize, ccc4f(0.0f, 0.0f, 0.0f, 0.5f));

            ccColor4F color = input.isRelease 
            ? ccc4f(0.8f, 0.0f, 0.0f, 1.0f) 
            : ccc4f(1.0f, 0.0f, 0.0f, 1.0f); 

            m_fields->m_drawNode->drawDot(input.pos, 
                0.45f*markerSize, color);
        }  
    }
}

void JMPlayLayer::levelComplete() {
    PlayLayer::levelComplete();

    if (m_fields->m_levelData.addRunInputsToPermanent) {
        for (auto const& input : m_fields->m_currentRunInputs) {
            /* 
                Since the player reached the end, 
                every input in this run is a survival
            */
            this->addToPermanent(input.pos, input.isRelease);
        }
    }
}