#pragma once

#include "gameplay.h"

#include <gsl/gsl>

#include <cmath>


namespace loader
{
    struct Room;
}


namespace core
{
    struct TRCoordinates
    {
        int X = 0, Y = 0, Z = 0;

        TRCoordinates() = default;
        TRCoordinates(const TRCoordinates&) = default;


        explicit TRCoordinates(const glm::vec3& v)
            : X{ gsl::narrow<int>(v.x) }
            , Y{ -gsl::narrow<int>(v.y) }
            , Z{ -gsl::narrow<int>(v.z) }
        {
        }


        TRCoordinates(int x, int y, int z)
            : X{ x }
            , Y{ y }
            , Z{ z }
        {
        }


        TRCoordinates operator-(const TRCoordinates& rhs) const noexcept
        {
            return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
        }


        TRCoordinates& operator-=(const TRCoordinates& rhs) noexcept
        {
            X -= rhs.X;
            Y -= rhs.Y;
            Z -= rhs.Z;
            return *this;
        }


        TRCoordinates operator+(const TRCoordinates& rhs) const noexcept
        {
            return {X + rhs.X, Y + rhs.Y, Z + rhs.Z};
        }


        TRCoordinates& operator+=(const TRCoordinates& rhs) noexcept
        {
            X += rhs.X;
            Y += rhs.Y;
            Z += rhs.Z;
            return *this;
        }


        TRCoordinates& operator=(const TRCoordinates&) = default;


        glm::vec3 toRenderSystem() const noexcept
        {
            return {gsl::narrow_cast<float>(X), -gsl::narrow_cast<float>(Y), -gsl::narrow_cast<float>(Z)};
        }


        int distanceTo(const TRCoordinates& rhs) const
        {
            const auto dx = gsl::narrow<float>(X - rhs.X);
            const auto dy = gsl::narrow<float>(Y - rhs.Y);
            const auto dz = gsl::narrow<float>(Z - rhs.Z);
            return static_cast<int>(std::sqrt(dx*dx + dy*dy + dz*dz));
        }
    };


    struct RoomBoundPosition
    {
        gsl::not_null<const loader::Room*> room;
        TRCoordinates position;


        explicit RoomBoundPosition(const gsl::not_null<const loader::Room*>& r, const TRCoordinates& pos = {})
            : room{ r }
            , position{ pos }
        {
        }
    };
}
