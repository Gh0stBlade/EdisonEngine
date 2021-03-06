#pragma once

#include "gl/util.h"

#include <vector>

#include <gsl/gsl>


namespace gameplay
{
namespace ext
{
template<typename TStorage>
class Image
{
public:
    using StorageType = TStorage;


    explicit Image(GLint width, GLint height, const StorageType* data = nullptr)
        : m_data{}
        , m_width{width}
        , m_height{height}
    {
        BOOST_ASSERT(width > 0 && height > 0);

        if( data == nullptr )
            m_data.resize(width * height);
        else
            m_data.assign(data, data + width * height);
    }


    ~Image() = default;


    const std::vector<StorageType>& getData() const
    {
        return m_data;
    }


    GLint getHeight() const
    {
        return m_height;
    }


    GLint getWidth() const
    {
        return m_width;
    }


    StorageType& at(GLint x, GLint y)
    {
        Expects(x >= 0);
        Expects(x < m_width);
        Expects(y >= 0);
        Expects(y < m_height);

        if( x < 0 || x >= m_width || y < 0 || y >= m_height )
        {
            BOOST_THROW_EXCEPTION(std::out_of_range{ "Image coordinates out of range" });
        }

        return m_data[y * m_width + x];
    }


    void set(GLint x, GLint y, const StorageType& pixel)
    {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height)
            return;

        m_data[y * m_width + x] = pixel;
    }


    const StorageType& at(GLint x, GLint y) const
    {
        Expects(x >= 0);
        Expects(x < m_width);
        Expects(y >= 0);
        Expects(y < m_height);

        if( x < 0 || x >= m_width || y < 0 || y >= m_height )
        {
            BOOST_THROW_EXCEPTION(std::out_of_range{ "Image coordinates out of range" });
        }

        return m_data[y * m_width + x];
    }


    void fill(const StorageType& color)
    {
        std::fill_n(m_data.data(), m_data.size(), color);
    }


    void line(GLint x0, GLint y0, GLint x1, GLint y1, const StorageType& color)
    {
        // shamelessly copied from wikipedia
        const GLint dx = abs(x1 - x0);
        const GLint sx = x0 < x1 ? 1 : -1;
        const GLint dy = -abs(y1 - y0);
        const GLint sy = y0 < y1 ? 1 : -1;

        GLint err = dx + dy;

        while( true )
        {
            set(x0, y0, color);

            if( x0 == x1 && y0 == y1 )
                break;

            auto e2 = 2 * err;
            if( e2 > dy )
            {
                err += dy;
                x0 += sx;
            }
            if( e2 < dx )
            {
                err += dx;
                y0 += sy;
            }
        }
    }


private:

    Image& operator=(const Image&) = delete;

    std::vector<StorageType> m_data;

    GLint m_width;

    GLint m_height;
};
}
}
