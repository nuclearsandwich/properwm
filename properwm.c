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

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (_MAX(0, _MIN((x)+(w),(m)->wx+(m)->ww) - _MAX((x),(m)->wx)) \
                               * _MAX(0, _MIN((y)+(h),(m)->wy+(m)->wh) - _MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->selected_tags]))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define _MAX(A, B)               ((A) > (B) ? (A) : (B))
#define _MIN(A, B)               ((A) < (B) ? (A) : (B))
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };
enum { NetSupported,
       NetWMName, NetWMState,
       NetWMFullscreen, NetActiveWindow,
       NetWMWindowType, NetWMWindowTypeDialog,
       NetClientList,
       NetLast };
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast };
enum { ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast };

typedef union {
    int i;
    unsigned int ui;
    float f;
    const void *v;
} Arg;

typedef struct {
    unsigned int click;
    unsigned int mask;
    unsigned int button;
    void (*func)(const Arg *arg);
    const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;

struct Client {
    char name[256];
    float mina, maxa;
    int x, y, w, h;
    int oldx, oldy, oldw, oldh;
    int basew, baseh, incw, inch, maxw, maxh, minw, minh;
    int bw, oldbw;
    unsigned int tags;
    bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
    Client *next;
    Client *snext;
    Monitor *mon;
    Window win;
};

typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func) (const Arg *);
    const Arg arg;
} Key;

typedef struct {
    const char *symbol;
    void (*arrange) (Monitor *);
} Layout;

typedef struct {
    const char *class;
    const char *instance;
    const char *title;
    unsigned int tags;
    bool isfloating;
    int monitor;
} Rule;

void applyrules (Client* c);
bool applysizehints (Client* c, int* x, int* y, int* w, int* h, bool interact);
void arrange (Monitor* m);
void arrangemon (Monitor* m);
void attach (Client* c);
void attachstack (Client* c);
void buttonpress (XEvent* e);
void checkotherwm (void);
void cleanup (void);
void cleanupmon (Monitor* mon);
void clearurgent (Client* c);
void clientmessage (XEvent* e);
void configure (Client* c);
void configurenotify (XEvent* e);
void configurerequest (XEvent* e);
Monitor* createmon (void);
void destroynotify (XEvent* e);
void detach (Client* c);
void detachstack (Client* c);
void die (const char* errstr, ...);
Monitor* dirtomon (int dir);
void enternotify (XEvent* e);
void focus (Client* c);
void focusin (XEvent* e);
void focusmon (const Arg* arg);
void focusstack (const Arg* arg);
unsigned long getcolor (const char* colstr);
bool getrootptr (int* x, int* y);
long getstate (Window w);
bool gettextprop (Window w, Atom atom, char* text, unsigned int size);
void grabbuttons (Client* c, bool focused);
void grabkeys (void);
void iteration (void);
void keypress (XEvent *e);
void killclient (const Arg* arg);
void manage (Window w, XWindowAttributes* wa);
void mappingnotify (XEvent* e);
void maprequest (XEvent* e);
void modmfactor (const Arg* arg);
void modnmaster (const Arg* arg);
void modpadding (const Arg* arg);
void monocle (Monitor* m);
void motionnotify (XEvent* e);
void movemouse (const Arg* arg);
Client* nexttiled (Client* c);
int ntiled (Monitor* m);
void pop (Client* c);
Client* prevtiled (Client* c);
void propertynotify (XEvent* e);
void pushdown (const Arg* arg);
void pushup (const Arg* arg);
void quit (const Arg* arg);
Monitor* recttomon (int x, int y, int w, int h);
void resetnmaster (const Arg* arg);
void resize (Client* c, int x, int y, int w, int h, bool interact);
void resizeclient (Client* c, int x, int y, int w, int h);
void resizemouse (const Arg* arg);
void restack (Monitor* m);
void run (void);
void scan (void);
bool sendevent (Client* c, Atom proto);
void sendmon (Client* c, Monitor* m);
void setclientstate (Client* c, long state);
void setfocus (Client* c);
void setfullscreen (Client* c, bool fullscreen);
void setlayout (const Arg* arg);
bool setstrut (Monitor* m, int pos, int px);
void setup (void);
void showhide (Client* c);
void sigchld (int unused);
void spawn (const Arg* arg);
void stack (Monitor* m);
void tag (const Arg* arg);
void tagmon (const Arg* arg);
void tile (Monitor* m);
void togglebar (const Arg* arg);
void togglebarpos (const Arg* arg);
void togglefloating (const Arg* arg);
void toggletag (const Arg* arg);
void toggleview (const Arg* arg);
void unfocus (Client* c, bool setfocus);
void unmanage (Client* c, bool destroyed);
void unmapnotify (XEvent *e);
void updatebars (void);
void updatebarlayout (Monitor* m);
void updatebartags (Monitor* m);
void updatebartitle (Monitor* m);
void updateborders (Monitor* m);
void updateclientlist (void);
bool updategeom (void);
void updatemonindicators (void);
void updatenumlockmask (void);
void updatesizehints (Client* c);
void updatestatus (void);
void updatestruts (Monitor* m);
void updatetitle (Client* c);
void updatewindowtype (Client* c);
void updatewmhints (Client* c);
void view (const Arg* arg);
Client* wintoclient (Window w);
Monitor* wintomon (Window w);
int xerror (Display* dpy, XErrorEvent* ee);
int xerrordummy (Display* dpy, XErrorEvent* ee);
int xerrorstart (Display* dpy, XErrorEvent* ee);
void zoom (const Arg* arg);

unsigned long border_normal;
unsigned long border_selected;
unsigned long border_urgent;

const char broken[] = "broken";
char status[256];

int screen;
int sw, sh; // screen geometry

unsigned int numlockmask = 0;

int (*xerrorxlib)(Display *, XErrorEvent *);

void (*handler[LASTEvent]) (XEvent *) = {
    [ButtonPress] = buttonpress,
    [ClientMessage] = clientmessage,
    [ConfigureRequest] = configurerequest,
    [ConfigureNotify] = configurenotify,
    [DestroyNotify] = destroynotify,
    [EnterNotify] = enternotify,
    [FocusIn] = focusin,
    [KeyPress] = keypress,
    [MappingNotify] = mappingnotify,
    [MapRequest] = maprequest,
    [PropertyNotify] = propertynotify,
    [UnmapNotify] = unmapnotify
};

bool running = true;

Atom wmatom[WMLast], netatom[NetLast];
Cursor cursor[CurLast];

Display* dpy;
Window root;

Monitor* mons = NULL;
Monitor* selmon = NULL;

#include "config.h"

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
    bool has_sel;
    bool selected;
    bool unused;
    bool urgent;
} TagLabel;

typedef struct SelectionIndicator {
    LoftWidget base;

    struct {
        LoftRGBAPair normal;
        LoftRGBAPair selected;
    } style;

    bool selected;
} SelectionIndicator;

typedef struct Bar {
    LoftWindow win;
    LoftLayout lt_main;

    LoftLayout lt_tagstrip;
    TagLabel* lb_tags;

    LoftLabel lb_layout;
    LoftLabel lb_title;
    LoftLabel lb_status;

    SelectionIndicator* indicator;
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

    const Layout *layouts[LENGTH(tags)];
    float mfactors[LENGTH(tags)];
    int nmasters[LENGTH(tags)];
    int padding[LENGTH(tags)];
    Client* tagfocus[LENGTH(tags)];

    int struts[4];
    char ltsymbol[16];

    Monitor *next;
};

struct CheckTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

// LOFT SIGNAL CALLBACKS

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
            toggletag(&arg);
        else
            toggleview(&arg);
    }
}

void _draw_tag (TagLabel* t) {
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

    bool multi = selmon->selected != NULL && selmon->selected->tags != 1 << t->num;

    if (client_indicator && t->has_sel && multi) {
        double ind_x_left = t->base.width / 4;
        double ind_x_center = t->base.width / 2;
        double ind_x_right = t->base.width - ind_x_left;

        double ind_y_base = selmon->bar_pos == TOP ? 0 : t->base.height;
        double ind_y_point = selmon->bar_pos == TOP ? t->base.height / 7 : t->base.height - (t->base.height / 7);

        cairo_move_to(cr, ind_x_left, ind_y_base);
        cairo_line_to(cr, ind_x_right, ind_y_base);
        cairo_line_to(cr, ind_x_center, ind_y_point);

        cairo_fill(cr);
    }

    cairo_restore(cr);
    cairo_destroy(cr);
}

void _draw_selection_indicator (SelectionIndicator* si) {
    cairo_t* cr = cairo_create(si->base.cs);
    cairo_save(cr);

    LoftRGBA* bg;
    LoftRGBA* fg;

    if (si->selected) {
        bg = &si->style.selected.bg;
        fg = &si->style.selected.fg;
    } else {
        bg = &si->style.normal.bg;
        fg = &si->style.normal.fg;
    }


    loft_cairo_set_rgba(cr, bg);
    cairo_rectangle(cr, 0, 0, si->base.width, si->base.height);
    cairo_fill(cr);

    int min_aspect = _MIN(si->base.width, si->base.height);

    loft_cairo_set_rgba(cr, fg);
    cairo_arc(cr, si->base.width / 2, si->base.height / 2, min_aspect / 6.0, 0.0, 2.0 * M_PI);
    cairo_fill(cr);

    cairo_restore(cr);
    cairo_destroy(cr);
}

// - - - - - - - - - - -

void applyrules (Client *c) {
    const char *class, *instance;
    unsigned int i;
    const Rule *r;
    Monitor *m;
    XClassHint ch = { NULL, NULL };

    /* rule matching */
    c->isfloating = false;
    c->tags = 0;

    XGetClassHint(dpy, c->win, &ch);
    class = ch.res_class ? ch.res_class : broken;
    instance = ch.res_name ? ch.res_name : broken;

    for (i = 0; i < LENGTH(rules); i++) {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title)) && (!r->class || strstr(class, r->class)) && (!r->instance || strstr(instance, r->instance))) {
            c->isfloating = r->isfloating;
            c->tags |= r->tags;
            for (m = mons; m && m->num != r->monitor; m = m->next);
            if (m)
                c->mon = m;
        }
    }

    if (ch.res_class)
        XFree(ch.res_class);
    if (ch.res_name)
        XFree(ch.res_name);

    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->selected_tags];
}

bool applysizehints (Client *c, int *x, int *y, int *w, int *h, bool interact) {
    bool baseismin;
    Monitor *m = c->mon;

    /* set minimum possible */
    *w = _MAX(1, *w);
    *h = _MAX(1, *h);

    if (interact) {
        if (*x > sw)
            *x = sw - WIDTH(c);
        if (*y > sh)
            *y = sh - HEIGHT(c);
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

        *w = _MAX(*w + c->basew, c->minw);
        *h = _MAX(*h + c->baseh, c->minh);

        if (c->maxw)
            *w = _MIN(*w, c->maxw);
        if (c->maxh)
            *h = _MIN(*h, c->maxh);
    }

    return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void arrange (Monitor *m) {
    if (m)
        showhide(m->stack);
    else for (m = mons; m; m = m->next)
        showhide(m->stack);

    if (m) {
        arrangemon(m);
        restack(m);
    } else for (m = mons; m; m = m->next)
        arrangemon(m);
}

void arrangemon (Monitor *m) {
    if (smart_borders)
        updateborders(m);

    if (m->layouts[m->current_tag]->arrange != monocle || m->selected == NULL)
        updatebarlayout(m);

    if (m->layouts[m->current_tag]->arrange != NULL)
        m->layouts[m->current_tag]->arrange(m);

    if (m->selected != NULL)
        XRaiseWindow(dpy, m->selected->win);
}

void attach (Client *c) {
    c->next = c->mon->clients;
    c->mon->clients = c;
}

void attachstack (Client *c) {
    c->snext = c->mon->stack;
    c->mon->stack = c;
}

void buttonpress (XEvent *e) {
    XButtonPressedEvent* ev = &e->xbutton;

    /* focus monitor if necessary */

    Monitor* m = wintomon(ev->window);

    if (m != NULL && m != selmon) {
        unfocus(selmon->selected, true);
        selmon = m;
        focus(NULL);
        updatemonindicators();
    }

    int click = ClkRootWin;
    Client* c = wintoclient(ev->window);

    if (c != NULL) {
        focus(c);
        click = ClkClientWin;
    }
    else if (ev->window == selmon->bar->lb_layout.base.xwin)
        click = ClkLtSymbol;
    else  if (ev->window == selmon->bar->lb_title.base.xwin)
        click = ClkWinTitle;
    else if (ev->window == selmon->bar->lb_status.base.xwin)
        click = ClkStatusText;

    int i;

    for (i = 0; i < LENGTH(buttons); i++)
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
        && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(&buttons[i].arg);
}

void checkotherwm (void) {
    xerrorxlib = XSetErrorHandler(xerrorstart);
    /* this causes an error ifsome other window manager is running */
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
    XSync(dpy, false);
    XSetErrorHandler(xerror);
    XSync(dpy, false);
}

void cleanup (void) {
    Arg a = {.ui = ~0};
    Layout foo = { "", NULL };
    Monitor *m;

    view(&a);
    selmon->layouts[selmon->current_tag] = &foo;

    for(m = mons; m; m = m->next)
        while(m->stack)
            unmanage(m->stack, false);

    XUngrabKey(dpy, AnyKey, AnyModifier, root);

    while (mons)
        cleanupmon(mons);

    XSync(dpy, false);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void cleanupmon (Monitor *mon) {
    Monitor *m;

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

    free(mon->bar->lb_tags);

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

void clearurgent (Client *c) {
    c->isurgent = false;

    XWMHints *wmh = XGetWMHints(dpy, c->win);
    if (wmh == NULL)
        return;

    wmh->flags &= ~XUrgencyHint;
    XSetWMHints(dpy, c->win, wmh);
    XFree(wmh);

    updatebartags(c->mon);
}

void clientmessage (XEvent *e) {
    XClientMessageEvent *cme = &e->xclient;
    Client *c = wintoclient(cme->window);

    if (c == NULL)
        return;

    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen])
            setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD */
                          || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
    }
    else if (cme->message_type == netatom[NetActiveWindow]) {
        if (!ISVISIBLE(c)) {
            c->mon->selected_tags ^= 1;
            c->mon->tagset[c->mon->selected_tags] = c->tags;
        }
        pop(c);
    }
}

void configure (Client *c) {
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

void configurenotify (XEvent *e) {
    Monitor *m;
    XConfigureEvent *ev = &e->xconfigure;
    bool dirty;

    if (ev->window == root) {
        dirty = (sw != ev->width);
        sw = ev->width;
        sh = ev->height;

        if (updategeom() || dirty) {
            updatebars();

            for (m = mons; m; m = m->next) {
                loft_widget_move(&m->bar->win.base, m->mx, m->bar_y);
                loft_widget_resize(&m->bar->win.base, m->mw, m->mh);
            }

            focus(NULL);
            arrange(NULL);
        }
    }
}

void configurerequest (XEvent *e) {
    Monitor *m;
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;

    Client *c = wintoclient(ev->window);

    if (c != NULL) {
        if (ev->value_mask & CWBorderWidth)
            c->bw = ev->border_width;
        else if (c->isfloating || !selmon->layouts[selmon->current_tag]->arrange) {
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
        }
        else
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

Monitor* createmon (void) {
    int i;

    Monitor *m;

    if (!(m = (Monitor *)calloc(1, sizeof(Monitor))))
        die("fatal: could not malloc() %u bytes\n", sizeof(Monitor));

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

    for (i = 0; i < LENGTH(tags); i++) {
        m->layouts[i] = &layouts[layouts_init[i]];
        m->mfactors[i] = mfactors_init[i];
        m->nmasters[i] = nmaster;
        m->padding[i] = padding_init[i];
        m->tagfocus[i] = NULL;
    }

    for (i = 0; i < 4; i++)
        m->struts[i] = 0;

    if (m->show_bar) {
        if (m->show_bar && m->bar_pos == TOP)
            m->struts[STRUT_TOP] = bar_height;
        else if (m->bar_pos == BOTTOM)
            m->struts[STRUT_BOTTOM] = bar_height;
    }

    updatestruts(m);
    strncpy(m->ltsymbol, layouts[0].symbol, sizeof(m->ltsymbol));

    return m;
}

void destroynotify (XEvent *e) {
    Client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if ((c = wintoclient(ev->window)))
        unmanage(c, true);
}

void detach (Client *c) {
    Client **tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
}

void detachstack (Client *c) {
    Client **tc, *t;

    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
    *tc = c->snext;

    if (c == c->mon->selected) {
        for (t = c->mon->stack; t && ISVISIBLE(t) == false; t = t->snext);
        c->mon->selected = t;
    }
}

void die (const char *errstr, ...) {
    va_list ap;

    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

Monitor* dirtomon (int dir) {
    Monitor *m = NULL;

    if (dir > 0) {
        if (!(m = selmon->next))
            m = mons;
    }
    else if (selmon == mons)
        for (m = mons; m->next; m = m->next);
    else
        for (m = mons; m->next != selmon; m = m->next);

    return m;
}

void enternotify (XEvent *e) {
    Client *c;
    Monitor *m;
    XCrossingEvent *ev = &e->xcrossing;

    if (ev->window != root && (ev->mode != NotifyNormal || ev->detail == NotifyInferior))
        return;

    c = wintoclient(ev->window);
    m = c ? c->mon : wintomon(ev->window);

    if (c == NULL || c == selmon->selected)
        return;

    if (click_to_focus)
        return;

    if (m != selmon) {
        unfocus(selmon->selected, true);
        selmon = m;
        updatemonindicators();
    }

    focus(c);
}

void focus (Client *c) {
    if (c == NULL || ISVISIBLE(c) == false)
        for (c = selmon->stack; c != NULL && ISVISIBLE(c) == false; c = c->snext);

    if (selmon->selected != NULL && selmon->selected != c)
        unfocus(selmon->selected, false);

    if (c) {
        if (c->mon != selmon) {
            selmon = c->mon;
            updatemonindicators();
        }

        if (c->isurgent)
            clearurgent(c);

        detachstack(c);
        attachstack(c);

        XSetWindowBorder(dpy, c->win, border_selected);

        grabbuttons(c, true);
        setfocus(c);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }

    selmon->selected = c;
    selmon->tagfocus[selmon->current_tag] = c;

    updatebartags(selmon);
    updatebartitle(selmon);
}

void focusin (XEvent* e) { /* there are some broken focus acquiring clients */
    XFocusChangeEvent* ev = &e->xfocus;

    if (selmon->selected && ev->window != selmon->selected->win)
        setfocus(selmon->selected);
}

void focusmon (const Arg* arg) {
    Monitor* m;

    if (mons->next == NULL)
        return;
    
    m = dirtomon(arg->i);
    if (m == selmon)
        return;

    unfocus(selmon->selected, true);
    selmon = m;

    focus(selmon->tagfocus[selmon->current_tag]);
    updatemonindicators();
}

void focusstack (const Arg* arg) {
    Client *c = NULL, *i;

    if (selmon->selected == NULL)
        return;

    if (arg->i > 0) {
        for (c = selmon->selected->next; c && !ISVISIBLE(c); c = c->next);
        if (c == NULL)
            for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
    } else {
        for (i = selmon->clients; i != selmon->selected; i = i->next)
            if (ISVISIBLE(i))
                c = i;
        if (c == NULL)
            for (; i; i = i->next)
                if (ISVISIBLE(i))
                    c = i;
    }

    if (c) {
        focus(c);
        restack(selmon);
    }
}

Atom getatomprop (Client *c, Atom prop) {
    int di;
    unsigned long dl;
    unsigned char *p = NULL;
    Atom da, atom = None;

    if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, false, XA_ATOM, &da, &di, &dl, &dl, &p) == Success && p) {
        atom = *(Atom *)p;
        XFree(p);
    }

    return atom;
}

unsigned long getcolor (const char *colstr) {
    Colormap cmap = DefaultColormap(dpy, screen);
    XColor color;

    if (!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
        die("error, cannot allocate color '%s'\n", colstr);

    return color.pixel;
}

bool getrootptr (int *x, int *y) {
    int di;
    unsigned int dui;
    Window dummy;

    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long getstate (Window w) {
    int format;
    long result = -1;
    unsigned char *p = NULL;
    unsigned long n, extra;
    Atom real;

    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, false, wmatom[WMState], &real, &format, &n, &extra, (unsigned char **)&p) != Success)
        return -1;

    if (n != 0)
        result = *p;

    XFree(p);
    return result;
}

bool gettextprop (Window w, Atom atom, char *text, unsigned int size) {
    char **list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0)
        return false;
    text[0] = '\0';
    XGetTextProperty(dpy, w, &name, atom);

    if (!name.nitems)
        return false;
    if (name.encoding == XA_STRING)
        strncpy(text, (char *)name.value, size - 1);
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

void grabbuttons (Client *c, bool focused) {
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (focused) {
            for(i = 0; i < LENGTH(buttons); i++)
                if (buttons[i].click == ClkClientWin)
                    for(j = 0; j < LENGTH(modifiers); j++)
                        XGrabButton(dpy, buttons[i].button,
                                    buttons[i].mask | modifiers[j],
                                    c->win, false, BUTTONMASK,
                                    GrabModeAsync, GrabModeSync, None, None);
        }
        else
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, false,
                        BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
    }
}

void grabkeys (void) {
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        KeyCode code;

        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        for(i = 0; i < LENGTH(keys); i++)
            if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
                for(j = 0; j < LENGTH(modifiers); j++)
                    XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
                         true, GrabModeAsync, GrabModeAsync);
    }
}

#ifdef XINERAMA
static bool isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
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

    loft_process_event(&ev);
}

void keypress (XEvent *e) {
    unsigned int i;
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XkbKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0, 0);

    for(i = 0; i < LENGTH(keys); i++)
        if (keysym == keys[i].keysym
        && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
        && keys[i].func)
            keys[i].func(&(keys[i].arg));
}

void killclient (const Arg *arg) {
    if (!selmon->selected)
        return;

    if (!sendevent(selmon->selected, wmatom[WMDelete])) {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, selmon->selected->win);
        XSync(dpy, false);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
}

void manage (Window w, XWindowAttributes *wa) {
    Client *c, *t = NULL;
    Window trans = None;
    XWindowChanges wc;

    if (!(c = calloc(1, sizeof(Client))))
        die("fatal: could not malloc() %u bytes\n", sizeof(Client));

    c->win = w;
    updatetitle(c);

    if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
        c->mon = t->mon;
        c->tags = t->tags;
    }
    else {
        c->mon = selmon;
        applyrules(c);
    }

    /* geometry */
    c->x = c->oldx = wa->x;
    c->y = c->oldy = wa->y;
    c->w = c->oldw = wa->width;
    c->h = c->oldh = wa->height;
    c->oldbw = wa->border_width;

    if (c->x + WIDTH(c) > c->mon->mx + c->mon->mw)
        c->x = c->mon->mx + c->mon->mw - WIDTH(c);
    if (c->y + HEIGHT(c) > c->mon->my + c->mon->mh)
        c->y = c->mon->my + c->mon->mh - HEIGHT(c);

    c->x = _MAX(c->x, c->mon->mx);
    /* only fix client y-offset, ifthe client center might cover the bar */
    c->y = _MAX(c->y, ((c->mon->bar_y == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
               && (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bar_height : c->mon->my);

    updatewindowtype(c);
    updatesizehints(c);
    updatewmhints(c);

    if (c->isfloating == false)
        c->isfloating = c->oldstate = trans != None || c->isfixed;

    if (c->isfullscreen || (smart_borders && c->mon->layouts[c->mon->current_tag]->arrange != NULL && c->isfloating == false
    && (c->mon->layouts[c->mon->current_tag]->arrange == &monocle || ntiled(c->mon) == 0)))
        c->bw = 0;
    else
        c->bw = border_width;

    wc.border_width = c->bw;
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);

    XSetWindowBorder(dpy, w, border_normal);
    configure(c);

    XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    grabbuttons(c, false);

    if (c->isfloating)
        XRaiseWindow(dpy, c->win);

    attach(c);
    attachstack(c);

    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
                    (unsigned char *) &(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h);

    setclientstate(c, NormalState);

    if (c->mon == selmon)
        unfocus(selmon->selected, false);

    c->mon->selected = c;

    arrange(c->mon);
    XMapWindow(dpy, c->win);
    focus(NULL);
}

void mappingnotify (XEvent *e) {
    XMappingEvent *ev = &e->xmapping;
    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard)
        grabkeys();
}

void maprequest (XEvent *e) {
    static XWindowAttributes wa;
    XMapRequestEvent *ev = &e->xmaprequest;

    if (!XGetWindowAttributes(dpy, ev->window, &wa))
        return;
    if (wa.override_redirect)
        return;
    if (!wintoclient(ev->window))
        manage(ev->window, &wa);
}

void modmfactor (const Arg *arg) {
    float f;

    if (arg == NULL || selmon->layouts[selmon->current_tag]->arrange == NULL)
        return;

    f = arg->f < 1.0 ? arg->f + selmon->mfactors[selmon->current_tag] : arg->f - 1.0;

    if (f < 0.1 || f > 0.9)
        return;

    selmon->mfactors[selmon->current_tag] = f;
    arrange(selmon);
}

void modnmaster (const Arg* arg) {
    selmon->nmasters[selmon->current_tag] = _MAX(selmon->nmasters[selmon->current_tag] + arg->i, 0);
    arrange(selmon);
}

void modpadding (const Arg* arg) {
    selmon->padding[selmon->current_tag] = _MAX(selmon->padding[selmon->current_tag] + arg->i, 0);
    arrange(selmon);
}

void
monocle(Monitor *m) {
    unsigned int n = 0;
    Client *c;

    int p = m->padding[m->current_tag];

    int x = m->wx + p;
    int y = m->wy + p;
    int w = m->ww - (2 * p);
    int h = m->wh - (2 * p);

    for (c = nexttiled(m->clients); c != NULL; c = nexttiled(c->next)) {
        resize(c, x,y, w - (2 * c->bw), h - (2 * c->bw), false);
        n++;
    }

    if (n > 0) {
        snprintf(m->ltsymbol, 15, "%d", n);
        REDRAW_IF_VISIBLE(&m->bar->lb_layout.base);
    }
}

void movemouse (const Arg *arg) {
    if (selmon->selected == NULL)
        return;

    int x, y, ocx, ocy, nx, ny;
    Monitor *m;
    XEvent ev;

    Client* c = selmon->selected;

    restack(selmon);

    ocx = c->x;
    ocy = c->y;

    if (XGrabPointer(dpy, root, false, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurMove], CurrentTime) != GrabSuccess)
        return;

    if (!getrootptr(&x, &y))
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

            if (c->isfloating == false && selmon->layouts[selmon->current_tag]->arrange != NULL &&
                (ev.xmotion.x < x - snap || ev.xmotion.x > x + snap || ev.xmotion.y < y - snap || ev.xmotion.y > y + snap))
            {
                togglefloating(NULL);
            }

            if (c->isfloating || selmon->layouts[selmon->current_tag]->arrange == NULL) {
                if (c->isfullscreen)
                    setfullscreen(c, false);
                resize(c, nx, ny, c->w, c->h, true);
            }

            break;
        }
    } while (ev.type != ButtonRelease);

    XUngrabPointer(dpy, CurrentTime);

    m = recttomon(c->x, c->y, c->w, c->h);

    if (m != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
        updatemonindicators();
    }
}

Client* nexttiled (Client *c) {
    for(; c && (c->isfloating || ISVISIBLE(c) == false); c = c->next);
    return c;
}

int ntiled (Monitor *m) {
    Client *c;
    int nt = 0;

    for (c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
        if (ISVISIBLE(c))
            nt++;
    }

    return nt;
}

void pop (Client *c) {
    detach(c);
    attach(c);
    focus(c);
    arrange(c->mon);
}

Client* prevtiled (Client *c) {
    Client* p;
    Client* r;

    for (p = selmon->clients, r = NULL; p && p != c; p = p->next) {
        if (p->isfloating == false && ISVISIBLE(p))
            r = p;
    }

    return r;
}

void propertynotify (XEvent *e) {
    Client *c;
    Window trans;
    XPropertyEvent *ev = &e->xproperty;

    if ((ev->window == root) && (ev->atom == XA_WM_NAME))
        updatestatus();
    else if (ev->state == PropertyDelete)
        return; /* ignore */
    else if ((c = wintoclient(ev->window))) {
        switch(ev->atom) {
        default: break;
        case XA_WM_TRANSIENT_FOR:
            if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
               (c->isfloating = (wintoclient(trans)) != NULL))
                arrange(c->mon);
            break;
        case XA_WM_NORMAL_HINTS:
            updatesizehints(c);
            break;
        case XA_WM_HINTS:
            updatewmhints(c);
            break;
        }

        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
            updatetitle(c);
            if (c == c->mon->selected)
                updatebartitle(c->mon);
        }

        if (ev->atom == netatom[NetWMWindowType])
            updatewindowtype(c);
    }
}

void pushdown (const Arg *arg) {
    Client *sel = selmon->selected;
    Client *c;

    if (sel == NULL || sel->isfloating)
        return;

    c = nexttiled(sel->next);

    if (c) {
        /* attach after c */
        detach(sel);
        sel->next = c->next;
        c->next = sel;
    } else {
        /* move to the front */
        detach(sel);
        attach(sel);
    }

    focus(sel);
    arrange(selmon);
}

void pushup (const Arg *arg) {
    Client *sel = selmon->selected;
    Client *c;

    if (sel == NULL || sel->isfloating)
        return;

    c = prevtiled(sel);

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

void quit (const Arg *arg) {
    running = false;
}

Monitor* recttomon (int x, int y, int w, int h) {
    Monitor *m, *r = selmon;
    int a, area = 0;

    for (m = mons; m; m = m->next) {
        if ((a = INTERSECT(x, y, w, h, m)) > area) {
            area = a;
            r = m;
        }
    }

    return r;
}

void resetnmaster (const Arg *arg) {
    selmon->nmasters[selmon->current_tag] = nmaster;
    arrange(selmon);
}

void resize (Client *c, int x, int y, int w, int h, bool interact) {
    if (applysizehints(c, &x, &y, &w, &h, interact))
        resizeclient(c, x, y, w, h);
}

void resizeclient (Client *c, int x, int y, int w, int h) {
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

void resizemouse (const Arg *arg) {
    int ocx, ocy;
    int nw, nh;
    Client *c;
    Monitor *m;
    XEvent ev;

    if (selmon->selected == NULL)
        return;

    c = selmon->selected;

    restack(selmon);
    ocx = c->x;
    ocy = c->y;

    if (XGrabPointer(dpy, root, false, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurResize], CurrentTime) != GrabSuccess)
        return;

    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);

    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            nw = _MAX(ev.xmotion.x - ocx - (2 * c->bw) + 1, 1);
            nh = _MAX(ev.xmotion.y - ocy - (2 * c->bw) + 1, 1);

            if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
            && c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
            {
                if (c->isfloating == false && selmon->layouts[selmon->current_tag]->arrange != NULL
                && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
                    togglefloating(NULL);
            }

            if (selmon->layouts[selmon->current_tag]->arrange == false || c->isfloating) {
                if (c->isfullscreen)
                    setfullscreen(c, false);
                resize(c, c->x, c->y, nw, nh, true);
            }
            break;
        }
    } while(ev.type != ButtonRelease);

    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    XUngrabPointer(dpy, CurrentTime);

    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));

    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
        updatemonindicators();
    }
}

void restack (Monitor *m) {
    Client *c;
    XEvent ev;
    XWindowChanges wc;

    if (m->selected == NULL)
        return;

    if (m->selected->isfloating || m->layouts[m->current_tag]->arrange == NULL)
        XRaiseWindow(dpy, m->selected->win);

    if (m->layouts[m->current_tag]->arrange != NULL) {
        wc.stack_mode = Below;
        wc.sibling = m->bar->win.base.xwin;

        for (c = m->stack; c; c = c->snext)
            if (!c->isfloating && ISVISIBLE(c)) {
                XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
                wc.sibling = c->win;
            }
    }

    XSync(dpy, false);
    while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void run (void) {
    XSync(dpy, false);

    // loft is "sorta running"

    loftenv.running = true;

    while (running)
        iteration();

    loftenv.running = false;
}

void scan (void) {
    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa)
            || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                continue;
            if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
                manage(wins[i], &wa);
        }
        for (i = 0; i < num; i++) { /* now the transients */
            if (!XGetWindowAttributes(dpy, wins[i], &wa))
                continue;
            if (XGetTransientForHint(dpy, wins[i], &d1)
            && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
                manage(wins[i], &wa);
        }
        if (wins)
            XFree(wins);
    }
}

bool sendevent (Client *c, Atom proto) {
    int n;
    Atom *protocols;
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

void sendmon (Client *c, Monitor *m) {
    if (c->mon == m)
        return;

    unfocus(c, true);
    detach(c);
    detachstack(c);

    updatebartitle(c->mon);
    updatebartags(c->mon);

    c->mon->tagfocus[c->mon->current_tag] = NULL;

    c->mon = m;
    c->tags = m->tagset[m->selected_tags]; // assign tags of target monitor

    attach(c);
    attachstack(c);

    m->selected = c;

    updatebartags(m);
    updatebartitle(m);

    focus(NULL);
    arrange(NULL);
}

void setclientstate (Client *c, long state) {
    long data[] = { state, None };

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
            PropModeReplace, (unsigned char *)data, 2);
}

void setfocus (Client *c) {
    if (!c->neverfocus) {
        XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow],
                         XA_WINDOW, 32, PropModeReplace,
                         (unsigned char *) &(c->win), 1);
    }
    sendevent(c, wmatom[WMTakeFocus]);
}

void setfullscreen (Client *c, bool fullscreen) {
    if (fullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);

        c->isfloating = true;
        c->isfullscreen = true;
        c->oldbw = c->bw;
        c->bw = 0;
        c->oldstate = c->isfloating;

        if (smart_borders)
            updateborders(c->mon);

        resize(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh, false);
        XRaiseWindow(dpy, c->win);
    }
    else {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)0, 0);

        c->isfloating = c->oldstate;
        c->isfullscreen = false;
        c->bw = c->oldbw;
        c->x = c->oldx;
        c->y = c->oldy;
        c->w = c->oldw;
        c->h = c->oldh;

        resize(c, c->x, c->y, c->w, c->h, false);
    }

    arrange(c->mon);
}

void setlayout (const Arg *arg) {
    if (!arg || !arg->v || arg->v == selmon->layouts[selmon->current_tag])
        return;

    if (arg && arg->v)
        selmon->layouts[selmon->current_tag] = (Layout*) arg->v;

    const char* ltsym = selmon->layouts[selmon->current_tag]->symbol;

    if (ltsym != NULL)
        strncpy(selmon->ltsymbol, ltsym, sizeof(selmon->ltsymbol));
    else
        selmon->ltsymbol[0] = '\0';

    if (selmon->selected != NULL)
        arrange(selmon);
    else
        updatebarlayout(selmon);
}

bool setstrut (Monitor *m, int pos, int px) {
    if (pos >= 4)
        return false;

    bool iter;

    iter = (m == NULL);

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

    updatestruts(m);

    if (running)
        arrange(m);

    return true;
}

void setup (void) {
    XSetWindowAttributes wa;

    /* clean up any zombies immediately */
    sigchld(0);

    /* init screen */
    screen = loftenv.screen;
    root = loftenv.root;

    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);

    updategeom();

    /* init atoms */

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

    /* init cursors */

    cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
    cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
    cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

    /* init bars */

    updatebars();
    updatestatus();

    /* init border colors */

    border_normal = getcolor(normal_border_color);
    border_selected = getcolor(selected_border_color);
    border_urgent = getcolor(urgent_border_color);

    // EWMH support per view

    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
            PropModeReplace, (unsigned char *) netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);

    // select events

    wa.cursor = cursor[CurNormal];
    wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
                    |EnterWindowMask|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;

    XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);

    grabkeys();
}

void showhide (Client *c) {
    if (c == NULL)
        return;

    /* show clients top down */

    if (ISVISIBLE(c)) {
        XMoveWindow(dpy, c->win, c->x, c->y);
        if ((c->mon->layouts[c->mon->current_tag]->arrange == NULL || c->isfloating) && c->isfullscreen == false)
            resize(c, c->x, c->y, c->w, c->h, false);
        showhide(c->snext);
    }
    
    /* hide clients bottom up */

    else {
        showhide(c->snext);
        XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
    }
}

void sigchld (int unused) {
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
        die("Can't install SIGCHLD handler");
    while(0 < waitpid(-1, NULL, WNOHANG));
}

void spawn (const Arg *arg) {
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

void stack (Monitor *m) {
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

    int i;
    Client *c;

    n = ntiled(m);
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

    for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
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

void tag (const Arg *arg) {
    if (selmon->selected && arg->ui & TAGMASK) {
        selmon->selected->tags = arg->ui & TAGMASK;
        focus(NULL);
        updatebartags(selmon);
        arrange(selmon);
    }
}

void tagmon (const Arg *arg) {
    if (selmon->selected == NULL || mons->next == NULL)
        return;

    Monitor* newmon = dirtomon(arg->i);
    sendmon(selmon->selected, newmon);
}

void tile (Monitor *m) {
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

    int i;
    Client *c;

    n = ntiled(m);
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

    for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
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

void togglebar (const Arg *arg) {
    selmon->show_bar = selmon->show_bar == false;

    int pos = selmon->bar_pos == TOP ? STRUT_TOP : STRUT_BOTTOM;
    setstrut(selmon, pos, selmon->show_bar ? bar_height : 0);

    if (selmon->show_bar)
        loft_widget_show(&selmon->bar->win.base);
    else
        loft_widget_hide(&selmon->bar->win.base);

    arrange(selmon);
}

void togglebarpos (const Arg* arg) {
    if (selmon->show_bar == false)
        return;

    int old_st_pos = selmon->bar_pos == TOP ? STRUT_TOP : STRUT_BOTTOM;
    selmon->bar_pos = selmon->bar_pos == false;
    int new_st_pos = selmon->bar_pos == TOP ? STRUT_TOP : STRUT_BOTTOM;

    selmon->struts[old_st_pos] = 0;
    selmon->struts[new_st_pos] += bar_height;

    updatestruts(selmon);

    if (selmon->bar_pos == TOP)
        selmon->bar_y = selmon->my;
    else
        selmon->bar_y = selmon->my + (selmon->mh - bar_height);

    loft_widget_move(&selmon->bar->win.base, selmon->mx, selmon->bar_y);

    arrange(selmon);
    updatebartags(selmon);
}

void togglefloating (const Arg* arg) {
    if (selmon->selected == NULL)
        return;

    selmon->selected->isfloating = selmon->selected->isfloating == false || selmon->selected->isfixed;

    int oldw;
    int oldh;

    if (selmon->selected->isfloating && selmon->selected->bw == 0) {
        oldw = WIDTH(selmon->selected);
        oldh = HEIGHT(selmon->selected);

        selmon->selected->bw = border_width;

        resize(selmon->selected, selmon->selected->x, selmon->selected->y, oldw, oldh, false);

        if (WIDTH(selmon->selected) > oldw && HEIGHT(selmon->selected) > oldh)
            resize(selmon->selected, selmon->selected->x, selmon->selected->y, oldw - (2 * border_width), oldh - (2 * border_width), false);

        if (WIDTH(selmon->selected) < oldw && HEIGHT(selmon->selected) < oldh)
            resize(selmon->selected, selmon->selected->x, selmon->selected->y, oldw, oldh, false);
    }
    else if (selmon->selected->isfullscreen)
        setfullscreen(selmon->selected, false);

    arrange(selmon);
}

void toggletag (const Arg *arg) {
    unsigned int newtags;

    if (selmon->selected == NULL)
        return;

    newtags = selmon->selected->tags ^ (arg->ui & TAGMASK);

    if (newtags) {
        selmon->selected->tags = newtags;
        focus(NULL);
        arrange(selmon);
    }
}

void toggleview (const Arg *arg) {
    unsigned int newtagset = selmon->tagset[selmon->selected_tags] ^ (arg->ui & TAGMASK);

    if (newtagset) {
        // update current tag if it was deselected

        if (((1 << selmon->current_tag) & newtagset) == false) {
            unsigned int i = 0;
            unsigned int mask = 1;

            while (i < LENGTH(tags)) {
                mask = 1 << i;
                if (mask & newtagset)
                    break;
                i++;
            }

            selmon->current_tag = i;
        }

        selmon->tagset[selmon->selected_tags] = newtagset;

        focus(NULL);
        arrange(selmon);
    }
}

void unfocus (Client *c, bool setfocus) {
    if (c == NULL)
        return;

    grabbuttons(c, false);
    XSetWindowBorder(dpy, c->win, border_normal);

    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void unmanage (Client *c, bool destroyed) {
    Monitor* m = c->mon;
    XWindowChanges wc;

    int i;
    for (i = 0; i < LENGTH(tags); i++) {
        if (m->tagfocus[i] == c)
            m->tagfocus[i] = NULL;
    }

    detach(c);
    detachstack(c);

    if (destroyed == false) {
        wc.border_width = c->oldbw;
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        setclientstate(c, WithdrawnState);
        XSync(dpy, false);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }

    free(c);
    focus(NULL);
    updateclientlist();
    arrange(m);
}

void unmapnotify (XEvent *e) {
    XUnmapEvent *ev = &e->xunmap;
    Client *c = wintoclient(ev->window);

    if (c != NULL) {
        if (ev->send_event)
            setclientstate(c, WithdrawnState);
        else
            unmanage(c, false);
    }
}

void updatebars (void) {
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
        loft_label_init(&m->bar->lb_title, m->selected != NULL ? m->selected->name : NULL, FLOW_L);
        loft_label_init(&m->bar->lb_status, status, FLOW_R);

        loft_label_truncate(&m->bar->lb_title, true);

        m->bar->win.base.draw_base = false;
        m->bar->lb_layout.base.draw_base = false;
        m->bar->lb_title.base.draw_base = false;
        m->bar->lb_status.base.draw_base = false;

        loft_label_set_padding(&m->bar->lb_layout, 10,0,10,0);
        loft_label_set_padding(&m->bar->lb_title, 8,0,8,0);
        loft_label_set_padding(&m->bar->lb_status, 8,0,8,0);

        loft_layout_attach(&m->bar->lt_main, &m->bar->lt_tagstrip.base, EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_layout.base, EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_title.base, EXPAND_X | EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_status.base, EXPAND_Y);

        if (mons->next != NULL) {
            m->bar->indicator = malloc(sizeof(SelectionIndicator));

            loft_rgba_set_from_str(&m->bar->indicator->style.normal.bg, (char*) normal_mon_indicator_bg);
            loft_rgba_set_from_str(&m->bar->indicator->style.normal.fg, (char*) normal_mon_indicator_fg);
            loft_rgba_set_from_str(&m->bar->indicator->style.selected.bg, (char*) selected_mon_indicator_bg);
            loft_rgba_set_from_str(&m->bar->indicator->style.selected.fg, (char*) selected_mon_indicator_fg);

            m->bar->indicator->selected = selmon == m;

            loft_widget_init(&m->bar->indicator->base, "selmon_indicator", 0);
            m->bar->indicator->base.draw_base = false;

            loft_widget_set_minimum_size(&m->bar->indicator->base, bar_height, bar_height);
            loft_signal_connect(&m->bar->indicator->base, "draw", _draw_selection_indicator, NULL);

            loft_layout_attach(&m->bar->lt_main, &m->bar->indicator->base, EXPAND_Y);
        }

        m->bar->lb_tags = malloc(sizeof(TagLabel) * LENGTH(tags));

        for (i = 0; i < LENGTH(tags); i++) {
            t = m->bar->lb_tags + i;

            loft_widget_init(&t->base, "tag_label", 0);
            t->base.draw_base = false;

            t->num = i;
            t->current = false;
            t->unused = true;
            t->selected = false;
            t->has_sel = false;
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
            loft_signal_connect(&t->base, "draw", _draw_tag, NULL);

            // attach to layout

            loft_layout_attach(&m->bar->lt_tagstrip, &t->base, EXPAND_X | EXPAND_Y);
        }

        // set colors

        loft_rgba_set_from_str(&m->bar->lb_layout.style.normal.bg, (char*) ltsym_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_layout.style.normal.fg, (char*) ltsym_fg_color);

        loft_rgba_set_from_str(&m->bar->lb_title.style.normal.bg, (char*) title_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_title.style.normal.fg, (char*) title_fg_color);

        loft_rgba_set_from_str(&m->bar->lb_status.style.normal.bg, (char*) status_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_status.style.normal.fg, (char*) status_fg_color);

        // override redirect, move, resize, lock size, show

        loft_widget_override_redirect(&m->bar->win.base, true);
        loft_widget_move(&m->bar->win.base, m->mx, m->bar_y);
        loft_widget_resize(&m->bar->win.base, m->mw, bar_height);
        loft_widget_lock_size(&m->bar->win.base, true);

        updatebartags(m);
        updatebarlayout(m);
        updatebartitle(m);

        loft_widget_show_all(&m->bar->lt_main.base);

        if (show_bar) {
            setstrut(m, STRUT_TOP, bar_height);
            loft_widget_show(&m->bar->win.base);
        }
    }
}

inline void updatebarlayout (Monitor* m) {
    strncpy(m->ltsymbol, m->layouts[m->current_tag]->symbol, sizeof(m->ltsymbol));
    REDRAW_IF_VISIBLE(&m->bar->lb_layout.base);
}

void updatebartags (Monitor* m) {
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
        t->has_sel = m->selected != NULL && mask & m->selected->tags;
        t->selected = mask & m->tagset[m->selected_tags];
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

inline void updatebartitle (Monitor* m) {
    loft_label_set_text(&m->bar->lb_title, m->selected != NULL ? m->selected->name : NULL);
}

void updateborders (Monitor *m) {
    Client *c;

    int oldw;
    int oldh;

    if (m->layouts[m->current_tag]->arrange == NULL) {
        for (c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
            if (c->bw == 0) {
                oldw = WIDTH(c);
                oldh = HEIGHT(c);

                c->bw = border_width;

                resize(c, c->x, c->y, oldw, oldh, false);

                if (WIDTH(c) > oldw && HEIGHT(c) > oldh)
                    resize(c, c->x, c->y, oldw - (2*c->bw), oldh - (2*c->bw), false);

                if (WIDTH(c) < oldw && HEIGHT(c) < oldh)
                    resize(c, c->x, c->y, oldw, oldh, false);
            }
        }
        return;
    }

    int bdr;

    if (m->layouts[m->current_tag]->arrange == &monocle || ntiled(m) == 1)
        bdr = 0;
    else
        bdr = border_width;

    for (c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
        if (ISVISIBLE(c) == false)
            continue;

        if (c->bw != bdr) {
            oldw = WIDTH(c);
            oldh = HEIGHT(c);

            c->bw = bdr;

            resize(c, c->x, c->y, oldw, oldh, false);
        }
    }
}

void updateclientlist (void) {
    Client *c;
    Monitor *m;

    XDeleteProperty(dpy, root, netatom[NetClientList]);
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            XChangeProperty(dpy, root, netatom[NetClientList],
                            XA_WINDOW, 32, PropModeAppend,
                            (unsigned char *) &(c->win), 1);
}

bool updategeom (void) {
    bool dirty = false;

#ifdef XINERAMA
    if (XineramaIsActive(dpy)) {
        int i, j, n, nn;
        Client *c;
        Monitor *m;

        XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo *unique = NULL;

        for (n = 0, m = mons; m; m = m->next, n++);

        unique = malloc(sizeof(XineramaScreenInfo) * nn);

        if (unique == NULL)
            die("fatal: out of memory\n");

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
                    m->next = createmon();
                else
                    mons = createmon();
            }

            for (i = 0, m = mons; i < nn && m; m = m->next, i++)
                if (i >= n || (unique[i].x_org != m->mx || unique[i].y_org != m->my || unique[i].width != m->mw || unique[i].height != m->mh)) {
                    dirty = true;

                    m->num = i;
                    m->mx = m->wx = unique[i].x_org;
                    m->my = m->wy = unique[i].y_org;
                    m->mw = m->ww = unique[i].width;
                    m->mh = m->wh = unique[i].height;

                    updatestruts(m);
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
                    detachstack(c);
                    c->mon = mons;
                    attach(c);
                    attachstack(c);
                }

                if (m == selmon) {
                    selmon = mons;
                    updatemonindicators();
                }

                cleanupmon(m);
            }
        }

        free(unique);
    }
    else
#endif
    {
        if (mons == NULL)
            mons = createmon();

        if (mons->mw != sw || mons->mh != sh) {
            dirty = true;
            mons->mw = mons->ww = sw;
            mons->mh = mons->wh = sh;
            updatestruts(mons);
        }
    }

    if (dirty) {
        selmon = mons;
        selmon = wintomon(root);
    }

    return dirty;
}

void updatemonindicators (void) {
    Monitor* m;

    for (m = mons; m != NULL; m = m->next) {
        m->bar->indicator->selected = selmon == m;
        REDRAW_IF_VISIBLE(&m->bar->indicator->base);
    }
}

void updatenumlockmask (void) {
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

void updatesizehints (Client *c) {
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

void updatestruts (Monitor *m) {
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

void updatetitle (Client *c) {
    if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
        gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
    if (c->name[0] == '\0')
        strcpy(c->name, broken);
}

void updatestatus (void) {
    if (!gettextprop(root, XA_WM_NAME, status, sizeof(status)))
        sprintf(status, "ProperWM %s", VERSION);

    Monitor* m;
    for (m = mons; m != NULL; m = m->next)
        loft_label_set_text(&m->bar->lb_status, status);
}

void updatewindowtype (Client *c) {
    Atom state = getatomprop(c, netatom[NetWMState]);
    Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen])
        setfullscreen(c, true);
    if (wtype == netatom[NetWMWindowTypeDialog])
        c->isfloating = true;
}

void updatewmhints (Client *c) {
    XWMHints *wmh = XGetWMHints(dpy, c->win);

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

        updatebartags(c->mon);
        XFree(wmh);
    }
}

void view (const Arg *arg) {
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

    focus(selmon->tagfocus[selmon->current_tag]);

    arrange(selmon);
}

Client* wintoclient (Window w) {
    Client *c;
    Monitor *m;

    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            if (c->win == w)
                return c;

    return NULL;
}

Monitor* wintomon (Window w) {
    int x, y;
    Client *c;
    Monitor *m;

    if (w == root && getrootptr(&x, &y))
        return recttomon(x, y, 1, 1);

    LoftWidget* lw;

    for (m = mons; m; m = m->next) {
        lw = loft_widget_from_xwin(&w);

        if (lw != NULL) {
            lw = loft_widget_toplevel(lw);
            if (lw == &m->bar->win.base)
                return m;
        }
    }

    if ((c = wintoclient(w)))
        return c->mon;

    return selmon;
}

int xerror (Display *dpy, XErrorEvent *ee) {
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

int xerrordummy (Display *dpy, XErrorEvent *ee) {
    return 0;
}

int xerrorstart (Display *dpy, XErrorEvent *ee) {
    die("properwm: another window manager is already running\n");
    return -1;
}

void zoom (const Arg *arg) {
    Client *c = selmon->selected;

    if (selmon->layouts[selmon->current_tag]->arrange == NULL || (selmon->selected && selmon->selected->isfloating))
        return;

    if (c == nexttiled(selmon->clients))
        if (c == NULL || (c = nexttiled(c->next)) == NULL)
            return;

    pop(c);
}

int main (int argc, char *argv[]) {
    loft_init();
    dpy = loftenv.display; 

    if (dpy == NULL)
        die("properwm: cannot open display");

    loftenv.font = (char*) font_name;
    loftenv.font_size = (int) font_size;

    if (argc == 2 && !strcmp("-v", argv[1]))
        die("ProperWM "VERSION", 2012 speeddefrost, 2006-2012 dwm authors -- see LICENSE for details\n");
    else if (argc != 1)
        die("usage: properwm [-v]\n");

    checkotherwm();
    setup();
    scan();
    run();
    cleanup();

    return EXIT_SUCCESS;
}
