#pragma once

#include "Model.h"
#include "Visitor.h"
#include "MaterialParameter.h"

#include <glm/gtc/matrix_transform.hpp>


namespace gameplay
{
    class Drawable;
    class Scene;


    /**
     * Defines a hierarchical structure of objects in 3D transformation spaces.
     */
    class Node : public std::enable_shared_from_this<Node>
    {
        friend class Scene;

    public:
        using List = std::vector<std::shared_ptr<Node>>;

        explicit Node(const std::string& id);
        virtual ~Node();

        /**
         * Gets the identifier for the node.
         *
         * @return The node identifier.
         */
        const std::string& getId() const;

        /**
         * Sets the identifier for the node.
         *
         * @param id The identifier to set for the node.
         */
        void setId(const std::string& id);

        /**
         * Adds a child node.
         *
         * @param child The child to add.
         */
        void addChild(const std::shared_ptr<Node>& child);

        /**
         * Returns the parent of this node.
         *
         * @return The parent.
         */
        const std::weak_ptr<Node>& getParent() const;

        /**
         * Returns the number of direct children of this item.
         *
         * @return The number of children.
         */
        size_t getChildCount() const;

        /**
         * Gets the top level node in this node's parent hierarchy.
         */
        Node* getRootNode() const;

        /**
         * Gets the scene this node is currenlty within.
         *
         * @return The scene.
         */
        virtual Scene* getScene() const;

        /**
         * Sets if the node is enabled in the scene.
         *
         * @param enabled if the node is enabled in the scene.
         */
        void setEnabled(bool enabled);

        /**
         * Gets if the node is enabled in the scene.
         *
         * @return if the node is enabled in the scene.
         */
        bool isEnabled() const;

        /**
         * Gets if the node inherently enabled.
         *
         * @return if components attached on this node should be running.
         */
        bool isEnabledInHierarchy() const;

        /**
         * Gets the world matrix corresponding to this node.
         *
         * @return The world matrix of this node.
         */
        virtual const glm::mat4& getWorldMatrix() const;

        /**
         * Gets the world view matrix corresponding to this node.
         *
         * @return The world view matrix of this node.
         */
        glm::mat4 getWorldViewMatrix() const;

        /**
         * Gets the inverse transpose world matrix corresponding to this node.
         *
         * This matrix is typically used to transform normal vectors into world space.
         *
         * @return The inverse world matrix of this node.
         */
        glm::mat4 getInverseTransposeWorldMatrix() const;

        /**
         * Gets the inverse transpose world view matrix corresponding to this node.
         *
         * This matrix is typically used to transform normal vectors into view space.
         *
         * @return The inverse world view matrix of this node.
         */
        glm::mat4 getInverseTransposeWorldViewMatrix() const;

        /**
         * Gets the view matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The view matrix of this node.
         */
        const glm::mat4& getViewMatrix() const;

        /**
         * Gets the inverse view matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The inverse view matrix of this node.
         */
        const glm::mat4& getInverseViewMatrix() const;

        /**
         * Gets the projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The projection matrix of this node.
         */
        const glm::mat4& getProjectionMatrix() const;

        /**
         * Gets the view * projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The view * projection matrix of this node.
         */
        const glm::mat4& getViewProjectionMatrix() const;

        /**
         * Gets the inverse view * projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The inverse view * projection matrix of this node.
         */
        const glm::mat4& getInverseViewProjectionMatrix() const;

        /**
         * Gets the world * view * projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The world * view * projection matrix of this node.
         */
        glm::mat4 getWorldViewProjectionMatrix() const;

        /**
         * Gets the translation vector (or position) of this Node in world space.
         *
         * @return The world translation vector.
         */
        glm::vec3 getTranslationWorld() const;

        /**
         * Gets the translation vector (or position) of this Node in view space.
         *
         * @return The view space translation vector.
         */
        glm::vec3 getTranslationView() const;

        /**
         * Returns the translation vector of the currently active camera for this node's scene.
         *
         * @return The translation vector of the scene's active camera.
         */
        glm::vec3 getActiveCameraTranslationWorld() const;

        /**
         * Gets the drawable object attached to this node.
         *
         * @return The drawable component attached to this node.
         */
        const std::shared_ptr<Drawable>& getDrawable() const;

        /**
         * Set the drawable object to be attached to this node
         *
         * This is typically a Model, ParticleEmiiter, Form, Terrrain, Sprite, TileSet or Text.
         *
         * This will increase the reference count of the new drawble and decrease
         * the reference count of the old drawable.
         *
         * @param drawable The new drawable component. May be NULL.
         */
        void setDrawable(const std::shared_ptr<Drawable>& drawable);


        const List& getChildren() const
        {
            return _children;
        }


        const std::shared_ptr<Node>& getChild(size_t idx) const
        {
            BOOST_ASSERT(idx < _children.size());
            return _children[idx];
        }


        const glm::mat4& getLocalMatrix() const
        {
            return m_localMatrix;
        }


        void setLocalMatrix(const glm::mat4& m)
        {
            m_localMatrix = m;
            transformChanged();
        }


        void accept(Visitor& visitor)
        {
            for( auto& node : _children )
                visitor.visit(*node);
        }


        void setParent(const std::shared_ptr<Node>& parent)
        {
            if( !_parent.expired() )
            {
                auto p = _parent.lock();
                auto it = std::find(p->_children.begin(), p->_children.end(), shared_from_this());
                BOOST_ASSERT(it != p->_children.end());
                _parent.lock()->_children.erase(it);
            }

            _parent = parent;

            if( parent != nullptr )
                parent->_children.push_back(shared_from_this());

            transformChanged();
        }


        void swapChildren(const std::shared_ptr<Node>& other)
        {
            auto otherChildren = other->_children;
            for( auto& child : otherChildren )
                child->setParent(nullptr);
            BOOST_ASSERT(other->_children.empty());

            auto thisChildren = _children;
            for( auto& child : thisChildren )
                child->setParent(nullptr);
            BOOST_ASSERT(_children.empty());

            for( auto& child : otherChildren )
                child->setParent(shared_from_this());

            for( auto& child : thisChildren )
                child->setParent(other);
        }


        void addMaterialParameterSetter(const std::string& name, const std::function<MaterialParameter::UniformValueSetter>& setter)
        {
            _materialParemeterSetters[name] = setter;
        }


        void addMaterialParameterSetter(const std::string& name, std::function<MaterialParameter::UniformValueSetter>&& setter)
        {
            _materialParemeterSetters[name] = std::move(setter);
        }


        const std::map<std::string, std::function<MaterialParameter::UniformValueSetter>>& getMaterialParameterSetters() const
        {
            return _materialParemeterSetters;
        }


    protected:

        /**
         * Called when this Node's transform changes.
         */
        void transformChanged();

    private:

        Node(const Node& copy) = delete;

        Node& operator=(const Node&) = delete;

        /** The scene this node is attached to. */
        Scene* _scene = nullptr;
        /** The nodes id. */
        std::string _id;

        List _children;

        /** The nodes parent. */
        std::weak_ptr<Node> _parent{};

        /** If this node is enabled. Maybe different if parent is enabled/disabled. */
        bool _enabled = true;
        /** The drawble component attached to this node. */
        std::shared_ptr<Drawable> _drawable = nullptr;

        glm::mat4 m_localMatrix{1.0f};
        mutable glm::mat4 m_worldMatrix{1.0f};
        mutable bool _dirty = false;

        std::map<std::string, std::function<MaterialParameter::UniformValueSetter>> _materialParemeterSetters;
    };
}
