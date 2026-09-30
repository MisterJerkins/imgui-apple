#pragma once

namespace Cupertino {
    enum class ProgressViewStyle {
        // A bar for a known value, the spinner otherwise.
        Automatic,
        Linear,
        Circular,
    };

    struct ProgressViewOptions {
        ProgressViewStyle style = ProgressViewStyle::Automatic;
        // Length of a bar in points; 0 takes the offered width.
        float width = 0.0f;
        // SwiftUI's label and currentValueLabel: the title over a bar (under a spinner or ring) and the value under it in
        // small secondary text.
        const char* label = nullptr;
        const char* currentValueLabel = nullptr;
    };

    // Work of unknown length: the spinning indicator (32 pt, 16 pt at the small control size), or in the linear style
    // the bar with an accent segment sweeping back and forth (NSProgressIndicator's indeterminate bar).
    void ProgressView();
    void ProgressView(const ProgressViewOptions& options);

    // Progress toward completion, value in [0, 1]: a bar (6 pt, 3 pt small) or a ring (32 pt, 16 pt small).
    void ProgressView(float value, const ProgressViewOptions& options = {});
} // namespace Cupertino
