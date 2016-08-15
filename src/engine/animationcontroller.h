#pragma once

#include "level/level.h"
#include "loader/animation.h"

#include <boost/optional.hpp>

namespace engine
{
/**
 * @brief Handles state transitions and animation playback.
 */
class AnimationController
{
private:
    const gsl::not_null<const level::Level*> m_level;
    const std::string m_name;

public:
    AnimationController(gsl::not_null<const level::Level*> level, const std::string& name)
        : m_level(level)
        , m_name(name)
    {
    }

    virtual ~AnimationController() = default;

    const std::string& getName() const
    {
        return m_name;
    }

    const gsl::not_null<const level::Level*>& getLevel() const
    {
        return m_level;
    }

    virtual osg::BoundingBoxImpl<osg::Vec3i> getBoundingBox() const = 0;
};

class MeshAnimationController final : public AnimationController
{
    const loader::AnimatedModel& m_model;
    uint16_t m_currentAnimationId;
    uint16_t m_targetState = 0;

public:
    MeshAnimationController(gsl::not_null<const level::Level*> level, const loader::AnimatedModel& model, const std::string& name);

    void setTargetState(uint16_t state) noexcept
    {
        if(state == m_targetState)
            return;

        BOOST_LOG_TRIVIAL(debug) << getName() << " -- set target state=" << state << " (was " << m_targetState << "), current state=" << getCurrentAnimState();
        m_targetState = state;
    }

    uint16_t getTargetState() const noexcept
    {
        return m_targetState;
    }

    uint16_t getCurrentAnimState() const;

    /**
    * @brief Play a specific animation.
    * @param anim Animation ID
    *
    * @details
    * Plays the animation specified; if the animation does not exist, nothing happens;
    * if it exists, the target state is changed to the animation's state.
    */
    void playGlobalAnimation(uint16_t anim, const boost::optional<uint32_t>& firstFrame = boost::none);

    void playLocalAnimation(uint16_t anim, const boost::optional<uint32_t>& firstFrame = boost::none)
    {
        playGlobalAnimation(m_model.animationIndex + anim, firstFrame);
    }

    uint16_t getCurrentAnimationId() const noexcept
    {
        return m_currentAnimationId;
    }

    bool handleTRTransitions();
    void handleAnimationEnd();

    float calculateFloorSpeed() const
    {
        BOOST_ASSERT(m_currentAnimationId < getLevel()->m_animations.size());
        const loader::Animation& currentAnim = getLevel()->m_animations[m_currentAnimationId];
        return float(currentAnim.speed + currentAnim.accelleration * getCurrentRelativeFrame()) / (1 << 16);
    }

    int getAccelleration() const
    {
        BOOST_ASSERT(m_currentAnimationId < getLevel()->m_animations.size());
        const loader::Animation& currentAnim = getLevel()->m_animations[m_currentAnimationId];
        return currentAnim.accelleration / (1 << 16);
    }

    void advanceFrame();
    uint32_t getCurrentFrame() const;
    uint32_t getAnimEndFrame() const;

    osg::BoundingBoxImpl<osg::Vec3i> getBoundingBox() const override;

    void resetPose()
    {
#if 0
        //! @todo
        m_node->setJointMode(irr::scene::EJUOR_CONTROL);
        m_node->animateJoints();
#endif
    }

    void rotateBone(uint32_t id, const core::TRRotation& dr)
    {
#if 0
        //! @todo
        Expects(id < m_node->getJointCount());
        auto bone = m_node->getJointNode(id);
        bone->getAbsoluteTransformation();

        bone->setRotation(bone->getRotation() + xyzToYprRad(dr));
#endif
    }

private:
    /**
    * @brief Starts to play the current animation at the specified frame.
    * @param[in] localFrame The animation-local frame number.
    */
    void startAnimLoop(uint32_t localFrame);
    uint32_t getCurrentRelativeFrame() const;
};

}
