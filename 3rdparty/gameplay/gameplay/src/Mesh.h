#pragma once

#include "BoundingBox.h"

#include "ext/structuredvertexbuffer.h"

#include <gsl/gsl>

#include <memory>


namespace gameplay
{
    class MeshPart;

    class Model;


    class Mesh
    {
        friend class Model;

    public:
        explicit Mesh(const ext::StructuredVertexBuffer::AttributeMapping& mapping, bool dynamic, const std::string& label = {})
        {
            addBuffer(mapping, dynamic, label);
        }


        static std::shared_ptr<Mesh> createQuadFullscreen(float width, float height, bool invertY = false);

        std::shared_ptr<MeshPart> addPart(GLenum primitiveType, GLint indexFormat, size_t indexCount, bool dynamic = false);

        size_t getPartCount() const;

        const std::shared_ptr<MeshPart>& getPart(size_t index);

        virtual ~Mesh() = default;


        ext::StructuredVertexBuffer& getBuffer(size_t idx)
        {
            BOOST_ASSERT(idx < m_buffers.size());

            return m_buffers[idx];
        }


        const std::vector<ext::StructuredVertexBuffer>& getBuffers() const
        {
            return m_buffers;
        }


        std::vector<ext::StructuredVertexBuffer>& getBuffers()
        {
            return m_buffers;
        }


        size_t addBuffer(const ext::StructuredVertexBuffer::AttributeMapping& mapping, bool dynamic, const std::string& label = {})
        {
            m_buffers.emplace_back(mapping, dynamic, label);
            return m_buffers.size() - 1;
        }


    private:

        Mesh(const Mesh& copy) = delete;

        Mesh& operator=(const Mesh&) = delete;

        std::vector<std::shared_ptr<MeshPart>> _parts{};

        std::vector<ext::StructuredVertexBuffer> m_buffers{};
    };
}
