#include "Layout.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/State.h"

#include <vector>

namespace Cupertino::Layout {
    struct ChildRecord {
        ImVec2 size;
        float slot = 0.0f;
        bool flexibleWidth = false;
        bool flexibleHeight = false;
    };

    // How far a view reaches above and below the baseline it is aligned on: its first text baseline, or its bottom.
    struct BaselineExtent {
        float ascent = 0.0f;
        float descent = 0.0f;
    };

    // What a container learned about its children, used by the next frame.
    struct LayoutCache {
        ImVector<ChildRecord> children;
        ImVec2 size;
        float scale = 0.0f;
        float fixedLength = 0.0f;
        int flexibleCount = 0;
        bool flexibleWidth = false;
        bool flexibleHeight = false;
        // The container's first text baseline from its top, and, for a baseline-aligned row, where the shared baseline
        // runs below the content top.
        float baseline = -1.0f;
        float ascent = 0.0f;
        // How far the content reaches above the container's top edge into a form row's inset.
        float overhang = 0.0f;
        bool valid = false;
    };

    struct Insets {
        float top = 0.0f;
        float leading = 0.0f;
        float bottom = 0.0f;
        float trailing = 0.0f;
    };

    struct Frame {
        ImGuiWindow* window = nullptr;
        Arrangement arrangement = Arrangement::Vertical;
        Alignment alignment;
        Role role = Role::None;
        float spacing = 0.0f;
        Insets padding;
        Insets childInsets;
        float minChildLength = 0.0f;
        bool allowsOverhang = false;
        ImVec2 fixedSize;
        ImVec2 minSize;
        bool fillWidth = false;
        bool fillHeight = false;
        bool hugsWidth = true;
        bool hugsHeight = true;
        LayoutCache* cache = nullptr;
        ImVector<ChildRecord> records;
        ImVector<ImRect> slots;
        ImVector<bool> fullWidthSeparators;
        ImVector<float> separatorLeadings;
        ImRect outer;
        ImRect content;
        ImVec2 offer;
        float cursor = 0.0f;
        float crossExtent = 0.0f;
        float share = 0.0f;
        int childIndex = 0;
        BaselineExtent baselineExtent;
        float baseline = -1.0f;
        float overhang = 0.0f;
        const ImVector<float>* columns = nullptr;
        ImVector<float>* measuredColumns = nullptr;
        HorizontalAlignment columnAlignment = HorizontalAlignment::Leading;
    };

    struct RootCounter {
        int frame = -1;
        int count = 0;
    };

    static std::vector<Frame>& Frames() {
        static std::vector<Frame> frames = [] {
            std::vector<Frame> reserved;
            reserved.reserve(64);
            return reserved;
        }();
        return frames;
    }

    static int measuringDepth = 0;

    static Frame* CurrentFrame() {
        std::vector<Frame>& frames = Frames();
        if (frames.empty() || frames.back().window != GImGui->CurrentWindow)
            return nullptr;
        return &frames.back();
    }

    static Insets ToPixels(const EdgeInsets& insets) {
        return Insets{Px(insets.top), Px(insets.leading), Px(insets.bottom), Px(insets.trailing)};
    }

    static bool IsHorizontal(const Frame& frame) {
        return frame.arrangement == Arrangement::Horizontal;
    }

    static float AlignmentFactor(HorizontalAlignment alignment) {
        return alignment == HorizontalAlignment::Leading ? 0.0f : alignment == HorizontalAlignment::Center ? 0.5f : 1.0f;
    }

    static float AlignmentFactor(VerticalAlignment alignment) {
        return alignment == VerticalAlignment::Center ? 0.5f : alignment == VerticalAlignment::Bottom ? 1.0f : 0.0f;
    }

    static bool AlignsBaselines(const Frame& frame) {
        return IsHorizontal(frame) && frame.alignment.vertical == VerticalAlignment::FirstTextBaseline;
    }

    static float BaselineOf(const Placement& placement, ImVec2 size) {
        return placement.baseline >= 0.0f ? placement.baseline : size.y;
    }

    static bool FlexibleAlongMain(const Frame& frame, bool flexible_width, bool flexible_height) {
        if (frame.arrangement == Arrangement::Overlay)
            return false;
        return IsHorizontal(frame) ? flexible_width : flexible_height;
    }

    static ImVec2 OfferFor(const Frame& frame, int child_index) {
        ImVec2 offer = frame.offer;
        offer.x = ImMax(0.0f, offer.x - frame.childInsets.leading - frame.childInsets.trailing);
        offer.y = ImMax(0.0f, offer.y - frame.childInsets.top - frame.childInsets.bottom);
        if (frame.arrangement == Arrangement::Overlay || !frame.cache->valid)
            return offer;

        // Along the axis a child is offered what its siblings left over last frame.
        const ImVector<ChildRecord>& children = frame.cache->children;
        const bool has_record = child_index < children.Size;
        const ChildRecord record = has_record ? children[child_index] : ChildRecord{};
        float& main = IsHorizontal(frame) ? offer.x : offer.y;
        if (main <= 0.0f)
            return offer;
        if (has_record && FlexibleAlongMain(frame, record.flexibleWidth, record.flexibleHeight))
            main = frame.share;
        else
            main = ImMax(0.0f, main - (frame.cache->fixedLength - (has_record ? record.slot : 0.0f)));
        return offer;
    }

    // The insets around a child; the rows of a form section take the ones set by ListRowInsets.
    static Insets InsetsFor(const Frame& frame, const Placement& placement) {
        if (placement.ignoresChildInsets)
            return {};
        Insets insets = frame.childInsets;
        if (frame.role == Role::FormSection) {
            const EnvironmentValues& environment = Environment();
            if (environment.rowTopInset >= 0.0f)
                insets.top = Px(environment.rowTopInset);
            if (environment.rowLeadingInset >= 0.0f)
                insets.leading = Px(environment.rowLeadingInset);
            if (environment.rowBottomInset >= 0.0f)
                insets.bottom = Px(environment.rowBottomInset);
        }
        return insets;
    }

    // Computes where the next child goes without advancing; slot receives the child's full slot.
    static ImRect Reserve(const Frame& frame, const Placement& placement, float* slot_length) {
        const Insets insets = InsetsFor(frame, placement);
        const ImVec2 size = placement.size;

        if (frame.arrangement == Arrangement::Overlay) {
            const float free_x = frame.content.GetWidth() - insets.leading - insets.trailing - size.x;
            const float free_y = frame.content.GetHeight() - insets.top - insets.bottom - size.y;
            const ImVec2 min(frame.content.Min.x + insets.leading + free_x * AlignmentFactor(frame.alignment.horizontal), frame.content.Min.y + insets.top + free_y * AlignmentFactor(frame.alignment.vertical));
            *slot_length = 0.0f;
            return ImRect(min, min + size);
        }

        const bool horizontal = IsHorizontal(frame);
        const bool flexible = FlexibleAlongMain(frame, placement.flexibleWidth, placement.flexibleHeight);
        const float length = flexible ? ImMax(placement.minLength, frame.share) : (horizontal ? size.x : size.y);
        const bool overhangs = frame.allowsOverhang && !horizontal && !placement.ignoresChildInsets;
        const float before = horizontal ? insets.leading : insets.top - (overhangs ? ImMin(placement.overhang, insets.top) : 0.0f);
        const float after = horizontal ? insets.trailing : insets.bottom;
        float slot = ImMax(before + length + after, placement.ignoresChildInsets ? 0.0f : frame.minChildLength);
        float factor_in_slot = 0.5f;
        // A grid row gives each child its column's width and lines it up in it.
        if (horizontal && frame.columns && frame.childIndex < frame.columns->Size) {
            slot = ImMax(slot, (*frame.columns)[frame.childIndex]);
            factor_in_slot = AlignmentFactor(frame.columnAlignment);
        }
        const float main = frame.cursor + before + (slot - before - length - after) * factor_in_slot;

        const float cross_before = horizontal ? insets.top : insets.leading;
        const float cross_after = horizontal ? insets.bottom : insets.trailing;
        const float cross_size = horizontal ? size.y : size.x;
        const float cross_space = (horizontal ? frame.content.GetHeight() : frame.content.GetWidth()) - cross_before - cross_after;
        const float factor = horizontal ? AlignmentFactor(frame.alignment.vertical) : AlignmentFactor(frame.alignment.horizontal);
        // Baseline alignment hangs every child from the shared baseline measured last frame.
        const float cross = AlignsBaselines(frame) ? frame.content.Min.y + frame.cache->ascent - BaselineOf(placement, size) : (horizontal ? frame.content.Min.y : frame.content.Min.x) + cross_before + (cross_space - cross_size) * factor;

        *slot_length = slot;
        if (horizontal)
            return ImRect(main, cross, main + length, cross + cross_size);
        return ImRect(cross, main, cross + cross_size, main + length);
    }

    static void Commit(Frame& frame, const Placement& placement, const ImRect& rect, float slot) {
        const Insets insets = InsetsFor(frame, placement);
        const bool horizontal = IsHorizontal(frame);
        const ImVec2 size = rect.GetSize();
        frame.records.push_back(ChildRecord{size, slot, placement.flexibleWidth, placement.flexibleHeight});
        frame.fullWidthSeparators.push_back(placement.fullWidthSeparator);
        // A row inset by ListRowInsets starts the separator under it where its content starts.
        const float leading = frame.role == Role::FormSection ? Environment().rowLeadingInset : -1.0f;
        frame.separatorLeadings.push_back(leading >= 0.0f ? Px(leading) : -1.0f);
        // The container's own baseline is its first child's with one; an overhang carries over as far as it passes the
        // container's top.
        if (frame.baseline < 0.0f && placement.baseline >= 0.0f)
            frame.baseline = rect.Min.y + placement.baseline - frame.outer.Min.y;
        frame.overhang = ImMax(frame.overhang, placement.overhang - (rect.Min.y - frame.outer.Min.y));
        if (frame.measuredColumns && horizontal) {
            ImVector<float>& measured = *frame.measuredColumns;
            while (measured.Size <= frame.childIndex)
                measured.push_back(0.0f);
            measured[frame.childIndex] = ImMax(measured[frame.childIndex], size.x + insets.leading + insets.trailing);
        }

        if (frame.arrangement == Arrangement::Overlay) {
            frame.slots.push_back(frame.content);
            frame.crossExtent = ImMax(frame.crossExtent, size.x + insets.leading + insets.trailing);
            frame.cursor = ImMax(frame.cursor, frame.content.Min.y + size.y + insets.top + insets.bottom);
        } else {
            if (horizontal)
                frame.slots.push_back(ImRect(frame.cursor, frame.content.Min.y, frame.cursor + slot, frame.content.Max.y));
            else
                frame.slots.push_back(ImRect(frame.content.Min.x, frame.cursor, frame.content.Max.x, frame.cursor + slot));
            frame.cursor += slot + frame.spacing;
            if (AlignsBaselines(frame)) {
                const float baseline = BaselineOf(placement, size);
                frame.baselineExtent.ascent = ImMax(frame.baselineExtent.ascent, baseline);
                frame.baselineExtent.descent = ImMax(frame.baselineExtent.descent, size.y - baseline);
                frame.crossExtent = frame.baselineExtent.ascent + frame.baselineExtent.descent;
            } else {
                const float cross = horizontal ? size.y + insets.top + insets.bottom : size.x + insets.leading + insets.trailing;
                frame.crossExtent = ImMax(frame.crossExtent, cross);
            }
        }
        ++frame.childIndex;
    }

    ImRect Place(const Placement& placement) {
        Frame* frame = CurrentFrame();
        if (!frame) {
            // Outside any container the view joins ImGui's own layout.
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            const ImRect rect(window->DC.CursorPos, window->DC.CursorPos + placement.size);
            ImGui::ItemSize(placement.size);
            return rect;
        }
        float slot = 0.0f;
        const ImRect rect = Reserve(*frame, placement, &slot);
        Commit(*frame, placement, rect, slot);
        return placement.snapsToPixels ? Draw::Snap(rect) : rect;
    }

    ImRect Place(ImVec2 size) {
        return Place(Placement{.size = size});
    }

    ImVec2 Proposal() {
        const Frame* frame = CurrentFrame();
        if (!frame)
            return ImGui::GetContentRegionAvail();
        return OfferFor(*frame, frame->childIndex);
    }

    ImVec2 FullProposal() {
        const Frame* frame = CurrentFrame();
        if (!frame)
            return ImGui::GetContentRegionAvail();
        ImVec2 offer = OfferFor(*frame, frame->childIndex);
        if (offer.x > 0.0f)
            offer.x += frame->childInsets.leading + frame->childInsets.trailing;
        if (offer.y > 0.0f)
            offer.y += frame->childInsets.top + frame->childInsets.bottom;
        return offer;
    }

    bool IsMeasuring() {
        return measuringDepth > 0;
    }

    // The id a container's children take theirs from: the container's own, or one pushed inside it, so views keep apart
    // what they show under different ids.
    static ImGuiID ChildSeed() {
        return GImGui->CurrentWindow->IDStack.back();
    }

    ImGuiID NextViewId() {
        const Frame* frame = CurrentFrame();
        if (!frame)
            return ImGui::GetID("##CupertinoView");
        return ImHashData(&frame->childIndex, sizeof(int), ChildSeed());
    }

    Arrangement ParentArrangement() {
        const Frame* frame = CurrentFrame();
        return frame ? frame->arrangement : Arrangement::Vertical;
    }

    Role ParentRole() {
        const Frame* frame = CurrentFrame();
        return frame ? frame->role : Role::None;
    }

    int ChildIndex() {
        const Frame* frame = CurrentFrame();
        return frame ? frame->childIndex : 0;
    }

    static ImGuiID RootId() {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        RootCounter& counter = State::Get<RootCounter>(window->ID);
        if (counter.frame != ImGui::GetFrameCount()) {
            counter.frame = ImGui::GetFrameCount();
            counter.count = 0;
        }
        const int index = counter.count++;
        return ImHashData(&index, sizeof(index), window->GetID("##CupertinoLayout"));
    }

    static void RescaleCache(LayoutCache& cache) {
        const float scale = Environment().Scale();
        if (cache.scale == scale || cache.scale <= 0.0f) {
            cache.scale = scale;
            return;
        }
        // Zoom changed: sizes from the previous frame are converted instead of being thrown away.
        const float factor = scale / cache.scale;
        cache.size = cache.size * factor;
        cache.fixedLength *= factor;
        for (ChildRecord& child : cache.children) {
            child.size = child.size * factor;
            child.slot *= factor;
        }
        cache.scale = scale;
    }

    // A region is placed at an explicit rectangle instead of in the current container.
    static void Open(ImGuiID id, const ContainerSpec& spec, const ImRect* region) {
        Frame* parent = region ? nullptr : CurrentFrame();
        LayoutCache& cache = State::Get<LayoutCache>(id);
        RescaleCache(cache);

        Frame frame;
        frame.window = GImGui->CurrentWindow;
        frame.arrangement = spec.arrangement;
        frame.alignment = spec.alignment;
        frame.role = spec.role;
        frame.spacing = Px(spec.spacing);
        frame.padding = ToPixels(spec.padding);
        frame.childInsets = ToPixels(spec.childInsets);
        frame.minChildLength = Px(spec.minChildLength);
        frame.allowsOverhang = spec.allowsOverhang;
        frame.fixedSize = ImVec2(Px(spec.width), Px(spec.height));
        frame.minSize = ImVec2(Px(spec.minWidth), Px(spec.minHeight));
        frame.fillWidth = spec.fillWidth;
        frame.fillHeight = spec.fillHeight;
        frame.cache = &cache;
        frame.columns = spec.columns;
        frame.measuredColumns = spec.measuredColumns;
        frame.columnAlignment = spec.columnAlignment;

        const ImVec2 offered = region ? region->GetSize() : parent ? OfferFor(*parent, parent->childIndex) : ImGui::GetContentRegionAvail();
        const bool stretches_width = region || spec.fillWidth || (cache.valid && cache.flexibleWidth);
        const bool stretches_height = region || spec.fillHeight || (cache.valid && cache.flexibleHeight);
        ImVec2 size = ImMax(cache.size, frame.minSize);
        if (frame.fixedSize.x > 0.0f)
            size.x = frame.fixedSize.x;
        else if (stretches_width && offered.x > 0.0f)
            size.x = offered.x;
        if (frame.fixedSize.y > 0.0f)
            size.y = frame.fixedSize.y;
        else if (stretches_height && offered.y > 0.0f)
            size.y = offered.y;
        frame.hugsWidth = frame.fixedSize.x <= 0.0f && !(stretches_width && offered.x > 0.0f);
        frame.hugsHeight = frame.fixedSize.y <= 0.0f && !(stretches_height && offered.y > 0.0f);

        if (region) {
            frame.outer = *region;
        } else if (parent) {
            Placement placement;
            placement.size = size;
            placement.flexibleWidth = stretches_width;
            placement.flexibleHeight = stretches_height;
            placement.baseline = cache.valid ? cache.baseline : -1.0f;
            placement.overhang = cache.valid ? cache.overhang : 0.0f;
            float slot = 0.0f;
            frame.outer = Reserve(*parent, placement, &slot);
        } else {
            const ImVec2 origin = frame.window->DC.CursorPos;
            frame.outer = ImRect(origin, origin + size);
        }
        // A parent may hand a flexible container more or less than it asked for.
        if (!frame.hugsWidth || stretches_width)
            size.x = frame.outer.GetWidth();
        if (!frame.hugsHeight || stretches_height)
            size.y = frame.outer.GetHeight();
        frame.outer.Max = frame.outer.Min + size;
        frame.content = ImRect(frame.outer.Min + ImVec2(frame.padding.leading, frame.padding.top), frame.outer.Max - ImVec2(frame.padding.trailing, frame.padding.bottom));

        // Children are offered the container's size where it is decided, and what the container was offered otherwise.
        frame.offer.x = frame.hugsWidth ? ImMax(0.0f, offered.x - frame.padding.leading - frame.padding.trailing) : frame.content.GetWidth();
        frame.offer.y = frame.hugsHeight ? ImMax(0.0f, offered.y - frame.padding.top - frame.padding.bottom) : frame.content.GetHeight();
        if (frame.hugsWidth && offered.x <= 0.0f)
            frame.offer.x = 0.0f;
        if (frame.hugsHeight && offered.y <= 0.0f)
            frame.offer.y = 0.0f;

        const bool horizontal = spec.arrangement == Arrangement::Horizontal;
        frame.cursor = horizontal ? frame.content.Min.x : frame.content.Min.y;
        if (spec.arrangement == Arrangement::Overlay)
            frame.cursor = frame.content.Min.y;
        if (cache.valid && cache.flexibleCount > 0 && spec.arrangement != Arrangement::Overlay) {
            const float main = horizontal ? frame.content.GetWidth() : frame.content.GetHeight();
            frame.share = ImMax(0.0f, main - cache.fixedLength) / float(cache.flexibleCount);
        }

        ImGui::PushOverrideID(id);
        Frames().push_back(std::move(frame));
    }

    static ImVec2 Close(const std::function<void(const ContainerFrame&)>& finish) {
        std::vector<Frame>& frames = Frames();
        Frame frame = std::move(frames.back());
        frames.pop_back();
        ImGui::PopID();

        const bool horizontal = IsHorizontal(frame);
        const bool overlay = frame.arrangement == Arrangement::Overlay;
        const float main = frame.childIndex > 0 ? frame.cursor - frame.spacing - (horizontal ? frame.content.Min.x : frame.content.Min.y) : 0.0f;
        ImVec2 measured;
        if (overlay)
            measured = ImVec2(frame.crossExtent, frame.childIndex > 0 ? frame.cursor - frame.content.Min.y : 0.0f);
        else
            measured = horizontal ? ImVec2(main, frame.crossExtent) : ImVec2(frame.crossExtent, main);
        measured += ImVec2(frame.padding.leading + frame.padding.trailing, frame.padding.top + frame.padding.bottom);

        ImVec2 size = frame.outer.GetSize();
        if (frame.hugsWidth)
            size.x = ImMax(measured.x, frame.minSize.x);
        if (frame.hugsHeight)
            size.y = ImMax(measured.y, frame.minSize.y);

        LayoutCache& cache = *frame.cache;
        cache.children.swap(frame.records);
        cache.fixedLength = 0.0f;
        cache.flexibleCount = 0;
        cache.flexibleWidth = frame.fillWidth;
        cache.flexibleHeight = frame.fillHeight;
        for (const ChildRecord& child : cache.children) {
            if (FlexibleAlongMain(frame, child.flexibleWidth, child.flexibleHeight))
                ++cache.flexibleCount;
            else
                cache.fixedLength += child.slot;
            cache.flexibleWidth |= child.flexibleWidth;
            cache.flexibleHeight |= child.flexibleHeight;
        }
        // A fixed size ends the flexibility of the content along that axis, like .frame(width:height:).
        cache.flexibleWidth = cache.flexibleWidth && frame.fixedSize.x <= 0.0f;
        cache.flexibleHeight = cache.flexibleHeight && frame.fixedSize.y <= 0.0f;
        if (!overlay && cache.children.Size > 1)
            cache.fixedLength += frame.spacing * float(cache.children.Size - 1);
        cache.size = size;
        cache.baseline = frame.baseline;
        cache.ascent = frame.baselineExtent.ascent;
        // A control reaches above the text line it shares with a label, as a push button on a row title's baseline
        // (Privacy & Security @2x); centered in a stack, or alone, it keeps the row's whole inset (Keyboard @2x).
        cache.overhang = AlignsBaselines(frame) ? frame.overhang : 0.0f;
        cache.valid = true;

        if (finish && !IsMeasuring()) {
            ContainerFrame result;
            result.rect = ImRect(frame.outer.Min, frame.outer.Min + size);
            result.slots.swap(frame.slots);
            result.fullWidthSeparators.swap(frame.fullWidthSeparators);
            result.separatorLeadings.swap(frame.separatorLeadings);
            finish(result);
        }
        return size;
    }

    // Reports the finished container to its parent, or to ImGui when it is a root.
    static void Report(ImGuiID id, ImVec2 origin, ImVec2 size) {
        Frame* parent = CurrentFrame();
        if (!parent) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            window->DC.CursorPos = origin;
            ImGui::ItemSize(size);
            return;
        }
        const LayoutCache& cache = State::Get<LayoutCache>(id);
        Placement placement;
        placement.size = size;
        placement.flexibleWidth = cache.flexibleWidth;
        placement.flexibleHeight = cache.flexibleHeight;
        placement.baseline = cache.baseline;
        placement.overhang = cache.overhang;
        float slot = 0.0f;
        const ImRect rect = Reserve(*parent, placement, &slot);
        Commit(*parent, placement, rect, slot);
    }

    // Lays the content out and returns the container's size; origin receives its top-left corner.
    static ImVec2 Run(ImGuiID id, const ContainerSpec& spec, const ImRect* region, const std::function<void()>& content, const std::function<void(const ContainerFrame&)>& finish, ImVec2* origin) {
        // First appearance: lay the content out once without drawing, so the real pass has every size.
        // The measuring pass reserves nothing in the parent; only the container's own cache is filled.
        if (!State::Get<LayoutCache>(id).valid && !IsMeasuring()) {
            ImGuiWindow* window = GImGui->CurrentWindow;
            const bool skip_items = window->SkipItems;
            window->SkipItems = true;
            ++measuringDepth;
            Open(id, spec, region);
            content();
            Close(nullptr);
            --measuringDepth;
            window->SkipItems = skip_items;
        }

        Open(id, spec, region);
        *origin = Frames().back().outer.Min;
        content();
        return Close(finish);
    }

    void ContainerBehind(const ContainerSpec& spec, const std::function<void()>& content, const std::function<void(const ContainerFrame&)>& behind) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImDrawListSplitter splitter;
        Container(spec, [&] {
            if (!IsMeasuring()) {
                splitter.Split(draw, 2);
                splitter.SetCurrentChannel(draw, 1);
            }
            content();
        }, [&](const ContainerFrame& frame) {
            splitter.SetCurrentChannel(draw, 0);
            behind(frame);
            splitter.Merge(draw);
        });
    }

    void Container(const ContainerSpec& spec, const std::function<void()>& content, const std::function<void(const ContainerFrame&)>& finish) {
        const Frame* parent = CurrentFrame();
        const ImGuiID id = parent ? ImHashData(&parent->childIndex, sizeof(int), ChildSeed()) : RootId();
        ImVec2 origin;
        const ImVec2 size = Run(id, spec, nullptr, content, finish, &origin);
        Report(id, origin, size);
    }

    void Region(ImGuiID id, const ImRect& rect, const ContainerSpec& spec, const std::function<void()>& content) {
        ImVec2 origin;
        Run(id, spec, &rect, content, nullptr, &origin);
    }
} // namespace Cupertino::Layout
