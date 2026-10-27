// Widget flag names, reserved fields and values.
const engine::pending::FlagDefinition widget_flags[] = {
    {"GS_DISABLE", 0, 1},
    {"GS_HIDE", 0, 2},
    {"GS_TAB", 0, 4},
    {"GS_TABWINDOW", 0, 131076},
    {"GS_ABSOLUTE", 0, 8},
    {"GS_OVERLAP", 0, 32},
    {"GS_NOSCROLL", 0, 64},
    {"GS_MODAL", 0, 128},
    {"GS_MOUSEFOCUS", 0, 256},
    {"GS_NOTIFY", 0, 512},
    {"GS_NOFOCUS", 0, 4096},
    {"GS_BACKGROUND", 0, 8192},
    {"GS_POPUP", 0, 16384},
    {"GS_NOCLIP", 0, 32768},
    {"GS_TOOLWINDOW", 0, 65536},
    {"GS_SILENT", 0, 262144},
    {"GS_INHERITTEXT", 0, 1048576},
    {"GS_CMDLISTENER", 0, 2097152},
    {"GS_DONTCLIPCHILDREN", 0, 524288},
    {nullptr, 0, 0}
};
const engine::pending::FlagDefinition layout_flags[] = {
    {"LAYOUT_RIGHT", 0, 1},
    {"LAYOUT_LEFT", 0, 2},
    {"LAYOUT_TOP", 0, 4},
    {"LAYOUT_BOTTOM", 0, 8},
    {"LAYOUT_VCENTER", 0, 16},
    {"LAYOUT_HCENTER", 0, 32},
    {"LAYOUT_SHRINKTOFIT", 0, 64},
    {"LAYOUT_SIZETOPARENT", 0, 128},
    {"LAYOUT_SIZETOVIEW", 0, 256},
    {"LAYOUT_SIZETOW", 0, 512},
    {"LAYOUT_SIZETOH", 0, 1024},
    {"LAYOUT_OUTER_RIGHT", 0, 2048},
    {"LAYOUT_OUTER_LEFT", 0, 4096},
    {"LAYOUT_OUTER_TOP", 0, 8192},
    {"LAYOUT_OUTER_BOTTOM", 0, 16384},
    {"LAYOUT_LAST", 0, 32768},
    {"LAYOUT_HIDDEN", 0, 65536},
    {"LAYOUT_LIMITTOPARENT", 0, 131072},
    {nullptr, 0, 0}
};
const engine::pending::FlagDefinition general_style_flags[] = {
    {"WS_STDTOOLTIP", 0, 1},
    {"WS_STDVSCROLL", 0, 4},
    {"WS_STDHSCROLL", 0, 8},
    {nullptr, 0, 0}
};
const engine::pending::FlagDefinition tab_key_flags[] = {
    {"TABKEY_TAB", 0, 1},
    {"TABKEY_LEFT", 0, 2},
    {"TABKEY_RIGHT", 0, 4},
    {"TABKEY_UP", 0, 8},
    {"TABKEY_DOWN", 0, 16},
    {nullptr, 0, 0}
};
