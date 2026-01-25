// src/ui/UIEnums.h
#pragma once

namespace ogle {

// Типы виджетов
enum class WidgetType {
    Button,
    Label,
    Checkbox,
    SliderFloat,
    SliderInt,
    InputText,
    Combo,
    Separator,
    SameLine,
    ProgressBar,
    ColorEdit,
    TreeNode,
    ListBox,
    Image,
    Custom
};

// Флаги окон
enum class WindowFlags {
    None = 0,
    NoTitleBar = 1 << 0,
    NoResize = 1 << 1,
    NoMove = 1 << 2,
    NoScrollbar = 1 << 3,
    NoCollapse = 1 << 4,
    AlwaysAutoResize = 1 << 5,
    NoBackground = 1 << 6,
    NoSavedSettings = 1 << 7,
    NoMouseInputs = 1 << 8,
    MenuBar = 1 << 9,
    HorizontalScrollbar = 1 << 10,
    NoFocusOnAppearing = 1 << 11,
    NoBringToFrontOnFocus = 1 << 12,
    AlwaysVerticalScrollbar = 1 << 13,
    AlwaysHorizontalScrollbar = 1 << 14,
    AlwaysUseWindowPadding = 1 << 15,
    NoNavInputs = 1 << 16,
    NoNavFocus = 1 << 17,
    UnsavedDocument = 1 << 18,
    NoDocking = 1 << 19,
    NoNav = NoNavInputs | NoNavFocus,
    NoDecoration = NoTitleBar | NoResize | NoScrollbar | NoCollapse,
    NoInputs = NoMouseInputs | NoNavInputs | NoNavFocus
};

// Флаги виджетов
enum class WidgetFlags {
    None = 0,
    ReadOnly = 1 << 0,
    NoLabel = 1 << 1,
    NoInput = 1 << 2,
    AlwaysActive = 1 << 3
};

// Выравнивание текста
enum class TextAlign {
    Left,
    Center,
    Right
};

// Состояния виджетов
enum class WidgetState {
    Normal,
    Hovered,
    Active,
    Disabled
};

} // namespace ogle