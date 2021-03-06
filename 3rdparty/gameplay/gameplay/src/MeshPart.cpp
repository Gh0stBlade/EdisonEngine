#include "MeshPart.h"

#include "Drawable.h"
#include "Material.h"
#include "RenderContext.h"


namespace gameplay
{
    MeshPart::MeshPart(const gsl::not_null<Mesh*>& mesh,
                       GLenum primitiveType,
                       GLint indexFormat,
                       size_t indexCount,
                       bool dynamic)
        : _mesh{mesh}
        , _primitiveType{primitiveType}
        , _indexFormat{indexFormat}
        , _indexCount{indexCount}
        , _dynamic{dynamic}
    {
        bind();

        size_t indexSize;
        switch( indexFormat )
        {
            case gl::TypeTraits<uint8_t>::TypeId:
                indexSize = 1;
                break;
            case gl::TypeTraits<uint16_t>::TypeId:
                indexSize = 2;
                break;
            case gl::TypeTraits<uint32_t>::TypeId:
                indexSize = 4;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported index format";
                BOOST_THROW_EXCEPTION(std::runtime_error("Unsupported index format"));
        }

        GL_ASSERT(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * indexCount, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
    }


    MeshPart::~MeshPart() = default;


    GLenum MeshPart::getPrimitiveType() const
    {
        return _primitiveType;
    }


    size_t MeshPart::getIndexCount() const
    {
        return _indexCount;
    }


    GLint MeshPart::getIndexFormat() const
    {
        return _indexFormat;
    }


    bool MeshPart::isDynamic() const
    {
        return _dynamic;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void MeshPart::setIndexData(const gsl::not_null<const void*>& indexData, size_t indexStart, size_t indexCount)
    {
        bind();

        size_t indexSize;
        switch( _indexFormat )
        {
            case gl::TypeTraits<uint8_t>::TypeId:
                indexSize = 1;
                break;
            case gl::TypeTraits<uint16_t>::TypeId:
                indexSize = 2;
                break;
            case gl::TypeTraits<uint32_t>::TypeId:
                indexSize = 4;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported index format";
                return;
        }

        if( indexStart == 0 && indexCount == 0 )
        {
            GL_ASSERT( glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * _indexCount, indexData, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW) );
        }
        else
        {
            if( indexCount == 0 )
            {
                indexCount = _indexCount - indexStart;
            }

            GL_ASSERT( glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexStart * indexSize, indexCount * indexSize, indexData) );
        }
    }


    void MeshPart::setMaterial(const std::shared_ptr<Material>& material)
    {
        BOOST_ASSERT(_mesh != nullptr);

        _material = material;
    }


    bool MeshPart::drawWireframe() const
    {
        size_t indexSize;
        switch(_indexFormat)
        {
            case gl::TypeTraits<uint8_t>::TypeId:
                indexSize = 1;
                break;
            case gl::TypeTraits<uint16_t>::TypeId:
                indexSize = 2;
                break;
            case gl::TypeTraits<uint32_t>::TypeId:
                indexSize = 4;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported index format";
                return false;
        }

        switch(_primitiveType)
        {
            case GL_TRIANGLES:
                for(size_t i = 0; i < _indexCount; i += 3)
                {
                    GL_ASSERT(glDrawElements(GL_LINE_LOOP, 3, static_cast<GLenum>(_indexFormat), reinterpret_cast<const GLvoid*>(i*indexSize)));
                }
                return true;

            case GL_TRIANGLE_STRIP:
                for(size_t i = 2; i < _indexCount; ++i)
                {
                    GL_ASSERT(glDrawElements(GL_LINE_LOOP, 3, static_cast<GLenum>(_indexFormat), reinterpret_cast<const GLvoid*>((i - 2)*indexSize)));
                }
                return true;

            default:
                // not supported
                return false;
        }
    }


    void MeshPart::draw(RenderContext& context) const
    {
        if(!_material)
            return;

        BOOST_ASSERT(context.getCurrentNode() != nullptr);

        for(const auto& mps : _materialParameterSetters)
            mps(*_material);

        _material->bind(*context.getCurrentNode());

        if(m_vao == nullptr)
        {
            m_vao = std::make_shared<gl::VertexArray>();
            m_vao->bind();
            bind();
            for(const auto& buffer : _mesh->getBuffers())
                buffer.bind(_material->getShaderProgram()->getHandle());
            m_vao->unbind();
        }

        m_vao->bind();

        if(!context.isWireframe() || !drawWireframe())
        {
            GL_ASSERT(glDrawElements(_primitiveType, gsl::narrow<GLsizei>(_indexCount), static_cast<GLenum>(_indexFormat), nullptr));
        }

        m_vao->unbind();
    }

}
