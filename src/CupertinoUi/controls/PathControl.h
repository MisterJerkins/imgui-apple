#pragma once

#include "IconPlate.h"

#include <initializer_list>
#include <span>

namespace Cupertino {
    // One location of a path: its name and its 16 pt icon.
    struct PathComponent {
        const char* title = "";
        Icon icon;
    };

    enum class PathControlStyle {
        // The locations side by side with chevrons between them, as Finder's path bar; names in the middle give way to
        // their icons when the path does not fit.
        Standard,
        // The last location with up and down chevrons at the trailing end; a click lists the path in a menu.
        PopUp,
    };

    struct PathControlOptions {
        PathControlStyle style = PathControlStyle::Standard;
    };

    // NSPathControl: the path from its root to the last location. Returns the index of the location clicked (or chosen
    // from the pop-up's menu) this frame, or -1.
    int PathControl(const char* id, std::span<const PathComponent> path, const PathControlOptions& options = {});
    int PathControl(const char* id, std::initializer_list<PathComponent> path, const PathControlOptions& options = {});
} // namespace Cupertino
