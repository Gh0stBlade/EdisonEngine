#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_10 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_10(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::Hang)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& collisionInfo) override
            {
                setCameraRotation(-60_deg, 0_deg);
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left || getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
                    setTargetState(LaraStateId::ShimmyLeft);
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right || getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
                    setTargetState(LaraStateId::ShimmyRight);

                return {};
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                auto nextHandler = commonEdgeHangHandling(collisionInfo);

                if( getTargetState() != LaraStateId::Hang )
                    return nextHandler;

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                    return nextHandler;

                const auto frontHeight = collisionInfo.front.floor.distance;
                const auto frontSpace = frontHeight - collisionInfo.front.ceiling.distance;
                const auto frontLeftSpace = collisionInfo.frontLeft.floor.distance - collisionInfo.frontLeft.ceiling.distance;
                const auto frontRightSpace = collisionInfo.frontRight.floor.distance - collisionInfo.frontRight.ceiling.distance;
                if( frontHeight <= -850 || frontHeight >= -650 || frontSpace < 0 || frontLeftSpace < 0 || frontRightSpace < 0 || collisionInfo.hasStaticMeshCollision )
                {
                    return nextHandler;
                }

                if( getLevel().m_inputHandler->getInputState().moveSlow )
                    setTargetState(LaraStateId::Handstand);
                else
                    setTargetState(LaraStateId::Climbing);

                return nextHandler;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }
        };
    }
}
