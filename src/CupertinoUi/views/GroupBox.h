#pragma once

#include <functional>
#include <initializer_list>
#include <span>
#include <string_view>

namespace Cupertino {
    // A group box like SwiftUI's GroupBox (NSBox): the content in a faintly tinted rounded box, under its label when it
    // has one. The box takes the offered width.
    void GroupBox(const std::function<void()>& content);
    void GroupBox(std::string_view label, const std::function<void()>& content);

    // A tab view as macOS draws SwiftUI's TabView (NSTabView): a group box with the tabs as a segmented control centered
    // on its top edge. content draws the selected tab.
    void TabView(int* selection, std::span<const char* const> tabs, const std::function<void(int tab)>& content);
    void TabView(int* selection, std::initializer_list<const char*> tabs, const std::function<void(int tab)>& content);
} // namespace Cupertino
