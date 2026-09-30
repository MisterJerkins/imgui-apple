#include "MenuContent.h"

#include "Environment.h"
#include "State.h"

#include <utility>
#include <vector>

namespace Cupertino::MenuContent {
    struct MenuBuilder {
        std::vector<Entry> entries;
        // Submenu entries being filled, innermost last.
        std::vector<int> submenus;
        // Chosen in the frame before, reported by its view in this collect.
        int chosen = -1;
        int pending = -1;
        // A section ended: the next entry of its level comes after a separator.
        bool sectionEnded = false;
    };

    static MenuBuilder* current = nullptr;
    static BarMenu barMenu = nullptr;

    bool IsCollecting() {
        return current != nullptr;
    }

    std::span<const Entry> Collect(ImGuiID id, const std::function<void()>& content) {
        MenuBuilder& builder = State::Get<MenuBuilder>(id);
        builder.entries.clear();
        builder.submenus.clear();
        builder.chosen = builder.pending;
        builder.pending = -1;
        builder.sectionEnded = false;
        MenuBuilder* outer = current;
        current = &builder;
        content();
        current = outer;
        return builder.entries;
    }

    void Choose(ImGuiID id, int entry) {
        State::Get<MenuBuilder>(id).pending = entry;
    }

    // Appends an entry to the innermost open submenu (or the menu itself) and counts it in every enclosing submenu.
    static int Append(Entry entry) {
        if (std::exchange(current->sectionEnded, false) && entry.kind != EntryKind::Separator)
            Append(Entry{.kind = EntryKind::Separator});
        for (const int submenu : current->submenus)
            ++current->entries[size_t(submenu)].children;
        current->entries.push_back(std::move(entry));
        return int(current->entries.size()) - 1;
    }

    bool AddItem(Entry item) {
        item.kind = EntryKind::Item;
        item.enabled = Environment().enabled;
        item.children = 0;
        const int index = Append(std::move(item));
        return index == current->chosen;
    }

    void AddSeparator() {
        Append(Entry{.kind = EntryKind::Separator});
    }

    void AddHeader(std::string_view title) {
        Append(Entry{.kind = EntryKind::Header, .title = std::string(title)});
    }

    // The entry added last to the innermost open level (or inside one of its submenus), or null while it is empty.
    static const Entry* LastInLevel() {
        const int first = current->submenus.empty() ? 0 : current->submenus.back() + 1;
        return int(current->entries.size()) > first ? &current->entries.back() : nullptr;
    }

    void BeginSection(const char* header) {
        const Entry* last = LastInLevel();
        if (last && last->kind != EntryKind::Separator)
            AddSeparator();
        if (header)
            AddHeader(header);
    }

    void EndSection() {
        current->sectionEnded = true;
    }

    void BeginSubmenu(std::string_view title, unsigned symbol) {
        const int index = Append(Entry{.kind = EntryKind::Submenu, .title = std::string(title), .symbol = symbol, .enabled = Environment().enabled});
        current->submenus.push_back(index);
    }

    void EndSubmenu() {
        current->submenus.pop_back();
        current->sectionEnded = false;
    }

    BarMenu CurrentBarMenu() {
        return barMenu;
    }

    void SetBarMenu(BarMenu bar_menu) {
        barMenu = bar_menu;
    }
} // namespace Cupertino::MenuContent
