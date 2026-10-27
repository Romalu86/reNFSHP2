
const FlagDefinition ss_greybar_flags[] = {
    {"SS_GREYBAR", 0, 1}, {"SS_HORZ", 0, 2}, {"SS_OUTSIDE", 0, 4}, {nullptr, 0, 0}};

const FlagDefinition sbs_greybar_flags[] = {{"SBS_GREYBAR", 0, 1}, {nullptr, 0, 0}};

const FlagDefinition mqs_move_flags[] = {
    {"MQS_MOVE", 0, 1}, {"MQS_SIZE", 0, 2}, {nullptr, 0, 0}};

const FlagDefinition fs_greyframe_flags[] = {
    {"FS_GREYFRAME", 0, 16}, {"FS_RESIZE", 0, 15},  {"FS_RESIZEX", 0, 3},
    {"FS_RESIZEY", 0, 12},    {"FS_RESIZEL", 0, 1}, {"FS_RESIZER", 0, 2},
    {"FS_RESIZET", 0, 4},    {"FS_RESIZEB", 0, 8}, {nullptr, 0, 0}};

const FlagDefinition svs_novscroll_flags[] = {
    {"SVS_NOVSCROLL", 0, 1}, {"SVS_NOHSCROLL", 0, 2}, {nullptr, 0, 0}};

const FlagDefinition ais_noloop_flags[] = {{"AIS_NOLOOP", 0, 1}, {nullptr, 0, 0}};

const FlagDefinition static_image_style_flags[] = {{"SIS_FLIPV", 0, 8},
                                                   {"SIS_FLIPH", 0, 16},
                                                   {"SIS_ROTATE90", 0, 32},
                                                   {"SIS_ROTATE180", 0, 64},
                                                   {"SIS_ROTATE270", 0, 128},
                                                   {"SIS_SHAPE_OFFSET", 0, 1},
                                                   {"SIS_PRESERVE_YASPECT", 0, 2},
                                                   {"SIS_PRESERVE_XASPECT", 0, 4},
                                                   {"SIS_BLEND_ADD", 0, 256},
                                                   {"SIS_BLEND_ATTENUTATE", 0, 512},
                                                   {"SIS_BLEND_MODUALTE", 0, 1024},
                                                   {"SIS_NOXSCALE", 0, 2048},
                                                   {"SIS_NOYSCALE", 0, 4096},
                                                   {"SIS_TILE", 0, 8192},
                                                   {nullptr, 0, 0}};

const FlagDefinition image_box_style_flags[] = {
    {"IBS_FILL", 0, 1},       {"IBS_SHADOW", 0, 2},      {"IBS_BUTTON", 0, 8},
    {"IBS_ETCHED", 0, 4},     {"IBS_GOURAUD", 0, 16},    {"IBS_X", 0, 32},
    {"IBS_THINFRAME", 0, 64}, {"IBS_THICKFRAME", 0, 128}, {nullptr, 0, 0}};

const FlagDefinition image_style_flags[] = {{"IS_INHERITCOLOR", 0, 1}, {nullptr, 0, 0}};

const FlagDefinition image_bar_style_flags[] = {{"IBRS_VERTICAL", 0, 1}, {nullptr, 0, 0}};

const FlagDefinition gripper_style_flags[] = {{"GRS_BRINGTOTOP", 0, 1}, {nullptr, 0, 0}};

const FlagDefinition ds_keyok_flags[] = {{"DS_KEYOK", 0, 1},
                                         {"DS_KEYCANCEL", 0, 2},
                                         {"DS_NOAUTOCENTER", 0, 4},
                                         {"DS_CANCELONNCHIT", 0, 8},
                                         {nullptr, 0, 0}};

const FlagDefinition ils_clip_flags[] = {{"ILS_CLIP", 0, 1},
                                         {"ILS_SHAPEOFFSET", 0, 2},
                                         {"ILS_SHAPESIZE", 0, 4},
                                         {"ILS_SELECTDISABLED", 0, 8},
                                         {nullptr, 0, 0}};

const FlagDefinition bs_greybutton_flags[] = {
    {"BS_GREYBUTTON", 0, 1},    {"BS_CHECK", 0, 2},
    {"BS_RADIO", 0, 4},         {"BS_REPEAT", 0, 8},
    {"BS_DEFAULTCHECK", 0, 16}, {"BS_INHERITSTATE", 0, 32},
    {"BS_AUTOBUTTON", 0, 64},   {nullptr, 0, 0}};

const FlagDefinition tbs_bringtotop_flags[] = {{"TBS_BRINGTOTOP", 0, 1}, {nullptr, 0, 0}};

const FlagDefinition text_style_flags[] = {{"TS_MULTILINE", 0, 1},   {"TS_RIGHT", 0, 2},
                                           {"TS_CENTER", 0, 4},      {"TS_CLIP", 0, 8},
                                           {"TS_WORDWRAP", 0, 17},   {"TS_OUTLINE", 0, 32},
                                           {"TS_BACKGROUND", 0, 64}, {"TS_VCENTER", 0, 128},
                                           {"TS_PASSWORD", 0, 256},  {nullptr, 0, 0}};

const FlagDefinition filter_alpha_flags[] = {{"FILTER_ALPHA", 0, 4},
                                             {"FILTER_NUMERIC", 0, 8},
                                             {"FILTER_PUNCT", 0, 16},
                                             {"FILTER_WHITESPACE", 0, 32},
                                             {"FILTER_LEADINGWHITESPACE", 0, 64},
                                             {"FILTER_HTMLTOKENS", 0, 128},
                                             {"FILTER_FORFILEPATH", 0, 4096},
                                             {"FILTER_FORFILEEXT", 0, 8192},
                                             {"FILTER_FORFILENAME", 0, 16384},
                                             {"FILTER_FORNUMBER", 0, 32768},
                                             {"FILTER_ENSUREINFONT", 0, 65536},
                                             {nullptr, 0, 0}};

const FlagDefinition ets_forceupper_flags[] = {
    {"ETS_FORCEUPPER", 0, 1},      {"ETS_FORCELOWER", 0, 2},
    {"ETS_READONLY", 0, 4},        {"ETS_MULTISELECT", 0, 8},
    {"ETS_HIDCARET", 0, 16},       {"ETS_NOAUTOSCROLL", 0, 32},
    {"ETS_PALETTE", 0, 64},        {"ETS_WANTRETURN", 0, 128},
    {"ETS_LIMITTOBOUNDS", 0, 256}, {nullptr, 0, 0}};

const FlagDefinition ls_multiselect_flags[] = {
    {"LS_MULTISELECT", 0, 1},  {"LS_CHECKSELECT", 0, 2},    {"LS_DIVIDER", 0, 4},
    {"LS_ITEMTOOLTIPS", 0, 8}, {"LS_MENUSELECT", 0, 16},    {"LS_VARITEM", 0, 32},
    {"LS_DROPDOWN", 0, 64},    {"LS_WANTKEYINPUT", 0, 128},  {"LS_OWNERDRAW", 0, 256},
    {"LS_HIDECARET", 0, 512},  {"LS_STDBACKROUND", 0, 1024}, {nullptr, 0, 0}};

const FlagDefinition cbs_userstrings_flags[] = {
    {"CBS_USERSTRINGS", 0, 1}, {"CBS_STATIC", 0, 2}, {nullptr, 0, 0}};

const FlagDefinition trs_lines_flags[] = {{"TRS_LINES", 0, 2},
                                          {"TRS_MULTISELECT", 0, 1},
                                          {"TRS_OWNERDRAW", 0, 4},
                                          {"TRS_DEFICON", 0, 8},
                                          {nullptr, 0, 0}};

const FlagDefinition ms_itemtooltips_flags[] = {{"MS_ITEMTOOLTIPS", 0, 1}, {nullptr, 0, 0}};
