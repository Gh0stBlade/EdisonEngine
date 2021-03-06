#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class SlopedBridge : public ItemNode
        {
        private:
            const int m_div;
        public:
            SlopedBridge(const gsl::not_null<level::Level*>& level,
                         const std::string& name,
                         const gsl::not_null<const loader::Room*>& room,
                         const core::Angle& angle,
                         const core::TRCoordinates& position,
                         const floordata::ActivationState& activationState,
                         int16_t darkness,
                         const loader::AnimatedModel& animatedModel,
                         int div)
                : ItemNode(level, name, room, angle, position, activationState, false, 0, darkness, animatedModel)
                , m_div(div)
            {
            }


            void onInteract(LaraNode& /*lara*/) override final
            {
            }


            void patchFloor(const core::TRCoordinates& pos, int& y) override final
            {
                auto tmp = getPosition().Y + getBridgeSlopeHeight(pos) / m_div;
                if( pos.Y <= tmp )
                    y = tmp;
            }


            void patchCeiling(const core::TRCoordinates& pos, int& y) override final
            {
                auto tmp = getPosition().Y + getBridgeSlopeHeight(pos) / m_div;
                if( pos.Y <= tmp )
                    return;

                y = tmp + loader::QuarterSectorSize;
            }


            void update() override final
            {
                ItemNode::update();
            }


        private:
            int getBridgeSlopeHeight(const core::TRCoordinates& pos) const
            {
                auto axis = core::axisFromAngle(getRotation().Y, 1_deg);
                Expects( axis.is_initialized() );

                switch( *axis )
                {
                    case core::Axis::PosZ: return loader::SectorSize - 1 - pos.X % loader::SectorSize;
                    case core::Axis::PosX: return pos.Z % loader::SectorSize;
                    case core::Axis::NegZ: return pos.X % loader::SectorSize;
                    case core::Axis::NegX: return loader::SectorSize - 1 - pos.Z % loader::SectorSize;
                    default: return 0;
                }
            }
        };


        class BridgeSlope1 final : public SlopedBridge
        {
        public:
            BridgeSlope1(const gsl::not_null<level::Level*>& level,
                         const std::string& name,
                         const gsl::not_null<const loader::Room*>& room,
                         const core::Angle& angle,
                         const core::TRCoordinates& position,
                         const floordata::ActivationState& activationState,
                         int16_t darkness,
                         const loader::AnimatedModel& animatedModel)
                : SlopedBridge(level, name, room, angle, position, activationState, darkness, animatedModel, 4)
            {
            }
        };


        class BridgeSlope2 final : public SlopedBridge
        {
        public:
            BridgeSlope2(const gsl::not_null<level::Level*>& level,
                         const std::string& name,
                         const gsl::not_null<const loader::Room*>& room,
                         const core::Angle& angle,
                         const core::TRCoordinates& position,
                         const floordata::ActivationState& activationState,
                         int16_t darkness,
                         const loader::AnimatedModel& animatedModel)
                : SlopedBridge(level, name, room, angle, position, activationState, darkness, animatedModel, 2)
            {
            }
        };
    }
}
