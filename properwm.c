#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))

#include <errno.h>
#include <locale.h>

#include <loft.h>
#include <math.h>

#include <pango/pangocairo.h>
#include <signal.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#undef __USE_BSD

#include <strext.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define ISVISIBLE(C)            (((C)->tags & (C)->mon->tagset[(C)->mon->selected_tags]))
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)

#define LENGTH(X)               (sizeof X / sizeof X[0])
#define WIDTH(C)                ((C)->w + (2 * (C)->bw))
#define HEIGHT(C)               ((C)->h + (2 * (C)->bw))

#define TAGMASK                 ((1 << LENGTH(tags)) - 1)

enum {
    CursorNormal,
    CursorMove,
    CursorResize,
    CursorLast
};

enum {
    NetActiveWindow,
    NetClientList,
    NetSupported,
    NetWMName,
    NetWMState,
    NetWMFullscreen,
    NetWMWindowType,
    NetWMWindowTypeDialog,
    NetLast
};

enum {
    WMDelete,
    WMProtocols,
    WMState,
    WMTakeFocus,
    WMLast
};

enum {
    ClickLayout,
    ClickTitle,
    ClickStatus,
    ClickWindow,
    ClickRoot
};

typedef struct Monitor Monitor;
typedef struct Client Client;

typedef void (ArrangeFunc) (Monitor*);

typedef union {
    int i;
    unsigned int ui;
    float f;
    const void *v;
} Arg;

typedef void (UserFunc) (const Arg*);

typedef struct {
    unsigned int click;
    unsigned int mask;
    unsigned int button;
    UserFunc* func;
    const Arg arg;
} Button;

typedef struct Client {
    char name[256];
    float mina, maxa;
    int x, y, w, h;
    int fx, fy, fw, fh;
    int oldx, oldy, oldw, oldh;
    int basew, baseh, incw, inch, maxw, maxh, minw, minh;
    int bw, oldbw;
    unsigned int tags;
    bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
    Client* next;
    Client* snext;
    Monitor* mon;
    Window win;
} Client;

typedef struct {
    unsigned int mod;
    KeySym keysym;
    UserFunc* func;
    const Arg arg;
} Key;

typedef struct {
    const char *symbol;
    ArrangeFunc* arrange;
} Layout;

typedef struct {
    const char* class;
    const char* instance;
    const char* title;
    unsigned int tags;
    bool isfloating;
    int monitor;
} Rule;

typedef struct Indicator {
    LoftWidget base;

    struct {
        LoftRGBAPair normal;
        LoftRGBAPair selected;
    } style;

    bool active;
} Indicator;

typedef enum StrutPosition {
    STRUT_TOP,
    STRUT_BOTTOM,
    STRUT_LEFT,
    STRUT_RIGHT
} StrutPosition;

typedef struct TagLabel {
    LoftWidget base;

    struct {
        LoftRGBAPair unused;
        LoftRGBAPair normal;
        LoftRGBAPair current;
        LoftRGBAPair selected;
        LoftRGBAPair urgent;
    } style;

    int num;

    bool current;
    bool selected;
    bool unused;
    bool urgent;
} TagLabel;


//
// FUNCTION DECLARATIONS
//


void apply_rules (Client* c);
bool apply_size_hints (Client* c, int* x, int* y, int* w, int* h, bool interact);
void arrange (Monitor* m);
void arrange_mon (Monitor* m);
void attach_head (Client* c);
void attach_stack (Client* c);
void attach_tail (Client* c);
void button_press (XEvent* e);
void check_other_wm (void);
void clean_tag_focus (Monitor* m, Client* c);
void cleanup (void);
void clear_urgent (Client* c);
void client_message (XEvent* e);
void configure (Client* c);
void configure_notify (XEvent* e);
void configure_request (XEvent* e);
Monitor* create_mon (void);
void destroy_mon (Monitor* mon);
void destroy_notify (XEvent* e);
void detach (Client* c);
void detach_stack (Client* c);
void die (const char* errstr, ...);
Monitor* dir_to_mon (int dir);
void enter_notify (XEvent* e);
void focus (Client* c);
void focus_in (XEvent* e);
void focus_mon (const Arg* arg);
void focus_stack (const Arg* arg);
unsigned long get_color (const char* colstr);
bool get_root_ptr (int* x, int* y);
long get_state (Window w);
bool get_text_prop (Window w, Atom atom, char* text, unsigned int size);
void grab_buttons (Client* c, bool focused);
void grab_keys (void);
void iteration (void);
void key_press (XEvent *e);
void kill_client (const Arg* arg);
void manage (Window w, XWindowAttributes* wa);
void mapping_notify (XEvent* e);
void map_request (XEvent* e);
void mod_mfactor (const Arg* arg);
void mod_nmaster (const Arg* arg);
void mod_padding (const Arg* arg);
void monocle (Monitor* m);
void move_mouse (const Arg* arg);
Client* next_tiled (Client* c);
Indicator* new_indicator (void);
int n_tiled (Monitor* m);
void pop (Client* c);
Client* prev_tiled (Client* c);
void property_notify (XEvent* e);
void push_down (const Arg* arg);
void push_up (const Arg* arg);
void quit (const Arg* arg);
void reset_nmaster (const Arg* arg);
void resize (Client* c, int x, int y, int w, int h, bool interact);
void resize_client (Client* c, int x, int y, int w, int h);
void resize_mouse (const Arg* arg);
void restack (Monitor* m);
void run (void);
void scan (void);
bool send_event (Client* c, Atom proto);
void send_to_mon (Client* c, Monitor* m);
void set_client_state (Client* c, long state);
void set_focus (Client* c);
void set_fullscreen (Client* c, bool fullscreen);
void set_layout (const Arg* arg);
bool set_strut (Monitor* m, int pos, int px);
void setup (void);
void show_hide (Client* c);
void sig_child (int unused);
void spawn (const Arg* arg);
void stack (Monitor* m);
void tag (const Arg* arg);
void tag_mon (const Arg* arg);
void tile (Monitor* m);
void toggle_bar (const Arg* arg);
void toggle_bar_pos (const Arg* arg);
void toggle_floating (const Arg* arg);
void toggle_tag (const Arg* arg);
void toggle_view (const Arg* arg);
void unfocus (Client* c, bool setfocus);
void unmanage (Client* c, bool destroyed);
void unmap_notify (XEvent *e);
void update_bars (void);
void update_bar_layout (Monitor* m);
void update_bar_mon_selections (void);
void update_bar_window_stat (Monitor* m);
void update_bar_tags (Monitor* m);
void update_bar_title (Monitor* m);
void update_client_list (void);
bool update_geom (void);
void update_mon_indicators (void);
void update_numlock_mask (void);
void update_smart_borders (Monitor* m);
void update_size_hints (Client* c);
void update_status (void);
void update_struts (Monitor* m);
void update_title (Client* c);
void update_window_type (Client* c);
void update_wm_hints (Client* c);
void view (const Arg* arg);
Client* win_to_client (Window w);
Monitor* win_to_mon (Window w);
int xerror (Display* dpy, XErrorEvent* ee);
int xerror_dummy (Display* dpy, XErrorEvent* ee);
int xerror_start (Display* dpy, XErrorEvent* ee);
Monitor* xy_to_mon (int x, int y);
void zoom (const Arg* arg);


//
// GLOBALS
//


bool running = true;

Display* dpy;
Window root;

int screen;
int scr_width, scr_height;

Monitor* mons = NULL;
Monitor* selmon = NULL;
bool multimon = false;

unsigned long border_normal;
unsigned long border_selected;
unsigned long border_urgent;

const char broken[] = "broken";
char status[256];

unsigned int numlockmask = 0;
Atom wmatom[WMLast], netatom[NetLast];
Cursor cursor[CursorLast];

int (*xerrorxlib)(Display *, XErrorEvent *);

void (*handler[LASTEvent]) (XEvent *) = {
    [ButtonPress] = button_press,
    [ClientMessage] = client_message,
    [ConfigureRequest] = configure_request,
    [ConfigureNotify] = configure_notify,
    [DestroyNotify] = destroy_notify,
    [EnterNotify] = enter_notify,
    [KeyPress] = key_press,
    [MappingNotify] = mapping_notify,
    [MapRequest] = map_request,
    [PropertyNotify] = property_notify,
    [UnmapNotify] = unmap_notify
};


#include "config.h"


//
// MONITOR (needs access to config.h)
//

typedef struct Bar {
    LoftWindow win;
    LoftLayout lt_main;

    LoftLayout lt_tagstrip;
    TagLabel lb_tags[LENGTH(tags)];

    LoftLabel lb_layout;
    LoftLabel lb_winstat;
    LoftLabel lb_title;
    LoftLabel lb_status;

    Indicator* indicator;
} Bar;

struct Monitor {
    int num;

    int mx, my, mw, mh;   /* screen size */
    int wx, wy, ww, wh;   /* window area  */

    unsigned int current_tag;
    unsigned int previous_tag;

    unsigned int selected_tags;
    unsigned int selected_lt;
    unsigned int tagset[2];

    Client *clients;
    Client *selected;
    Client *stack;

    Bar* bar;
    bool bar_pos;
    int bar_y;
    bool show_bar;

    const Layout* layouts[LENGTH(tags)];
    float mfactors[LENGTH(tags)];
    int nmasters[LENGTH(tags)];
    int padding[LENGTH(tags)];
    Client* tag_focus[LENGTH(tags)];

    int struts[4];
    char ltsymbol[12];
    char selstat[12];

    Monitor* next;
};


//
// LOFT SIGNAL CALLBACKS
//


void _on_tag_pressed (TagLabel* t, LoftButtonEvent* be) {
    Arg arg = { .ui = 1 << t->num};

    if (be->id == 1) {
        if (be->mask & MODKEY)
            tag(&arg);
        else
            view(&arg);
    }
    else if (be->id == 3) {
        if (be->mask & MODKEY)
            toggle_tag(&arg);
        else
            toggle_view(&arg);
    }
}

void _draw_tag (TagLabel* t, void* m) {
    Monitor* mon = (Monitor*) m;

    LoftRGBA* bg;
    LoftRGBA* fg;

    if (t->current) {
        bg = &t->style.current.bg;
        fg = &t->style.current.fg;
    }
    else if (t->selected) {
        bg = &t->style.selected.bg;
        fg = &t->style.selected.fg;
    }
    else if (t->unused) {
        bg = &t->style.unused.bg;
        fg = &t->style.unused.fg;
    }
    else if (t->urgent) {
        bg = &t->style.urgent.bg;
        fg = &t->style.urgent.fg;
    }
    else {
        bg = &t->style.normal.bg;
        fg = &t->style.normal.fg;
    }

    cairo_t* cr = cairo_create(t->base.cs);
    cairo_save(cr);

    loft_cairo_set_rgba(cr, bg);

    cairo_rectangle(cr, 0, 0, t->base.width, t->base.height);
    cairo_fill(cr);

    char* fstr = malloc(strlen(loftenv.font) + 5);
    sprintf(fstr, "%s %d", loftenv.font, loftenv.font_size);

    PangoLayout* layout = pango_cairo_create_layout(cr);
    PangoFontDescription* font_desc = pango_font_description_from_string(fstr);
    pango_layout_set_font_description(layout, font_desc);

    free(fstr);

    PangoRectangle ink;
    PangoRectangle logical;

    pango_layout_set_text(layout, tags[t->num], -1);
    pango_layout_get_extents(layout, &ink, &logical);

    int realw = logical.width / PANGO_SCALE;
    int realh = logical.height / PANGO_SCALE;

    double x = (t->base.width / 2) - (realw / 2);
    double y = (t->base.height / 2) - (realh / 2);

    loft_cairo_set_rgba(cr, fg);

    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, layout);

    pango_font_description_free(font_desc);
    g_object_unref(layout);

    if (selmon == mon) {
        int mask = (1 << t->num);
        bool indicate = selmon->selected != NULL && (mask & selmon->selected->tags) != 0 && selmon->selected->tags != mask;

        if (client_indicators && indicate) {
            double p = t->base.width / 8;

            double w = t->base.width - (p * 2);
            double h = 1;

            double y = selmon->bar_pos == TOP ? 0 : t->base.height - h;

            cairo_rectangle(cr, p, y, w, h);
            cairo_fill(cr);
        }
    }

    cairo_restore(cr);
    cairo_destroy(cr);
}

void _draw_indicator (Indicator* i) {
    cairo_t* cr = cairo_create(i->base.cs);
    cairo_save(cr);

    LoftRGBA* bg;
    LoftRGBA* fg;

    if (i->active) {
        bg = &i->style.selected.bg;
        fg = &i->style.selected.fg;
    } else {
        bg = &i->style.normal.bg;
        fg = &i->style.normal.fg;
    }

    loft_cairo_set_rgba(cr, bg);
    cairo_rectangle(cr, 0, 0, i->base.width, i->base.height);
    cairo_fill(cr);

    int min_aspect = MIN(i->base.width, i->base.height);

    loft_cairo_set_rgba(cr, fg);
    cairo_arc(cr, i->base.width / 2, i->base.height / 2, min_aspect / 6.0, 0.0, 2.0 * M_PI);
    cairo_fill(cr);

    cairo_restore(cr);
    cairo_destroy(cr);
}

// - - - - - - - - - - -

void apply_rules (Client* c) {
    c->isfloating = false;
    c->tags = 0;

    XClassHint ch = { NULL, NULL };
    XGetClassHint(dpy, c->win, &ch);

    const char* class = ch.res_class ? ch.res_class : broken;
    const char* instance = ch.res_name ? ch.res_name : broken;

    int i;
    const Rule* r;
    Monitor* m;

    bool tmatch;
    bool cmatch;
    bool imatch;

    for (i = 0; i < LENGTH(rules); i++) {
        r = &rules[i];

        tmatch = r->title == NULL || strstr(c->name, r->title);
        cmatch = r->class == NULL || strstr(class, r->class);
        imatch = r->instance == NULL || strstr(instance, r->instance);

        if (tmatch && cmatch && imatch) {
            c->isfloating = r->isfloating;
            c->tags |= r->tags;

            for (m = mons; m && r->monitor != m->num; m = m->next);

            if (m != NULL)
                c->mon = m;
        }
    }

    if (ch.res_class)
        XFree(ch.res_class);
    if (ch.res_name)
        XFree(ch.res_name);

    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->selected_tags];
}

bool apply_size_hints (Client* c, int* x, int* y, int* w, int* h, bool interact) {
    bool baseismin;
    Monitor* m = c->mon;

    /* set minimum possible */
    *w = MAX(1, *w);
    *h = MAX(1, *h);

    if (interact) {
        if (*x > scr_width)
            *x = scr_width - WIDTH(c);
        if (*y > scr_height)
            *y = scr_height - HEIGHT(c);
        if (*x + *w + 2 * c->bw < 0)
            *x = 0;
        if (*y + *h + 2 * c->bw < 0)
            *y = 0;
    } else {
        if (*x >= m->wx + m->ww)
            *x = m->wx + m->ww - WIDTH(c);
        if (*y >= m->wy + m->wh)
            *y = m->wy + m->wh - HEIGHT(c);
        if (*x + *w + 2 * c->bw <= m->wx)
            *x = m->wx;
        if (*y + *h + 2 * c->bw <= m->wy)
            *y = m->wy;
    }

    if (*h < bar_height)
        *h = bar_height;
    if (*w < bar_height)
        *w = bar_height;

    if (tiled_size_hints || c->isfloating || c->mon->layouts[c->mon->current_tag]->arrange == NULL) {
        /* see last two sentences in ICCCM 4.1.2.3 */

        baseismin = c->basew == c->minw && c->baseh == c->minh;
        if (baseismin == false) { /* temporarily remove base dimensions */
            *w -= c->basew;
            *h -= c->baseh;
        }

        /* adjust for aspect limits */

        if (c->mina > 0 && c->maxa > 0) {
            if (c->maxa < (float)*w / *h)
                *w = *h * c->maxa + 0.5;
            else if (c->mina < (float)*h / *w)
                *h = *w * c->mina + 0.5;
        }

        /* increment calculation requires this */

        if (baseismin) {
            *w -= c->basew;
            *h -= c->baseh;
        }

        /* adjust for increment value */

        if (c->incw)
            *w -= *w % c->incw;
        if (c->inch)
            *h -= *h % c->inch;

        /* restore base dimensions */

        *w = MAX(*w + c->basew, c->minw);
        *h = MAX(*h + c->baseh, c->minh);

        if (c->maxw)
            *w = MIN(*w, c->maxw);
        if (c->maxh)
            *h = MIN(*h, c->maxh);
    }

    return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void arrange (Monitor* m) {
    if (m) {
        show_hide(m->stack);
        arrange_mon(m);
        restack(m);
    }
    else for (m = mons; m != NULL; m = m->next) {
        show_hide(m->stack);
        arrange_mon(m);
    }
}

void arrange_mon (Monitor* m) {
    if (smart_borders)
        update_smart_borders(m);

    update_bar_layout(m);

    if (m->layouts[m->current_tag]->arrange != NULL)
        m->layouts[m->current_tag]->arrange(m);

    // restore floating geometry

    else {
        Client* c;
        for (c = m->clients; c != NULL; c = c->next) {
            if (ISVISIBLE(c) && c->isfullscreen == false)
                resize(c, c->fx, c->fy, c->fw, c->fh, false);
        }
    }
}

void attach_head (Client* c) {
    c->next = c->mon->clients;
    c->mon->clients = c;
}

void attach_stack (Client* c) {
    c->snext = c->mon->stack;
    c->mon->stack = c;
}

void attach_tail (Client* c) {
    Client* ct;

    for (ct = c->mon->clients; ct != NULL; ct = ct->next) {
        if (ct->next == NULL) {
            ct->next = c;
            return;
        }
    }

    if (ct == NULL) {
        c->next = c->mon->clients;
        c->mon->clients = c;
    }
}

void button_press (XEvent* e) {
    XButtonPressedEvent* ev = &e->xbutton;

    Client* c = win_to_client(ev->window);
    Monitor* m = win_to_mon(ev->window);

    if (m != NULL && m != selmon) {
        unfocus(selmon->selected, true);
        selmon = m;

        if (c == NULL)
            focus(m->tag_focus[m->current_tag]);

        update_bar_mon_selections();
    }

    int click = ClickRoot;

    if (c != NULL) {
        focus(c);
        restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
        click = ClickWindow;
    }
    else if (ev->window == selmon->bar->lb_layout.base.xwin)
        click = ClickLayout;
    else  if (ev->window == selmon->bar->lb_title.base.xwin)
        click = ClickTitle;
    else if (ev->window == selmon->bar->lb_status.base.xwin)
        click = ClickStatus;

    int i;

    for (i = 0; i < LENGTH(buttons); i++)
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
        && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(&buttons[i].arg);
}

void check_other_wm (void) {
    xerrorxlib = XSetErrorHandler(xerror_start);
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
    XSync(dpy, false);
    XSetErrorHandler(xerror);
    XSync(dpy, false);
}

void clean_tag_focus (Monitor* m, Client* c) {
    int i;
    for (i = 0; i < LENGTH(tags); i++) {
        if (m->tag_focus[i] == c)
            m->tag_focus[i] = NULL;
    }
}

void cleanup (void) {
    Arg a = {.ui = ~0};
    Layout foo = { "", NULL };
    Monitor *m;

    view(&a);
    selmon->layouts[selmon->current_tag] = &foo;

    for (m = mons; m; m = m->next) {
        while (m->stack)
            unmanage(m->stack, false);
    }

    XUngrabKey(dpy, AnyKey, AnyModifier, root);

    while (mons != NULL)
        destroy_mon(mons);

    XSync(dpy, false);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void clear_urgent (Client* c) {
    c->isurgent = false;

    XWMHints* wmh = XGetWMHints(dpy, c->win);

    if (wmh == NULL)
        return;

    wmh->flags &= ~XUrgencyHint;
    XSetWMHints(dpy, c->win, wmh);
    XFree(wmh);

    update_bar_tags(c->mon);
}

void client_message (XEvent* e) {
    XClientMessageEvent* cme = &e->xclient;
    Client* c = win_to_client(cme->window);

    if (c == NULL)
        return;

    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen])
            set_fullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD */
                          || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && c->isfullscreen == false)));
    }
    else if (cme->message_type == netatom[NetActiveWindow]) {
        if (ISVISIBLE(c) == false) {
            c->mon->selected_tags ^= 1;
            c->mon->tagset[c->mon->selected_tags] = c->tags;
        }
        pop(c);
    }
}

void configure (Client* c) {
    XConfigureEvent ce;

    ce.type = ConfigureNotify;
    ce.display = dpy;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = c->bw;
    ce.above = None;
    ce.override_redirect = false;

    XSendEvent(dpy, c->win, false, StructureNotifyMask, (XEvent *)&ce);
}

void configure_notify (XEvent* e) {
    XConfigureEvent* ev = &e->xconfigure;

    if (ev->window == root) {
        bool dirty = scr_width != ev->width || scr_height != ev->height;

        scr_width = ev->width;
        scr_height = ev->height;

        if (update_geom() || dirty) {
            update_bars();

            Monitor* m;
            for (m = mons; m; m = m->next) {
                loft_widget_move(&m->bar->win.base, m->mx, m->bar_y);
                loft_widget_resize(&m->bar->win.base, m->mw, m->mh);
            }

            focus(NULL);
            arrange(NULL);
        }
    }
}

void configure_request (XEvent* e) {
    Monitor* m;
    XConfigureRequestEvent* ev = &e->xconfigurerequest;
    XWindowChanges wc;

    Client* c = win_to_client(ev->window);

    if (c != NULL) {
        if (ev->value_mask & CWBorderWidth)
            c->bw = ev->border_width;
        else if (c->isfloating || selmon->layouts[selmon->current_tag]->arrange == NULL) {
            m = c->mon;

            if (ev->value_mask & CWX) {
                c->oldx = c->x;
                c->x = m->mx + ev->x;
            }
            if (ev->value_mask & CWY) {
                c->oldy = c->y;
                c->y = m->my + ev->y;
            }
            if (ev->value_mask & CWWidth) {
                c->oldw = c->w;
                c->w = ev->width;
            }
            if (ev->value_mask & CWHeight) {
                c->oldh = c->h;
                c->h = ev->height;
            }

            if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
                c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
            if ((c->y + c->h) > m->my + m->mh && c->isfloating)
                c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */

            if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
                configure(c);

            if (ISVISIBLE(c))
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        } else
            configure(c);
    }
    else {
        wc.x = ev->x;
        wc.y = ev->y;
        wc.width = ev->width;
        wc.height = ev->height;
        wc.border_width = ev->border_width;
        wc.sibling = ev->above;
        wc.stack_mode = ev->detail;
        XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
    }

    XSync(dpy, false);
}

Monitor* create_mon (void) {
    int i;

    Monitor* m = malloc(sizeof(Monitor));

    if (m == NULL)
        die("out of memory\n");

    m->current_tag = 0;
    m->previous_tag = 0;
    m->tagset[0] = m->tagset[1] = 1;

    m->clients = NULL;
    m->selected = NULL;
    m->stack = NULL;

    m->bar = NULL;
    m->bar_pos = bar_pos;
    m->bar_y = 0;
    m->show_bar = show_bar;

    m->ltsymbol[0] = '\0';
    m->selstat[0] = '\0';

    for (i = 0; i < LENGTH(tags); i++) {
        m->layouts[i] = &layouts[layouts_init[i]];
        m->mfactors[i] = mfactors_init[i];
        m->nmasters[i] = nmaster;
        m->padding[i] = padding_init[i];
        m->tag_focus[i] = NULL;
    }

    for (i = 0; i < 4; i++)
        m->struts[i] = 0;

    if (m->show_bar) {
        if (m->show_bar && m->bar_pos == TOP)
            m->struts[STRUT_TOP] = bar_height;
        else if (m->bar_pos == BOTTOM)
            m->struts[STRUT_BOTTOM] = bar_height;
    }

    update_struts(m);
    strncpy(m->ltsymbol, layouts[0].symbol, sizeof(m->ltsymbol));

    return m;
}

void destroy_mon (Monitor* mon) {
    Monitor* m;

    if (mon == mons)
        mons = mons->next;
    else {
        for (m = mons; m != NULL && m->next != mon; m = m->next);
        m->next = mon->next;
    }

    loft_widget_hide_all(&mon->bar->win.base);

    int i;
    for (i = 0; i < LENGTH(tags); i++)
        loft_widget_destroy(&mon->bar->lb_tags[i].base);

    loft_widget_destroy(&mon->bar->lb_layout.base);
    loft_widget_destroy(&mon->bar->lb_title.base);
    loft_widget_destroy(&mon->bar->lb_status.base);
    loft_widget_destroy(&mon->bar->lt_tagstrip.base);
    loft_widget_destroy(&mon->bar->lt_main.base);

    if (mon->bar->indicator != NULL) {
        loft_widget_destroy(&mon->bar->indicator->base);
        free(mon->bar->indicator);
    }

    free(mon->bar);
    free(mon);
}

void destroy_notify (XEvent* e) {
    XDestroyWindowEvent* ev = &e->xdestroywindow;
    Client* c = win_to_client(ev->window);

    if (c != NULL)
        unmanage(c, true);
}

void detach (Client* c) {
    Client** tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
}

void detach_stack (Client* c) {
    Client** tc;
    Client* t;

    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
    *tc = c->snext;

    if (c == c->mon->selected) {
        for (t = c->mon->stack; t != NULL && ISVISIBLE(t) == false; t = t->snext);
        c->mon->selected = t;
    }
}

void die (const char* errstr, ...) {
    va_list ap;

    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

Monitor* dir_to_mon (int dir) {
    Monitor* m = NULL;

    if (dir > 0) {
        m = selmon->next; // next monitor

        if (m == NULL)
            m = mons;
    }
    else if (selmon == mons)
        for (m = mons; m->next != NULL; m = m->next);
    else
        for (m = mons; m->next != selmon; m = m->next);

    return m;
}

void enter_notify (XEvent* e) {
    if (click_to_focus)
        return;

    XCrossingEvent* ev = &e->xcrossing;
    Client* c = win_to_client(ev->window);

    if (c != NULL) {
        focus(c);
        restack(c->mon);
    }
}

void focus (Client* c) {
    if (c == NULL || ISVISIBLE(c) == false)
        for (c = selmon->stack; c != NULL && ISVISIBLE(c) == false; c = c->snext);

    if (c != NULL && c != selmon->selected)
        unfocus(selmon->selected, true);

    if (c) {
        if (c->mon != selmon) {
            selmon = c->mon;
            update_bar_mon_selections();
        }

        if (c->isurgent)
            clear_urgent(c);

        detach_stack(c);
        attach_stack(c);

        XSetWindowBorder(dpy, c->win, border_selected);

        grab_buttons(c, true);
        set_focus(c);
    }
    else {
        XSetInputFocus(dpy, root, RevertToNone, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }

    selmon->selected = c;
    selmon->tag_focus[selmon->current_tag] = c;

    update_bar_tags(selmon);
    update_bar_title(selmon);

    update_bar_window_stat(selmon);
}

void focus_mon (const Arg* arg) {
    if (mons->next == NULL)
        return;

    Monitor* m = dir_to_mon(arg->i);

    if (m == selmon)
        return;

    unfocus(selmon->selected, true);

    Monitor* oldmon = selmon;
    selmon = m;

    update_bar_tags(oldmon);
    update_bar_mon_selections();

    focus(m->tag_focus[m->current_tag]);
}

void focus_stack (const Arg* arg) {
    Client* c = NULL;

    if (selmon->selected == NULL)
        return;

    if (arg->i > 0) {
        for (c = selmon->selected->next; c && !ISVISIBLE(c); c = c->next);
        if (c == NULL)
            for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
    }
    else {
        Client* i;

        for (i = selmon->clients; i != selmon->selected; i = i->next) {
            if (ISVISIBLE(i))
                c = i;
        }

        if (c == NULL) {
            for (; i; i = i->next) {
                if (ISVISIBLE(i))
                    c = i;
            }
        }
    }

    if (c) {
        focus(c);
        restack(selmon);
    }
}

Atom get_atom_prop (Client* c, Atom prop) {
    int di;
    unsigned long dl;
    unsigned char* p = NULL;
    Atom da;
    Atom atom = None;

    if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, false, XA_ATOM, &da, &di, &dl, &dl, &p) == Success && p) {
        memcpy(&atom, p, sizeof(Atom));
        XFree(p);
    }

    return atom;
}

unsigned long get_color (const char* cstr) {
    Colormap cmap = DefaultColormap(dpy, screen);
    XColor color;

    if (!XAllocNamedColor(dpy, cmap, cstr, &color, &color))
        die("error allocating color: \"%s\"\n", cstr);

    return color.pixel;
}

bool get_root_ptr (int* x, int* y) {
    int di;
    unsigned int dui;
    Window dummy;

    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long get_state (Window w) {
    int format;
    long result = -1;
    unsigned char* p = NULL;
    unsigned long n;
    unsigned long extra;
    Atom real;

    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, false, wmatom[WMState], &real, &format, &n, &extra, (unsigned char **)&p) != Success)
        return -1;

    if (n != 0)
        result = *p;

    XFree(p);
    return result;
}

bool get_text_prop (Window w, Atom atom, char* text, unsigned int size) {
    char** list = NULL;
    int n;
    XTextProperty name;

    if (text == NULL || size == 0)
        return false;

    text[0] = '\0';
    XGetTextProperty(dpy, w, &name, atom);

    if (name.nitems == 0)
        return false;

    if (name.encoding == XA_STRING)
        strncpy(text, (char*) name.value, size - 1);
    else {
        if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
            strncpy(text, *list, size - 1);
            XFreeStringList(list);
        }
    }

    text[size - 1] = '\0';
    XFree(name.value);

    return true;
}

void grab_buttons (Client* c, bool focused) {
    update_numlock_mask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (focused) {
            for (i = 0; i < LENGTH(buttons); i++) {
                if (buttons[i].click == ClickWindow) {
                    for (j = 0; j < LENGTH(modifiers); j++) {
                        XGrabButton(dpy, buttons[i].button,
                                    buttons[i].mask | modifiers[j],
                                    c->win, false, BUTTONMASK,
                                    GrabModeAsync, GrabModeAsync, None, None);
                    }
                }
            }
        }
        else
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, false,
                        BUTTONMASK, GrabModeAsync, GrabModeAsync, None, None);
    }
}

void grab_keys (void) {
    update_numlock_mask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        KeyCode code;

        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        for (i = 0; i < LENGTH(keys); i++) {
            if ((code = XKeysymToKeycode(dpy, keys[i].keysym))) {
                for (j = 0; j < LENGTH(modifiers); j++) {
                    XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
                         true, GrabModeAsync, GrabModeAsync);
                }
            }
        }
    }
}

#ifdef XINERAMA
static bool isuniquegeom(XineramaScreenInfo* unique, size_t n, XineramaScreenInfo* info) {
    while (n--) {
        if (unique[n].x_org == info->x_org
        && unique[n].y_org == info->y_org
        && unique[n].width == info->width
        && unique[n].height == info->height)
            return false;
    }
    return true;
}
#endif/* XINERAMA */

void iteration (void) {
    XEvent ev;
    XNextEvent(dpy, &ev);

    if (handler[ev.type])
        handler[ev.type](&ev);

    loft_process(&ev);
}

void key_press (XEvent* e) {
    unsigned int i;
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XkbKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0, 0);

    for (i = 0; i < LENGTH(keys); i++)
        if (keysym == keys[i].keysym
        && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
        && keys[i].func)
            keys[i].func(&(keys[i].arg));
}

void kill_client (const Arg* arg) {
    if (selmon->selected == NULL)
        return;

    if (!send_event(selmon->selected, wmatom[WMDelete])) {
        XGrabServer(dpy);
        XSetErrorHandler(xerror_dummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, selmon->selected->win);
        XSync(dpy, false);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
}

void manage (Window w, XWindowAttributes* wa) {
    Window trans = None;
    XWindowChanges wc;

    Client* c = malloc(sizeof(Client));

    if (c == NULL)
        die("out of memory\n");

    c->win = w;
    update_title(c);

    int ret = XGetTransientForHint(dpy, w, &trans);
    Client* t = win_to_client(trans);

    if (ret && t != NULL) {
        c->mon = t->mon;
        c->tags = t->tags;
    }
    else {
        c->mon = selmon;
        apply_rules(c);
    }

    c->oldbw = wa->border_width;

    c->w = c->oldw = wa->width;
    c->h = c->oldh = wa->height;

    // center window if coords are unset

    if (wa->x == 0 && wa->y == 0) {
        c->x = c->oldx = (c->mon->mw / 2) - (c->w / 2);
        c->y = c->oldy = (c->mon->wh / 2) - (c->h / 2);
    } else {
        c->x = c->oldx = wa->x;
        c->y = c->oldy = wa->y;
    }

    // set floating geometry

    c->fx = c->x;
    c->fy = c->y;
    c->fw = c->w;
    c->fh = c->h;

    update_window_type(c);
    update_size_hints(c);
    update_wm_hints(c);

    if (c->isfloating == false && (trans != None || c->isfixed))
        c->isfloating = c->oldstate = true;

    if (c->isfullscreen || (smart_borders && c->mon->layouts[c->mon->current_tag]->arrange != NULL && c->isfloating == false
    && (c->mon->layouts[c->mon->current_tag]->arrange == &monocle || n_tiled(c->mon) == 0)))
        c->bw = 0;
    else
        c->bw = border_width;

    wc.border_width = c->bw;

    XConfigureWindow(dpy, w, CWBorderWidth, &wc);
    XSetWindowBorder(dpy, w, border_normal);

    configure(c);

    XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    grab_buttons(c, false);

    if (attach_pos == HEAD)
        attach_head(c);
    else if (attach_pos == TAIL)
        attach_tail(c);

    attach_stack(c);

    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
                    (unsigned char*) &(c->win), 1);
//    XMoveResizeWindow(dpy, c->win, c->x + scr_width * 2, c->y, c->w, c->h);

    if (c->isfloating)
        XRaiseWindow(dpy, c->win);

    set_client_state(c, NormalState);

    if (c->mon == selmon)
        unfocus(selmon->selected, false);

    c->mon->selected = c;

    arrange(c->mon);
    XMapWindow(dpy, c->win);
    focus(NULL);
}

void mapping_notify (XEvent* e) {
    XMappingEvent* ev = &e->xmapping;
    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard)
        grab_keys();
}

void map_request (XEvent* e) {
    static XWindowAttributes wa;
    XMapRequestEvent* ev = &e->xmaprequest;

    if (!XGetWindowAttributes(dpy, ev->window, &wa))
        return;
    if (wa.override_redirect)
        return;
    if (win_to_client(ev->window) == NULL)
        manage(ev->window, &wa);
}

void mod_mfactor (const Arg* arg) {
    float f;

    if (arg == NULL || selmon->layouts[selmon->current_tag]->arrange == NULL)
        return;

    f = arg->f < 1.0 ? arg->f + selmon->mfactors[selmon->current_tag] : arg->f - 1.0;

    if (f < 0.1 || f > 0.9)
        return;

    selmon->mfactors[selmon->current_tag] = f;
    arrange(selmon);
}

void mod_nmaster (const Arg* arg) {
    selmon->nmasters[selmon->current_tag] = MAX(selmon->nmasters[selmon->current_tag] + arg->i, 0);
    arrange(selmon);
}

void mod_padding (const Arg* arg) {
    selmon->padding[selmon->current_tag] = MAX(selmon->padding[selmon->current_tag] + arg->i, 0);
    arrange(selmon);
}

void monocle (Monitor* m) {
    int p = m->padding[m->current_tag];
    int x = m->wx + p;
    int y = m->wy + p;
    int w = m->ww - (p * 2);
    int h = m->wh - (p * 2);

    Client* c;

    for (c = next_tiled(m->clients); c != NULL; c = next_tiled(c->next))
        resize(c, x,y, w - (c->bw * 2), h - (c->bw * 2), false);
}

void move_mouse (const Arg* arg) {
    if (selmon->selected == NULL)
        return;

    int x, y, ocx, ocy, nx, ny;
    XEvent ev;

    restack(selmon);

    Client* c = selmon->selected;
    ocx = c->x;
    ocy = c->y;

    if (XGrabPointer(dpy, root, false, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CursorMove], CurrentTime) != GrabSuccess)
        return;

    if (!get_root_ptr(&x, &y))
        return;

    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if (ev.xmotion.x < x)
                nx = ocx - (x - ev.xmotion.x);
            else
                nx = ocx + (ev.xmotion.x - x);

            if (ev.xmotion.y < y)
                ny = ocy - (y - ev.xmotion.y);
            else
                ny = ocy + (ev.xmotion.y - y);

            if (c->isfloating == false) {
                if (nx < selmon->wx)
                    nx = selmon->wx;
                else if (nx + WIDTH(c) > selmon->wx + selmon->ww)
                    nx = selmon->wx + selmon->ww - WIDTH(c);

                if (ny < selmon->wy)
                    ny = selmon->wy;
                else if (ny + HEIGHT(c) > selmon->wy + selmon->wh)
                    ny = selmon->wy + selmon->wh - HEIGHT(c);
            } else {
                if (nx > selmon->wx && nx < selmon->wx + snap)
                    nx = selmon->wx;
                else if (nx + WIDTH(c) < selmon->wx + selmon->ww && nx + WIDTH(c) > selmon->wx + selmon->ww - snap)
                    nx = selmon->wx + selmon->ww - WIDTH(c);

                if (ny > selmon->wy && ny < selmon->wy + snap)
                    ny = selmon->wy;
                else if (ny + HEIGHT(c) < selmon->wy + selmon->wh && ny + HEIGHT(c) > selmon->wy + selmon->wh - snap)
                    ny = selmon->wy + selmon->wh - HEIGHT(c);
            }

            if (c->isfloating == false && selmon->layouts[selmon->current_tag]->arrange != NULL
            && (ev.xmotion.x < x - snap || ev.xmotion.x > x + snap || ev.xmotion.y < y - snap || ev.xmotion.y > y + snap)) {
                c->fx = c->x;
                c->fy = c->y;
                c->fw = c->w;
                c->fh = c->h;
                toggle_floating(NULL);
            }

            if (c->isfloating || selmon->layouts[selmon->current_tag]->arrange == NULL) {
                if (c->isfullscreen)
                    set_fullscreen(c, false);

                resize(c, nx, ny, c->w, c->h, true);

                c->fx = c->x;
                c->fy = c->y;
            }

            break;
        }
    } while (ev.type != ButtonRelease);

    XUngrabPointer(dpy, CurrentTime);

    Monitor* m = xy_to_mon(c->x + (c->w / 2), c->y + (c->h / 2));

    if (m != NULL && m != selmon) {
        send_to_mon(c, m);
        selmon = m;
        focus(NULL);
        update_bar_mon_selections();
    }
}

Client* next_tiled (Client* c) {
    for(; c && (c->isfloating || ISVISIBLE(c) == false); c = c->next);
    return c;
}

Indicator* new_indicator (void) {
    Indicator* mi = malloc(sizeof(Indicator));
    loft_widget_init(&mi->base, "indicator", 0);
    mi->base.draw_base = false;

    loft_rgba_set_from_str(&mi->style.normal.bg, (char*) normal_mon_indicator_bg);
    loft_rgba_set_from_str(&mi->style.normal.fg, (char*) normal_mon_indicator_fg);
    loft_rgba_set_from_str(&mi->style.selected.bg, (char*) selected_mon_indicator_bg);
    loft_rgba_set_from_str(&mi->style.selected.fg, (char*) selected_mon_indicator_fg);

    mi->active = false;

    loft_widget_set_minimum_size(&mi->base, bar_height, bar_height);
    loft_signal_connect(&mi->base, "draw", _draw_indicator, NULL);

    return mi;
}

int n_tiled (Monitor* m) {
    Client* c;
    int nt = 0;

    for (c = next_tiled(m->clients); c != NULL; c = next_tiled(c->next)) {
        if (ISVISIBLE(c))
            nt++;
    }

    return nt;
}

void pop (Client* c) {
    detach(c);
    attach_head(c);
    focus(c);
    arrange(c->mon);
}

Client* prev_tiled (Client* c) {
    Client* p;
    Client* r;

    for (p = selmon->clients, r = NULL; p && p != c; p = p->next) {
        if (p->isfloating == false && ISVISIBLE(p))
            r = p;
    }

    return r;
}

void property_notify (XEvent* e) {
    Client* c;
    Window trans;
    XPropertyEvent* ev = &e->xproperty;

    if ((ev->window == root) && (ev->atom == XA_WM_NAME))
        update_status();
    else if (ev->state == PropertyDelete)
        return; /* ignore */
    else if ((c = win_to_client(ev->window))) {
        switch(ev->atom) {
        default: break;
        case XA_WM_TRANSIENT_FOR:
            if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
               (c->isfloating = (win_to_client(trans)) != NULL))
                arrange(c->mon);
            break;
        case XA_WM_NORMAL_HINTS:
            update_size_hints(c);
            break;
        case XA_WM_HINTS:
            update_wm_hints(c);
            break;
        }

        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
            update_title(c);
            if (c == c->mon->selected)
                update_bar_title(c->mon);
        }

        if (ev->atom == netatom[NetWMWindowType])
            update_window_type(c);
    }
}

void push_down (const Arg* arg) {
    Client* sel = selmon->selected;
    Client* c;

    if (sel == NULL || sel->isfloating)
        return;

    c = next_tiled(sel->next);

    if (c) {
        /* attach after c */
        detach(sel);
        sel->next = c->next;
        c->next = sel;
    } else {
        /* move to the front */
        detach(sel);
        attach_head(sel);
    }

    focus(sel);
    arrange(selmon);
}

void push_up (const Arg* arg) {
    Client* sel = selmon->selected;
    Client* c;

    if (sel == NULL || sel->isfloating)
        return;

    c = prev_tiled(sel);

    if (c) {
        /* attach before c */
        detach(sel);
        sel->next = c;
        if (selmon->clients == c)
            selmon->clients = sel;
        else {
            for(c = selmon->clients; c->next != sel->next; c = c->next);
            c->next = sel;
        }
    } else {
        /* move to the end */
        for (c = sel; c->next; c = c->next);
        detach(sel);
        sel->next = NULL;
        c->next = sel;
    }

    focus(sel);
    arrange(selmon);
}

void quit (const Arg* arg) {
    running = false;
}

void reset_nmaster (const Arg* arg) {
    selmon->nmasters[selmon->current_tag] = nmaster;
    arrange(selmon);
}

void resize (Client* c, int x, int y, int w, int h, bool interact) {
    if (apply_size_hints(c, &x, &y, &w, &h, interact))
        resize_client(c, x, y, w, h);
}

void resize_client (Client* c, int x, int y, int w, int h) {
    XWindowChanges wc;

    c->oldx = c->x;
    c->x = wc.x = x;

    c->oldy = c->y;
    c->y = wc.y = y;

    c->oldw = c->w;
    c->w = wc.width = w;

    c->oldh = c->h;
    c->h = wc.height = h;

    wc.border_width = c->bw;

    XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
    configure(c);

    XSync(dpy, false);
}

void resize_mouse (const Arg* arg) {
    if (selmon->selected == NULL)
        return;

    restack(selmon);

    Client* c = selmon->selected;
    int ocx = c->x;
    int ocy = c->y;

    if (XGrabPointer(dpy, root, false, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CursorResize], CurrentTime) != GrabSuccess)
        return;

    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);

    XEvent ev;
    int nw;
    int nh;

    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            nw = MAX(ev.xmotion.x - ocx - (2 * c->bw) + 1, 1);
            nh = MAX(ev.xmotion.y - ocy - (2 * c->bw) + 1, 1);

            if (c->mon->wx + nw >= selmon->wx && c->mon->wy + nh >= selmon->wy) {
                if (c->isfloating == false && selmon->layouts[selmon->current_tag]->arrange != NULL
                && (abs(nw - c->w) > snap || abs(nh - c->h) > snap)) {
                    c->fx = c->x;
                    c->fy = c->y;
                    c->fw = c->w;
                    c->fh = c->h;
                    toggle_floating(NULL);
                }
            }

            if (selmon->layouts[selmon->current_tag]->arrange == false || c->isfloating) {
                if (c->isfullscreen)
                    set_fullscreen(c, false);

                resize(c, c->x, c->y, nw, nh, true);

                c->fw = c->w;
                c->fh = c->h;
            }

            break;
        }
    } while (ev.type != ButtonRelease);

    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    XUngrabPointer(dpy, CurrentTime);

    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));

    Monitor* m = xy_to_mon(c->x + (c->w / 2), c->y + (c->h / 2));

    if (m != NULL && m != selmon) {
        send_to_mon(c, m);
        selmon = m;
        focus(NULL);
        update_bar_mon_selections();
    }
}

void restack (Monitor* m) {
    Client* c;
    XEvent ev;
    XWindowChanges wc;

    if (m->selected == NULL)
        return;

    if (m->selected->isfloating)
        XRaiseWindow(dpy, m->selected->win);

    if (m->layouts[m->current_tag]->arrange != NULL) {
        wc.stack_mode = Below;
        wc.sibling = m->bar->win.base.xwin;

        for (c = m->stack; c; c = c->snext) {
            if (c->isfloating == false && ISVISIBLE(c)) {
                XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
                wc.sibling = c->win;
            }
        }
    }

    XSync(dpy, false);
    while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void run (void) {
    XSync(dpy, false);

    loftenv.running = true;

    while (running)
        iteration();

    loftenv.running = false;
}

void scan (void) {
    unsigned int i, num;
    Window d1, d2;
    Window* wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa)
            || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                continue;
            if (wa.map_state == IsViewable || get_state(wins[i]) == IconicState)
                manage(wins[i], &wa);
        }

        for (i = 0; i < num; i++) { // transients
            if (!XGetWindowAttributes(dpy, wins[i], &wa))
                continue;
            if (XGetTransientForHint(dpy, wins[i], &d1)
            && (wa.map_state == IsViewable || get_state(wins[i]) == IconicState))
                manage(wins[i], &wa);
        }

        if (wins)
            XFree(wins);
    }
}

bool send_event (Client* c, Atom proto) {
    int n;
    Atom* protocols;
    bool exists = false;
    XEvent ev;

    if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
        while(!exists && n--)
            exists = protocols[n] == proto;
        XFree(protocols);
    }

    if (exists) {
        ev.type = ClientMessage;
        ev.xclient.window = c->win;
        ev.xclient.message_type = wmatom[WMProtocols];
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = proto;
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(dpy, c->win, false, NoEventMask, &ev);
    }

    return exists;
}

void send_to_mon (Client* c, Monitor* m) {
    if (c->mon == m)
        return;

    clean_tag_focus(c->mon, c);

    if (c->mon->selected == c)
        c->mon->selected = NULL;

    unfocus(c, true);

    detach(c);
    detach_stack(c);

    // - - - - - - - - - - -

    c->mon = m;
    c->tags = m->tagset[m->selected_tags];

    attach_head(c);
    attach_stack(c);

    if (m->selected == NULL) {
        m->selected = c;
        m->tag_focus[m->current_tag] = c;

        update_bar_tags(m);
        update_bar_title(m);
    }

    update_bar_window_stat(m); // update window stat of target monitor

    focus(NULL); // triggers update of tags,layout,title,stat
    arrange(NULL);
}

void set_client_state (Client* c, long state) {
    long data[] = { state, None };

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
            PropModeReplace, (unsigned char *)data, 2);
}

void set_focus (Client* c) {
    if (c->neverfocus == false) {
        XSetInputFocus(dpy, c->win, RevertToNone, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow],
                         XA_WINDOW, 32, PropModeReplace,
                         (unsigned char *) &(c->win), 1);
    }
    send_event(c, wmatom[WMTakeFocus]);
}

void set_fullscreen (Client* c, bool fullscreen) {
    if (fullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char*) &netatom[NetWMFullscreen], 1);

        c->oldstate = c->isfloating;
        c->isfloating = true;
        c->isfullscreen = true;

        c->oldbw = c->bw;
        c->bw = 0;

        resize_client(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(dpy, c->win);
    }
    else {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char*) 0, 0);

        c->isfloating = c->oldstate;
        c->isfullscreen = false;

        c->bw = c->oldbw;

        if (c->isfloating)
            resize(c, c->fx, c->fy, c->fw, c->fh, false);
    }

    arrange(c->mon);
}

void set_layout (const Arg* arg) {
    if (!arg || !arg->v || arg->v == selmon->layouts[selmon->current_tag])
        return;

    if (arg && arg->v)
        selmon->layouts[selmon->current_tag] = (Layout*) arg->v;

    const char* ltsym = selmon->layouts[selmon->current_tag]->symbol;

    if (ltsym != NULL)
        strncpy(selmon->ltsymbol, ltsym, sizeof(selmon->ltsymbol));
    else
        sprintf(selmon->ltsymbol, "<N>");

    arrange(selmon);
}

bool set_strut (Monitor* m, int pos, int px) {
    if (pos >= 4)
        return false;

    bool iter = (m == NULL);

    if (px <= 0)
        px = 0;
    else if (pos < 2 && px > m->mh/4)
        px = m->mh/4;
    else if (pos < 4 && px > m->mw/4)
        px = m->mw/4;

    if (iter) {
        for (m = mons; m != NULL; m = m->next)
            m->struts[pos] = px;
    } else
        m->struts[pos] = px;

    update_struts(m);

    if (running)
        arrange(m);

    return true;
}

void setup (void) {
    XSetWindowAttributes wa;

    sig_child(0);

    screen = loftenv.screen;
    root = loftenv.root;

    scr_width = DisplayWidth(dpy, screen);
    scr_height = DisplayHeight(dpy, screen);

    update_geom();
    update_bars();

    if (multimon) {
        update_mon_indicators();
        update_bar_mon_selections();
    }

    update_status();

    wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", false);
    wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
    wmatom[WMState] = XInternAtom(dpy, "WM_STATE", false);
    wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", false);

    netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false);
    netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", false);
    netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", false);
    netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", false);
    netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false);
    netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false);
    netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false);
    netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", false);

    cursor[CursorNormal] = XCreateFontCursor(dpy, XC_left_ptr);
    cursor[CursorResize] = XCreateFontCursor(dpy, XC_sizing);
    cursor[CursorMove] = XCreateFontCursor(dpy, XC_fleur);

    border_normal = get_color(normal_border_color);
    border_selected = get_color(selected_border_color);
    border_urgent = get_color(urgent_border_color);

    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
            PropModeReplace, (unsigned char *) netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);

    wa.cursor = cursor[CursorNormal];
    wa.event_mask = ButtonPressMask |
                    EnterWindowMask | LeaveWindowMask |
                    PropertyChangeMask |
                    StructureNotifyMask |
                    SubstructureNotifyMask | SubstructureRedirectMask;

    XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);

    grab_keys();
}

void show_hide (Client* c) {
    for (; c != NULL; c = c->snext) {
        if (ISVISIBLE(c)) {
            XMoveWindow(dpy, c->win, c->x, c->y);
        } else
            XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
    }
}

void sig_child (int unused) {
    if (signal(SIGCHLD, sig_child) == SIG_ERR)
        die("Can't install SIGCHLD handler");
    while(0 < waitpid(-1, NULL, WNOHANG));
}

void spawn (const Arg* arg) {
    if (fork() == 0) {
        char** dcp = (char**) arg->v;

        if (dpy)
            close(ConnectionNumber(dpy));

        setsid();
        execvp(dcp[0], dcp);

        fprintf(stderr, "properwm: execvp %s", dcp[0]);
        perror(" failed");

        exit(EXIT_SUCCESS);
    }
}

void stack (Monitor* m) {
    int n; // window count
    int nm; // master area window count
    int ns; // stack area window count
    int mh; // master area height
    int bw; // border width to use for calculations
    int p; // window padding
    int mx, sx, rem; // x positions and initial offset calculation
    int mww, mwh; // dimensions for windows in master area
    int sww, swh; // dimensions for windows in stack area
    int syp; // stack area y position padding

    n = n_tiled(m);
    nm = m->nmasters[m->current_tag];

    if (n <= nm) {
        if (n > 2)
            nm = n-1;
        else
            nm = 1;
    }

    ns = n-nm;

    if (n > nm)
        mh = nm ? m->wh * m->mfactors[m->current_tag] : 0;
    else
        mh = m->wh;

    bw = (smart_borders && n == 1 ? 0 : border_width);
    p = m->padding[m->current_tag];

    mx = 0;
    sx = 0;

    if (nm > 0) {
        // MASTER WINDOW WIDTH

        mww = (m->ww - ((2*bw) * nm) - (p * (nm+1))) / nm;

        // MASTER WINDOW HEIGHT
        //
        // when (n > nmaster) - subtract top padding and half of middle padding
        // when (n <= nmaster) - subtract both top and bottom padding

        mwh = mh - (2*bw) - (n > nm ? p + (p/2) : 2*p);

        /* MASTER X OFFSET */

        rem = (m->ww - ((2*bw) * nm) - (p * (nm+1))) % nm;
        if (rem > 0)
            mx += (rem/2);
    }
    else {
        mww = 0;
        mwh = 0; // make compiler stfu
    }

    if (ns > 0) {
        /* STACK WINDOW WIDTH */

        sww = (m->ww - ((2*bw) * ns) - (p * (ns+1))) / ns;

        // STACK WINDOW HEIGHT
        //
        // when (nmaster > 0) - subtract half of middle padding and all of bottom padding
        // when (nmaster == 0) - subtract both top and bottom padding

        swh = m->wh - mh - (2*bw) - (nm > 0 ? (p/2) + p : 2*p);

        /* STACK X OFFSET */

        rem = (m->ww - ((2*bw) * ns) - (p * (ns+1))) % ns;
        if (rem > 0)
            sx += (rem/2);
    }
    else {
        sww = 0;
        swh = 0; // make compiler stfu
    }

    // STACK Y PADDING
    //
    // when (nmaster > 0) - half y padding
    // when (nmaster == 0) - complete y padding

    syp = (nm > 0 ? p/2 : p);

    int i;
    Client* c;

    for (i = 0, c = next_tiled(m->clients); c; c = next_tiled(c->next), i++) {
        if (i < nm) {
            resize(c, m->wx + mx + p, m->wy + p, mww, mwh, false);
            mx += p + WIDTH(c);
        }
        else {
            resize(c, m->wx + sx + p, m->wy + mh + syp, sww, swh, false);
            sx += p + WIDTH(c);
        }
    }
}

void tag (const Arg* arg) {
    if (selmon->selected != NULL && arg->ui & TAGMASK) {
        selmon->selected->tags = arg->ui & TAGMASK;
        unfocus(selmon->selected, true);
        focus(NULL);
        arrange(selmon);
    }
}

void tag_mon (const Arg* arg) {
    if (selmon->selected == NULL || mons->next == NULL)
        return;

    send_to_mon(selmon->selected, dir_to_mon(arg->i));
}

void tile (Monitor* m) {
    int n; // window count
    int nm; // master area window count
    int nt; // tile area window count
    int mw; // master area width
    int bw; // border width to use for calculations
    int p; // window padding
    int my, ty, rem; // y positions and initial offset calculation
    int mww, mwh; // dimensions for windows in master area
    int tww, twh; // dimensions for windows in tile area
    int txp; // tile area x padding

    n = n_tiled(m);
    nm = m->nmasters[m->current_tag];

    if (n <= nm) {
        if (n > 2)
            nm = n-1;
        else
            nm = 1;
    }

    nt = n-nm;

    if (n > nm)
        mw = nm ? m->ww * m->mfactors[m->current_tag] : 0;
    else
        mw = m->ww;

    bw = (smart_borders && n == 1 ? 0 : border_width);
    p = m->padding[m->current_tag];

    my = 0;
    ty = 0;

    if (nm > 0) {
        // MASTER WINDOW WIDTH
        //
        // when (n > nmaster) - subtract all of left padding and half of center padding
        // when (n <= nmaster) - subtract both left and right padding for consistency

        mww = mw - (2*bw) - (n > nm ? p + (p/2) : 2*p);
        mwh = (m->wh - ((2*bw) * nm) - (p * (nm+1))) / nm;

        // MASTER Y OFFSET

        rem = (m->wh - ((2*bw) * nm) - (p * (nm+1))) % nm;
        if (rem > 0)
            my += (rem/2);
    }
    else {
        mww = 0;
        mwh = 0; // make compiler stfu
    }

    if (nt > 0) {
        // TILE WINDOW WIDTH
        // 
        // when (nmaster > 0) - subtract half of center padding and all of right padding
        // when (nmaster == 0) - subtract both left and right padding for consistency

        tww = m->ww - mw - (2*bw) - (nm > 0 ? (p/2) + p : 2*p);
        twh = (m->wh - ((2*bw) * nt) - (p * (nt+1))) / nt;

        // TILE Y OFFSET

        rem = (m->wh - ((2*bw) * nt) - (p * (nt+1))) % nt;
        if (rem > 0)
            ty += (rem/2);
    }
    else {
        tww = 0;
        twh = 0; // make compiler stfu
    }

    // TILE X PADDING
    //
    // when (nmaster > 0) - half padding
    // when (nmaster == 0) - full padding

    txp = (nm > 0 ? p/2 : p);

    int i;
    Client* c;

    for (i = 0, c = next_tiled(m->clients); c; c = next_tiled(c->next), i++) {
        if (i < nm) {
            resize(c, m->wx + p, m->wy + my + p, mww, mwh, false);
            my += p + HEIGHT(c);
        }
        else {
            resize(c, m->wx + mw + txp, m->wy + ty + p, tww, twh, false);
            ty += p + HEIGHT(c);
        }
    }
}

void toggle_bar (const Arg* arg) {
    selmon->show_bar = selmon->show_bar == false;

    int pos = selmon->bar_pos == TOP ? STRUT_TOP : STRUT_BOTTOM;
    set_strut(selmon, pos, selmon->show_bar ? bar_height : 0);

    if (selmon->show_bar)
        loft_widget_show(&selmon->bar->win.base);
    else
        loft_widget_hide(&selmon->bar->win.base);

    arrange(selmon);
}

void toggle_bar_pos (const Arg* arg) {
    if (selmon->show_bar == false)
        return;

    int old_st_pos = selmon->bar_pos == TOP ? STRUT_TOP : STRUT_BOTTOM;
    selmon->bar_pos = selmon->bar_pos == false;
    int new_st_pos = selmon->bar_pos == TOP ? STRUT_TOP : STRUT_BOTTOM;

    selmon->struts[old_st_pos] = 0;
    selmon->struts[new_st_pos] += bar_height;

    update_struts(selmon);

    if (selmon->bar_pos == TOP)
        selmon->bar_y = selmon->my;
    else
        selmon->bar_y = selmon->my + (selmon->mh - bar_height);

    loft_widget_move(&selmon->bar->win.base, selmon->mx, selmon->bar_y);

    arrange(selmon);
    update_bar_tags(selmon);
}

void toggle_floating (const Arg* arg) {
    if (selmon->selected == NULL || selmon->layouts[selmon->current_tag]->arrange == NULL)
        return;

    Client* sel = selmon->selected;

    sel->isfloating = sel->isfloating == false || sel->isfixed;

    if (sel->isfloating) {
        if (sel->bw == 0)
            sel->bw = border_width;

        // HACK: borders are not restored if size doesn't change

        if (sel->fw == sel->w && sel->fh == sel->h)
            resize(sel, sel->fx, sel->fy, sel->fw + 5, sel->fh + 5, false);

        resize(sel, sel->fx, sel->fy, sel->fw, sel->fh, false);
        configure(sel);
        XRaiseWindow(dpy, sel->win);
    }
    else if (sel->isfullscreen)
        set_fullscreen(sel, false);

    arrange(selmon);
}

void toggle_tag (const Arg* arg) {
    unsigned int newtags;

    if (selmon->selected == NULL)
        return;

    newtags = selmon->selected->tags ^ (arg->ui & TAGMASK);

    if (newtags) {
        selmon->selected->tags = newtags;
        focus(selmon->tag_focus[selmon->current_tag]);
        arrange(selmon);
    }
}

void toggle_view (const Arg* arg) {
    unsigned int newtagset = selmon->tagset[selmon->selected_tags] ^ (arg->ui & TAGMASK);

    if (newtagset) {
        selmon->tagset[selmon->selected_tags] = newtagset;

        // reset current tag if it was deselected
        if (((1 << selmon->current_tag) & newtagset) == false) {
            unsigned int i = 0;
            unsigned int mask = 1;

            for (i = 0; i < LENGTH(tags); i++) {
                mask = 1 << i;

                if (mask & newtagset) {
                    selmon->current_tag = i;
                    break;
                }
            }
        }

        focus(selmon->tag_focus[selmon->current_tag]);
        arrange(selmon);
    }
}

void unfocus (Client* c, bool setfocus) {
    if (c == NULL)
        return;

    grab_buttons(c, false);
    XSetWindowBorder(dpy, c->win, border_normal);

    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToNone, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void unmanage (Client* c, bool destroyed) {
    Monitor* m = c->mon;
    XWindowChanges wc;

    detach(c);
    detach_stack(c);

    clean_tag_focus(c->mon, c);

    if (destroyed == false) {
        wc.border_width = c->oldbw;
        XGrabServer(dpy);
        XSetErrorHandler(xerror_dummy);
        XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        set_client_state(c, WithdrawnState);
        XSync(dpy, false);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }

    free(c);

    if (m != selmon) {
        update_bar_tags(m);
        update_bar_title(m);
    } else 
        focus(NULL);

    update_client_list();
    arrange(m);
}

void unmap_notify (XEvent* e) {
    XUnmapEvent* ev = &e->xunmap;
    Client* c = win_to_client(ev->window);

    if (c != NULL) {
        if (ev->send_event)
            set_client_state(c, WithdrawnState);
        else
            unmanage(c, false);
    }
}

void update_bars (void) {
    Monitor *m;

    int i;
    TagLabel* t;

    cairo_t* cr;
    cairo_text_extents_t ext;
    cairo_font_extents_t f_ext;

    for (m = mons; m; m = m->next) {
        if (m->bar != NULL)
            continue;

        m->bar = malloc(sizeof(Bar));

        loft_window_init(&m->bar->win, 0);
        loft_layout_init(&m->bar->lt_main, ASPECT_H, 0, 0);
        loft_layout_init(&m->bar->lt_tagstrip, ASPECT_H, 0, 0);
        loft_window_set_layout(&m->bar->win, &m->bar->lt_main);

        loft_label_init(&m->bar->lb_layout, m->ltsymbol, 0);
        loft_label_init(&m->bar->lb_winstat, m->selstat, 0);
        loft_label_init(&m->bar->lb_title, m->selected != NULL ? m->selected->name : NULL, FLOW_L);
        loft_label_init(&m->bar->lb_status, status, FLOW_R);

        loft_label_truncate(&m->bar->lb_title, true);

        m->bar->indicator = NULL;

        m->bar->win.base.draw_base = false;
        m->bar->lb_layout.base.draw_base = false;
        m->bar->lb_winstat.base.draw_base = false;
        m->bar->lb_title.base.draw_base = false;
        m->bar->lb_status.base.draw_base = false;

        loft_label_set_padding(&m->bar->lb_layout, 10,0,10,0);
        loft_label_set_padding(&m->bar->lb_winstat, 10,0,10,0);
        loft_label_set_padding(&m->bar->lb_title, 8,0,8,0);
        loft_label_set_padding(&m->bar->lb_status, 8,0,8,0);

        loft_layout_attach(&m->bar->lt_main, &m->bar->lt_tagstrip.base, EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_layout.base, EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_title.base, EXPAND_X | EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_winstat.base, EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_status.base, EXPAND_Y);

        for (i = 0; i < LENGTH(tags); i++) {
            t = &m->bar->lb_tags[i];

            loft_widget_init(&t->base, "tag_label", 0);
            t->base.draw_base = false;

            t->num = i;
            t->current = false;
            t->unused = true;
            t->selected = false;
            t->urgent = false;

            // set colors

            loft_rgba_set_from_str(&t->style.unused.bg, (char*) unused_tag_bg_color);
            loft_rgba_set_from_str(&t->style.unused.fg, (char*) unused_tag_fg_color);

            loft_rgba_set_from_str(&t->style.normal.bg, (char*) normal_tag_bg_color);
            loft_rgba_set_from_str(&t->style.normal.fg, (char*) normal_tag_fg_color);

            loft_rgba_set_from_str(&t->style.current.bg, (char*) current_tag_bg_color);
            loft_rgba_set_from_str(&t->style.current.fg, (char*) current_tag_fg_color);

            loft_rgba_set_from_str(&t->style.selected.bg, (char*) selected_tag_bg_color);
            loft_rgba_set_from_str(&t->style.selected.fg, (char*) selected_tag_fg_color);

            loft_rgba_set_from_str(&t->style.urgent.bg, (char*) urgent_tag_bg_color);
            loft_rgba_set_from_str(&t->style.urgent.fg, (char*) urgent_tag_fg_color);

            // set minimum size

            cr = cairo_create(t->base.cs);

            cairo_select_font_face(cr, loftenv.font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(cr, loftenv.font_size);

            cairo_text_extents(cr, tags[i], &ext);
            cairo_font_extents(cr, &f_ext);

            cairo_destroy(cr);

            loft_widget_set_minimum_size(&t->base, ext.width + 16, f_ext.height);

            // listen for events

            loft_signal_connect(&t->base, "button-press", _on_tag_pressed, NULL);
            loft_signal_connect(&t->base, "draw", _draw_tag, m);

            // attach to layout

            loft_layout_attach(&m->bar->lt_tagstrip, &t->base, EXPAND_X | EXPAND_Y);
        }

        loft_rgba_set_from_str(&m->bar->lb_layout.style.normal.bg, (char*) ltsym_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_layout.style.normal.fg, (char*) ltsym_fg_color);

        loft_rgba_set_from_str(&m->bar->lb_winstat.style.normal.bg, (char*) selstat_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_winstat.style.normal.fg, (char*) selstat_fg_color);

        loft_rgba_set_from_str(&m->bar->lb_title.style.normal.bg, (char*) title_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_title.style.normal.fg, (char*) title_fg_color);

        loft_rgba_set_from_str(&m->bar->lb_status.style.normal.bg, (char*) status_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_status.style.normal.fg, (char*) status_fg_color);

        loft_widget_override_redirect(&m->bar->win.base, true);
        loft_widget_move(&m->bar->win.base, m->mx, m->bar_y);
        loft_widget_resize(&m->bar->win.base, m->mw, bar_height);
        loft_widget_lock_size(&m->bar->win.base, true);

        update_bar_tags(m);
        update_bar_layout(m);
        update_bar_title(m);

        loft_widget_show_all(&m->bar->lt_main.base);

        if (show_bar) {
            set_strut(m, STRUT_TOP, bar_height);
            loft_widget_show(&m->bar->win.base);
        }
    }
}

inline void update_bar_layout (Monitor* m) {
    sprintf(m->ltsymbol, "%s", m->layouts[m->current_tag]->symbol);
    loft_label_set_text(&m->bar->lb_layout, m->ltsymbol);
}

void update_bar_mon_selections (void) {
    if (mons->next == NULL)
        return;

    Monitor* m;

    for (m = mons; m != NULL; m = m->next) {
        m->bar->indicator->active = selmon == m;
        REDRAW_IF_VISIBLE(&m->bar->indicator->base);
    }
}

void update_bar_window_stat (Monitor* m) {
    Client* c;
    int n = 0;
    int sel = -1;

    for (c = m->clients; c != NULL; c = c->next) {
        if (ISVISIBLE(c) == false)
            continue;

        n++;

        if (c == m->selected)
            sel = n;
    }

    if (n > 0) {
        sprintf(m->selstat, "%d/%d", sel, n);
        loft_label_set_text(&m->bar->lb_winstat, m->selstat);
        loft_widget_show(&m->bar->lb_winstat.base);
    } else {
        m->selstat[0] = '\0';
        loft_widget_hide(&m->bar->lb_winstat.base);
    }
}

void update_bar_tags (Monitor* m) {
    int i;
    int cc;
    int mask;

    TagLabel* t;
    Client* c = NULL;

    for (i = 0; i < LENGTH(tags); i++) {
        cc = 0;
        mask = 1 << i;

        t = m->bar->lb_tags + i;
        t->current = i == m->current_tag;
        t->selected = (mask & m->tagset[m->selected_tags]) != 0;
        t->urgent = false;

        for (c = m->clients; c != NULL; c = c->next) {
            if (c->tags & mask) {
                cc++;

                if (c->isurgent) {
                    t->urgent = true;
                    break;
                }
            }
        }

        t->unused = cc == 0;

        loft_widget_draw(&t->base);
    }
}

inline void update_bar_title (Monitor* m) {
    loft_label_set_text(&m->bar->lb_title, m->selected != NULL ? m->selected->name : NULL);
}

void update_client_list (void) {
    Client *c;
    Monitor *m;

    XDeleteProperty(dpy, root, netatom[NetClientList]);
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            XChangeProperty(dpy, root, netatom[NetClientList],
                            XA_WINDOW, 32, PropModeAppend,
                            (unsigned char *) &(c->win), 1);
}

bool update_geom (void) {
    bool dirty = false;

#ifdef XINERAMA
    if (XineramaIsActive(dpy)) {
        int i, j, n, nn;
        Client* c;
        Monitor* m;

        XineramaScreenInfo* info = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo* unique = NULL;

        for (n = 0, m = mons; m; m = m->next, n++);

        unique = malloc(sizeof(XineramaScreenInfo) * nn);

        if (unique == NULL)
            die("out of memory\n");

        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));

        XFree(info);
        nn = j;

        // init new monitors

        if (n <= nn) {
            for (i = 0; i < (nn - n); i++) {
                for (m = mons; m != NULL && m->next; m = m->next);
                if (m != NULL)
                    m->next = create_mon();
                else
                    mons = create_mon();
            }

            for (i = 0, m = mons; i < nn && m; m = m->next, i++) {
                if (i >= n || (unique[i].x_org != m->mx || unique[i].y_org != m->my || unique[i].width != m->mw || unique[i].height != m->mh)) {
                    dirty = true;

                    m->num = i;
                    m->mx = m->wx = unique[i].x_org;
                    m->my = m->wy = unique[i].y_org;
                    m->mw = m->ww = unique[i].width;
                    m->mh = m->wh = unique[i].height;

                    update_struts(m);
                }
            }
        }

        // clean up unavailable monitors

        else {
            for (i = nn; i < n; i++) {
                for (m = mons; m && m->next; m = m->next);

                while (m->clients) {
                    dirty = true;
                    c = m->clients;
                    m->clients = c->next;
                    detach_stack(c);
                    c->mon = mons;
                    attach_head(c);
                    attach_stack(c);
                }

                if (m == selmon) {
                    selmon = mons;
                    update_bar_mon_selections();
                }

                destroy_mon(m);
            }
        }

        update_mon_indicators();
        free(unique);
    }
    else
#endif
    {
        if (mons == NULL)
            mons = create_mon();

        if (mons->mw != scr_width || mons->mh != scr_height) {
            dirty = true;
            mons->mw = mons->ww = scr_width;
            mons->mh = mons->wh = scr_height;
            update_struts(mons);
        }
    }

    if (dirty) {
        selmon = mons;
        selmon = win_to_mon(root);
    }

    multimon = mons->next != NULL;

    return dirty;
}

void update_mon_indicators (void) {
    Monitor* m;

    if (mons->next != NULL) {
        for (m = mons; m != NULL; m = m->next) {
            if (m->bar != NULL && m->bar->indicator == NULL) {
                m->bar->indicator = new_indicator();
                m->bar->indicator->active = selmon == m;
                loft_layout_attach(&m->bar->lt_main, &m->bar->indicator->base, EXPAND_Y);
                loft_widget_show(&m->bar->indicator->base);
            }
        }
    } else {
        for (m = mons; m != NULL; m = m->next) {
            if (m->bar != NULL && m->bar->indicator != NULL) {
                loft_layout_detach(&m->bar->lt_main, &m->bar->indicator->base);
                loft_widget_destroy(&m->bar->indicator->base);
                free(m->bar->indicator);
                m->bar->indicator = NULL;
            }
        }
    }
}

void update_numlock_mask (void) {
    unsigned int i, j;
    XModifierKeymap *modmap;

    numlockmask = 0;
    modmap = XGetModifierMapping(dpy);

    for (i = 0; i < 8; i++) {
        for (j = 0; j < modmap->max_keypermod; j++) {
            if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
                numlockmask = (1 << i);
        }
    }

    XFreeModifiermap(modmap);
}

void update_smart_borders (Monitor* m) {
    Client* c;

    if (m->layouts[m->current_tag]->arrange == NULL) {
        for (c = m->clients; c != NULL; c = c->next) { // restore borders on all clients
            if (ISVISIBLE(c) && c->bw == 0) {
                c->bw = border_width;
                configure(c);
            }
        }
        return;
    }

    int bw;

    // disable borders if using monocle layout or if tiled window count is 1

    if (m->layouts[m->current_tag]->arrange == &monocle || n_tiled(m) == 1)
        bw = 0;
    else
        bw = border_width;

    for (c = next_tiled(m->clients); c != NULL; c = next_tiled(c->next)) {
        if (ISVISIBLE(c) && c->bw != bw) {
            c->bw = bw;

            // HACK: borders are not restored if size doesn't change

            resize_client(c, c->x, c->y, c->w + 5, c->h + 5);
        }
    }
}

void update_size_hints (Client* c) {
    long msize;
    XSizeHints size;

    if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
        // size is uninitialized - ensure size.flags isn't used */
        size.flags = PSize;

    if (size.flags & PBaseSize) {
        c->basew = size.base_width;
        c->baseh = size.base_height;
    }
    else if (size.flags & PMinSize) {
        c->basew = size.min_width;
        c->baseh = size.min_height;
    }
    else
        c->basew = c->baseh = 0;

    if (size.flags & PResizeInc) {
        c->incw = size.width_inc;
        c->inch = size.height_inc;
    }
    else
        c->incw = c->inch = 0;

    if (size.flags & PMaxSize) {
        c->maxw = size.max_width;
        c->maxh = size.max_height;
    }
    else
        c->maxw = c->maxh = 0;

    if (size.flags & PMinSize) {
        c->minw = size.min_width;
        c->minh = size.min_height;
    }
    else if (size.flags & PBaseSize) {
        c->minw = size.base_width;
        c->minh = size.base_height;
    }
    else
        c->minw = c->minh = 0;

    if (size.flags & PAspect) {
        c->mina = (float) size.min_aspect.y / size.min_aspect.x;
        c->maxa = (float) size.max_aspect.x / size.max_aspect.y;
    }
    else
        c->maxa = c->mina = 0.0;

    c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
                 && c->maxw == c->minw && c->maxh == c->minh);
}

void update_struts (Monitor* m) {
    int i;
    int px;

    m->wx = m->mx;
    m->wy = m->my;

    m->ww = m->mw;
    m->wh = m->mh;

    for (i = 0; i < 4; i++) {
        px = m->struts[i];

        if (i == STRUT_TOP) {
            m->wh -= px;
            m->wy += px;
        }
        else if (i == STRUT_BOTTOM)
            m->wh -= px;
        else if (i == STRUT_LEFT) {
            m->ww -= px;
            m->wx += px;
        }
        else if (i == STRUT_RIGHT)
            m->ww -= px;
    }
}

void update_title (Client* c) {
    if (!get_text_prop(c->win, netatom[NetWMName], c->name, sizeof c->name))
        get_text_prop(c->win, XA_WM_NAME, c->name, sizeof c->name);

    if (c->name[0] == '\0')
        strcpy(c->name, broken);
}

void update_status (void) {
    if (!get_text_prop(root, XA_WM_NAME, status, sizeof(status)))
        sprintf(status, "ProperWM %s", VERSION);

    Monitor* m;
    for (m = mons; m != NULL; m = m->next)
        loft_label_set_text(&m->bar->lb_status, status);
}

void update_window_type (Client *c) {
    Atom state = get_atom_prop(c, netatom[NetWMState]);
    Atom wtype = get_atom_prop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen])
        set_fullscreen(c, true);
    if (wtype == netatom[NetWMWindowTypeDialog])
        c->isfloating = true;
}

void update_wm_hints (Client* c) {
    XWMHints* wmh = XGetWMHints(dpy, c->win);

    if (wmh != NULL) {
        bool urgent = wmh->flags & XUrgencyHint;

        c->isurgent = urgent;

        if (urgent) {
            if (c == selmon->selected) {
                wmh->flags &= ~XUrgencyHint;
                XSetWMHints(dpy, c->win, wmh);
            }
            else XSetWindowBorder(dpy, c->win, border_urgent);
        }

        if (wmh->flags & InputHint)
            c->neverfocus = !wmh->input;
        else
            c->neverfocus = false;

        update_bar_tags(c->mon);
        XFree(wmh);
    }
}

void view (const Arg* arg) {
    if ((arg->ui & TAGMASK) == selmon->tagset[selmon->selected_tags])
        return;

    selmon->selected_tags ^= 1;

    if (arg->ui & TAGMASK) {
        selmon->tagset[selmon->selected_tags] = arg->ui & TAGMASK;

        selmon->previous_tag = selmon->current_tag;
        selmon->current_tag = arg->ui;

        if (arg->ui != -1) {
            int i;
            for (i = 0; (arg->ui & 1 << i) == 0; i++);
            selmon->current_tag = i;
        } else
            selmon->current_tag = 0;
    }
    else {
        int oldcur = selmon->current_tag;
        selmon->current_tag = selmon->previous_tag;
        selmon->previous_tag = oldcur;
    }

    focus(selmon->tag_focus[selmon->current_tag]);
    arrange(selmon);
}

Client* win_to_client (Window w) {
    Client* c;
    Monitor* m;

    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            if (c->win == w)
                return c;

    return NULL;
}

Monitor* win_to_mon (Window w) {
    int x, y;
    Client* c;
    Monitor* m;

    if (w == root && get_root_ptr(&x, &y))
        return xy_to_mon(x, y);

    LoftWidget* lw;

    for (m = mons; m; m = m->next) {
        lw = loft_widget_from_xwin(&w);

        if (lw != NULL) {
            lw = loft_widget_toplevel(lw);
            if (lw == &m->bar->win.base)
                return m;
        }
    }

    c = win_to_client(w);

    if (c != NULL)
        return c->mon;

    return selmon;
}

int xerror (Display* dpy, XErrorEvent* ee) {
    if (ee->error_code == BadWindow
    || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
    || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
    || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
        return 0;

    fprintf(stderr, "properwm: fatal error: request code=%d, error code=%d\n", ee->request_code, ee->error_code);
    return xerrorxlib(dpy, ee); /* may call exit */
}

int xerror_dummy (Display* dpy, XErrorEvent* ee) {
    return 0;
}

int xerror_start (Display* dpy, XErrorEvent* ee) {
    die("properwm: another window manager is already running\n");
    return -1;
}

Monitor* xy_to_mon (int x, int y) {
    Monitor* m;

    for (m = mons; m; m = m->next) {
        if (x >= m->mx && x <= m->mx + m->mw && y >= m->my && y <= m->my + m->mh)
            return m;
    }

    return NULL;
}

void zoom (const Arg* arg) {
    Client* c = selmon->selected;

    if (selmon->layouts[selmon->current_tag]->arrange == NULL || (selmon->selected && selmon->selected->isfloating))
        return;

    if (c == next_tiled(selmon->clients)) {
        if (c == NULL || next_tiled(c->next) == NULL)
            return;
    }

    pop(c);
}

int main (int argc, char** argv) {
    loft_init();
    dpy = loftenv.display; 

    if (dpy == NULL)
        die("properwm: cannot open display");

    loftenv.font = (char*) font_name;
    loftenv.font_size = (int) font_size;

    if (argc == 2 && !strcmp("-v", argv[1]))
        die("ProperWM "VERSION", 2012 speeddefrost, 2006-2012 dwm authors\n");
    else if (argc != 1)
        die("usage: properwm [-v]\n");

    check_other_wm();
    setup();
    scan();
    run();
    cleanup();

    return EXIT_SUCCESS;
}
