//
// SETTINGS
//

enum {
    TOP,
    BOTTOM
};

static const char font_name[] =     "Droid Sans";
static const int font_size =        11;
static const int bar_height =       26;

static int  bar_pos =               TOP;    // bar position, TOP or BOTTOM
static unsigned int border_width =  1;      // window border width
static bool click_to_focus =        true;   // require click to focus windows with mouse, otherwise automatic
static bool client_indicator =      true;   // draw indicator showing which tag(s) selected client is on
static int nmaster =                2;      // number of windows in master tiling area
static bool show_bar =              true;   // bar visibility
static bool smart_borders =         true;   // only show borders if practical, ie: more than one tiled window + not using monocle layout
static unsigned int snap =          10;     // window snap threshold
static bool tiled_size_hints =      false;  // respect size hints for tiled windows

//
// COLORS
//

// 6-digit colors only

static const char normal_border_color[] = "#444444";
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
    /* symbol     arrange function */
    { "M", monocle },
    { "S", stack },
    { "T", tile },
    { "F", NULL },
};

//
// COMMANDS
//

static const char* dmenucmd[] = {
    "dmenu_run", "-b", "-fn", "-*-terminus-medium-*-*-*-16-*-*-*-*-*-*-*",
    "-nb", "#000000", "-nf", "#666666",
    "-sb", "#252525", "-sf", "#AAAAAA",
    NULL
};

static char* termcmd[]  = { "roxterm", NULL };
static char* browsercmd[] = { "chromium", NULL };

//
// WINDOW RULES
//

static Rule rules[] = {
    /* xprop(1):
     *    WM_CLASS(STRING) = instance, class
     *    WM_NAME(STRING) = title
     */
    /* class    instance    title    tags mask    isfloating    monitor */
    { "Gimp",   NULL,       NULL,    0,           true,         -1 },
    { "Pavucontrol", NULL,  NULL,    0,           true,         -1 },
    { "Pulseaudio-equalizer.py", NULL, NULL, 0,   true,         -1 },
};

//
// KEYS
//

#define MODKEY Mod4Mask

#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

static Key keys[] = {
    /* modifier                     key             function           argument */
    { MODKEY,                       XK_p,           spawn,             {.v = dmenucmd } },
    { MODKEY|ShiftMask,             XK_Return,      spawn,             {.v = termcmd } },
    { MODKEY|ShiftMask,             XK_b,           spawn,             {.v = browsercmd } },
    { MODKEY,                       XK_b,           togglebar,         {0} },
    { MODKEY,                       XK_v,           togglebarpos,      {0} },
    { MODKEY,                       XK_j,           focusstack,        {.i = +1 } },
    { MODKEY,                       XK_k,           focusstack,        {.i = -1 } },
    { MODKEY|ShiftMask,             XK_j,           pushdown,          {.i = +1 } },
    { MODKEY|ShiftMask,             XK_k,           pushup,            {.i = -1 } },
    { MODKEY,                       XK_a,           modnmaster,        {.i = +1 } },
    { MODKEY,                       XK_z,           modnmaster,        {.i = -1 } },
    { MODKEY,                       XK_minus,       modpadding,        {.i = -2 } },
    { MODKEY,                       XK_equal,       modpadding,        {.i = +2 } },
    { MODKEY,                       XK_x,           resetnmaster,      {0} },
    { MODKEY,                       XK_h,           modmfactor,        {.f = -0.01} },
    { MODKEY,                       XK_l,           modmfactor,        {.f = +0.01} },
    { MODKEY,                       XK_Return,      zoom,              {0} },
    { MODKEY,                       XK_Tab,         view,              {0} },
    { MODKEY|ShiftMask,             XK_c,           killclient,        {0} },
    { MODKEY,                       XK_m,           setlayout,         {.v = &layouts[0]} },
    { MODKEY,                       XK_s,           setlayout,         {.v = &layouts[1]} },
    { MODKEY,                       XK_t,           setlayout,         {.v = &layouts[2]} },
    { MODKEY,                       XK_f,           setlayout,         {.v = &layouts[3]} },
    { MODKEY,                       XK_space,       togglefloating,    {0} },
    { MODKEY,                       XK_0,           view,              {.ui = -1 } },
    { MODKEY|ShiftMask,             XK_0,           tag,               {.ui = -1 } },
    { MODKEY,                       XK_comma,       focusmon,          {.i = -1 } },
    { MODKEY,                       XK_period,      focusmon,          {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,       tagmon,            {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period,      tagmon,            {.i = +1 } },
    TAGKEYS(                        XK_1,                              0)
    TAGKEYS(                        XK_2,                              1)
    TAGKEYS(                        XK_3,                              2)
    TAGKEYS(                        XK_4,                              3)
    TAGKEYS(                        XK_5,                              4)
    TAGKEYS(                        XK_6,                              5)
    TAGKEYS(                        XK_7,                              6)
    TAGKEYS(                        XK_8,                              7)
    TAGKEYS(                        XK_9,                              8)
    { MODKEY|ShiftMask,             XK_q,           quit,              {0} },
};

//
// BUTTONS
//
// click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin
//

static Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {.v = &layouts[0]} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[1]} },
    { ClkLtSymbol,          MODKEY,         Button1,        setlayout,      {.v = &layouts[2]} },
    { ClkLtSymbol,          MODKEY,         Button3,        setlayout,      {.v = &layouts[3]} },
    { ClkStatusText,        0,              Button3,        spawn,          {.v = termcmd } },
    { ClkWinTitle,          0,              Button1,        zoom,           {0} },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
};

