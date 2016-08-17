#include "level/level.h"
#include "engine/laracontroller.h"

#include <boost/range/adaptors.hpp>

#if 0
//! @fixme Re-enable me!
namespace
{
    void drawText(irr::gui::IGUIEnvironment* env, int x, int y, const irr::core::stringw& txt, const irr::video::SColor& col = irr::video::SColor(255, 255, 255, 255))
    {
        auto fnt = env->getBuiltInFont();
        irr::core::recti r;
        r.UpperLeftCorner.X = x;
        r.UpperLeftCorner.Y = y;
        r.LowerRightCorner = r.UpperLeftCorner;
        const auto dim = fnt->getDimension(txt.c_str());
        r.LowerRightCorner.X += dim.Width;
        r.LowerRightCorner.Y += dim.Height;
        fnt->draw(txt, r, col);
    }

    void drawDebugInfo(gsl::not_null<irr::IrrlichtDevice*> device, gsl::not_null<level::Level*> lvl)
    {
        // position/rotation
        drawText(device->getGUIEnvironment(), 10, 40, lvl->m_lara->getCurrentRoom()->node->getName());
        drawText(device->getGUIEnvironment(), 100, 40, irr::core::stringw(std::lround(lvl->m_lara->getRotation().Y.toDegrees())));
        drawText(device->getGUIEnvironment(), 140, 20, irr::core::stringw(std::lround(lvl->m_lara->getPosition().X)));
        drawText(device->getGUIEnvironment(), 140, 30, irr::core::stringw(std::lround(lvl->m_lara->getPosition().Y)));
        drawText(device->getGUIEnvironment(), 140, 40, irr::core::stringw(std::lround(lvl->m_lara->getPosition().Z)));

        // physics
        drawText(device->getGUIEnvironment(), 180, 40, irr::core::stringw(std::lround(lvl->m_lara->getFallSpeed().getCurrentValue())));

        // animation
        drawText(device->getGUIEnvironment(), 10, 60, loader::toString(lvl->m_lara->getCurrentAnimState()));
        drawText(device->getGUIEnvironment(), 100, 60, loader::toString(lvl->m_lara->getTargetState()));
        drawText(device->getGUIEnvironment(), 10, 80, irr::core::stringw(lvl->m_lara->getCurrentFrame()));
        drawText(device->getGUIEnvironment(), 100, 80, toString(static_cast<loader::AnimationId>(lvl->m_lara->getCurrentAnimationId())));

        // triggers
        {
            int y = 100;
            for(const std::unique_ptr<engine::ItemController>& item : lvl->m_itemControllers | boost::adaptors::map_values)
            {
                if(!item->m_isActive)
                    continue;

                drawText(device->getGUIEnvironment(), 10, y, item->getName().c_str());
                if(item->m_flags2_02_toggledOn)
                    drawText(device->getGUIEnvironment(), 180, y, "toggled");
                if(item->m_flags2_04_ready)
                    drawText(device->getGUIEnvironment(), 220, y, "ready");
                drawText(device->getGUIEnvironment(), 260, y, irr::core::stringw(item->m_triggerTimeout));
                y += 20;
            }
        }

        // collision
        drawText(device->getGUIEnvironment(), 200, 20,  irr::core::stringw("AxisColl: ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.axisCollisions));
        drawText(device->getGUIEnvironment(), 200, 40,  irr::core::stringw("Current floor:   ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.current.floor.distance));
        drawText(device->getGUIEnvironment(), 200, 60,  irr::core::stringw("Current ceiling: ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.current.ceiling.distance));
        drawText(device->getGUIEnvironment(), 200, 80,  irr::core::stringw("Front floor:     ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.front.floor.distance));
        drawText(device->getGUIEnvironment(), 200, 100, irr::core::stringw("Front ceiling:   ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.front.ceiling.distance));
        drawText(device->getGUIEnvironment(), 200, 120, irr::core::stringw("Front/L floor:   ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.frontLeft.floor.distance));
        drawText(device->getGUIEnvironment(), 200, 140, irr::core::stringw("Front/L ceiling: ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.frontLeft.ceiling.distance));
        drawText(device->getGUIEnvironment(), 200, 160, irr::core::stringw("Front/R floor:   ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.frontRight.floor.distance));
        drawText(device->getGUIEnvironment(), 200, 180, irr::core::stringw("Front/R ceiling: ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.frontRight.ceiling.distance));
        drawText(device->getGUIEnvironment(), 200, 200, irr::core::stringw("Need bottom:     ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.neededFloorDistanceBottom));
        drawText(device->getGUIEnvironment(), 200, 220, irr::core::stringw("Need top:        ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.neededFloorDistanceTop));
        drawText(device->getGUIEnvironment(), 200, 240, irr::core::stringw("Need ceiling:    ") + irr::core::stringw(lvl->m_lara->lastUsedCollisionInfo.neededCeilingDistance));

        device->getGUIEnvironment()->drawAll();
    }
}
#endif

int main(int argc, char* argv[])
{
#ifndef NDEBUG
    osg::setNotifyLevel(osg::NotifySeverity::ALWAYS);
#endif

    const int windowWidth = 1024;
    const int windowHeight = 768;
    const std::string version("4.2");

    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
    traits->x = 20; traits->y = 30;
    traits->width = windowWidth;
    traits->height = windowHeight;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->glContextVersion = version;
    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    if(!gc.valid())
    {
        BOOST_LOG_TRIVIAL(fatal) << "Unable to create OpenGL v" << version << " context.";
        return EXIT_FAILURE;
    }

    struct LevelInfo
    {
        std::string filename;
        std::string title;
        int track;
        int secrets;
    };

    LevelInfo levels[] = {
        {"GYM", "Lara's Home", 0, 0},
        {"LEVEL1", "Caves", 57, 3}, // 1
        {"LEVEL2", "City of Vilcabamba", 57, 3},
        {"LEVEL3A", "Lost Valley", 57, 5},
        {"LEVEL3B", "Tomb of Qualopec", 57, 3},
        {"LEVEL4", "St. Francis' Folly", 59, 4},
        {"LEVEL5", "Colosseum", 59, 3}, // 6
        {"LEVEL6", "Palace Midas", 59, 3},
        {"LEVEL7A", "The Cistern", 58, 3},
        {"LEVEL7B", "Tomb of Tihocan", 58, 2},
        {"LEVEL8A", "City of Khamoon", 59, 3},
        {"LEVEL8B", "Obelisk of Khamoon", 59, 3}, // 11
        {"LEVEL8C", "Sanctuary of the Scion", 59, 1},
        {"LEVEL10A", "Natla's Mines", 58, 3},
        {"LEVEL10B", "Atlantis", 60, 3},
        {"LEVEL10C", "The Great Pyramid", 60, 3} // 15
    };

    const LevelInfo& lvlInfo = levels[0];

    auto lvl = level::Level::createLoader("data/tr1/data/" + lvlInfo.filename + ".PHD", level::Game::Unknown);

    BOOST_ASSERT(lvl != nullptr);
    lvl->load();
    auto viewer = lvl->toIrrlicht();

    viewer->getCamera()->setGraphicsContext(gc);
    gc->getState()->setUseModelViewAndProjectionUniforms(true);
    gc->getState()->setUseVertexAttributeAliasing(true);

    const auto aspect = windowWidth * 1.0f / windowHeight;
    viewer->getCamera()->setProjectionMatrixAsPerspective(osg::DegreesToRadians(80 / aspect), aspect, 10, 20480);
    viewer->getCamera()->setViewport(0, 0, windowWidth, windowHeight);

    viewer->realize();

    if(lvlInfo.track > 0)
        lvl->playCdTrack(lvlInfo.track);

    while(!viewer->done())
    {
        lvl->m_audioDev.update();

        lvl->m_inputHandler->update();

        for(const std::unique_ptr<engine::ItemController>& ctrl : lvl->m_itemControllers | boost::adaptors::map_values)
        {
            if(ctrl.get() == lvl->m_lara) // Lara is special and needs to be updated last
                continue;

            ctrl->update(1);
        }

        lvl->m_lara->update(1);
        lvl->m_cameraController->update(1);

        lvl->m_audioDev.setListenerTransform(lvl->m_cameraController->getPosition(),
                                             lvl->m_cameraController->getFrontVector(),
                                             lvl->m_cameraController->getUpVector());

        viewer->frame();

        lvl->drawBars();

#if 0
        drawDebugInfo(device, lvl.get());

        // update information about current frame-rate
        irr::core::stringw str(L"FPS: ");
        str.append(irr::core::stringw(driver->getFPS()));
        str += L" Tris: ";
        str.append(irr::core::stringw(driver->getPrimitiveCountDrawn()));
        device->setWindowCaption(str.c_str());
#endif
    }

    return EXIT_SUCCESS;
}
