//
// SETTINGS
//

enum {
    TOP,
    BOTTOM
};

static const char font_name[] =         "Droid Sans";
static const int font_size =            11;
static const int bar_height =           26;

static int  bar_pos =                   TOP;            // bar position, TOP or BOTTOM
static unsigned int border_width =      1;              // window border width
static bool click_to_focus =            true;           // require click to focus windows with mouse, otherwise automatic
static bool client_indicators =         true;           // draw indicators showing which tag(s) selected client is on
static int nmaster =                    2;              // number of windows in master tiling area
static bool show_bar =                  true;           // bar visibility
static bool smart_borders =             true;           // only show borders if practical, ie: more than one tiled window + not using monocle layout
static unsigned int snap =              10;             // window snap threshold
static bool tiled_size_hints =          false;          // respect size hints for tiled windows

//
// COLORS
//

// 6-digit colors only

static const char normal_border_color[] = "#222222";
static const char selected_border_color[] = "#BFBFBF";
static const char urgent_border_color[] = "#FF0000";

// 8-digit colors optional

static const char unused_tag_bg_color[] = "#000000CC";
static const char unused_tag_fg_color[] = "#808080";

static const char normal_tag_bg_color[] = "#000000CC";
static const char normal_tag_fg_color[] = "#628AB3";

static const char current_tag_bg_color[] = "#000000CC";
static const char current_tag_fg_color[] = "#BFBFBF";

static const char selected_tag_bg_color[] = "#000000CC";
static const char selected_tag_fg_color[] = "#B39462";

static const char urgent_tag_bg_color[] = "#990000CC";
static const char urgent_tag_fg_color[] = "#FFFFFF";

static const char ltsym_bg_color[] = "#222222CC";
static const char ltsym_fg_color[] = "#BFBFBF";

static const char title_bg_color[] = "#000000CC";
static const char title_fg_color[] = "#BFBFBF";

static const char selstat_bg_color[] = "#000000CC";
static const char selstat_fg_color[] = "#BFBFBF";

static const char status_bg_color[] = "#222222CC";
static const char status_fg_color[] = "#BFBFBF";

static const char normal_mon_indicator_bg[] = "#000000CC";
static const char normal_mon_indicator_fg[] = "#444444CC";

static const char selected_mon_indicator_bg[] = "#000000CC";
static const char selected_mon_indicator_fg[] = "#628AB3";

//
// TAGS
//

static char* tags[] =           { "M1", "M2", "M3", "M4", "M5", "E6", "E7", "E8", "E9" };

static int layouts_init[] =     {    0,    0,    1,    1,    1,    2,    2,    2,    3 };
static float mfactors_init[] =  { 0.50, 0.50, 0.65, 0.65, 0.65, 0.65, 0.65, 0.65, 0.50 };
static int padding_init[] =     {    0,    0,    6,    6,    6,    6,    6,    6,    6 };

//
// LAYOUTS
//

static Layout layouts[] = {
//    symbol    function
    { "M",      monocle },
    { "S",      stack },
    { "T",      tile },
    { "F",      NULL }
};

//
// COMMANDS
//

static const char* dmenucmd[] = {
    "dmenu_run", "-b", "-fn", "-*-terminus-medium-*-*-*-16-*-*-*-*-*-*-*",
    "-nb", "#000000", "-nf", "#808080",
    "-sb", "#000000", "-sf", "#BFBFBF",
    NULL
};

static char* termcmd[]  = { "roxterm", NULL };
static char* browsercmd[] = { "chromium", NULL };

//
// WINDOW RULES
//

#define TAG(N) 1 << (N - 1)

static Rule rules[] = {
//
// xprop(1):
//    WM_CLASS(STRING) = instance, class
//    WM_NAME(STRING) = title
//
// ARGUMENTS: class, instance, title, tag mask, isfloating, monitor
//
    { "Roxterm", NULL, NULL, 0, false, -1 },
    { "Pavucontrol", NULL, NULL, 0, true, -1 },
    { "Pulseaudio-equalizer.py", NULL, NULL, 0, true, -1 },
};

//
// KEYS
//

#define MODKEY Mod4Mask

#define TAG_KEYS(KEY,TAG) \
    { MODKEY,                       KEY,            view,               {.ui = 1 << (TAG - 1)} }, \
    { MODKEY|ControlMask,           KEY,            toggle_view,        {.ui = 1 << (TAG - 1)} }, \
    { MODKEY|ShiftMask,             KEY,            tag,                {.ui = 1 << (TAG - 1)} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,            toggle_tag,         {.ui = 1 << (TAG - 1)} }

static Key keys[] = {
//  modifier                        key             function            argument */
    { MODKEY,                       XK_p,           spawn,              { .v = dmenucmd } },
    { MODKEY|ShiftMask,             XK_Return,      spawn,              { .v = termcmd } },
    { MODKEY|ShiftMask,             XK_b,           spawn,              { .v = browsercmd } },
    { MODKEY,                       XK_b,           toggle_bar,         { 0 } },
    { MODKEY,                       XK_v,           toggle_bar_pos,     { 0 } },
    { MODKEY,                       XK_j,           focus_stack,        { .i = +1 } },
    { MODKEY,                       XK_k,           focus_stack,        { .i = -1 } },
    { MODKEY|ShiftMask,             XK_j,           push_down,          { .i = +1 } },
    { MODKEY|ShiftMask,             XK_k,           push_up,            { .i = -1 } },
    { MODKEY,                       XK_a,           mod_nmaster,        { .i = +1 } },
    { MODKEY,                       XK_z,           mod_nmaster,        { .i = -1 } },
    { MODKEY,                       XK_minus,       mod_padding,        { .i = -2 } },
    { MODKEY,                       XK_equal,       mod_padding,        { .i = +2 } },
    { MODKEY,                       XK_x,           reset_nmaster,      { 0} },
    { MODKEY,                       XK_h,           mod_mfactor,        { .f = -0.01 } },
    { MODKEY,                       XK_l,           mod_mfactor,        { .f = +0.01 } },
    { MODKEY,                       XK_Return,      zoom,               { 0 } },
    { MODKEY,                       XK_Tab,         view,               { 0 } },
    { MODKEY|ShiftMask,             XK_c,           kill_client,        { 0 } },
    { MODKEY,                       XK_m,           set_layout,         { .v = &layouts[0] } },
    { MODKEY,                       XK_s,           set_layout,         { .v = &layouts[1] } },
    { MODKEY,                       XK_t,           set_layout,         { .v = &layouts[2] } },
    { MODKEY,                       XK_f,           set_layout,         { .v = &layouts[3] } },
    { MODKEY,                       XK_space,       toggle_floating,    { 0} },
    { MODKEY,                       XK_0,           view,               { .ui = -1 } },
    { MODKEY|ShiftMask,             XK_0,           tag,                { .ui = -1 } },
    { MODKEY,                       XK_comma,       focus_mon,          { .i = -1 } },
    { MODKEY,                       XK_period,      focus_mon,          { .i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,       tag_mon,            { .i = -1 } },
    { MODKEY|ShiftMask,             XK_period,      tag_mon,            { .i = +1 } },
    TAG_KEYS(                       XK_1,                               1),
    TAG_KEYS(                       XK_2,                               2),
    TAG_KEYS(                       XK_3,                               3),
    TAG_KEYS(                       XK_4,                               4),
    TAG_KEYS(                       XK_5,                               5),
    TAG_KEYS(                       XK_6,                               6),
    TAG_KEYS(                       XK_7,                               7),
    TAG_KEYS(                       XK_8,                               8),
    TAG_KEYS(                       XK_9,                               9),
    { MODKEY|ShiftMask,             XK_q,           quit,               { 0 } },
};

//
// BUTTONS
//
// click: ClickLayout, ClickStatus, ClickTitle, ClickWindow, ClickRoot
//

static Button buttons[] = {
//    click             event mask      button      function          argument */
    { ClickLayout,      0,              Button1,    set_layout,       { .v = &layouts[0] } },
    { ClickLayout,      0,              Button3,    set_layout,       { .v = &layouts[1] } },
    { ClickLayout,      MODKEY,         Button1,    set_layout,       { .v = &layouts[2] } },
    { ClickLayout,      MODKEY,         Button3,    set_layout,       { .v = &layouts[3] } },
    { ClickStatus,      0,              Button3,    spawn,            { .v = termcmd } },
    { ClickTitle,       0,              Button1,    zoom,             { 0 } },
    { ClickWindow,      MODKEY,         Button1,    move_mouse,       { 0 } },
    { ClickWindow,      MODKEY,         Button2,    toggle_floating,  { 0 } },
    { ClickWindow,      MODKEY,         Button3,    resize_mouse,     { 0 } },
};

