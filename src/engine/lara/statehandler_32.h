#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_32 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_32(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::SlideBackward)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::JumpBack);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y + 180_deg);
                commonSlideHandling(collisionInfo);
            }
        };
    }
}
