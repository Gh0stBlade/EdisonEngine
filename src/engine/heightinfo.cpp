#include "heightinfo.h"

#include "cameracontroller.h"
#include "level/level.h"

namespace engine
{
    bool HeightInfo::skipSteepSlants = false;

    HeightInfo HeightInfo::fromFloor(gsl::not_null<const loader::Sector*> roomSector, const core::TRCoordinates& pos, const CameraController* camera)
    {
        HeightInfo hi;

        hi.slantClass = SlantClass::None;

        while( roomSector->roomBelow != 0xff )
        {
            BOOST_ASSERT(roomSector->roomBelow < camera->getLevel()->m_rooms.size());
            auto room = &camera->getLevel()->m_rooms[roomSector->roomBelow];
            roomSector = room->getSectorByAbsolutePosition(pos);
        }

        hi.distance = roomSector->floorHeight * core::QuarterSectorSize;
        hi.lastTriggerOrKill = nullptr;

        if( roomSector->floorDataIndex == 0 )
        {
            return hi;
        }

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while( true )
        {
            const bool isLast = loader::isLastFloordataEntry(*floorData);
            const auto currentFd = *floorData;
            ++floorData;
            switch( loader::extractFDFunction(currentFd) )
            {
            case loader::FDFunction::FloorSlant:
                {
                    const int8_t xSlant = gsl::narrow_cast<int8_t>(*floorData & 0xff);
                    const auto absX = std::abs(xSlant);
                    const int8_t zSlant = gsl::narrow_cast<int8_t>((*floorData >> 8) & 0xff);
                    const auto absZ = std::abs(zSlant);
                    if( !skipSteepSlants || (absX <= 2 && absZ <= 2) )
                    {
                        if( absX <= 2 && absZ <= 2 )
                            hi.slantClass = SlantClass::Max512;
                        else
                            hi.slantClass = SlantClass::Steep;

                        const auto localX = pos.X % core::SectorSize;
                        const auto localZ = pos.Z % core::SectorSize;

                        if( zSlant > 0 ) // lower edge at -Z
                        {
                            auto dist = core::SectorSize - localZ;
                            hi.distance += dist * zSlant * core::QuarterSectorSize / core::SectorSize;
                        }
                        else if( zSlant < 0 ) // lower edge at +Z
                        {
                            auto dist = localZ;
                            hi.distance -= dist * zSlant * core::QuarterSectorSize / core::SectorSize;
                        }

                        if( xSlant > 0 ) // lower edge at -X
                        {
                            auto dist = core::SectorSize - localX;
                            hi.distance += dist * xSlant * core::QuarterSectorSize / core::SectorSize;
                        }
                        else if( xSlant < 0 ) // lower edge at +X
                        {
                            auto dist = localX;
                            hi.distance -= dist * xSlant * core::QuarterSectorSize / core::SectorSize;
                        }
                    }
                }
                // Fall-through
            case loader::FDFunction::CeilingSlant:
            case loader::FDFunction::PortalSector:
                ++floorData;
                break;
            case loader::FDFunction::Death:
                hi.lastTriggerOrKill = floorData - 1;
                break;
            case loader::FDFunction::Trigger:
                if( !hi.lastTriggerOrKill )
                    hi.lastTriggerOrKill = floorData - 1;
                ++floorData;
                while( true )
                {
                    bool isLastTrigger = loader::isLastFloordataEntry(*floorData);

                    const auto func = loader::extractTriggerFunction(*floorData);
                    const auto param = loader::extractTriggerFunctionParam(*floorData);
                    ++floorData;

                    if( func != loader::TriggerFunction::Object )
                    {
                        if( func == loader::TriggerFunction::CameraTarget )
                        {
                            isLastTrigger = loader::isLastFloordataEntry(*floorData);
                            ++floorData;
                        }
                    }
                    else
                    {
                        BOOST_ASSERT(func == loader::TriggerFunction::Object);
                        auto it = camera->getLevel()->m_itemControllers.find(param);
                        Expects(it != camera->getLevel()->m_itemControllers.end());
                        it->second->patchFloor(pos, hi.distance);
                    }

                    if( isLastTrigger )
                        break;
                }
            default:
                break;
            }
            if( isLast )
                break;
        }

        return hi;
    }

    HeightInfo HeightInfo::fromCeiling(gsl::not_null<const loader::Sector*> roomSector, const core::TRCoordinates& pos, const CameraController* camera)
    {
        HeightInfo hi;

        while( roomSector->roomAbove != 0xff )
        {
            BOOST_ASSERT(roomSector->roomAbove < camera->getLevel()->m_rooms.size());
            auto room = &camera->getLevel()->m_rooms[roomSector->roomAbove];
            roomSector = room->getSectorByAbsolutePosition(pos);
        }

        hi.distance = roomSector->ceilingHeight * core::QuarterSectorSize;

        if( roomSector->floorDataIndex != 0 )
        {
            const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
            auto fdFunc = loader::extractFDFunction(*floorData);
            ++floorData;

            if(fdFunc == loader::FDFunction::FloorSlant)
            {
                ++floorData;

                fdFunc = loader::extractFDFunction(*floorData);
                ++floorData;
            }

            if(fdFunc == loader::FDFunction::CeilingSlant)
            {
                const int8_t xSlant = gsl::narrow_cast<int8_t>(*floorData & 0xff);
                const auto absX = std::abs(xSlant);
                const int8_t zSlant = gsl::narrow_cast<int8_t>((*floorData >> 8) & 0xff);
                const auto absZ = std::abs(zSlant);
                if(!skipSteepSlants || (absX <= 2 && absZ <= 2))
                {
                    const auto localX = pos.X % core::SectorSize;
                    const auto localZ = pos.Z % core::SectorSize;

                    if(zSlant > 0) // lower edge at -Z
                    {
                        auto dist = core::SectorSize - localZ;
                        hi.distance -= dist * zSlant * core::QuarterSectorSize / core::SectorSize;
                    }
                    else if(zSlant < 0) // lower edge at +Z
                    {
                        auto dist = localZ;
                        hi.distance += dist * zSlant * core::QuarterSectorSize / core::SectorSize;
                    }

                    if(xSlant > 0) // lower edge at -X
                    {
                        auto dist = localX;
                        hi.distance -= dist * xSlant * core::QuarterSectorSize / core::SectorSize;
                    }
                    else if(xSlant < 0) // lower edge at +X
                    {
                        auto dist = core::SectorSize - localX;
                        hi.distance += dist * xSlant * core::QuarterSectorSize / core::SectorSize;
                    }
                }
            }
        }

        while(true)
        {
            if(roomSector->roomBelow == 255)
                break;

            roomSector = camera->getLevel()->m_rooms[roomSector->roomBelow].getSectorByAbsolutePosition(pos);
        }

        if(roomSector->floorDataIndex == 0)
            return hi;

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while( true )
        {
            const bool isLast = loader::isLastFloordataEntry(*floorData);
            const auto fdFunc = loader::extractFDFunction(*floorData);
            ++floorData;
            switch(fdFunc)
            {
            case loader::FDFunction::CeilingSlant:
            case loader::FDFunction::FloorSlant:
            case loader::FDFunction::PortalSector:
                ++floorData;
                break;
            case loader::FDFunction::Death:
                break;
            case loader::FDFunction::Trigger:
                ++floorData;
                while( true )
                {
                    bool isLastTrigger = loader::isLastFloordataEntry(*floorData);

                    const auto func = loader::extractTriggerFunction(*floorData);
                    const auto param = loader::extractTriggerFunctionParam(*floorData);
                    ++floorData;

                    if( func != loader::TriggerFunction::Object )
                    {
                        if( func == loader::TriggerFunction::CameraTarget )
                        {
                            isLastTrigger = loader::isLastFloordataEntry(*floorData);
                            ++floorData;
                        }
                    }
                    else
                    {
                        BOOST_ASSERT(func == loader::TriggerFunction::Object);
                        auto it = camera->getLevel()->m_itemControllers.find(param);
                        Expects(it != camera->getLevel()->m_itemControllers.end());
                        it->second->patchCeiling(pos, hi.distance);
                    }

                    if( isLastTrigger )
                        break;
                }
            default:
                break;
            }
            if( isLast )
                break;
        }

        return hi;
    }
}
