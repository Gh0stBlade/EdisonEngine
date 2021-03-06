#pragma once

#include "statehandler_underwater.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_17 final : public StateHandler_Underwater
        {
        public:
            explicit StateHandler_17(LaraNode& lara)
                : StateHandler_Underwater(lara, LaraStateId::UnderwaterForward)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() < 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                    return;
                }

                handleDiveRotationInput();

                if( !getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::UnderwaterInertia);

                setFallSpeed(std::min(getFallSpeed() + 8, 200));
            }
        };
    }
}
