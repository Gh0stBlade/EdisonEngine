#include "Base.h"
#include "RenderState.h"
#include "Node.h"
#include "Scene.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>

// Render state override bits
#define RS_BLEND 1
#define RS_BLEND_FUNC 2
#define RS_CULL_FACE 4
#define RS_DEPTH_TEST 8
#define RS_DEPTH_WRITE 16
#define RS_DEPTH_FUNC 32
#define RS_CULL_FACE_SIDE 64
#define RS_FRONT_FACE 2048

#define RS_ALL_ONES 0xFFFFFFFF


namespace gameplay
{
    std::shared_ptr<RenderState::StateBlock> RenderState::StateBlock::_defaultState = nullptr;


    RenderState::RenderState()
        : _state(nullptr)
        , _parent(nullptr)
    {
    }


    RenderState::~RenderState() = default;


    void RenderState::initialize()
    {
        if( StateBlock::_defaultState == nullptr )
        {
            StateBlock::_defaultState = std::make_shared<StateBlock>();
        }
    }


    void RenderState::finalize()
    {
        StateBlock::_defaultState.reset();
    }


    std::shared_ptr<MaterialParameter> RenderState::getParameter(const std::string& name) const
    {
        // Search for an existing parameter with this name.
        for( const auto& param : _parameters )
        {
            BOOST_ASSERT(param);
            if( param->getName() == name )
            {
                return param;
            }
        }

        // Create a new parameter and store it in our list.
        auto param = std::make_shared<MaterialParameter>(name);
        _parameters.push_back(param);

        return param;
    }


    size_t RenderState::getParameterCount() const
    {
        return _parameters.size();
    }


    const std::shared_ptr<MaterialParameter>& RenderState::getParameterByIndex(size_t index) const
    {
        return _parameters[index];
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::addParameter(const std::shared_ptr<MaterialParameter>& param)
    {
        _parameters.push_back(param);
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::removeParameter(const char* name)
    {
        for( size_t i = 0, count = _parameters.size(); i < count; ++i )
        {
            auto p = _parameters[i];
            if( p->m_name == name )
            {
                _parameters.erase(_parameters.begin() + i);
                break;
            }
        }
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::setStateBlock(const std::shared_ptr<StateBlock>& state)
    {
        _state = state;
    }


    const std::shared_ptr<RenderState::StateBlock>& RenderState::getStateBlock() const
    {
        if( _state == nullptr )
        {
            _state = std::make_shared<StateBlock>();
        }

        return _state;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::initStateBlockDefaults()
    {
        getStateBlock(); // alloc if not done yet
        _state->setDepthTest(true);
        _state->setDepthFunction(DEPTH_LESS);
        _state->setCullFace(true);
        _state->setFrontFace(FRONT_FACE_CW);
        _state->setBlend(true);
        _state->setBlendSrc(BLEND_SRC_ALPHA);
        _state->setBlendDst(BLEND_ONE_MINUS_SRC_ALPHA);
    }


    void RenderState::bind(const Node& node, Material* material)
    {
        BOOST_ASSERT(material);

        // Get the combined modified state bits for our RenderState hierarchy.
        long stateOverrideBits = _state ? _state->_bits : 0;
        auto rs = _parent;
        while( rs )
        {
            if( rs->_state )
            {
                stateOverrideBits |= rs->_state->_bits;
            }
            rs = rs->_parent;
        }

        // Restore renderer state to its default, except for explicitly specified states
        StateBlock::restore(stateOverrideBits);

        // Apply parameter bindings and renderer state for the entire hierarchy, top-down.
        rs = nullptr;
        auto shader = material->getShaderProgram();
        while( (rs = getTopmost(rs)) )
        {
            for( const auto& param : rs->_parameters )
            {
                BOOST_ASSERT(param);
                param->bind(node, shader);
            }

            if( rs->_state )
            {
                rs->_state->bindNoRestore();
            }
        }
    }


    RenderState* RenderState::getTopmost(const RenderState* below)
    {
        RenderState* rs = this;
        if( rs == below )
        {
            // Nothing below ourself.
            return nullptr;
        }

        while( rs )
        {
            if( rs->_parent == below || rs->_parent == nullptr )
            {
                // Stop traversing up here.
                return rs;
            }
            rs = rs->_parent;
        }

        return nullptr;
    }


    RenderState::StateBlock::StateBlock()
    {
    }


    RenderState::StateBlock::~StateBlock() = default;


    void RenderState::StateBlock::bind()
    {
        // When the public bind() is called with no RenderState object passed in,
        // we assume we are being called to bind the state of a single StateBlock,
        // irrespective of whether it belongs to a hierarchy of RenderStates.
        // Therefore, we call restore() here with only this StateBlock's override
        // bits to restore state before applying the new state.
        StateBlock::restore(_bits);

        bindNoRestore();
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::StateBlock::bindNoRestore()
    {
        BOOST_ASSERT(_defaultState);

        // Update any state that differs from _defaultState and flip _defaultState bits
        if( (_bits & RS_BLEND) && (_blendEnabled != _defaultState->_blendEnabled) )
        {
            if(_blendEnabled)
            {
                GL_ASSERT(glEnable(GL_BLEND));
            }
            else
            {
                GL_ASSERT(glDisable(GL_BLEND));
            }
            _defaultState->_blendEnabled = _blendEnabled;
        }
        if( (_bits & RS_BLEND_FUNC) && (_blendSrc != _defaultState->_blendSrc || _blendDst != _defaultState->_blendDst) )
        {
            GL_ASSERT( glBlendFunc(static_cast<GLenum>(_blendSrc), static_cast<GLenum>(_blendDst)) );
            _defaultState->_blendSrc = _blendSrc;
            _defaultState->_blendDst = _blendDst;
        }
        if( (_bits & RS_CULL_FACE) && (_cullFaceEnabled != _defaultState->_cullFaceEnabled) )
        {
            if(_cullFaceEnabled)
            {
                GL_ASSERT(glEnable(GL_CULL_FACE));
            }
            else
            {
                GL_ASSERT(glDisable(GL_CULL_FACE));
            }
            _defaultState->_cullFaceEnabled = _cullFaceEnabled;
        }
        if( (_bits & RS_CULL_FACE_SIDE) && (_cullFaceSide != _defaultState->_cullFaceSide) )
        {
            GL_ASSERT( glCullFace(static_cast<GLenum>(_cullFaceSide)) );
            _defaultState->_cullFaceSide = _cullFaceSide;
        }
        if( (_bits & RS_FRONT_FACE) && (_frontFace != _defaultState->_frontFace) )
        {
            GL_ASSERT( glFrontFace(static_cast<GLenum>(_frontFace)) );
            _defaultState->_frontFace = _frontFace;
        }
        if( (_bits & RS_DEPTH_TEST) && (_depthTestEnabled != _defaultState->_depthTestEnabled) )
        {
            if(_depthTestEnabled)
            {
                GL_ASSERT(glEnable(GL_DEPTH_TEST));
            }
            else
            {
                GL_ASSERT(glDisable(GL_DEPTH_TEST));
            }
            _defaultState->_depthTestEnabled = _depthTestEnabled;
        }
        if( (_bits & RS_DEPTH_WRITE) && (_depthWriteEnabled != _defaultState->_depthWriteEnabled) )
        {
            GL_ASSERT( glDepthMask(_depthWriteEnabled ? GL_TRUE : GL_FALSE) );
            _defaultState->_depthWriteEnabled = _depthWriteEnabled;
        }
        if( (_bits & RS_DEPTH_FUNC) && (_depthFunction != _defaultState->_depthFunction) )
        {
            GL_ASSERT( glDepthFunc(static_cast<GLenum>(_depthFunction)) );
            _defaultState->_depthFunction = _depthFunction;
        }

        _defaultState->_bits |= _bits;
    }


    void RenderState::StateBlock::restore(long stateOverrideBits)
    {
        BOOST_ASSERT(_defaultState);

        // If there is no state to restore (i.e. no non-default state), do nothing.
        if( _defaultState->_bits == 0 )
        {
            return;
        }

        // Restore any state that is not overridden and is not default
        if( !(stateOverrideBits & RS_BLEND) && (_defaultState->_bits & RS_BLEND) )
        {
            GL_ASSERT( glDisable(GL_BLEND) );
            _defaultState->_bits &= ~RS_BLEND;
            _defaultState->_blendEnabled = false;
        }
        if( !(stateOverrideBits & RS_BLEND_FUNC) && (_defaultState->_bits & RS_BLEND_FUNC) )
        {
            GL_ASSERT( glBlendFunc(GL_ONE, GL_ZERO) );
            _defaultState->_bits &= ~RS_BLEND_FUNC;
            _defaultState->_blendSrc = RenderState::BLEND_ONE;
            _defaultState->_blendDst = RenderState::BLEND_ZERO;
        }
        if( !(stateOverrideBits & RS_CULL_FACE) && (_defaultState->_bits & RS_CULL_FACE) )
        {
            GL_ASSERT( glDisable(GL_CULL_FACE) );
            _defaultState->_bits &= ~RS_CULL_FACE;
            _defaultState->_cullFaceEnabled = false;
        }
        if( !(stateOverrideBits & RS_CULL_FACE_SIDE) && (_defaultState->_bits & RS_CULL_FACE_SIDE) )
        {
            GL_ASSERT( glCullFace(GL_BACK) );
            _defaultState->_bits &= ~RS_CULL_FACE_SIDE;
            _defaultState->_cullFaceSide = RenderState::CULL_FACE_SIDE_BACK;
        }
        if( !(stateOverrideBits & RS_FRONT_FACE) && (_defaultState->_bits & RS_FRONT_FACE) )
        {
            GL_ASSERT( glFrontFace(GL_CCW) );
            _defaultState->_bits &= ~RS_FRONT_FACE;
            _defaultState->_frontFace = RenderState::FRONT_FACE_CCW;
        }
        if( !(stateOverrideBits & RS_DEPTH_TEST) && (_defaultState->_bits & RS_DEPTH_TEST) )
        {
            GL_ASSERT( glDisable(GL_DEPTH_TEST) );
            _defaultState->_bits &= ~RS_DEPTH_TEST;
            _defaultState->_depthTestEnabled = false;
        }
        if( !(stateOverrideBits & RS_DEPTH_WRITE) && (_defaultState->_bits & RS_DEPTH_WRITE) )
        {
            GL_ASSERT( glDepthMask(GL_TRUE) );
            _defaultState->_bits &= ~RS_DEPTH_WRITE;
            _defaultState->_depthWriteEnabled = true;
        }
        if( !(stateOverrideBits & RS_DEPTH_FUNC) && (_defaultState->_bits & RS_DEPTH_FUNC) )
        {
            GL_ASSERT( glDepthFunc(GL_LESS) );
            _defaultState->_bits &= ~RS_DEPTH_FUNC;
            _defaultState->_depthFunction = RenderState::DEPTH_LESS;
        }
    }


    void RenderState::StateBlock::enableDepthWrite()
    {
        BOOST_ASSERT(_defaultState);

        // Internal method used by Game::clear() to restore depth writing before a
        // clear operation. This is necessary if the last code to draw before the
        // next frame leaves depth writing disabled.
        if( !_defaultState->_depthWriteEnabled )
        {
            GL_ASSERT( glDepthMask(GL_TRUE) );
            _defaultState->_bits &= ~RS_DEPTH_WRITE;
            _defaultState->_depthWriteEnabled = true;
        }
    }


    void RenderState::StateBlock::setBlend(bool enabled)
    {
        _blendEnabled = enabled;
        if( !enabled )
        {
            _bits &= ~RS_BLEND;
        }
        else
        {
            _bits |= RS_BLEND;
        }
    }


    void RenderState::StateBlock::setBlendSrc(Blend blend)
    {
        _blendSrc = blend;
        if( _blendSrc == BLEND_ONE && _blendDst == BLEND_ZERO )
        {
            // Default blend func
            _bits &= ~RS_BLEND_FUNC;
        }
        else
        {
            _bits |= RS_BLEND_FUNC;
        }
    }


    void RenderState::StateBlock::setBlendDst(Blend blend)
    {
        _blendDst = blend;
        if( _blendSrc == BLEND_ONE && _blendDst == BLEND_ZERO )
        {
            // Default blend func
            _bits &= ~RS_BLEND_FUNC;
        }
        else
        {
            _bits |= RS_BLEND_FUNC;
        }
    }


    void RenderState::StateBlock::setCullFace(bool enabled)
    {
        _cullFaceEnabled = enabled;
        if( !enabled )
        {
            _bits &= ~RS_CULL_FACE;
        }
        else
        {
            _bits |= RS_CULL_FACE;
        }
    }


    void RenderState::StateBlock::setCullFaceSide(CullFaceSide side)
    {
        _cullFaceSide = side;
        if( _cullFaceSide == CULL_FACE_SIDE_BACK )
        {
            // Default cull side
            _bits &= ~RS_CULL_FACE_SIDE;
        }
        else
        {
            _bits |= RS_CULL_FACE_SIDE;
        }
    }


    void RenderState::StateBlock::setFrontFace(FrontFace winding)
    {
        _frontFace = winding;
        if( _frontFace == FRONT_FACE_CCW )
        {
            // Default front face
            _bits &= ~RS_FRONT_FACE;
        }
        else
        {
            _bits |= RS_FRONT_FACE;
        }
    }


    void RenderState::StateBlock::setDepthTest(bool enabled)
    {
        _depthTestEnabled = enabled;
        if( !enabled )
        {
            _bits &= ~RS_DEPTH_TEST;
        }
        else
        {
            _bits |= RS_DEPTH_TEST;
        }
    }


    void RenderState::StateBlock::setDepthWrite(bool enabled)
    {
        _depthWriteEnabled = enabled;
        if( enabled )
        {
            _bits &= ~RS_DEPTH_WRITE;
        }
        else
        {
            _bits |= RS_DEPTH_WRITE;
        }
    }


    void RenderState::StateBlock::setDepthFunction(DepthFunction func)
    {
        _depthFunction = func;
        if( _depthFunction == DEPTH_LESS )
        {
            // Default depth function
            _bits &= ~RS_DEPTH_FUNC;
        }
        else
        {
            _bits |= RS_DEPTH_FUNC;
        }
    }
}
