#include "Draw.h"

#include "Environment.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace Cupertino::Draw {
    ImVec2 Normalized(ImVec2 v) {
        return v * ImInvLength(v, 0.0f);
    }

    int ArcSegments(float sweep, int minimum) {
        return ImMax(minimum, int(ImCeil(ImFabs(sweep) / (2.0f * IM_PI) * 96.0f)));
    }

    // A corner of a triangle: the unit directions along its two sides, the unit bisector into the triangle and half the
    // angle between the sides.
    struct TriangleCorner {
        ImVec2 toPrevious;
        ImVec2 toNext;
        ImVec2 bisector;
        float halfAngle = 0.0f;
    };

    static TriangleCorner CornerOf(const ImVec2 (&corners)[3], int i) {
        TriangleCorner corner;
        corner.toPrevious = Normalized(corners[(i + 2) % 3] - corners[i]);
        corner.toNext = Normalized(corners[(i + 1) % 3] - corners[i]);
        corner.bisector = Normalized(corner.toPrevious + corner.toNext);
        corner.halfAngle = std::acos(ImClamp(ImDot(corner.toPrevious, corner.toNext), -1.0f, 1.0f)) * 0.5f;
        return corner;
    }

    void OffsetTriangle(const ImVec2 (&corners)[3], float distance, ImVec2 (&out)[3]) {
        for (int i = 0; i < 3; ++i) {
            const TriangleCorner corner = CornerOf(corners, i);
            out[i] = corners[i] - corner.bisector * (distance / std::sin(corner.halfAngle));
        }
    }

    // Apple's continuous corner (UIBezierPath since iOS 7): three cubic pieces spanning 1.52866483 r of each edge.
    // Points are in units of the radius as (distance back along the incoming edge, distance along the outgoing edge).
    static constexpr float ContinuousExtent = 1.52866483f;
    static const ImVec2 ContinuousCurve[10] = {
        {ContinuousExtent, 0.0f},
        {1.08849323f, 0.0f},
        {0.86840689f, 0.0f},
        {0.63149399f, 0.07491100f},
        {0.37282392f, 0.16905899f},
        {0.16905899f, 0.37282392f},
        {0.07491100f, 0.63149399f},
        {0.0f, 0.86840689f},
        {0.0f, 1.08849323f},
        {0.0f, ContinuousExtent},
    };

    static ImVec2 CubicPoint(ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, float t) {
        const float u = 1.0f - t;
        return p0 * (u * u * u) + p1 * (3.0f * u * u * t) + p2 * (3.0f * u * t * t) + p3 * (t * t * t);
    }

    // Vertices per corner for a radius in pixels: a multiple of three pieces plus the start point.
    static int CornerPointsFor(float radius) {
        if (radius < 0.5f)
            return 1;
        return ImClamp(int(ImCeil(radius / 4.0f)), 1, 6) * 3 + 1;
    }

    // A corner of radius 1 by its vertex count, as (distance back along the incoming edge, distance along the outgoing
    // edge): the continuous curve in its three pieces or the quarter circle, computed once for each count.
    static const ImVector<ImVec2>& UnitCorner(bool continuous, int corner_points) {
        static ImVector<ImVec2> corners[2][20];
        ImVector<ImVec2>& corner = corners[continuous ? 1 : 0][corner_points];
        if (!corner.empty())
            return corner;
        if (continuous) {
            const int segments = corner_points - 1;
            corner.push_back(ContinuousCurve[0]);
            for (int piece = 0; piece < 3; ++piece) {
                const int piece_segments = segments / 3 + (piece < segments % 3 ? 1 : 0);
                const ImVec2* p = &ContinuousCurve[piece * 3];
                for (int step = 1; step <= piece_segments; ++step)
                    corner.push_back(CubicPoint(p[0], p[1], p[2], p[3], float(step) / float(piece_segments)));
            }
        } else {
            for (int i = 0; i < corner_points; ++i) {
                const float angle = IM_PI * 0.5f * float(i) / float(corner_points - 1);
                corner.push_back(ImVec2(1.0f - ImSin(angle), 1.0f - ImCos(angle)));
            }
        }
        return corner;
    }

    // The corner vertex, the unit direction back along the incoming edge and the one along the outgoing edge.
    // limit is half the shorter side: a continuous corner that does not fit falls back to a circular one.
    static void AppendCorner(ImVector<ImVec2>& out, ImVec2 vertex, ImVec2 back, ImVec2 ahead, float radius, float limit, CornerStyle style, int corner_points, bool collapse_sharp) {
        if (radius < 0.25f || corner_points <= 1) {
            for (int i = 0; i < (collapse_sharp ? 1 : corner_points); ++i)
                out.push_back(vertex);
            return;
        }
        const bool continuous = style == CornerStyle::Continuous && radius * ContinuousExtent <= limit && corner_points >= 4;
        if (!continuous)
            radius = ImMin(radius, limit);
        for (const ImVec2& local : UnitCorner(continuous, corner_points))
            out.push_back(vertex + back * (local.x * radius) + ahead * (local.y * radius));
    }

    static void Contour(ImVector<ImVec2>& out, const ImRect& rect, const CornerRadii& radii, CornerStyle style, int corner_points, bool collapse_sharp) {
        const float limit = ImMax(0.0f, ImMin(rect.GetWidth(), rect.GetHeight()) * 0.5f);
        AppendCorner(out, rect.Min, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), radii.topLeft, limit, style, corner_points, collapse_sharp);
        AppendCorner(out, ImVec2(rect.Max.x, rect.Min.y), ImVec2(-1.0f, 0.0f), ImVec2(0.0f, 1.0f), radii.topRight, limit, style, corner_points, collapse_sharp);
        AppendCorner(out, rect.Max, ImVec2(0.0f, -1.0f), ImVec2(-1.0f, 0.0f), radii.bottomRight, limit, style, corner_points, collapse_sharp);
        AppendCorner(out, ImVec2(rect.Min.x, rect.Max.y), ImVec2(1.0f, 0.0f), ImVec2(0.0f, -1.0f), radii.bottomLeft, limit, style, corner_points, collapse_sharp);
    }

    void RoundedRectContour(ImVector<ImVec2>& out, const ImRect& rect, const CornerRadii& radii, CornerStyle style, int corner_points) {
        Contour(out, rect, radii, style, corner_points, false);
    }

    float CornerReach(float radius, CornerStyle style) {
        return style == CornerStyle::Continuous ? radius * ContinuousExtent : radius;
    }

    static ImVector<ImVec2>& Scratch() {
        static ImVector<ImVec2> points;
        points.resize(0);
        return points;
    }

    void RemoveDuplicates(ImVector<ImVec2>& points) {
        int count = 0;
        for (int i = 0; i < points.Size; ++i) {
            if (count > 0 && ImLengthSqr(points[i] - points[count - 1]) < 1e-4f)
                continue;
            points[count++] = points[i];
        }
        while (count > 1 && ImLengthSqr(points[count - 1] - points[0]) < 1e-4f)
            --count;
        points.resize(count);
    }

    static void FillPacked(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, ImU32 color, CornerStyle style) {
        ImVector<ImVec2>& points = Scratch();
        Contour(points, rect, radii, style, CornerPointsFor(radii.Largest()), true);
        RemoveDuplicates(points);
        draw->AddConvexPolyFilled(points.Data, points.Size, color);
    }

    void FillRect(ImDrawList* draw, const ImRect& rect, Rgba color) {
        if (color.a > 0.0f)
            draw->AddRectFilled(rect.Min, rect.Max, color.Packed());
    }

    void FillRoundedRect(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba color, CornerStyle style) {
        if (color.a <= 0.0f || rect.GetWidth() <= 0.0f || rect.GetHeight() <= 0.0f)
            return;
        FillPacked(draw, rect, radii, color.Packed(), style);
    }

    void StrokeRoundedRect(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba color, float thickness, StrokeAlignment alignment, CornerStyle style) {
        if (color.a <= 0.0f || thickness <= 0.0f)
            return;
        const float inset = alignment == StrokeAlignment::Inside ? thickness * 0.5f : alignment == StrokeAlignment::Outside ? -thickness * 0.5f : 0.0f;
        const ImRect path(rect.Min + ImVec2(inset, inset), rect.Max - ImVec2(inset, inset));
        const CornerRadii path_radii = radii.Offset(-inset);
        // ImGui draws anything thinner than a pixel one pixel wide, so the alpha carries the coverage instead.
        if (thickness < 1.0f) {
            color = color.Opacity(thickness);
            thickness = 1.0f;
        }
        ImVector<ImVec2>& points = Scratch();
        Contour(points, path, path_radii, style, CornerPointsFor(path_radii.Largest()), true);
        RemoveDuplicates(points);
        draw->AddPolyline(points.Data, points.Size, color.Packed(), thickness, ImDrawFlags_Closed);
    }

    void Image(ImDrawList* draw, const ImRect& rect, const Bitmap& bitmap, float radius, float opacity) {
        if (bitmap.IsEmpty())
            return;
        draw->AddImageRounded(bitmap.texture, rect.Min, rect.Max, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), Rgba::White(opacity).Packed(), radius);
    }

    void FillVerticalGradient(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba top, Rgba bottom, CornerStyle style) {
        if ((top.a <= 0.0f && bottom.a <= 0.0f) || rect.GetHeight() <= 0.0f)
            return;
        const int first = draw->VtxBuffer.Size;
        FillPacked(draw, rect, radii, IM_COL32_WHITE, style);
        // The white fill leaves anti-aliasing coverage in the vertex alpha; the gradient color is scaled by it.
        for (int i = first; i < draw->VtxBuffer.Size; ++i) {
            ImDrawVert& vertex = draw->VtxBuffer[i];
            const float t = ImSaturate((vertex.pos.y - rect.Min.y) / rect.GetHeight());
            const float coverage = float((vertex.col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;
            vertex.col = Blend::Mix(top, bottom, t).Opacity(coverage).Packed();
        }
    }

    void FillRoundedTriangle(ImDrawList* draw, const ImVec2 (&corners)[3], const float (&radii)[3], Rgba top, Rgba bottom) {
        for (int i = 0; i < 3; ++i) {
            const TriangleCorner corner = CornerOf(corners, i);
            const ImVec2 center = corners[i] + corner.bisector * (radii[i] / std::sin(corner.halfAngle));
            const ImVec2 from = corners[i] + corner.toPrevious * (radii[i] / std::tan(corner.halfAngle)) - center;
            const ImVec2 to = corners[i] + corner.toNext * (radii[i] / std::tan(corner.halfAngle)) - center;
            const float start = std::atan2(from.y, from.x);
            float sweep = std::atan2(to.y, to.x) - start;
            sweep = sweep > IM_PI ? sweep - 2.0f * IM_PI : sweep < -IM_PI ? sweep + 2.0f * IM_PI : sweep;
            draw->PathArcTo(center, radii[i], start, start + sweep, 10);
        }
        float top_y = FLT_MAX;
        float bottom_y = -FLT_MAX;
        for (const ImVec2& point : draw->_Path) {
            top_y = ImMin(top_y, point.y);
            bottom_y = ImMax(bottom_y, point.y);
        }
        const int first = draw->VtxBuffer.Size;
        draw->PathFillConvex(IM_COL32_WHITE);
        ShadeVertices(draw, first, [&](ImVec2 point) { return Blend::Mix(top, bottom, ImSaturate((point.y - top_y) / ImMax(bottom_y - top_y, 1.0f))); });
    }

    void FillCircle(ImDrawList* draw, ImVec2 center, float radius, Rgba color) {
        FillRoundedRect(draw, ImRect(center - ImVec2(radius, radius), center + ImVec2(radius, radius)), CornerRadii(radius), color, CornerStyle::Circular);
    }

    void FillCapsule(ImDrawList* draw, const ImRect& rect, Rgba color) {
        const ImRect capsule(rect.Min, ImVec2(ImMax(rect.Max.x, rect.Min.x + rect.GetHeight()), rect.Max.y));
        FillRoundedRect(draw, capsule, CornerRadii(capsule.GetHeight() * 0.5f), color, CornerStyle::Circular);
    }

    void StrokeCircle(ImDrawList* draw, ImVec2 center, float radius, Rgba color, float thickness, StrokeAlignment alignment) {
        StrokeRoundedRect(draw, ImRect(center - ImVec2(radius, radius), center + ImVec2(radius, radius)), CornerRadii(radius), color, thickness, alignment, CornerStyle::Circular);
    }

    // Normal distribution function: the share of a gaussian-blurred edge that lies below z sigmas.
    static float NormalCdf(float z) {
        return 0.5f * (1.0f + std::erf(z * 0.70710678f));
    }

    // Writes the triangles joining consecutive rings of ring_size vertices each.
    static void WriteRingStrips(ImDrawList* draw, unsigned first_index, int rings, int ring_size) {
        for (int ring = 0; ring + 1 < rings; ++ring) {
            const unsigned inner = first_index + unsigned(ring * ring_size);
            const unsigned outer = inner + unsigned(ring_size);
            for (int i = 0; i < ring_size; ++i) {
                const unsigned next = unsigned((i + 1) % ring_size);
                draw->PrimWriteIdx(ImDrawIdx(inner + unsigned(i)));
                draw->PrimWriteIdx(ImDrawIdx(inner + next));
                draw->PrimWriteIdx(ImDrawIdx(outer + next));
                draw->PrimWriteIdx(ImDrawIdx(inner + unsigned(i)));
                draw->PrimWriteIdx(ImDrawIdx(outer + next));
                draw->PrimWriteIdx(ImDrawIdx(outer + unsigned(i)));
            }
        }
    }

    // Signed distance from a point to a rounded rectangle with circular corners (negative inside).
    static float RoundedRectDistance(ImVec2 point, const ImRect& rect, const CornerRadii& radii) {
        const ImVec2 center = rect.GetCenter();
        const ImVec2 half = rect.GetSize() * 0.5f;
        const float corner = point.x < center.x ? (point.y < center.y ? radii.topLeft : radii.bottomLeft) : (point.y < center.y ? radii.topRight : radii.bottomRight);
        const float radius = ImMin(corner, ImMin(half.x, half.y));
        const ImVec2 q(ImFabs(point.x - center.x) - half.x + radius, ImFabs(point.y - center.y) - half.y + radius);
        const ImVec2 outside(ImMax(q.x, 0.0f), ImMax(q.y, 0.0f));
        return ImSqrt(outside.x * outside.x + outside.y * outside.y) + ImMin(ImMax(q.x, q.y), 0.0f) - radius;
    }

    // Adds points along the four straight edges of a contour, 1, 2 and 3 sigmas from each end and in the middle: the
    // separable blur of a rectangle fades within a few sigmas of a corner, which the corners' own vertices would
    // interpolate straight across the edge.
    static void SubdivideEdges(ImVector<ImVec2>& points, int corner_points, float sigma) {
        static ImVector<ImVec2> out;
        out.resize(0);
        const float steps[] = {1.0f, 2.0f, 3.0f};
        for (int corner = 0; corner < 4; ++corner) {
            for (int i = 0; i < corner_points; ++i)
                out.push_back(points[corner * corner_points + i]);
            const ImVec2 from = points[corner * corner_points + corner_points - 1];
            const ImVec2 to = points[((corner + 1) % 4) * corner_points];
            const float length = ImSqrt(ImLengthSqr(to - from));
            const ImVec2 direction = Normalized(to - from);
            for (const float step : steps)
                out.push_back(from + direction * ImMin(step * sigma, length * 0.5f));
            out.push_back(from + direction * (length * 0.5f));
            for (int i = IM_ARRAYSIZE(steps) - 1; i >= 0; --i)
                out.push_back(to - direction * ImMin(steps[i] * sigma, length * 0.5f));
        }
        points.swap(out);
    }

    // A shadow's rings relative to its rectangle, with the coverage at each vertex: the same size, corners and blur give
    // the same mesh, so it is built once and then only moved and colored.
    struct ShadowMesh {
        float key[13] = {};
        ImVector<ImVec2> points;
        ImVector<float> coverage;
        int rings = 0;
        int ringSize = 0;
        int lastFrame = 0;
    };

    // Blurred shadows are sampled on rings of the shape grown by d. While the blur is smaller than the corner radius,
    // the falloff depends on the distance to the edge; beyond that the corners matter little and the exact
    // separable blur of a rectangle is used instead.
    static void BuildShadow(ShadowMesh& mesh, ImVec2 size, const CornerRadii& radii, float sigma, float spread, ImVec2 offset, CornerStyle style, bool cut_out) {
        const ImRect rect(ImVec2(0.0f, 0.0f), size);
        const ImRect base(offset - ImVec2(spread, spread), size + offset + ImVec2(spread, spread));
        const CornerRadii base_radii = radii.Offset(spread);
        const float half = ImMin(base.GetWidth(), base.GetHeight()) * 0.5f;
        mesh.points.resize(0);
        mesh.coverage.resize(0);
        mesh.rings = 0;
        if (half <= 0.0f)
            return;

        const float inner = -ImMin(2.5f * sigma, ImMax(half - 0.5f, 0.0f));
        const float outer = 3.0f * sigma;
        const int rings = ImClamp(int(ImCeil((outer - inner) / ImMax(sigma / 3.0f, 0.5f))), 2, 32) + 1;
        const int corner_points = CornerPointsFor(base_radii.Largest() + outer);
        const bool separable = sigma > base_radii.Largest();
        mesh.rings = rings;
        mesh.ringSize = (corner_points + (separable ? 7 : 0)) * 4;
        ImVector<ImVec2>& points = Scratch();
        for (int ring = 0; ring < rings; ++ring) {
            const float d = ImLerp(inner, outer, float(ring) / float(rings - 1));
            points.resize(0);
            RoundedRectContour(points, ImRect(base.Min - ImVec2(d, d), base.Max + ImVec2(d, d)), base_radii.Offset(d), style, corner_points);
            if (separable)
                SubdivideEdges(points, corner_points, sigma);
            const float ring_coverage = NormalCdf(-d / sigma);
            for (const ImVec2& point : points) {
                float coverage = ring_coverage;
                if (separable) {
                    const float x = NormalCdf((point.x - base.Min.x) / sigma) - NormalCdf((point.x - base.Max.x) / sigma);
                    const float y = NormalCdf((point.y - base.Min.y) / sigma) - NormalCdf((point.y - base.Max.y) / sigma);
                    coverage = x * y;
                }
                if (cut_out)
                    coverage *= ImSaturate(RoundedRectDistance(point, rect, radii) + 0.5f);
                mesh.points.push_back(point);
                mesh.coverage.push_back(ImSaturate(coverage));
            }
        }
    }

    static void BuildInnerShadow(ShadowMesh& mesh, ImVec2 size, const CornerRadii& radii, float sigma, float spread, ImVec2 offset, CornerStyle style);

    // The mesh for a drop or an inner shadow, from the cache when the same one was drawn lately; meshes unused for two
    // seconds go.
    static const ShadowMesh& CachedShadow(bool inner, ImVec2 size, const CornerRadii& radii, float sigma, float spread, ImVec2 offset, CornerStyle style, bool cut_out) {
        static std::unordered_map<ImGuiID, ShadowMesh> meshes;
        static int swept = 0;
        const int frame = ImGui::GetFrameCount();
        if (frame - swept > 60) {
            std::erase_if(meshes, [&](const auto& entry) { return frame - entry.second.lastFrame > 120; });
            swept = frame;
        }
        const float key[13] = {size.x, size.y, radii.topLeft, radii.topRight, radii.bottomRight, radii.bottomLeft, sigma, spread, offset.x, offset.y, float(style), cut_out ? 1.0f : 0.0f, inner ? 1.0f : 0.0f};
        ShadowMesh& mesh = meshes[ImHashData(key, sizeof(key))];
        if (mesh.lastFrame == 0 || std::memcmp(mesh.key, key, sizeof(key)) != 0) {
            std::memcpy(mesh.key, key, sizeof(key));
            if (inner)
                BuildInnerShadow(mesh, size, radii, sigma, spread, offset, style);
            else
                BuildShadow(mesh, size, radii, sigma, spread, offset, style, cut_out);
        }
        mesh.lastFrame = frame;
        return mesh;
    }

    // Writes a cached mesh at origin in the shadow's color, its alpha times each vertex's coverage as Rgba::Packed gives
    // for color.Opacity(coverage); a drop shadow's innermost ring is filled.
    static void WriteShadow(ImDrawList* draw, ImVec2 origin, const ShadowMesh& mesh, Rgba color, bool filled) {
        const int ring_size = mesh.ringSize;
        draw->PrimReserve((mesh.rings - 1) * ring_size * 6 + (filled ? (ring_size - 2) * 3 : 0), mesh.points.Size);
        const ImVec2 uv = draw->_Data->TexUvWhitePixel;
        const unsigned first_index = draw->_VtxCurrentIdx;
        const ImU32 rgb = color.Packed() & ~IM_COL32_A_MASK;
        const float alpha = ImSaturate(color.a * ImGui::GetStyle().Alpha) * 255.0f;
        for (int i = 0; i < mesh.points.Size; ++i)
            draw->PrimWriteVtx(origin + mesh.points[i], uv, rgb | (ImU32(alpha * mesh.coverage[i] + 0.5f) << IM_COL32_A_SHIFT));
        WriteRingStrips(draw, first_index, mesh.rings, ring_size);
        for (int i = 1; filled && i + 1 < ring_size; ++i) {
            draw->PrimWriteIdx(ImDrawIdx(first_index));
            draw->PrimWriteIdx(ImDrawIdx(first_index + unsigned(i)));
            draw->PrimWriteIdx(ImDrawIdx(first_index + unsigned(i + 1)));
        }
    }

    void DropShadow(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, const Shadow& shadow, CornerStyle style, bool cut_out) {
        if (shadow.color.a <= 0.0f)
            return;
        const float scale = Environment().Scale();
        const ShadowMesh& mesh = CachedShadow(false, rect.GetSize(), radii, ImMax(shadow.blur * 0.5f * scale, 0.3f), shadow.spread * scale, shadow.offset * scale, style, cut_out);
        if (mesh.rings > 0)
            WriteShadow(draw, rect.Min, mesh, shadow.color, true);
    }

    static Shadow Faded(Shadow shadow, float opacity) {
        shadow.color = shadow.color.Opacity(opacity);
        return shadow;
    }

    void DropShadows(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, std::span<const Shadow> shadows, CornerStyle style, float opacity, bool cut_out) {
        for (const Shadow& shadow : shadows)
            DropShadow(draw, rect, radii, Faded(shadow, opacity), style, cut_out);
    }

    // Outward normal of contour vertex i, skipping duplicated vertices of sharp corners.
    static ImVec2 OutwardNormal(const ImVector<ImVec2>& points, int i) {
        const int count = points.Size;
        int previous = (i + count - 1) % count;
        int next = (i + 1) % count;
        for (int guard = 0; guard < count && ImLengthSqr(points[previous] - points[i]) < 1e-6f; ++guard)
            previous = (previous + count - 1) % count;
        for (int guard = 0; guard < count && ImLengthSqr(points[next] - points[i]) < 1e-6f; ++guard)
            next = (next + 1) % count;
        const ImVec2 direction = Normalized(points[next] - points[previous]);
        return ImVec2(direction.y, -direction.x);
    }

    // The shadow of the area outside the shape, moved by the offset and grown by the spread, seen inside the shape.
    static void BuildInnerShadow(ShadowMesh& mesh, ImVec2 size, const CornerRadii& radii, float sigma, float spread, ImVec2 offset, CornerStyle style) {
        const ImRect rect(ImVec2(0.0f, 0.0f), size);
        const float half = ImMin(rect.GetWidth(), rect.GetHeight()) * 0.5f;
        mesh.points.resize(0);
        mesh.coverage.resize(0);
        mesh.rings = 0;
        if (half <= 0.5f)
            return;

        // The first ring sits half a pixel outside with zero alpha, matching the anti-aliased edge of the fill.
        const float depth = ImMin(3.0f * sigma + ImMax(ImFabs(offset.x), ImFabs(offset.y)) + ImMax(spread, 0.0f), half - 0.25f);
        const int steps = ImClamp(int(ImCeil((depth + 0.5f) / ImMax(sigma / 3.0f, 0.5f))), 2, 32);
        const int rings = steps + 1;
        const int corner_points = CornerPointsFor(radii.Largest());
        mesh.rings = rings;
        mesh.ringSize = corner_points * 4;
        ImVector<ImVec2>& points = Scratch();
        for (int ring = 0; ring < rings; ++ring) {
            const float inset = ImLerp(-0.5f, depth, float(ring) / float(rings - 1));
            points.resize(0);
            RoundedRectContour(points, ImRect(rect.Min + ImVec2(inset, inset), rect.Max - ImVec2(inset, inset)), radii.Offset(-inset), style, corner_points);
            for (int i = 0; i < points.Size; ++i) {
                float coverage = 0.0f;
                if (ring > 0) {
                    const ImVec2 normal = OutwardNormal(points, i);
                    const float distance = ImMax(inset, 0.0f) + normal.x * offset.x + normal.y * offset.y - spread;
                    coverage = NormalCdf(-distance / sigma);
                }
                mesh.points.push_back(points[i]);
                mesh.coverage.push_back(ImSaturate(coverage));
            }
        }
    }

    void InnerShadow(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, const Shadow& shadow, CornerStyle style) {
        if (shadow.color.a <= 0.0f)
            return;
        const float scale = Environment().Scale();
        const ShadowMesh& mesh = CachedShadow(true, rect.GetSize(), radii, ImMax(shadow.blur * 0.5f * scale, 0.3f), shadow.spread * scale, shadow.offset * scale, style, false);
        if (mesh.rings > 0)
            WriteShadow(draw, rect.Min, mesh, shadow.color, false);
    }

    void InnerShadows(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, std::span<const Shadow> shadows, CornerStyle style, float opacity) {
        for (const Shadow& shadow : shadows)
            InnerShadow(draw, rect, radii, Faded(shadow, opacity), style);
    }

    void HorizontalLine(ImDrawList* draw, float x0, float x1, float y, float thickness, Rgba color) {
        if (thickness < 1.0f) {
            color = color.Opacity(thickness);
            thickness = 1.0f;
        }
        const float top = Snap(y);
        FillRect(draw, ImRect(Snap(x0), top, Snap(x1), top + ImMax(1.0f, Snap(thickness))), color);
    }

    void VerticalLine(ImDrawList* draw, float x, float y0, float y1, float thickness, Rgba color) {
        if (thickness < 1.0f) {
            color = color.Opacity(thickness);
            thickness = 1.0f;
        }
        const float left = Snap(x);
        FillRect(draw, ImRect(left, Snap(y0), left + ImMax(1.0f, Snap(thickness)), Snap(y1)), color);
    }

    void FillShaded(ImDrawList* draw, ImVec2 center, Rgba center_color, std::span<const ImVec2> contour, const std::function<Rgba(ImVec2)>& color) {
        const int count = int(contour.size());
        if (count < 3)
            return;
        draw->PrimReserve(count * 3, count + 1);
        const ImVec2 uv = draw->_Data->TexUvWhitePixel;
        const ImDrawIdx base = ImDrawIdx(draw->_VtxCurrentIdx);
        draw->PrimWriteVtx(center, uv, center_color.Packed());
        for (const ImVec2& point : contour)
            draw->PrimWriteVtx(point, uv, color(point).Packed());
        for (int i = 0; i < count; ++i) {
            draw->PrimWriteIdx(base);
            draw->PrimWriteIdx(ImDrawIdx(base + 1 + i));
            draw->PrimWriteIdx(ImDrawIdx(base + 1 + (i + 1) % count));
        }
    }

    // CoreGraphics bevels where a miter would reach past 10 line widths: turns tighter than 11.5 degrees.
    static bool IsSharpTurn(ImVec2 previous, ImVec2 point, ImVec2 next) {
        const ImVec2 in = point - previous;
        const ImVec2 out = next - point;
        const float lengths = ImSqrt(ImLengthSqr(in) * ImLengthSqr(out));
        return lengths > 0.0f && -ImDot(in, out) > 0.98f * lengths;
    }

    // Fills the notch on the outer side of a corner between two butt-ended runs.
    static void Bevel(ImDrawList* draw, ImVec2 previous, ImVec2 point, ImVec2 next, ImU32 color, float thickness) {
        const ImVec2 in = point - previous;
        const ImVec2 out = next - point;
        const float in_length = ImSqrt(ImLengthSqr(in));
        const float out_length = ImSqrt(ImLengthSqr(out));
        const float side = in.x * out.y - in.y * out.x > 0.0f ? -thickness * 0.5f : thickness * 0.5f;
        const ImVec2 in_normal(-in.y / in_length * side, in.x / in_length * side);
        const ImVec2 out_normal(-out.y / out_length * side, out.x / out_length * side);
        // A sliver triangle's fringe would spike at its acute corners, so the bevel goes without one.
        const ImDrawListFlags flags = draw->Flags;
        draw->Flags &= ~ImDrawListFlags_AntiAliasedFill;
        draw->AddTriangleFilled(point, point + in_normal, point + out_normal, color);
        draw->Flags = flags;
    }

    void Polyline(ImDrawList* draw, std::span<const ImVec2> points, Rgba color, float thickness, bool closed) {
        const int count = int(points.size());
        if (count < 2)
            return;
        const ImU32 packed = color.Packed();
        const auto at = [&](int i) { return points[size_t((i % count + count) % count)]; };
        std::vector<int> sharp;
        for (int i = closed ? 0 : 1; i < (closed ? count : count - 1); ++i) {
            if (IsSharpTurn(at(i - 1), at(i), at(i + 1)))
                sharp.push_back(i);
        }
        if (sharp.empty()) {
            draw->AddPolyline(points.data(), count, packed, closed ? ImDrawFlags_Closed : ImDrawFlags_None, thickness);
            return;
        }
        // ImGui mitres every joint of a polyline, so the path is drawn in runs that end at the sharp corners.
        const int first = closed ? sharp.front() : 0;
        const int length = closed ? count + 1 : count;
        std::vector<ImVec2> run;
        for (int step = 0; step < length; ++step) {
            const int i = first + step;
            run.push_back(at(i));
            const bool corner = std::find(sharp.begin(), sharp.end(), i % count) != sharp.end();
            if (step == 0 || (!corner && step + 1 < length))
                continue;
            draw->AddPolyline(run.data(), int(run.size()), packed, ImDrawFlags_None, thickness);
            if (corner)
                Bevel(draw, at(i - 1), at(i), at(i + 1), packed, thickness);
            run.assign(1, at(i));
        }
    }

    void Arc(ImDrawList* draw, ImVec2 center, float radius, float thickness, float start, float end, Rgba color, bool round_start, bool round_end) {
        const bool full = end - start >= 2.0f * IM_PI - 1e-4f;
        const int segments = ArcSegments(end - start, 8);
        if (full) {
            // The closing segment joins the last point to the first; drawing the first twice would spike the join.
            const float step = 2.0f * IM_PI / float(segments);
            draw->PathArcTo(center, radius, start - IM_PI * 0.5f, start + 2.0f * IM_PI - step - IM_PI * 0.5f, segments - 1);
            draw->PathStroke(color.Packed(), ImDrawFlags_Closed, thickness);
            return;
        }
        // One outline with its caps, so that a translucent arc shows no seams: the outer edge, the end cap, the inner
        // edge back and the start cap. The tangent at an angle from 12 o'clock points at that angle in ImGui's convention.
        const float half = thickness * 0.5f;
        const int cap_segments = 10;
        const auto on_ring = [&](float angle, float distance) { return center + ImVec2(ImSin(angle), -ImCos(angle)) * distance; };
        const auto cap = [&](float angle, float facing, bool round) {
            for (int k = 1; k < cap_segments && round; ++k) {
                const float turn = facing - IM_PI * 0.5f + IM_PI * float(k) / float(cap_segments);
                draw->_Path.push_back(on_ring(angle, radius) + ImVec2(ImCos(turn), ImSin(turn)) * half);
            }
        };
        draw->_Path.resize(0);
        for (int i = 0; i <= segments; ++i)
            draw->_Path.push_back(on_ring(ImLerp(start, end, float(i) / float(segments)), radius + half));
        cap(end, end, round_end);
        for (int i = segments; i >= 0; --i)
            draw->_Path.push_back(on_ring(ImLerp(start, end, float(i) / float(segments)), ImMax(radius - half, 0.0f)));
        cap(start, start + IM_PI, round_start);
        draw->AddConcavePolyFilled(draw->_Path.Data, draw->_Path.Size, color.Packed());
        draw->_Path.resize(0);
    }

    void RotateVertices(ImDrawList* draw, int first_vertex, ImVec2 center, float radians) {
        const float sine = ImSin(radians);
        const float cosine = ImCos(radians);
        for (int i = first_vertex; i < draw->VtxBuffer.Size; ++i) {
            const ImVec2 offset = draw->VtxBuffer[i].pos - center;
            draw->VtxBuffer[i].pos = center + ImVec2(offset.x * cosine - offset.y * sine, offset.x * sine + offset.y * cosine);
        }
    }

    void TintVertices(ImDrawList* draw, int first_vertex, Rgba color, float amount) {
        const float keep = 1.0f - amount;
        const float add[3] = {color.r * amount * 255.0f + 0.5f, color.g * amount * 255.0f + 0.5f, color.b * amount * 255.0f + 0.5f};
        const int shifts[3] = {IM_COL32_R_SHIFT, IM_COL32_G_SHIFT, IM_COL32_B_SHIFT};
        for (int i = first_vertex; i < draw->VtxBuffer.Size; ++i) {
            ImU32& packed = draw->VtxBuffer[i].col;
            ImU32 mixed = packed & IM_COL32_A_MASK;
            for (int channel = 0; channel < 3; ++channel) {
                const float value = float((packed >> shifts[channel]) & 0xFF) * keep + add[channel];
                mixed |= ImU32(ImMin(value, 255.0f)) << shifts[channel];
            }
            packed = mixed;
        }
    }

    void FadeVertices(ImDrawList* draw, int first_vertex, const std::function<float(ImVec2)>& opacity) {
        for (int i = first_vertex; i < draw->VtxBuffer.Size; ++i) {
            ImDrawVert& vertex = draw->VtxBuffer[i];
            const float alpha = float((vertex.col >> IM_COL32_A_SHIFT) & 0xFF) * opacity(vertex.pos);
            vertex.col = (vertex.col & ~IM_COL32_A_MASK) | (ImU32(alpha) << IM_COL32_A_SHIFT);
        }
    }

    void ShadeVertices(ImDrawList* draw, int first_vertex, const std::function<Rgba(ImVec2)>& color) {
        for (int i = first_vertex; i < draw->VtxBuffer.Size; ++i) {
            ImDrawVert& vertex = draw->VtxBuffer[i];
            const Rgba shade = color(vertex.pos);
            const float alpha = float((vertex.col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;
            vertex.col = Rgba(shade.r, shade.g, shade.b, alpha).Packed();
        }
    }

    Opacity::Opacity(float opacity) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * opacity);
    }

    Opacity::~Opacity() {
        ImGui::PopStyleVar();
    }

    void FadedGroup(ImDrawList* draw, Rgba backdrop, float opacity, const std::function<void()>& content) {
        const int first_vertex = draw->VtxBuffer.Size;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
        content();
        ImGui::PopStyleVar();
        if (opacity < 1.0f)
            TintVertices(draw, first_vertex, backdrop, 1.0f - opacity);
    }

    void FocusRing(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba accent, CornerStyle style) {
        const float key = Environment().KeyAmount();
        if (key <= 0.0f)
            return;
        const ImRect ring(rect.Min + Px(ImVec2(0.5f, 0.5f)), rect.Max - Px(ImVec2(0.5f, 0.5f)));
        StrokeRoundedRect(draw, ring, radii.Offset(-Px(0.5f)), accent.Opacity(0.5f * key), Px(3.5f), StrokeAlignment::Outside, style);
    }

    float Snap(float value) {
        return ImFloor(value + 0.5f);
    }

    ImVec2 Snap(ImVec2 point) {
        return ImVec2(Snap(point.x), Snap(point.y));
    }

    ImRect Snap(const ImRect& rect) {
        return ImRect(Snap(rect.Min.x), Snap(rect.Min.y), Snap(rect.Max.x), Snap(rect.Max.y));
    }
} // namespace Cupertino::Draw
