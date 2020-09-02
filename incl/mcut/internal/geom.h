#ifndef MCUT_GEOM_H_
#define MCUT_GEOM_H_

#include "mcut/internal/math.h"

namespace mcut {
namespace geom {

    template <typename vector_type>
    struct bounding_box_t {

        vector_type m_minimum;
        vector_type m_maximum;

        bounding_box_t(const vector_type& minimum, const vector_type& maximum)
        {
            m_minimum = minimum;
            m_maximum = maximum;
        }

        bounding_box_t()
        {
            m_minimum = vector_type(std::numeric_limits<double>::max());
            m_maximum = vector_type(std::numeric_limits<double>::min());
        }

        const vector_type& minimum() const
        {
            return m_minimum;
        }

        const vector_type& maximum() const
        {
            return m_maximum;
        }

        void expand(const vector_type& point)
        {
            m_maximum = compwise_max(m_maximum, point);
            m_minimum = compwise_min(m_minimum, point);
        }

        void expand(const bounding_box_t<vector_type>& bbox)
        {
            m_maximum = compwise_max(m_maximum, bbox.maximum());
            m_minimum = compwise_min(m_minimum, bbox.minimum());
        }
    };

    bool intersect_bounding_boxes(const bounding_box_t<math::vec3>& a, const bounding_box_t<math::vec3>& b)
    {
        return (a.minimum().x() <= b.maximum().x() && a.maximum().x() >= b.minimum().x()) && (a.minimum().y() <= b.maximum().y() && a.maximum().y() >= b.minimum().y()) && (a.minimum().z() <= b.maximum().z() && a.maximum().z() >= b.minimum().z());
    }

    bool point_in_bounding_box(const math::vec2& point, const bounding_box_t<math::vec2>& bbox)
    {
        if ((point.x() < bbox.m_minimum.x() || point.x() > bbox.m_maximum.x()) || //
            (point.y() < bbox.m_minimum.y() || point.y() > bbox.m_maximum.y())) {
            return false;
        } else {
            return true;
        }
    }

    bool point_in_bounding_box(const math::vec3& point, const bounding_box_t<math::vec3>& bbox)
    {
        if ((point.x() < bbox.m_minimum.x() || point.x() > bbox.m_maximum.x()) || //
            (point.y() < bbox.m_minimum.y() || point.y() > bbox.m_maximum.y()) || //
            (point.z() < bbox.m_minimum.z() || point.z() > bbox.m_maximum.z())) { //
            return false;
        } else {
            return true;
        }
    }

    // http://cs.haifa.ac.il/~gordon/plane.pdf
    void polygon_normal(math::vec3& normal, const math::vec3* vertices, const int num_vertices)
    {
        normal = math::vec3(0);
        for (int i = 0; i < num_vertices; ++i) {
            normal = normal + cross_product(vertices[i] - vertices[0], vertices[(i + 1) % num_vertices] - vertices[0]);
        }
        normal = normalize(normal);
    }

    void polygon_plane_d_param(math::real_t& d, const math::vec3& plane_normal, const math::vec3* polygon_vertices, const int num_polygon_vertices)
    {
        math::real_t frac = 1.0 / num_polygon_vertices;
        math::vec3 p = polygon_vertices[0];
        for (int i = 1; i < num_polygon_vertices; ++i) {
            p = p + polygon_vertices[i];
        }

        p = p * frac;

        d = (dot_product(p, plane_normal));
    }

    void project_point(math::vec2& projected_point, const math::vec3& point, const math::vec3& u, const math::vec3& v)
    {
        MCUT_ASSERT(length(u) > 0);
        MCUT_ASSERT(length(v) > 0);

        projected_point = math::vec2(dot_product(point, u), dot_product(point, v));
    }

    void polygon_span(math::vec3& span, math::real_t& span_length, const math::vec3* vertices, const int num_vertices)
    {
        MCUT_ASSERT(num_vertices >= 3);

        span = math::vec3(0);
        span_length = 0;

        for (int i = 0; i < num_vertices; ++i) {
            for (int j = 0; j < i; ++j) {
                const math::vec3 diff = vertices[i] - vertices[j];
                const math::real_t len = length(diff);
                if (len > span_length) {
                    span_length = len;
                    span = diff;
                }
            }
        }
    }

    inline long direction(const math::vec2& a, const math::vec2& b, const math::vec2& c)
    {
        math::real_t acx = a.x() - c.x();
        math::real_t bcx = b.x() - c.x();
        math::real_t acy = a.y() - c.y();
        math::real_t bcy = b.y() - c.y();
        math::real_t result = acx * bcy - acy * bcx;
        return (result == 0 ? 0 : (result > 0 ? 1 : -1));
    }

    enum sign_t {
        ON_NEGATIVE_SIDE = -1, // left
        ON_ORIENTED_BOUNDARY = 0, // on boundary
        ON_POSITIVE_SIDE = 1 // right
    };

    typedef sign_t orientation_t;

    // on which side does "c" lie on the left of, or right right of "ab"

    inline orientation_t orientation(const math::vec2& a, const math::vec2& b, const math::vec2& c)
    {
        const long result = direction(a, b, c);

        sign_t side = ON_ORIENTED_BOUNDARY;

        if (result > 0) {
            side = ON_POSITIVE_SIDE;
        } else if (result < 0) {
            side = ON_NEGATIVE_SIDE;
        }
        return side;
    }

    bool point_on_segment(const math::vec2& pi, const math::vec2& pj, const math::vec2& pk)
    {
        return (math::min(pi.x(), pj.x()) <= pk.x() && pk.x() <= math::max(pi.x(), pj.x()) && math::min(pi.y(), pj.y()) <= pk.y() && pk.y() <= math::max(pi.y(), pj.y()));
    }

    // segment 0: ab
    // segment 1: cd
    bool segments_intersect(const math::vec2& a, const math::vec2& b, const math::vec2& c, const math::vec2& d)
    {
        orientation_t orient_a = orientation(c, d, a);
        orientation_t orient_b = orientation(c, d, b);
        orientation_t orient_c = orientation(a, b, c);
        orientation_t orient_d = orientation(a, b, d);

        bool result = false;

        if (((orient_a == ON_POSITIVE_SIDE && orient_b == ON_NEGATIVE_SIDE) || (orient_a == ON_NEGATIVE_SIDE && orient_b == ON_POSITIVE_SIDE)) && //
            ((orient_c == ON_POSITIVE_SIDE && orient_d == ON_NEGATIVE_SIDE) || (orient_c == ON_NEGATIVE_SIDE && orient_d == ON_POSITIVE_SIDE))) {
            result = true;
        } else if (orient_a == ON_ORIENTED_BOUNDARY && point_on_segment(c, d, a)) {
            result = true;
        } else if (orient_b == ON_ORIENTED_BOUNDARY && point_on_segment(c, d, b)) {
            result = true;
        } else if (orient_c == ON_ORIENTED_BOUNDARY && point_on_segment(a, b, c)) {
            result = true;
        } else if (orient_d == ON_ORIENTED_BOUNDARY && point_on_segment(a, b, d)) {
            result = true;
        }

        return result;
    }

    template <typename vector_type>
    void make_bbox(bounding_box_t<vector_type>& bbox, const vector_type* vertices, const int num_vertices)
    {
        MCUT_ASSERT(vertices != nullptr);
        MCUT_ASSERT(num_vertices >= 3);

        for (int i = 0; i < num_vertices; ++i) {
            const vector_type& vertex = vertices[i];
            bbox.expand(vertex);
        }
    }

    // returns true if point lies [inside] the given polygon.
    // returns false if point lies on the [border] or [outside] the polygon

    bool point_in_polygon(const math::vec3& point, const math::vec3& polygon_normal, const math::vec3* vertices, const int num_vertices)
    {
        MCUT_ASSERT(vertices != nullptr);
        MCUT_ASSERT(num_vertices >= 3);

        // bounding_box_t<math::vec3 > bbox3;
        // make_bbox(bbox3, vertices, num_vertices);

        //if (!point_in_bounding_box(point, bbox3)) { // TODO: this check should be done outside of this function
        //    return false;
        //}

        std::unique_ptr<math::vec2[]> projected_vertices = std::unique_ptr<math::vec2[]>(new math::vec2[num_vertices]);

        //math::vec3 normal;
        // polygon_normal(normal, vertices, num_vertices);

        math::vec3 span;
        math::real_t span_length;
        polygon_span(span, span_length, vertices, num_vertices);

        const math::vec3 u = normalize(span); // first unit vector on the plane
        const math::vec3 v = normalize(cross_product(u, polygon_normal)); // second unit vector just calculate

        for (int i = 0; i < num_vertices; ++i) {
            project_point(projected_vertices[i], vertices[i], u, v);
        }

        bounding_box_t<math::vec2> bbox2;
        make_bbox(bbox2, projected_vertices.get(), num_vertices);

        math::vec2 projected_point;
        project_point(projected_point, point, u, v);

        if (!point_in_bounding_box(projected_point, bbox2)) {
            return false;
        }

        //create a ray (segment) starting from the given point,
        // and to the point outside of polygon.
        math::vec2 outside(bbox2.m_minimum.x() - 1, bbox2.m_minimum.y());

        int intersections = 0;

        // check intersections between the ray and every side of the polygon.
        for (int i = 0; i < num_vertices - 1; ++i) {
            if (segments_intersect(projected_point, outside, projected_vertices[i], projected_vertices[i + 1])) {
                intersections++;
            }
        }

        // check the last line
        if (segments_intersect(projected_point, outside, projected_vertices[num_vertices - 1], projected_vertices[0])) {
            intersections++;
        }

        return (intersections % 2 != 0);
    }

    bool intersect_plane_with_segment(
        math::vec3& point,
        math::real_t& t,
        // plane
        const math::vec3& normal,
        math::real_t& distance,
        // segment
        const math::vec3& source,
        const math::vec3& target)
    {
        MCUT_ASSERT(!(target == source));
        MCUT_ASSERT(length(normal) != 0);

        // Compute the t value for the directed line ab intersecting the plane
        const math::vec3 dir = target - source;
        const math::real_t nom = (distance - dot_product(normal, source));
        const math::real_t denom = dot_product(normal, dir);
        bool result = false;

#if defined(USE_NATIVE_FLOATING_POINT_NUMBERS)
        if (!(0 == denom)) {
#else
        if (!math::real_t::is_zero(denom)) {
#endif // #if defined(USE_NATIVE_FLOATING_POINT_NUMBERS)
            t = nom / denom;
            // If t in [0..1] compute and return intersection point
            if (t >= 0.0 && t <= 1.0) {
                point = source + dir * t;
                result = true;
            }
        }

        return result;
    }

} // namespace geom {
} // namespace mcut {

#endif // MCUT_GEOM_H_