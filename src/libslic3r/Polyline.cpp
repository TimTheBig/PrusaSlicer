#include "BoundingBox.hpp"
#include "Polyline.hpp"
#include "Exception.hpp"
#include "ExPolygon.hpp"
#include "Line.hpp"
#include "Polygon.hpp"
#include "SVG.hpp"
#include <iostream>
#include <utility>

namespace Slic3r {

const Point& Polyline::leftmost_point() const
{
    const Point *p = &this->points.front();
    for (Points::const_iterator it = this->points.begin() + 1; it != this->points.end(); ++ it) {
        if (it->x() < p->x()) 
        	p = &(*it);
    }
    return *p;
}

double Polyline::length() const
{
    double l = 0;
    for (size_t i = 1; i < this->points.size(); ++ i)
        l += (this->points[i] - this->points[i - 1]).cast<double>().norm();
    return l;
}

Lines Polyline::lines() const
{
    Lines lines;
    if (this->points.size() >= 2) {
        lines.reserve(this->points.size() - 1);
        for (Points::const_iterator it = this->points.begin(); it != this->points.end()-1; ++it) {
            lines.push_back(Line(*it, *(it + 1)));
        }
    }
    return lines;
}

// removes the given distance from the end of the polyline
void Polyline::clip_end(double distance)
{
    while (distance > 0) {
        Vec2d  last_point = this->last_point().cast<double>();
        this->points.pop_back();
        if (this->points.empty())
            break;
        Vec2d  v    = this->last_point().cast<double>() - last_point;
        double lsqr = v.squaredNorm();
        if (lsqr > distance * distance) {
            Point p((last_point + v * (distance / sqrt(lsqr))).cast<coord_t>());
            p.nonplanar_z = this->last_point().nonplanar_z;
            this->points.emplace_back(p);
            return;
        }
        distance -= sqrt(lsqr);
    }
}

// removes the given distance from the start of the polyline
void Polyline::clip_start(double distance)
{
    this->reverse();
    this->clip_end(distance);
    if (this->points.size() >= 2)
        this->reverse();
}

void Polyline::extend_end(double distance)
{
    // relocate last point by extending the last segment by the specified length
    Vec2d v = (this->points.back() - *(this->points.end() - 2)).cast<double>().normalized();
    this->points.back() += (v * distance).cast<coord_t>();
}

void Polyline::extend_start(double distance)
{
    // relocate first point by extending the first segment by the specified length
    Vec2d v = (this->points.front() - this->points[1]).cast<double>().normalized();
    this->points.front() += (v * distance).cast<coord_t>();
}

/* this method returns a collection of points picked on the polygon contour
   so that they are evenly spaced according to the input distance */
Points Polyline::equally_spaced_points(double distance) const
{
    Points points;
    points.emplace_back(this->first_point());
    double len = 0;
    
    for (Points::const_iterator it = this->points.begin() + 1; it != this->points.end(); ++it) {
        Vec2d  p1 = (it-1)->cast<double>();
        Vec2d  v  = it->cast<double>() - p1;
        double segment_length = v.norm();
        len += segment_length;
        if (len < distance)
            continue;
        if (len == distance) {
            points.emplace_back(*it);
            len = 0;
            continue;
        }
        double take = segment_length - (len - distance);  // how much we take of this segment
        Point p((p1 + v * (take / v.norm())).cast<coord_t>());
        p.nonplanar_z = (*it).nonplanar_z;
        points.emplace_back(p);
        -- it;
        len = - take;
    }
    return points;
}

void Polyline::simplify(double tolerance)
{
    this->points = MultiPoint::douglas_peucker(this->points, tolerance);
}

#if 0
// This method simplifies all *lines* contained in the supplied area
template <class T>
void Polyline::simplify_by_visibility(const T &area)
{
    Points &pp = this->points;
    
    size_t s = 0;
    bool did_erase = false;
    for (size_t i = s+2; i < pp.size(); i = s + 2) {
        if (area.contains(Line(pp[s], pp[i]))) {
            pp.erase(pp.begin() + s + 1, pp.begin() + i);
            did_erase = true;
        } else {
            ++s;
        }
    }
    if (did_erase)
        this->simplify_by_visibility(area);
}
template void Polyline::simplify_by_visibility<ExPolygon>(const ExPolygon &area);
template void Polyline::simplify_by_visibility<ExPolygonCollection>(const ExPolygonCollection &area);
#endif

void Polyline::split_at(const Point &point, Polyline* p1, Polyline* p2) const
{
    if (this->size() < 2) {
        *p1 = *this;
        p2->clear();
        return;
    }

    if (this->points.front() == point) {
        *p1 = { point };
        *p2 = *this;
    }

    auto  min_dist2    = std::numeric_limits<double>::max();
    auto  min_point_it = this->points.cbegin();
    Point prev         = this->points.front();
    for (auto it = this->points.cbegin() + 1; it != this->points.cend(); ++ it) {
        Point proj;
        if (double d2 = line_alg::distance_to_squared(Line(prev, *it), point, &proj); d2 < min_dist2) {
	        min_dist2    = d2;
	        min_point_it = it;
        }
        prev = *it;
    }

    p1->points.assign(this->points.cbegin(), min_point_it);
    if (p1->points.back() != point)
        p1->points.emplace_back(point);
    
    p2->points = { point };
    if (*min_point_it == point)
        ++ min_point_it;
    p2->points.insert(p2->points.end(), min_point_it, this->points.cend());
}

bool Polyline::is_straight() const
{
    // Check that each segment's direction is equal to the line connecting
    // first point and last point. (Checking each line against the previous
    // one would cause the error to accumulate.)
    double dir = Line(this->first_point(), this->last_point()).direction();
    for (const auto &line: this->lines())
        if (! line.parallel_to(dir))
            return false;
    return true;
}

BoundingBox get_extents(const Polyline &polyline)
{
    return polyline.bounding_box();
}

BoundingBox get_extents(const Polylines &polylines)
{
    BoundingBox bb;
    if (! polylines.empty()) {
        bb = polylines.front().bounding_box();
        for (size_t i = 1; i < polylines.size(); ++ i)
            bb.merge(polylines[i].points);
    }
    return bb;
}

const Point& leftmost_point(const Polylines &polylines)
{
    if (polylines.empty())
        throw Slic3r::InvalidArgument("leftmost_point() called on empty Polylines");
    Polylines::const_iterator it = polylines.begin();
    const Point *p = &it->leftmost_point();
    for (++ it; it != polylines.end(); ++it) {
        const Point *p2 = &it->leftmost_point();
        if (p2->x() < p->x())
            p = p2;
    }
    return *p;
}

bool remove_degenerate(Polylines &polylines)
{
    bool modified = false;
    size_t j = 0;
    for (size_t i = 0; i < polylines.size(); ++ i) {
        if (polylines[i].points.size() >= 2) {
            if (j < i) 
                std::swap(polylines[i].points, polylines[j].points);
            ++ j;
        } else
            modified = true;
    }
    if (j < polylines.size())
        polylines.erase(polylines.begin() + j, polylines.end());
    return modified;
}

std::pair<int, Point> foot_pt(const Points &polyline, const Point &pt)
{
    if (polyline.size() < 2)
        return std::make_pair(-1, Point(0, 0));

    auto  d2_min  = std::numeric_limits<double>::max();
    Point foot_pt_min;
    Point prev = polyline.front();
    auto  it = polyline.begin();
    auto  it_proj = polyline.begin();
    for (++ it; it != polyline.end(); ++ it) {
        Point foot_pt;
        if (double d2 = line_alg::distance_to_squared(Line(prev, *it), pt, &foot_pt); d2 < d2_min) {
            d2_min      = d2;
            foot_pt_min = foot_pt;
            foot_pt_min.nonplanar_z = (*it).nonplanar_z;
            it_proj     = it;
        }
        prev = *it;
    }
    return std::make_pair(int(it_proj - polyline.begin()) - 1, foot_pt_min);
}

ThickLines ThickPolyline::thicklines() const
{
    ThickLines lines;
    if (this->points.size() >= 2) {
        lines.reserve(this->points.size() - 1);
        for (size_t i = 0; i + 1 < this->points.size(); ++ i)
            lines.emplace_back(this->points[i], this->points[i + 1], this->width[2 * i], this->width[2 * i + 1]);
    }
    return lines;
}

// Removes the given distance from the end of the ThickPolyline
void ThickPolyline::clip_end(double distance)
{
    if (! this->empty()) {
        assert(this->width.size() == (this->points.size() - 1) * 2);
        while (distance > 0) {
            Vec2d last_point = this->last_point().cast<double>();
            coord_t last_z = this->last_point().nonplanar_z;
            this->points.pop_back();
            if (this->points.empty()) {
                assert(this->width.empty());
                break;
            }
            coordf_t last_width = this->width.back();
            this->width.pop_back();

            Vec2d    vec            = this->last_point().cast<double>() - last_point;
            coordf_t width_diff     = this->width.back() - last_width;
            double   vec_length_sqr = vec.squaredNorm();
            if (vec_length_sqr > distance * distance) {
                double t = (distance / std::sqrt(vec_length_sqr));
                Point p((last_point + vec * t).cast<coord_t>());
                p.nonplanar_z = last_z;
                this->points.emplace_back(p);
                this->width.emplace_back(last_width + width_diff * t);
                assert(this->width.size() == (this->points.size() - 1) * 2);
                return;
            } else
                this->width.pop_back();

            distance -= std::sqrt(vec_length_sqr);
        }
    }
    assert(this->points.empty() ? this->width.empty() : this->width.size() == (this->points.size() - 1) * 2);
}

void ThickPolyline::start_at_index(int index)
{
    assert(index >= 0 && index < this->points.size());
    assert(this->points.front() == this->points.back() && this->width.front() == this->width.back());
    if (index != 0 && index + 1 != int(this->points.size()) && this->points.front() == this->points.back() && this->width.front() == this->width.back()) {
        this->points.pop_back();
        assert(this->points.size() * 2 == this->width.size());
        std::rotate(this->points.begin(), this->points.begin() + index, this->points.end());
        std::rotate(this->width.begin(), this->width.begin() + 2 * index, this->width.end());
        this->points.emplace_back(this->points.front());
    }
}

double Polyline3::length() const
{
    double l = 0;
    for (size_t i = 1; i < this->points.size(); ++ i)
        l += (this->points[i] - this->points[i - 1]).cast<double>().norm();
    return l;
}

Lines3 Polyline3::lines() const
{
    Lines3 lines;
    if (points.size() >= 2)
    {
        lines.reserve(points.size() - 1);
        for (Points3::const_iterator it = points.begin(); it != points.end() - 1; ++it)
        {
            lines.emplace_back(*it, *(it + 1));
        }
    }
    return lines;
}

bool export_to_svg(const char *path, const Polyline &polyline, BoundingBox bbox, const float stroke_width)
{
    SVG svg(path, bbox);
    svg.draw(polyline, "black", stroke_width);
    svg.Close();
    return true;
}

bool export_to_svg(const char *path, const Polylines &polylines, BoundingBox bbox, const float stroke_width)
{
    SVG svg(path, bbox);
    svg.draw(polylines, "black", stroke_width);
    svg.Close();
    return true;
}

bool export_to_svg(const char *path, const ThickPolylines &polylines, BoundingBox bbox, const float stroke_width)
{
    SVG svg(path, bbox);
    svg.draw(polylines, "black", stroke_width);
    svg.Close();
    return true;
}


}
