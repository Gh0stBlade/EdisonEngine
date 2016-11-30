#pragma once

#include "statehandler_standing.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_4 final : public StateHandler_Standing
        {
        public:
            explicit StateHandler_4(LaraNode& lara)
                    : StateHandler_Standing(lara)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::Pose;
            }
        };
    }
}