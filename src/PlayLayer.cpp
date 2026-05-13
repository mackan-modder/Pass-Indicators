#include "PlayLayer.hpp"
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/DefaultInclude.hpp>


using namespace geode::prelude;

JMPlayLayer::Fields::~Fields() {
    if (m_fileId.empty()) return;

    auto levelsDir = Mod::get()->getSaveDir() / "levels";
    
    (void)utils::file::createDirectory(levelsDir);

    auto filePath = levelsDir / (m_fileId + ".json");
    
    auto res = utils::file::writeToJson(filePath, m_levelData);
    if (!res) {
        log::error("Failed to save level data for ID {}: {}", m_fileId, res.unwrapErr());
    }
}



bool JMPlayLayer::init(
    GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    int id = (level->m_levelType == GJLevelType::Saved || level->m_levelID > 0) 
        ? level->m_levelID.value() 
        : EditorIDs::getID(level);
    
    m_fields->m_fileId 
    = std::to_string(id);

    auto filePath 
    = Mod::get()->getSaveDir() / "levels" / (m_fields->m_fileId + ".json");
    
    if (std::filesystem::exists(filePath)) {
        auto content = file::readString(filePath);
        if (content) {
            auto parsed = matjson::parse(content.unwrap());
            if (parsed) {
                auto result = parsed.unwrap().as<LevelModData>();
                if (result.isOk()) {
                    m_fields->m_levelData = result.unwrap();
                }
            }
        }
    }

    m_fields->m_drawNode = CCDrawNode::create();
    if (m_objectLayer) {
        m_objectLayer->addChild(m_fields->m_drawNode, 999);
        m_fields->m_drawNode->setBlendFunc(
            { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
    }

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
        if (m_player1->m_isDead || m_fields->m_showVisualizer) {
            // Only redraw if a change was flagged or the player is moving
            if (m_fields->m_shouldRedraw || !m_player1->m_isDead) {
                this->drawVisuals();
                m_fields->m_shouldRedraw = false; 
            }
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
    = static_cast<int>(std::round(pos.x / m_fields->m_gridCellSizeX));

    int gy 
    = static_cast<int>(std::round(pos.y / m_fields->m_gridCellSizeY));

    GridPos g = { gx, gy, isRelease };
    
    
    auto& permInputs 
    = m_fields->m_levelData.permanentInputs;

    auto insertRes = permInputs.insert(g);
    auto it = insertRes.first;

    const int maxGapCells = 150; 
    // 150 is half a block in-game. Maybe I'll change it in the future.
    

    /*
        Below is my logic for connecting lines. 
        If there are points to the right or left 
        of the point we added(with the exact same y-position) 
        we connect them. This should save major time when collecting data.
    */
    
    // Checking to the left
    if (it != permInputs.begin()) {
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

    it = permInputs.find(g);
    
    // Checking to the right
    auto itNext = std::next(it);
    if (itNext != permInputs.end()) {
        if (itNext->y == gy && itNext->isRelease == isRelease) {
            int dist = itNext->x - gx;
            if (dist > 1 && dist <= maxGapCells) {
                for (int i = gx + 1; i < itNext->x; ++i) {
                    permInputs.insert({i, gy, isRelease});
                }
            }
        }
    }
}

void JMPlayLayer::drawVisuals() {
    if (!m_fields->m_drawNode) return;
    m_fields->m_drawNode->clear();

    if (m_fields->m_hoverLabel) m_fields->m_hoverLabel->setVisible(false);
    
    static auto mod = Mod::get();

    // Fetching settings

    auto cPermPress = ccc4FFromccc4B(
        mod->getSettingValue<ccColor4B>("colorPermPress"));

    auto cPermRelease = ccc4FFromccc4B(
        mod->getSettingValue<ccColor4B>("colorPermRelease"));

    auto cMarkPress = ccc4FFromccc4B(
        mod->getSettingValue<ccColor4B>("colorMarkerPress"));

    auto cMarkRelease = ccc4FFromccc4B(
        mod->getSettingValue<ccColor4B>("colorMarkerRelease"));

        
    auto indicatorOpacity 
    = mod->getSettingValue<float>("slider-opacity-Perm");
    auto markerOpacity 
    = mod->getSettingValue<float>("slider-opacity-Mark");

    cPermPress.a = indicatorOpacity;
    cPermRelease.a = indicatorOpacity;
    
    cMarkPress.a = markerOpacity;
    cMarkRelease.a = markerOpacity;

    bool showIndicatorGrid 
    = mod->getSettingValue<bool>("death-effect-toggle");
    bool showMarkers 
    = mod->getSettingValue<bool>("ClickMarker-toggle");
    bool showPredict 
    = mod->getSettingValue<bool>("ClickMarkerPredict-toggle");
    
    auto indicatorSize 
    = mod->getSettingValue<float>("slider-size-Perm");
    auto markerSize 
    = mod->getSettingValue<float>("slider-size-Mark");

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
            = static_cast<float>(start->x) * m_fields->m_gridCellSizeX;

            float x2 
            = static_cast<float>(current->x + 1) * m_fields->m_gridCellSizeX;

            float y 
            = static_cast<float>(start->y) * m_fields->m_gridCellSizeY;

            float h 
            = m_fields->m_gridCellSizeY / 2.0f;

            if (x2 < pX - 400.0f || x1 > pX + 400.0f) { 
                it = next; continue; 
            }

            if (start->x == current->x) {
                m_fields->m_drawNode->drawDot(
                CCPoint(x1, y),
                m_fields->m_gridCellSizeY * 2.25f*indicatorSize, 
                ccc4f(0, 0, 0, indicatorOpacity)); 
            } else {
                float p = m_fields->m_gridCellSizeY*1.2f;
                CCPoint borderVerts[4] = {
                    {x1 - p, y - h - p}, {x1 - p, y + h + p},
                    {x2 + p, y + h + p}, {x2 + p, y - h - p}
                };
                m_fields->m_drawNode->drawPolygon(
                borderVerts, 4,
                ccc4f(0, 0, 0, indicatorOpacity), 
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
            = static_cast<float>(start->x) * m_fields->m_gridCellSizeX;

            float x2 
            = static_cast<float>(current->x + 1) * m_fields->m_gridCellSizeX;

            float y 
            = static_cast<float>(start->y) * m_fields->m_gridCellSizeY;

            float h = m_fields->m_gridCellSizeY / 2.0f;

            if (x2 < pX - 500.0f || x1 > pX + 500.0f) {
                it = next; continue; 
            }

            // Paused Hover Logic
            bool isHovered = false;
            if (m_isPaused 
                && (m_player1->m_isDead || m_fields->m_showVisualizer)) {
                float hitBuffer = 5.0f; 
                auto mPos = m_fields->m_hoverWorldPos;
                
                if (start->x == current->x) {
                    float radius 
                    = m_fields->m_gridCellSizeY * indicatorSize;

                    if (mPos.x >= x1 - radius - hitBuffer 
                        && mPos.x <= x1 + radius + hitBuffer &&
                        mPos.y >= y - radius - hitBuffer 
                        && mPos.y <= y + radius + hitBuffer) {
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
                , m_fields->m_gridCellSizeY *indicatorSize
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
                    ? (m_fields->m_gridCellSizeX / velocity) * 1000.0f : 0.0f;

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
                1.8f*markerSize, 
                ccc4f(0.0f, 0.0f, 0.0f, markerOpacity));

            ccColor4F color = input.isRelease ? cMarkRelease : cMarkPress;
            m_fields->m_drawNode->drawDot(input.pos, 
                0.9f*markerSize, color);
        }
    }

    // Post Death Click Markers
    if (showPredict){
        for (auto const& input : m_fields->m_postDeathInputs) {
            m_fields->m_drawNode->drawDot(input.pos, 
                1.8f*markerSize, ccc4f(0.0f, 0.0f, 0.0f, markerOpacity));

            ccColor4F color = input.isRelease 
            ? ccc4f(0.8f, 0.0f, 0.4f, markerOpacity) 
            : ccc4f(1.0f, 0.0f, 0.0f, markerOpacity); 

            m_fields->m_drawNode->drawDot(input.pos, 
                0.9f*markerSize, color);
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