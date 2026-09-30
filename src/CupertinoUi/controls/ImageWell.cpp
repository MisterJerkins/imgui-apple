#include "ImageWell.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "layout/Layout.h"

namespace Cupertino {
    void ImageWell(const char* id, const Icon& image, const ImageWellOptions& options) {
        const float kit = Metrics::ImageWell().size;
        const Metrics::ImageWellMetrics& metrics = Metrics::ImageWell(options.size.x > kit || options.size.y > kit);
        const ImRect frame = Layout::Place(Px(options.size));
        if (Layout::IsMeasuring())
            return;
        ImGui::ItemAdd(frame, ImGui::GetID(id), nullptr, ImGuiItemFlags_NoNav);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const Interaction::DisabledFade fade;
        // The well: a tinted square inside a white ring and a faint outer line, shaded from the top edge.
        const ImRect well(frame.Min + Px(ImVec2(metrics.inset, metrics.inset)), frame.Max - Px(ImVec2(metrics.inset, metrics.inset)));
        const CornerRadii radii(Px(metrics.radius));
        Draw::StrokeRoundedRect(draw, well, radii, colors.imageWellEdge, Px(metrics.rim + 0.5f), StrokeAlignment::Outside);
        Draw::StrokeRoundedRect(draw, well, radii, colors.imageWellRim, Px(metrics.rim), StrokeAlignment::Outside);
        Draw::FillRoundedRect(draw, well, radii, colors.imageWellFill);
        Draw::InnerShadows(draw, well, radii, Theme::ImageWellInnerShadows());
        Draw::StrokeRoundedRect(draw, well, radii, colors.imageWellEdge, Px(0.5f), StrokeAlignment::Inside);
        if (image.IsEmpty())
            return;
        const ImRect picture(well.Min + Px(ImVec2(metrics.imageInset, metrics.imageInset)), well.Max - Px(ImVec2(metrics.imageInset, metrics.imageInset)));
        if (!image.image.IsEmpty())
            Draw::Image(draw, picture, image.image, Px(metrics.imageRadius));
        else
            DrawIcon(draw, picture, image);
    }
} // namespace Cupertino
