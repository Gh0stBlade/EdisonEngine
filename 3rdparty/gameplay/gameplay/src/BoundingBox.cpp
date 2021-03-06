#include "Base.h"
#include "BoundingBox.h"
#include "Plane.h"


namespace gameplay
{
    BoundingBox::BoundingBox()
    {
    }


    BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max)
    {
        set(min, max);
    }


    BoundingBox::BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        set(minX, minY, minZ, maxX, maxY, maxZ);
    }


    BoundingBox::BoundingBox(const BoundingBox& copy)
    {
        set(copy);
    }


    BoundingBox::~BoundingBox()
    {
    }


    const BoundingBox& BoundingBox::empty()
    {
        static BoundingBox b;
        return b;
    }


    void BoundingBox::getCorners(glm::vec3* dst) const
    {
        BOOST_ASSERT(dst);

        // Near face, specified counter-clockwise looking towards the origin from the positive z-axis.
        // Left-top-front.
        dst[0] = { min.x, max.y, max.z };
        // Left-bottom-front.
        dst[1] = { min.x, min.y, max.z };
        // Right-bottom-front.
        dst[2] = { max.x, min.y, max.z };
        // Right-top-front.
        dst[3] = { max.x, max.y, max.z };

        // Far face, specified counter-clockwise looking towards the origin from the negative z-axis.
        // Right-top-back.
        dst[4] = { max.x, max.y, min.z };
        // Right-bottom-back.
        dst[5] = { max.x, min.y, min.z };
        // Left-bottom-back.
        dst[6] = { min.x, min.y, min.z };
        // Left-top-back.
        dst[7] = { min.x, max.y, min.z };
    }


    glm::vec3 BoundingBox::getCenter() const
    {
        glm::vec3 center;
        getCenter(&center);
        return center;
    }


    void BoundingBox::getCenter(glm::vec3* dst) const
    {
        BOOST_ASSERT(dst);

        *dst = glm::mix(min, max, 0.5f);
    }


    bool BoundingBox::intersects(const BoundingBox& box) const
    {
        return ((min.x >= box.min.x && min.x <= box.max.x) || (box.min.x >= min.x && box.min.x <= max.x)) &&
            ((min.y >= box.min.y && min.y <= box.max.y) || (box.min.y >= min.y && box.min.y <= max.y)) &&
            ((min.z >= box.min.z && min.z <= box.max.z) || (box.min.z >= min.z && box.min.z <= max.z));
    }


    bool BoundingBox::intersects(const Frustum& frustum) const
    {
        // The box must either intersect or be in the positive half-space of all six planes of the frustum.
        return (intersects(frustum.getNear()) != Plane::INTERSECTS_BACK &&
            intersects(frustum.getFar()) != Plane::INTERSECTS_BACK &&
            intersects(frustum.getLeft()) != Plane::INTERSECTS_BACK &&
            intersects(frustum.getRight()) != Plane::INTERSECTS_BACK &&
            intersects(frustum.getBottom()) != Plane::INTERSECTS_BACK &&
            intersects(frustum.getTop()) != Plane::INTERSECTS_BACK);
    }


    int BoundingBox::intersects(const Plane& plane) const
    {
        // Calculate the distance from the center of the box to the plane.
        glm::vec3 center((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f);
        float distance = plane.distance(center);

        // Get the extents of the box from its center along each axis.
        const auto extent = (max - min) * 0.5f;

        const glm::vec3& planeNormal = plane.getNormal();
        if( fabsf(distance) <= (fabsf(extent.x * planeNormal.x) + fabsf(extent.y * planeNormal.y) + fabsf(extent.z * planeNormal.z)) )
        {
            return Plane::INTERSECTS_INTERSECTING;
        }

        return (distance > 0.0f) ? Plane::INTERSECTS_FRONT : Plane::INTERSECTS_BACK;
    }


    bool BoundingBox::intersects(const Ray& ray) const
    {
        // Intermediate calculation variables.
        float dnear = 0.0f;
        float dfar = 0.0f;

        const glm::vec3& origin = ray.getOrigin();
        const glm::vec3& direction = ray.getDirection();

        // X direction.
        float div = 1.0f / direction.x;
        if( div >= 0.0f )
        {
            dnear = (min.x - origin.x) * div;
            dfar = (max.x - origin.x) * div;
        }
        else
        {
            dnear = (max.x - origin.x) * div;
            dfar = (min.x - origin.x) * div;
        }

        // Check if the ray misses the box.
        if( dnear > dfar || dfar < 0.0f )
        {
            return false;
        }

        // Y direction.
        div = 1.0f / direction.y;
        float tmin = 0.0f;
        float tmax = 0.0f;
        if( div >= 0.0f )
        {
            tmin = (min.y - origin.y) * div;
            tmax = (max.y - origin.y) * div;
        }
        else
        {
            tmin = (max.y - origin.y) * div;
            tmax = (min.y - origin.y) * div;
        }

        // Update the near and far intersection distances.
        if( tmin > dnear )
        {
            dnear = tmin;
        }
        if( tmax < dfar )
        {
            dfar = tmax;
        }
        // Check if the ray misses the box.
        if( dnear > dfar || dfar < 0.0f )
        {
            return false;
        }

        // Z direction.
        div = 1.0f / direction.z;
        if( div >= 0.0f )
        {
            tmin = (min.z - origin.z) * div;
            tmax = (max.z - origin.z) * div;
        }
        else
        {
            tmin = (max.z - origin.z) * div;
            tmax = (min.z - origin.z) * div;
        }

        // Update the near and far intersection distances.
        if( tmin > dnear )
        {
            dnear = tmin;
        }
        if( tmax < dfar )
        {
            dfar = tmax;
        }

        // Check if the ray misses the box.
        if( dnear > dfar || dfar < 0.0f )
        {
            return false;
        }
        // The ray intersects the box (and since the direction of a Ray is normalized, dnear is the distance to the ray).
        return true;
    }


    bool BoundingBox::isEmpty() const
    {
        return min.x == max.x && min.y == max.y && min.z == max.z;
    }


    void BoundingBox::merge(const BoundingBox& box)
    {
        // Calculate the new minimum point.
        min.x = std::min(min.x, box.min.x);
        min.y = std::min(min.y, box.min.y);
        min.z = std::min(min.z, box.min.z);

        // Calculate the new maximum point.
        max.x = std::max(max.x, box.max.x);
        max.y = std::max(max.y, box.max.y);
        max.z = std::max(max.z, box.max.z);
    }


    void BoundingBox::set(const glm::vec3& min, const glm::vec3& max)
    {
        this->min = min;
        this->max = max;
    }


    void BoundingBox::set(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        min = { minX, minY, minZ };
        max = { maxX, maxY, maxZ };
    }


    static void updateMinMax(glm::vec3* point, glm::vec3* min, glm::vec3* max)
    {
        BOOST_ASSERT(point);
        BOOST_ASSERT(min);
        BOOST_ASSERT(max);

        // Leftmost point.
        if( point->x < min->x )
        {
            min->x = point->x;
        }

        // Rightmost point.
        if( point->x > max->x )
        {
            max->x = point->x;
        }

        // Lowest point.
        if( point->y < min->y )
        {
            min->y = point->y;
        }

        // Highest point.
        if( point->y > max->y )
        {
            max->y = point->y;
        }

        // Farthest point.
        if( point->z < min->z )
        {
            min->z = point->z;
        }

        // Nearest point.
        if( point->z > max->z )
        {
            max->z = point->z;
        }
    }


    void BoundingBox::set(const BoundingBox& box)
    {
        min = box.min;
        max = box.max;
    }


    void BoundingBox::transform(const glm::mat4& matrix)
    {
        // Calculate the corners.
        glm::vec3 corners[8];
        getCorners(corners);

        // Transform the corners, recalculating the min and max points along the way.
        corners[0] = glm::vec3(matrix * glm::vec4(corners[0], 1.0f));
        glm::vec3 newMin = corners[0];
        glm::vec3 newMax = corners[0];
        for( int i = 1; i < 8; i++ )
        {
            corners[i] = glm::vec3(matrix * glm::vec4(corners[i], 1));
            updateMinMax(&corners[i], &newMin, &newMax);
        }
        this->min.x = newMin.x;
        this->min.y = newMin.y;
        this->min.z = newMin.z;
        this->max.x = newMax.x;
        this->max.y = newMax.y;
        this->max.z = newMax.z;
    }
}
