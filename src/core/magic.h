#pragma once

namespace core
{
    constexpr int SectorSize = 1024;
    constexpr int QuarterSectorSize = SectorSize / 4;
    constexpr int HeightLimit = 127 * QuarterSectorSize;

    constexpr int SteppableHeight = QuarterSectorSize / 2;
    constexpr int ClimbLimit2ClickMin = QuarterSectorSize + SteppableHeight;
    constexpr int ClimbLimit2ClickMax = QuarterSectorSize + ClimbLimit2ClickMin;
    constexpr int ClimbLimit3ClickMax = QuarterSectorSize + ClimbLimit2ClickMax;

    constexpr int ScalpHeight = 762;
    constexpr int ScalpToHandsHeight = 160;
    constexpr int JumpReachableHeight = 896 + SectorSize;

    constexpr int FreeFallSpeedThreshold = 131;
    constexpr int MaxGrabbableGradient = 60;

    constexpr int FrameRate = 30;
}
