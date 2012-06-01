//
// GENERAL APPEARANCE
//

static const char font_name[] = "Terminus";
static const int font_size = 16;
static const int bar_height = 22;

//
// COLORS
//

// 6-digit colors only

static const char normal_border_color[] = "#383838";
static const char selected_border_color[] = "#ABCC00";

// 8-digit colors optional

static const char base_color[] = "#000000CC";

static const char normal_tag_bg_color[] = "#000000CC";
static const char normal_tag_fg_color[] = "#666666";

static const char selected_tag_bg_color[] = "#252525CC";
static const char selected_tag_fg_color[] = "#AAAAAA";

static const char ltsym_bg_color[] = "#3A6CA6CC";
static const char ltsym_fg_color[] = "#AAAAAA";

static const char title_bg_color[] = "#000000CC";
static const char title_fg_color[] = "#AAAAAA";

static const char status_bg_color[] = "#252525CC";
static const char status_fg_color[] = "#AAAAAA";

//
// GENERAL SETTINGS
//

static unsigned int borderpx = 1;   /* window border width */
static unsigned int padding = 10;   /* window padding */
static unsigned int snap = 10;      /* snap threshold */

static bool smartborders = true;    /* smart borders */
static bool showbar = true;         /* bar visibility */
static bool topbar = true;          /* place bar at top (false = bottom) */
static bool ffm = false;            /* focus follows mouse */

//
// TAGS
//

static char* tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

//
// WINDOW RULES
//

static Rule rules[] = {
    /* xprop(1):
     *    WM_CLASS(STRING) = instance, class
     *    WM_NAME(STRING) = title
     */
    /* class    instance    title    tags mask    isfloating    monitor */
    { "Gimp", NULL, NULL, 0, true, -1 },
    { "Pavucontrol", NULL, NULL, 0, true, -1 },
    { "Pulseaudio-equalizer.py", NULL, NULL, 0, true, -1 },
};

//
// LAYOUTS
//

static Layout layouts[] = {
    /* symbol     arrange function */
    { "ST", stack },
    { "TI", tile },
    { "FL", NULL },
    { "MO", monocle },
};

//
// LAYOUT SETTINGS
//

static float mfact = 0.50;
static int nmaster = 2;
static bool tiled_size_hints = false;

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
// KEYS
//

#define MODKEY Mod4Mask

#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

static Key keys[] = {
    /* modifier                     key             function        argument */
    { MODKEY,                       XK_p,           spawn,          {.v = dmenucmd } },
    { MODKEY|ShiftMask,             XK_Return,      spawn,          {.v = termcmd } },
    { MODKEY|ShiftMask,             XK_b,           spawn,          {.v = browsercmd } },
    { MODKEY,                       XK_b,           togglebar,      {0} },
    { MODKEY,                       XK_v,           togglebarpos,   {0} },
    { MODKEY,                       XK_j,           focusstack,     {.i = +1 } },
    { MODKEY,                       XK_k,           focusstack,     {.i = -1 } },
    { MODKEY|ShiftMask,             XK_j,           pushdown,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_k,           pushup,         {.i = -1 } },
    { MODKEY,                       XK_a,           incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_z,           incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_x,           resetnmaster,   {0} },
    { MODKEY,                       XK_h,           setmfact,       {.f = -0.02} },
    { MODKEY,                       XK_l,           setmfact,       {.f = +0.02} },
    { MODKEY,                       XK_Return,      zoom,           {0} },
    { MODKEY,                       XK_Tab,         view,           {0} },
    { MODKEY|ShiftMask,             XK_c,           killclient,     {0} },
    { MODKEY,                       XK_s,           setlayout,      {.v = &layouts[0]} },
    { MODKEY,                       XK_t,           setlayout,      {.v = &layouts[1]} },
    { MODKEY,                       XK_f,           setlayout,      {.v = &layouts[2]} },
    { MODKEY,                       XK_m,           setlayout,      {.v = &layouts[3]} },
    { MODKEY,                       XK_space,       togglefloating, {0} },
    { MODKEY,                       XK_0,           view,           {.ui = -1 } },
    { MODKEY|ShiftMask,             XK_0,           tag,            {.ui = -1 } },
    { MODKEY,                       XK_comma,       focusmon,       {.i = -1 } },
    { MODKEY,                       XK_period,      focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,       tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period,      tagmon,         {.i = +1 } },
    TAGKEYS(                        XK_1,                           0)
    TAGKEYS(                        XK_2,                           1)
    TAGKEYS(                        XK_3,                           2)
    TAGKEYS(                        XK_4,                           3)
    TAGKEYS(                        XK_5,                           4)
    TAGKEYS(                        XK_6,                           5)
    TAGKEYS(                        XK_7,                           6)
    TAGKEYS(                        XK_8,                           7)
    TAGKEYS(                        XK_9,                           8)
    { MODKEY|ShiftMask,             XK_q,           quit,           {0} },
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
    { ClkLtSymbol,          MODKEY,         Button3,        setlayout,      {.v = &layouts[2]} },
    { ClkStatusText,        0,              Button3,        spawn,          {.v = termcmd } },
    { ClkWinTitle,          0,              Button1,        zoom,           {0} },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
};

