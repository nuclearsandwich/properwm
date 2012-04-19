/* See LICENSE file for copyright and license details. */

/* appearance */
static const char font[]            = "-*-terminus-medium-r-*-*-16-*-*-*-*-*-*-*";
//static const char font              = "Terminus"
//static const char fontsize          = 16;
static const char normbordercolor[] = "#222222";
static const char normbgcolor[]     = "#111111";
static const char normfgcolor[]     = "#888888";
static const char selbordercolor[]  = "#CCCCCC";
static const char selbgcolor[]      = "#333333";
static const char selfgcolor[]      = "#CCCCCC";
static const unsigned int borderpx  = 1;        /* window border width */
static const unsigned int padding   = 10;       /* window padding */
static const unsigned int snap      = 20;       /* snap threshold */
static const bool showbar           = true;     /* bar visibility */
static const bool topbar            = true;     /* place bar at top (false = bottom) */
static const bool ffm               = false;    /* focus follows mouse */
static const bool smartborders      = true;     /* smart borders */

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
    /* xprop(1):
     *    WM_CLASS(STRING) = instance, class
     *    WM_NAME(STRING) = title
     */
    /* class      instance    title       tags mask     isfloating   monitor */
    { "Gimp",     NULL,       NULL,       0,            true,        -1 },
    { "Firefox",  NULL,       NULL,       1 << 8,       false,       -1 },
};

/* layout(s) */
static const float mfact      = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster      = 2;    /* number of clients in master area */
static const bool resizehints = false; /* true means respect size hints in tiled resizals */

static const Layout layouts[] = {
    /* symbol     arrange function */
    { "ST", stack },
    { "TI", tile },
    { "FL", NULL },
    { "MO", monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *dmenucmd[] = { "dmenu_run", "-b", "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "roxterm", NULL };
static const char *browsercmd[] = { "chromium", NULL };

static Key keys[] = {
    /* modifier                     key        function        argument */
    { MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
    { MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
    { MODKEY|ShiftMask,             XK_b,      spawn,          {.v = browsercmd } },
    { MODKEY,                       XK_b,      togglebar,      {0} },
    { MODKEY,                       XK_v,      togglebarpos,   {0} },
    { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
    { MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
    { MODKEY|ShiftMask,             XK_j,      pushdown,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_k,      pushup,         {.i = -1 } },
    { MODKEY,                       XK_a,      incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_z,      incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_x,      resetnmaster,   {0} },
    { MODKEY,                       XK_h,      setmfact,       {.f = -0.02} },
    { MODKEY,                       XK_l,      setmfact,       {.f = +0.02} },
    { MODKEY,                       XK_Return, zoom,           {0} },
    { MODKEY,                       XK_Tab,    view,           {0} },
    { MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
    { MODKEY,                       XK_s,      setlayout,      {.v = &layouts[0]} },
    { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[1]} },
    { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[2]} },
    { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[3]} },
    { MODKEY,                       XK_space,  togglefloating, {0} },
    { MODKEY,                       XK_0,      view,           {.ui = -1 } },
    { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = -1 } },
    { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
    { MODKEY,                       XK_period, focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
    TAGKEYS(                        XK_1,                      0)
    TAGKEYS(                        XK_2,                      1)
    TAGKEYS(                        XK_3,                      2)
    TAGKEYS(                        XK_4,                      3)
    TAGKEYS(                        XK_5,                      4)
    TAGKEYS(                        XK_6,                      5)
    TAGKEYS(                        XK_7,                      6)
    TAGKEYS(                        XK_8,                      7)
    TAGKEYS(                        XK_9,                      8)
    { MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
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
    { ClkTagBar,            0,              Button1,        view,           {0} },
    { ClkTagBar,            0,              Button3,        toggleview,     {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

