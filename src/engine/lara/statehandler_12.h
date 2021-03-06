#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_12 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_12(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::Unknown12)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.facingAngle = getMovementAngle();
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                applyShift(collisionInfo);
            }
        };
    }
}
