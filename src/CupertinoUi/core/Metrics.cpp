#include "Metrics.h"

namespace Cupertino::Metrics {
    SwitchMetrics Switch(ControlSize size) {
        if (Environment().platform == Platform::IOS)
            return SwitchMetrics{ImVec2(51.0f, 31.0f), 27.0f};
        switch (size) {
            case ControlSize::Mini:
                return SwitchMetrics{ImVec2(26.0f, 15.0f), 13.0f};
            case ControlSize::Small:
                return SwitchMetrics{ImVec2(32.0f, 18.0f), 16.0f};
            case ControlSize::Regular:
            case ControlSize::Large:
            case ControlSize::ExtraLarge:
                break;
        }
        return SwitchMetrics{ImVec2(38.0f, 22.0f), 20.0f};
    }

    CheckboxMetrics Checkbox(ControlSize size, bool in_form) {
        CheckboxMetrics metrics;
        // A small checkbox's title stands 4 pt after the box (Get Info @2x).
        if (size == ControlSize::Small) {
            metrics.box = 12.0f;
            metrics.radius = 3.0f;
            metrics.height = 14.0f;
            metrics.labelSpacing = 4.0f;
        } else if (size == ControlSize::Mini) {
            // The mini title all but touches its box (Fleeting Pixels @2x).
            metrics.box = 10.0f;
            metrics.radius = 2.5f;
            metrics.height = 12.0f;
            metrics.labelSpacing = 1.0f;
        } else if (size == ControlSize::Large || size == ControlSize::ExtraLarge) {
            // The large box is 16 pt with the regular label (Fleeting Pixels @2x).
            metrics.box = 16.0f;
            metrics.radius = 4.0f;
            metrics.height = 18.0f;
        }
        if (in_form)
            metrics.labelSpacing = 5.0f;
        return metrics;
    }

    RadioMetrics Radio(ControlSize size, bool in_form) {
        RadioMetrics metrics;
        if (size == ControlSize::Small) {
            metrics.diameter = 12.0f;
            metrics.height = 14.0f;
        } else if (size == ControlSize::Mini) {
            metrics.diameter = 10.0f;
            metrics.height = 12.0f;
        }
        if (in_form) {
            metrics.labelSpacing = 5.0f;
            metrics.rowPitch = 19.0f;
        }
        return metrics;
    }

    const FormPopUpMetrics& FormPopUp() {
        static const FormPopUpMetrics metrics;
        return metrics;
    }

    BezelPopUpMetrics BezelPopUp() {
        BezelPopUpMetrics metrics;
        switch (Environment().controlSize) {
            case ControlSize::Mini:
                metrics.labelSize = 9.0f;
                metrics.height = 15.0f;
                metrics.radius = 3.5f;
                metrics.labelInset = 5.0f;
                metrics.gap = 6.0f;
                metrics.indicator = 9.0f;
                metrics.indicatorRadius = 2.5f;
                metrics.chevrons = ChevronPair{3.75f, ImVec2(4.8f, 4.8f), 0.5f, 3.6f};
                break;
            case ControlSize::Small:
                metrics.labelSize = 11.0f;
                metrics.height = 18.0f;
                metrics.radius = 4.0f;
                metrics.labelInset = 6.0f;
                metrics.gap = 8.0f;
                metrics.indicator = 12.0f;
                metrics.indicatorRadius = 3.0f;
                metrics.chevrons = ChevronPair{4.9f, ImVec2(6.2f, 6.2f), 0.7f, 5.25f};
                break;
            case ControlSize::Large:
            case ControlSize::ExtraLarge:
                metrics.height = 30.0f;
                metrics.radius = 7.0f;
                metrics.gap = 10.0f;
                metrics.indicator = 24.0f;
                metrics.indicatorRadius = 6.0f;
                metrics.chevrons = ChevronPair{8.5f, ImVec2(10.8f, 10.8f), 3.0f, 10.7f};
                break;
            case ControlSize::Regular:
                break;
        }
        return metrics;
    }

    const SegmentedMetrics& Segmented() {
        static const SegmentedMetrics metrics;
        return metrics;
    }

    SliderMetrics Slider() {
        SliderMetrics metrics;
        if (Environment().controlSize == ControlSize::Mini) {
            metrics.height = 13.0f;
            metrics.knob = 13.0f;
            metrics.trackHeight = 3.0f;
            metrics.barKnob = ImVec2(5.5f, 13.0f);
        } else if (Environment().controlSize == ControlSize::Small) {
            metrics.height = 16.0f;
            metrics.knob = 16.0f;
            metrics.barKnob = ImVec2(6.5f, 16.0f);
        }
        return metrics;
    }

    const PathControlMetrics& PathControl() {
        static const PathControlMetrics metrics;
        return metrics;
    }

    const LevelIndicatorMetrics& LevelIndicator() {
        static const LevelIndicatorMetrics metrics;
        return metrics;
    }

    const DatePickerMetrics& DatePicker() {
        static const DatePickerMetrics metrics;
        return metrics;
    }

    const StepperMetrics& Stepper() {
        static const StepperMetrics metrics;
        return metrics;
    }

    const TextFieldMetrics& TextField() {
        static const TextFieldMetrics metrics;
        return metrics;
    }

    const ProgressMetrics& Progress() {
        static const ProgressMetrics metrics;
        return metrics;
    }

    const DisclosureMetrics& Disclosure() {
        static const DisclosureMetrics metrics;
        return metrics;
    }

    const ColorWellMetrics& ColorWell() {
        static const ColorWellMetrics metrics;
        return metrics;
    }

    const ColorPanelMetrics& ColorPanel() {
        static const ColorPanelMetrics metrics;
        return metrics;
    }

    const ImageWellMetrics& ImageWell(bool large) {
        static const ImageWellMetrics kit;
        static const ImageWellMetrics wide = {.inset = 2.5f, .radius = 4.0f, .rim = 2.0f, .imageRadius = 2.0f};
        return large ? wide : kit;
    }

    const PopoverMetrics& Popover() {
        static const PopoverMetrics metrics;
        return metrics;
    }

    const AlertMetrics& Alert() {
        static const AlertMetrics metrics;
        return metrics;
    }

    const TooltipMetrics& Tooltip() {
        static const TooltipMetrics metrics;
        return metrics;
    }

    const TokenFieldMetrics& TokenField() {
        static const TokenFieldMetrics metrics;
        return metrics;
    }

    const ControlGroupMetrics& ControlGroup() {
        static const ControlGroupMetrics metrics;
        return metrics;
    }

    const NotificationMetrics& Notification() {
        static const NotificationMetrics metrics;
        return metrics;
    }

    const GroupBoxMetrics& GroupBox() {
        static const GroupBoxMetrics metrics;
        return metrics;
    }

    const ScrollViewMetrics& ScrollView() {
        static const ScrollViewMetrics metrics;
        return metrics;
    }

    const TabBarMetrics& TabBar() {
        static const TabBarMetrics metrics;
        return metrics;
    }

    const SheetMetrics& Sheet() {
        static const SheetMetrics metrics;
        return metrics;
    }

    const FormMetrics& Form() {
        static const FormMetrics metrics;
        return metrics;
    }

    const TableMetrics& Table() {
        static const TableMetrics metrics;
        return metrics;
    }

    const BorderedListMetrics& BorderedList() {
        static const BorderedListMetrics metrics;
        return metrics;
    }

    const ChartMetrics& Chart() {
        static const ChartMetrics metrics;
        return metrics;
    }

    const GaugeMetrics& Gauge() {
        static const GaugeMetrics metrics;
        return metrics;
    }

    PushButtonMetrics PushButton(ControlSize size) {
        switch (size) {
            case ControlSize::Mini:
                return PushButtonMetrics{.height = 13.0f, .radius = 3.0f, .titleInset = 6.0f, .titleRaise = 0.0f, .titleSize = 9.0f, .titleLineHeight = 12.0f};
            case ControlSize::Small:
                return PushButtonMetrics{.height = 16.0f, .radius = 4.0f, .titleInset = 7.0f, .titleRaise = 0.0f, .titleSize = 11.0f, .titleLineHeight = 14.0f};
            case ControlSize::Large:
            case ControlSize::ExtraLarge:
                return PushButtonMetrics{.height = 28.0f, .radius = 6.0f, .titleInset = 12.0f};
            case ControlSize::Regular:
                break;
        }
        return PushButtonMetrics{};
    }

    const AccessoryBarMetrics& AccessoryBar() {
        static const AccessoryBarMetrics metrics;
        return metrics;
    }

    const HelpButtonMetrics& HelpButton() {
        static const HelpButtonMetrics metrics;
        return metrics;
    }

    const SidebarMetrics& Sidebar() {
        static const SidebarMetrics metrics;
        return metrics;
    }

    const WindowMetrics& Window() {
        static const WindowMetrics metrics;
        return metrics;
    }

    const ContentUnavailableMetrics& ContentUnavailable() {
        static const ContentUnavailableMetrics metrics;
        return metrics;
    }

    const SplitViewMetrics& SplitView() {
        static const SplitViewMetrics metrics;
        return metrics;
    }

    const MenuMetrics& Menu() {
        static const MenuMetrics metrics;
        return metrics;
    }

    const MenuBarMetrics& MenuBar() {
        static const MenuBarMetrics metrics;
        return metrics;
    }
} // namespace Cupertino::Metrics
