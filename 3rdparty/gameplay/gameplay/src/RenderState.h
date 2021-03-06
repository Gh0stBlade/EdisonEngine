#pragma once

#include "Base.h"
#include "gl/util.h"

#include <map>
#include <memory>
#include <vector>

namespace gameplay
{
    class Material;
    class Game;
    class Model;
    class MaterialParameter;
    class Node;


    /**
     * Defines the rendering state of the graphics device.
     */
    class RenderState
    {
        friend class Game;
        friend class Material;
        friend class Model;

    public:
        /**
         * Defines blend constants supported by the blend function.
         */
        enum Blend : GLenum
        {
            BLEND_ZERO = GL_ZERO,
            BLEND_ONE = GL_ONE,
            BLEND_SRC_COLOR = GL_SRC_COLOR,
            BLEND_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
            BLEND_DST_COLOR = GL_DST_COLOR,
            BLEND_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
            BLEND_SRC_ALPHA = GL_SRC_ALPHA,
            BLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
            BLEND_DST_ALPHA = GL_DST_ALPHA,
            BLEND_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
            BLEND_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
            BLEND_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
            BLEND_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
        };


        /**
         * Defines the supported depth compare functions.
         *
         * Depth compare functions specify the comparison that takes place between the
         * incoming pixel's depth value and the depth value already in the depth buffer.
         * If the compare function passes, the new pixel will be drawn.
         *
         * The intial depth compare function is DEPTH_LESS.
         */
        enum DepthFunction : GLenum
        {
            DEPTH_NEVER = GL_NEVER,
            DEPTH_LESS = GL_LESS,
            DEPTH_EQUAL = GL_EQUAL,
            DEPTH_LEQUAL = GL_LEQUAL,
            DEPTH_GREATER = GL_GREATER,
            DEPTH_NOTEQUAL = GL_NOTEQUAL,
            DEPTH_GEQUAL = GL_GEQUAL,
            DEPTH_ALWAYS = GL_ALWAYS
        };


        /**
         * Defines culling criteria for front-facing, back-facing and both-side
         * facets.
         */
        enum CullFaceSide : GLenum
        {
            CULL_FACE_SIDE_BACK = GL_BACK,
            CULL_FACE_SIDE_FRONT = GL_FRONT,
            CULL_FACE_SIDE_FRONT_AND_BACK = GL_FRONT_AND_BACK
        };


        /**
         * Defines the winding of vertices in faces that are considered front facing.
         *
         * The initial front face mode is set to FRONT_FACE_CCW.
         */
        enum FrontFace : GLenum
        {
            FRONT_FACE_CW = GL_CW,
            FRONT_FACE_CCW = GL_CCW
        };


        /**
         * Defines a block of fixed-function render states that can be applied to a
         * RenderState object.
         */
        class StateBlock
        {
            friend class RenderState;
            friend class Game;

        public:
            explicit StateBlock();
            ~StateBlock();

            /**
             * Binds the state in this StateBlock to the renderer.
             *
             * This method handles both setting and restoring of render states to ensure that
             * only the state explicitly defined by this StateBlock is applied to the renderer.
             */
            void bind();

            /**
             * Toggles blending.
             *
              * @param enabled true to enable, false to disable.
             */
            void setBlend(bool enabled);

            /**
             * Explicitly sets the source used in the blend function for this render state.
             *
             * Note that the blend function is only applied when blending is enabled.
             *
             * @param blend Specifies how the source blending factors are computed.
             */
            void setBlendSrc(Blend blend);

            /**
             * Explicitly sets the source used in the blend function for this render state.
             *
             * Note that the blend function is only applied when blending is enabled.
             *
             * @param blend Specifies how the destination blending factors are computed.
             */
            void setBlendDst(Blend blend);

            /**
             * Explicitly enables or disables backface culling.
             *
             * @param enabled true to enable, false to disable.
             */
            void setCullFace(bool enabled);

            /**
             * Sets the side of the facets to cull.
             *
             * When not explicitly set, the default is to cull back-facing facets.
             *
             * @param side The side to cull.
             */
            void setCullFaceSide(CullFaceSide side);

            /**
             * Sets the winding for front facing polygons.
             *
             * By default, counter-clockwise wound polygons are considered front facing.
             *
             * @param winding The winding for front facing polygons.
             */
            void setFrontFace(FrontFace winding);

            /**
             * Toggles depth testing.
             *
             * By default, depth testing is disabled.
             *
             * @param enabled true to enable, false to disable.
             */
            void setDepthTest(bool enabled);

            /**
             * Toggles depth writing.
             *
             * @param enabled true to enable, false to disable.
             */
            void setDepthWrite(bool enabled);

            /**
             * Sets the depth function to use when depth testing is enabled.
             *
             * When not explicitly set and when depth testing is enabled, the default
             * depth function is DEPTH_LESS.
             *
             * @param func The depth function.
             */
            void setDepthFunction(DepthFunction func);

        private:

            /**
             * Copy constructor.
             */
            StateBlock(const StateBlock& copy) = delete;

            void bindNoRestore();

            static void restore(long stateOverrideBits);

            static void enableDepthWrite();

            // States
            bool _cullFaceEnabled = false;
            bool _depthTestEnabled = false;
            bool _depthWriteEnabled = true;
            DepthFunction _depthFunction = RenderState::DEPTH_LESS;
            bool _blendEnabled = false;
            Blend _blendSrc = RenderState::BLEND_ONE;
            Blend _blendDst = RenderState::BLEND_ZERO;
            CullFaceSide _cullFaceSide = CULL_FACE_SIDE_BACK;
            FrontFace _frontFace = FRONT_FACE_CCW;
            long _bits = 0;

            static std::shared_ptr<StateBlock> _defaultState;
        };


        /**
         * Gets a MaterialParameter for the specified name.
         *
         * The returned MaterialParameter can be used to set values for the specified
         * parameter name.
         *
         * Note that this method causes a new MaterialParameter to be created if one
         * does not already exist for the given parameter name.
         *
         * @param name Material parameter (uniform) name.
         *
         * @return A MaterialParameter for the specified name.
         */
        std::shared_ptr<MaterialParameter> getParameter(const std::string& name) const;

        /**
         * Gets the number of material parameters.
         *
         * @return The number of material parameters.
         */
        size_t getParameterCount() const;

        /**
         * Gets a MaterialParameter for the specified index.
         *
         * @return A MaterialParameter for the specified index.
         */
        const std::shared_ptr<MaterialParameter>& getParameterByIndex(size_t index) const;

        /**
         * Adds a MaterialParameter to the render state.
         *
         * @param param The parameters to to added.
         */
        void addParameter(const std::shared_ptr<MaterialParameter>& param);

        /**
         * Removes(clears) the MaterialParameter with the given name.
         *
         * If a material parameter exists for the given name, it is destroyed and
         * removed from this RenderState.
         *
         * @param name Material parameter (uniform) name.
         */
        void removeParameter(const char* name);

        /**
         * Sets the fixed-function render state of this object to the state contained
         * in the specified StateBlock.
         *
         * The passed in StateBlock is stored in this RenderState object with an
         * increased reference count and released when either a different StateBlock
         * is assigned, or when this RenderState object is destroyed.
         *
         * @param state The state block to set.
         */
        void setStateBlock(const std::shared_ptr<StateBlock>& state);

        /**
         * Gets the fixed-function StateBlock for this RenderState object.
         *
         * The returned StateBlock is referenced by this RenderState and therefore
         * should not be released by the user. To release a StateBlock for a
         * RenderState, the setState(StateBlock*) method should be called, passing
         * NULL. This removes the StateBlock and resets the fixed-function render
         * state to the default state.
         *
         * It is legal to pass the returned StateBlock to another RenderState object.
         * In this case, the StateBlock will be referenced by both RenderState objects
         * and any changes to the StateBlock will be reflected in all objects
         * that reference it.
         *
         * @return The StateBlock for this RenderState.
         */
        const std::shared_ptr<RenderState::StateBlock>& getStateBlock() const;

        void initStateBlockDefaults();

    protected:

        /**
         * Constructor.
         */
        RenderState();

        /**
         * Destructor.
         */
        virtual ~RenderState();

        /**
         * Static initializer that is called during game startup.
         */
        static void initialize();

        /**
         * Static finalizer that is called during game shutdown.
         */
        static void finalize();

        /**
         * Binds the render state for this RenderState and any of its parents, top-down,
         * for the given pass.
         */
        void bind(const Node& node, Material* material);

        /**
         * Returns the topmost RenderState in the hierarchy below the given RenderState.
         */
        RenderState* getTopmost(const RenderState* below);

    private:

        RenderState(const RenderState& copy) = delete;

        RenderState& operator=(const RenderState&) = delete;

    protected:

        /**
         * Collection of MaterialParameter's to be applied to the gameplay::Effect.
         */
        mutable std::vector<std::shared_ptr<MaterialParameter>> _parameters;

        /**
         * The StateBlock of fixed-function render states that can be applied to the RenderState.
         */
        mutable std::shared_ptr<StateBlock> _state;

        /**
         * The RenderState's parent.
         */
        RenderState* _parent = nullptr;
    };
}
