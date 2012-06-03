#include <errno.h>
#include <locale.h>

#include <loft.h>

#include <pthread.h>
#include <signal.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#undef __USE_BSD

#include <string.h>
#include <strext.h>

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
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
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

void applyrules (Client *c);
bool applysizehints (Client *c, int *x, int *y, int *w, int *h, bool interact);
void arrange (Monitor *m);
void arrangemon (Monitor *m);
void attach (Client *c);
void attachstack (Client *c);
void buttonpress (XEvent *e);
void checkotherwm (void);
void cleanup (void);
void cleanupmon (Monitor *mon);
void clearurgent (Client *c);
void clientmessage (XEvent *e);
void configure (Client *c);
void configurenotify (XEvent *e);
void configurerequest (XEvent *e);
Monitor* createmon (void);
void destroynotify (XEvent *e);
void detach (Client *c);
void detachstack (Client *c);
void die (const char *errstr, ...);
Monitor* dirtomon (int dir);
void enternotify (XEvent *e);
void focus (Client *c);
void focusin (XEvent *e);
void focusmon (const Arg *arg);
void focusstack (const Arg *arg);
unsigned long getcolor (const char *colstr);
bool getrootptr (int *x, int *y);
long getstate (Window w);
bool gettextprop (Window w, Atom atom, char *text, unsigned int size);
void grabbuttons (Client *c, bool focused);
void grabkeys (void);
void incnmaster (const Arg *arg);
void iteration (void);
void keypress (XEvent *e);
void killclient (const Arg *arg);
void manage (Window w, XWindowAttributes *wa);
void mappingnotify (XEvent *e);
void maprequest (XEvent *e);
void monocle (Monitor *m);
void motionnotify (XEvent *e);
void movemouse (const Arg *arg);
Client* nexttiled (Client *c);
int ntiled (Monitor *m);
void pop (Client *);
Client* prevtiled (Client *c);
void propertynotify (XEvent *e);
void pushdown (const Arg *arg);
void pushup (const Arg *arg);
void quit (const Arg *arg);
Monitor* recttomon (int x, int y, int w, int h);
void resetnmaster (const Arg *arg);
void resize (Client *c, int x, int y, int w, int h, bool interact);
void resizeclient (Client *c, int x, int y, int w, int h);
void resizemouse (const Arg *arg);
void restack (Monitor *m);
void run (void);
void scan (void);
bool sendevent (Client *c, Atom proto);
void sendmon (Client *c, Monitor *m);
void setclientstate (Client *c, long state);
void setfocus (Client *c);
void setfullscreen (Client *c, bool fullscreen);
void setlayout (const Arg *arg);
void setmfact (const Arg *arg);
bool setstrut (Monitor *m, int pos, int px);
void setup (void);
void showhide (Client *c);
void sigchld (int unused);
void spawn (const Arg *arg);
void stack (Monitor *m );
void tag (const Arg *arg);
void tagmon (const Arg *arg);
void tile (Monitor *m);
void togglebar (const Arg *arg);
void togglebarpos (const Arg *arg);
void togglefloating (const Arg *arg);
void toggletag (const Arg *arg);
void toggleview (const Arg *arg);
void unfocus (Client *c, bool setfocus);
void unmanage (Client *c, bool destroyed);
void unmapnotify (XEvent *e);
bool updategeom (void);
void updatebars (void);
void updatebarlayout (Monitor* m);
void updatebarstatus (Monitor* m);
void updatebartags (Monitor* m);
void updatebartitle (Monitor* m);
void updateborders (Monitor *m);
void updateclientlist (void);
void updatenumlockmask (void);
void updatesizehints (Client *c);
void updatestatus (void);
void updatestruts (Monitor *m);
void updatewindowtype (Client *c);
void updatebartags (Monitor* m);
void updatetitle (Client *c);
void updatewmhints (Client *c);
void view (const Arg *arg);
Client* wintoclient (Window w);
Monitor* wintomon (Window w);
int xerror (Display *dpy, XErrorEvent *ee);
int xerrordummy (Display *dpy, XErrorEvent *ee);
int xerrorstart (Display *dpy, XErrorEvent *ee);
void zoom (const Arg *arg);

unsigned long border_normal;
unsigned long border_selected;

const char broken[] = "broken";
char stext[256];
int screen;
int sw, sh;           /* X display screen geometry width, height */
int bh = 0;           /* bar geometry */
int (*xerrorxlib)(Display *, XErrorEvent *);
unsigned int numlockmask = 0;
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
    [MotionNotify] = motionnotify,
    [PropertyNotify] = propertynotify,
    [UnmapNotify] = unmapnotify
};
Atom wmatom[WMLast], netatom[NetLast];
bool running = true;
Cursor cursor[CurLast];
Display *dpy;
Monitor *mons = NULL, *selmon = NULL;
Window root;

#include "config.h"

typedef enum StrutPosition {
    STRUT_TOP,
    STRUT_BOTTOM,
    STRUT_LEFT,
    STRUT_RIGHT
} StrutPosition;

typedef struct Bar {
    LoftWindow win;
    LoftLayout lt_main;
    LoftLayout lt_tags;
    LoftLabel lb_layout;
    LoftLabel lb_title;
    LoftLabel lb_status;
} Bar;

typedef struct TagLabel {
    LoftWidget base;

    int num;

    bool current;
    bool has_sel;
    bool selected;
    bool unused;
    bool urgent;

    struct {
        LoftRGBAPair unused;
        LoftRGBAPair normal;
        LoftRGBAPair selected;
        LoftRGBAPair urgent;
    } style;
} TagLabel;

struct Monitor {
    int num;
    int by;               /* bar geometry */
    int mx, my, mw, mh;   /* screen size */
    int wx, wy, ww, wh;   /* window area  */

    int struts[4];

    unsigned int curtag;
    unsigned int prevtag;

    char ltsymbol[16];
    unsigned int seltags;
    unsigned int sellt;
    unsigned int tagset[2];

    bool show_bar;
    bool top_bar;

    Client *clients;
    Client *sel;
    Client *stack;

    Bar* bar;
    TagLabel* lb_tags;

    Client* focus[LENGTH(tags)];
    float mfacts[LENGTH(tags)];
    int nmasters[LENGTH(tags)];
    const Layout *lts[LENGTH(tags)];

    Monitor *next;
};

struct CheckTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };


// SIGNAL CALLBACKS

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

    if (t->selected) {
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

    loft_cairo_set_rgba(cr, bg);

    cairo_rectangle(cr, 0, 0, t->base.width, t->base.height);
    cairo_fill(cr);

    cairo_select_font_face(cr, loftenv.font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, loftenv.font_size);

    cairo_text_extents_t ext;
    cairo_text_extents(cr, tags[t->num], &ext);

    double x = (t->base.width / 2) - ((ext.width / 2) + ext.x_bearing);
    double y = (t->base.height / 2) - ((ext.height / 2) + ext.y_bearing);

    loft_cairo_set_rgba(cr, fg);

    cairo_move_to(cr, x, y);
    cairo_show_text(cr, tags[t->num]);

    if (client_indicator && t->has_sel) {
        cairo_rectangle(cr, 0, 0, t->base.width, 1);
        cairo_fill(cr);
    }

    if (tag_indicator && t->current) {
        int x_center = t->base.width / 2;
        int y_between = t->base.height - (t->base.height / 6);

        cairo_new_path(cr);
        cairo_set_line_width(cr, 1.5);

        cairo_move_to(cr, x_center / 2, t->base.height - 1);
        cairo_line_to(cr, x_center, y_between);
        cairo_line_to(cr, x_center + (x_center / 2), t->base.height - 1);

        cairo_close_path(cr);
        cairo_fill(cr);
    }

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

    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

bool applysizehints (Client *c, int *x, int *y, int *w, int *h, bool interact) {
    bool baseismin;
    Monitor *m = c->mon;

    /* set minimum possible */
    *w = MAX(1, *w);
    *h = MAX(1, *h);

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

    if (*h < bh)
        *h = bh;
    if (*w < bh)
        *w = bh;

    if (tiled_size_hints || c->isfloating || c->mon->lts[c->mon->curtag]->arrange == NULL) {
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
    strncpy(m->ltsymbol, m->lts[m->curtag]->symbol, sizeof(m->ltsymbol));

    if (smart_borders)
        updateborders(m);

    if (m->lts[m->curtag]->arrange)
        m->lts[m->curtag]->arrange(m);
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
    unsigned int i, click;
    Monitor *m;
    XButtonPressedEvent *ev = &e->xbutton;

    click = ClkRootWin;

    /* focus monitor if necessary */

    m = wintomon(ev->window);

    if (m != NULL && m != selmon) {
        unfocus(selmon->sel, true);
        selmon = m;
        focus(NULL);
    }

    Client *c = wintoclient(ev->window);

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
    selmon->lts[selmon->curtag] = &foo;

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
        for (m = mons; m && m->next != mon; m = m->next);
        m->next = mon->next;
    }

    loft_widget_hide_all(&mon->bar->win.base);

    int i;
    for (i = 0; i < LENGTH(tags); i++)
        loft_widget_destroy(&mon->lb_tags[i].base);

    free(mon->lb_tags);

    loft_widget_destroy(&mon->bar->lb_layout.base);
    loft_widget_destroy(&mon->bar->lb_title.base);
    loft_widget_destroy(&mon->bar->lb_status.base);
    loft_widget_destroy(&mon->bar->lt_tags.base);
    loft_widget_destroy(&mon->bar->lt_main.base);

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
            c->mon->seltags ^= 1;
            c->mon->tagset[c->mon->seltags] = c->tags;
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
                loft_widget_move(&m->bar->win.base, m->mx, m->by);
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
        else if (c->isfloating || !selmon->lts[selmon->curtag]->arrange) {
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
    Monitor *m;

    if (!(m = (Monitor *)calloc(1, sizeof(Monitor))))
        die("fatal: could not malloc() %u bytes\n", sizeof(Monitor));

    m->by = 0;

    m->prevtag = m->curtag = 0;
    m->tagset[0] = m->tagset[1] = 1;
    m->show_bar = show_bar;
    m->top_bar = top_bar;

    m->sel = NULL;

    m->bar = NULL;
    m->lb_tags = NULL;

    int i;

    for (i = 0; i < 4; i++) {
        if (m->show_bar && ((i == STRUT_TOP && m->top_bar) || (i == STRUT_BOTTOM && m->top_bar == false)))
            m->struts[i] = bh;
        else
            m->struts[i] = 0;
    }

    updatestruts(m);

    for (i = 0; i < LENGTH(tags); i++) {
        m->focus[i] = NULL;
        m->lts[i] = &layouts[0];
        m->mfacts[i] = mfact;
        m->nmasters[i] = nmaster;
    }

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

    if (c == c->mon->sel) {
        for (t = c->mon->stack; t && ISVISIBLE(t) == false; t = t->snext);
        c->mon->sel = t;
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

    if (click_to_focus)
        return;

    if (ev->window != root && (ev->mode != NotifyNormal || ev->detail == NotifyInferior))
        return;

    c = wintoclient(ev->window);
    m = c ? c->mon : wintomon(ev->window);

    if (m != selmon) {
        unfocus(selmon->sel, true);
        selmon = m;
    }
    else if (c == NULL || c == selmon->sel)
        return;

    focus(c);
}

void focus (Client *c) {
    if (c == NULL || ISVISIBLE(c) == false)
        for (c = selmon->stack; c != NULL && ISVISIBLE(c) == false; c = c->snext);

    if (selmon->sel && selmon->sel != c)
        unfocus(selmon->sel, false);

    if (c) {
        if (c->mon != selmon)
            selmon = c->mon;
        if (c->isurgent)
            clearurgent(c);
        detachstack(c);
        attachstack(c);
        grabbuttons(c, true);
        XSetWindowBorder(dpy, c->win, border_selected);
        setfocus(c);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }

    selmon->sel = c;
    selmon->focus[selmon->curtag] = c;

    updatebartags(selmon);
    updatebartitle(selmon);
}

void focusin (XEvent *e) { /* there are some broken focus acquiring clients */
    XFocusChangeEvent *ev = &e->xfocus;

    if (selmon->sel && ev->window != selmon->sel->win)
        setfocus(selmon->sel);
}

void focusmon (const Arg *arg) {
    Monitor *m;

    if (mons->next == NULL)
        return;
    
    m = dirtomon(arg->i);
    if (m == selmon)
        return;

    unfocus(selmon->sel, true);
    selmon = m;
    focus(NULL);
}

void focusstack (const Arg *arg) {
    Client *c = NULL, *i;

    if (selmon->sel == NULL)
        return;

    if (arg->i > 0) {
        for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
        if (c == NULL)
            for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
    } else {
        for (i = selmon->clients; i != selmon->sel; i = i->next)
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

void incnmaster (const Arg *arg) {
    selmon->nmasters[selmon->curtag] = MAX(selmon->nmasters[selmon->curtag] + arg->i, 0);
    arrange(selmon);
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

    // always check if loft wants the event first

    loft_process_event(&ev);

    if (handler[ev.type])
        handler[ev.type](&ev);
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
    if (!selmon->sel)
        return;

    if (!sendevent(selmon->sel, wmatom[WMDelete])) {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, selmon->sel->win);
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

    c->x = MAX(c->x, c->mon->mx);
    /* only fix client y-offset, ifthe client center might cover the bar */
    c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
               && (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bh : c->mon->my);

    updatewindowtype(c);
    updatesizehints(c);
    updatewmhints(c);

    if (c->isfloating == false)
        c->isfloating = c->oldstate = trans != None || c->isfixed;

    if (c->isfullscreen || (smart_borders && c->mon->lts[c->mon->curtag]->arrange != NULL && c->isfloating == false
    && (c->mon->lts[c->mon->curtag]->arrange == &monocle || ntiled(c->mon) == 0)))
        c->bw = 0;
    else
        c->bw = border_width;

    wc.border_width = c->bw;
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);

    XSetWindowBorder(dpy, w, border_normal);
    configure(c); /* propagates border_width, ifsize doesn't change */

    XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    grabbuttons(c, false);

    if (c->isfloating)
        XRaiseWindow(dpy, c->win);

    attach(c);
    attachstack(c);

    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
                    (unsigned char *) &(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */

    setclientstate(c, NormalState);

    if (c->mon == selmon)
        unfocus(selmon->sel, false);
    c->mon->sel = c;

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

void
monocle(Monitor *m) {
    unsigned int n = 0;
    Client *c;

    for (c = m->clients; c; c = c->next)
        if (ISVISIBLE(c))
            n++;

    if (n > 0) { /* override layout symbol */
        snprintf(m->ltsymbol, sizeof(m->ltsymbol), "%d", n);
        updatebarlayout(m);
    }

    for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
        resize(c, m->wx + padding, m->wy + padding, m->ww - (2*c->bw) - (2*padding), m->wh - (2*c->bw) - (2*padding), false);
}

void motionnotify (XEvent *e) {
    static Monitor *mon = NULL;
    Monitor *m;
    XMotionEvent *ev = &e->xmotion;

    if (ev->window != root)
        return;
    if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
        unfocus(selmon->sel, true);
        selmon = m;
        focus(NULL);
    }
    mon = m;
}

void movemouse (const Arg *arg) {
    int x, y, ocx, ocy, nx, ny;
    Client *c;
    Monitor *m;
    XEvent ev;

    if (!(c = selmon->sel))
        return;

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

            if (c->isfloating == false && selmon->lts[selmon->curtag]->arrange && (ev.xmotion.x < x - snap || ev.xmotion.x > x + snap || ev.xmotion.y < y - snap || ev.xmotion.y > y + snap))
                togglefloating(NULL);

            if (c->isfloating || selmon->lts[selmon->curtag]->arrange == NULL) {
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
    }
}

Client* nexttiled (Client *c) {
    for(; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
    return c;
}

int ntiled (Monitor *m) {
    Client *c;
    int nt = 0;
    for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
        if (ISVISIBLE(c)) nt++;
    return nt;
}

void pop (Client *c) {
    detach(c);
    attach(c);
    focus(c);
    arrange(c->mon);
}

Client* prevtiled (Client *c) {
    Client *p, *r;

    for(p = selmon->clients, r = NULL; p && p != c; p = p->next)
        if (!p->isfloating && ISVISIBLE(p))
            r = p;

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
            if (c == c->mon->sel)
                updatebartitle(c->mon);
        }

        if (ev->atom == netatom[NetWMWindowType])
            updatewindowtype(c);
    }
}

void pushdown (const Arg *arg) {
    Client *sel = selmon->sel;
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
    Client *sel = selmon->sel;
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

    for (m = mons; m; m = m->next)
        if ((a = INTERSECT(x, y, w, h, m)) > area) {
            area = a;
            r = m;
        }
    return r;
}

void resetnmaster (const Arg *arg) {
    selmon->nmasters[selmon->curtag] = nmaster;
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

    if (selmon->sel == NULL)
        return;

    c = selmon->sel;

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
            nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
            nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);

            if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
            && c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
            {
                if (!c->isfloating && selmon->lts[selmon->curtag]->arrange
                && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
                    togglefloating(NULL);
            }

            if (!selmon->lts[selmon->curtag]->arrange || c->isfloating) {
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
    }
}

void restack (Monitor *m) {
    Client *c;
    XEvent ev;
    XWindowChanges wc;

    if (m->sel == NULL)
        return;

    if (m->sel->isfloating || m->lts[m->curtag]->arrange == NULL)
        XRaiseWindow(dpy, m->sel->win);

    if (m->lts[m->curtag]->arrange) {
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

void sendmon (Client *c, Monitor *m) {
    if (c->mon == m)
        return;
    unfocus(c, true);
    detach(c);
    detachstack(c);
    c->mon = m;
    c->tags = m->tagset[m->seltags]; // assign tags of target monitor
    attach(c);
    attachstack(c);
    focus(NULL);
    arrange(NULL);
}

void setclientstate (Client *c, long state) {
    long data[] = { state, None };

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
            PropModeReplace, (unsigned char *)data, 2);
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
        c->isfullscreen = true;
        c->oldbw = c->bw;
        c->bw = 0;
        c->oldstate = c->isfloating;
        c->isfloating = true;
        resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        if (smart_borders)
            updateborders(c->mon);
        XRaiseWindow(dpy, c->win);
    }
    else {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)0, 0);
        c->isfullscreen = false;
        c->isfloating = c->oldstate;
        c->bw = c->oldbw;
        c->x = c->oldx;
        c->y = c->oldy;
        c->w = c->oldw;
        c->h = c->oldh;
        resizeclient(c, c->x, c->y, c->w, c->h);
    }

    arrange(c->mon);
}

void setlayout (const Arg *arg) {
    if (!arg || !arg->v || arg->v == selmon->lts[selmon->curtag])
        return;

    if (arg && arg->v)
        selmon->lts[selmon->curtag] = (Layout *)arg->v;

    strncpy(selmon->ltsymbol, selmon->lts[selmon->curtag]->symbol, sizeof(selmon->ltsymbol));
    updatebarlayout(selmon);

    if (selmon->sel)
        arrange(selmon);
}

/* arg > 1.0 will set mfact absolutly */
void setmfact (const Arg *arg) {
    float f;

    if (arg == NULL || selmon->lts[selmon->curtag]->arrange == NULL)
        return;

    f = arg->f < 1.0 ? arg->f + selmon->mfacts[selmon->curtag] : arg->f - 1.0;

    if (f < 0.1 || f > 0.9)
        return;

    selmon->mfacts[selmon->curtag] = f;
    arrange(selmon);
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
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    bh = bar_height;

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

    // EWMH support per view

    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
            PropModeReplace, (unsigned char *) netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);

    // select events

    wa.cursor = cursor[CurNormal];
    wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask|PointerMotionMask
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
        if ((c->mon->lts[c->mon->curtag]->arrange == NULL || c->isfloating) && c->isfullscreen == false)
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
        if (dpy)
            close(ConnectionNumber(dpy));
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        fprintf(stderr, "properwm: execvp %s", ((char **)arg->v)[0]);
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
    int mx, sx, rem; // x positions and initial offset calculation
    int mww, mwh; // dimensions for windows in master area
    int sww, swh; // dimensions for windows in stack area
    int syp; // stack area y position padding

    int i;
    Client *c;

    n = ntiled(m);

    if (n <= m->nmasters[m->curtag]) {
        if (n > 2)
            nm = n-1;
        else
            nm = 1;
    } else
        nm = m->nmasters[m->curtag];

    ns = n-nm;

    if (n > nm)
        mh = nm ? m->wh * m->mfacts[m->curtag] : 0;
    else
        mh = m->wh;

    bw = (smart_borders && n == 1 ? 0 : border_width);

    mx = 0;
    sx = 0;

    if (nm > 0) {
        // MASTER WINDOW WIDTH

        mww = (m->ww - ((2*bw) * nm) - (padding * (nm+1))) / nm;

        // MASTER WINDOW HEIGHT
        //
        // when (n > nmaster) - subtract top padding and half of middle padding
        // when (n <= nmaster) - subtract both top and bottom padding

        mwh = mh - (2*bw) - (n > nm ? padding + (padding/2) : 2*padding);

        /* MASTER X OFFSET */

        rem = (m->ww - ((2*bw) * nm) - (padding * (nm+1))) % nm;
        if (rem > 0)
            mx += (rem/2);
    }
    else {
        mww = 0;
        mwh = 0; // make compiler stfu
    }

    if (ns > 0) {
        /* STACK WINDOW WIDTH */

        sww = (m->ww - ((2*bw) * ns) - (padding * (ns+1))) / ns;

        // STACK WINDOW HEIGHT
        //
        // when (nmaster > 0) - subtract half of middle padding and all of bottom padding
        // when (nmaster == 0) - subtract both top and bottom padding

        swh = m->wh - mh - (2*bw) - (nm > 0 ? (padding/2) + padding : 2*padding);

        /* STACK X OFFSET */

        rem = (m->ww - ((2*bw) * ns) - (padding * (ns+1))) % ns;
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

    syp = (nm > 0 ? padding/2 : padding);

    for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
        if (i < nm) {
            resize(c, m->wx + mx + padding, m->wy + padding, mww, mwh, false);
            mx += padding + WIDTH(c);
        }
        else {
            resize(c, m->wx + sx + padding, m->wy + mh + syp, sww, swh, false);
            sx += padding + WIDTH(c);
        }
    }
}

void tag (const Arg *arg) {
    if (selmon->sel && arg->ui & TAGMASK) {
        selmon->sel->tags = arg->ui & TAGMASK;
        focus(NULL);
        updatebartags(selmon);
        arrange(selmon);
    }
}

void tagmon (const Arg *arg) {
    if (selmon->sel == NULL || mons->next == NULL)
        return;

    Monitor* newmon = dirtomon(arg->i);
    sendmon(selmon->sel, newmon);

    updatebartags(selmon);
    updatebartags(newmon);
}

void tile (Monitor *m) {
    int n; // window count
    int nm; // master area window count
    int nt; // tile area window count
    int mw; // master area width
    int bw; // border width to use for calculations
    int my, ty, rem; // y positions and initial offset calculation
    int mww, mwh; // dimensions for windows in master area
    int tww, twh; // dimensions for windows in tile area
    int txp; // tile area x padding

    int i;
    Client *c;

    n = ntiled(m);

    if (n <= m->nmasters[m->curtag]) {
        if (n > 2)
            nm = n-1;
        else
            nm = 1;
    } else
        nm = m->nmasters[m->curtag];

    nt = n-nm;

    if (n > nm)
        mw = nm ? m->ww * m->mfacts[m->curtag] : 0;
    else
        mw = m->ww;

    bw = (smart_borders && n == 1 ? 0 : border_width);

    my = 0;
    ty = 0;

    if (nm > 0) {
        // MASTER WINDOW WIDTH
        //
        // when (n > nmaster) - subtract all of left padding and half of center padding
        // when (n <= nmaster) - subtract both left and right padding for consistency

        mww = mw - (2*bw) - (n > nm ? padding + (padding/2) : 2*padding);
        mwh = (m->wh - ((2*bw) * nm) - (padding * (nm+1))) / nm;

        // MASTER Y OFFSET

        rem = (m->wh - ((2*bw) * nm) - (padding * (nm+1))) % nm;
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

        tww = m->ww - mw - (2*bw) - (nm > 0 ? (padding/2) + padding : 2*padding);
        twh = (m->wh - ((2*bw) * nt) - (padding * (nt+1))) / nt;

        // TILE Y OFFSET

        rem = (m->wh - ((2*bw) * nt) - (padding * (nt+1))) % nt;
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

    txp = (nm > 0 ? padding/2 : padding);

    for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
        if (i < nm) {
            resize(c, m->wx + padding, m->wy + my + padding, mww, mwh, false);
            my += padding + HEIGHT(c);
        }
        else {
            resize(c, m->wx + mw + txp, m->wy + ty + padding, tww, twh, false);
            ty += padding + HEIGHT(c);
        }
    }
}

void togglebar (const Arg *arg) {
    selmon->show_bar = selmon->show_bar == false;

    selmon->struts[(selmon->top_bar ? STRUT_TOP : STRUT_BOTTOM)] = (selmon->show_bar ? bh : 0);
    updatestruts(selmon);

    if (selmon->show_bar)
        loft_widget_show(&selmon->bar->win.base);
    else
        loft_widget_hide(&selmon->bar->win.base);

    arrange(selmon);
}

void togglebarpos (const Arg *arg) {
    if (selmon->show_bar == false)
        return;

    selmon->struts[(selmon->top_bar ? STRUT_TOP : STRUT_BOTTOM)] = 0;
    selmon->top_bar = selmon->top_bar == false;
    selmon->struts[(selmon->top_bar ? STRUT_TOP : STRUT_BOTTOM)] += bh;

    updatestruts(selmon);

    if (selmon->top_bar)
        selmon->by = selmon->my;
    else
        selmon->by = selmon->my + (selmon->mh - bh);

    loft_widget_move(&selmon->bar->win.base, selmon->mx, selmon->by);
    arrange(selmon);
}

void togglefloating (const Arg *arg) {
    if (selmon->sel == NULL)
        return;

    selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;

    int oldw;
    int oldh;

    if (selmon->sel->isfloating && selmon->sel->bw == 0) {
        oldw = WIDTH(selmon->sel);
        oldh = HEIGHT(selmon->sel);

        selmon->sel->bw = border_width;

        resize(selmon->sel, selmon->sel->x, selmon->sel->y, oldw, oldh, false);

        if (WIDTH(selmon->sel) > oldw && HEIGHT(selmon->sel) > oldh)
            resize(selmon->sel, selmon->sel->x, selmon->sel->y, oldw - (2*selmon->sel->bw), oldh - (2*selmon->sel->bw), false);

        if (WIDTH(selmon->sel) < oldw && HEIGHT(selmon->sel) < oldh)
            resize(selmon->sel, selmon->sel->x, selmon->sel->y, oldw, oldh, false);
    }
    else if (selmon->sel->isfullscreen)
        setfullscreen(selmon->sel, false);

    arrange(selmon);
}

void toggletag (const Arg *arg) {
    unsigned int newtags;

    if (selmon->sel == NULL)
        return;

    newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);

    if (newtags) {
        selmon->sel->tags = newtags;
        focus(NULL);
        arrange(selmon);
    }
}

void toggleview (const Arg *arg) {
    unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

    if (newtagset) {
        selmon->tagset[selmon->seltags] = newtagset;
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
        if (m->focus[i] == c)
            m->focus[i] = NULL;
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
        loft_layout_init(&m->bar->lt_main, ASPECT_H, 10);
        loft_layout_init(&m->bar->lt_tags, ASPECT_H, 0);

        loft_window_set_layout(&m->bar->win, &m->bar->lt_main);

        loft_label_init(&m->bar->lb_layout, ALIGN_CENTER, m->ltsymbol);
        loft_label_init(&m->bar->lb_title, ALIGN_LEFT, m->sel != NULL ? m->sel->name : NULL);
        loft_label_init(&m->bar->lb_status, ALIGN_CENTER, stext);

        m->bar->lb_layout.base.draw_base = false;
        m->bar->lb_title.base.draw_base = false;
        m->bar->lb_status.base.draw_base = false;

        loft_label_set_padding(&m->bar->lb_layout, 5, 0);
        loft_label_set_padding(&m->bar->lb_title, 5, 0);
        loft_label_set_padding(&m->bar->lb_status, 5, 0);

        loft_layout_attach(&m->bar->lt_main, &m->bar->lt_tags.base, EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_layout.base, EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_title.base, EXPAND_X | EXPAND_Y);
        loft_layout_attach(&m->bar->lt_main, &m->bar->lb_status.base, EXPAND_Y);

        m->lb_tags = malloc(sizeof(TagLabel) * LENGTH(tags));

        for (i = 0; i < LENGTH(tags); i++) {
            t = m->lb_tags + i;

            // init base

            loft_widget_init(&t->base, "taglabel", 0);
            t->base.draw_base = false;

            // init defaults

            t->num = i;

            t->current = false;
            t->unused = true;
            t->selected = false;
            t->has_sel = false;
            t->urgent = false;

            // set tag colors

            loft_rgba_set_from_str(&t->style.unused.bg, (char*) unused_tag_bg_color);
            loft_rgba_set_from_str(&t->style.unused.fg, (char*) unused_tag_fg_color);

            loft_rgba_set_from_str(&t->style.normal.bg, (char*) normal_tag_bg_color);
            loft_rgba_set_from_str(&t->style.normal.fg, (char*) normal_tag_fg_color);

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

            loft_widget_set_minimum_size(&t->base, ext.width + 10, f_ext.height + 8);

            cairo_destroy(cr);

            // catch events

            loft_signal_connect(&t->base, "button-press", _on_tag_pressed, NULL);
            loft_signal_connect(&t->base, "draw", _draw_tag, NULL);

            // attach to layout

            loft_layout_attach(&m->bar->lt_tags, &t->base, EXPAND_X | EXPAND_Y);
        }

        // set label colors

        loft_rgba_set_from_str(&m->bar->lb_layout.style.normal.bg, (char*) ltsym_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_layout.style.normal.fg, (char*) ltsym_fg_color);

        loft_rgba_set_from_str(&m->bar->lb_title.style.normal.bg, (char*) title_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_title.style.normal.fg, (char*) title_fg_color);

        loft_rgba_set_from_str(&m->bar->lb_status.style.normal.bg, (char*) status_bg_color);
        loft_rgba_set_from_str(&m->bar->lb_status.style.normal.fg, (char*) status_fg_color);

        // override redirect, move, resize, and show

        loft_widget_move(&m->bar->win.base, m->mx, m->by);
        loft_widget_resize(&m->bar->win.base, m->mw, bh);
        loft_widget_override_redirect(&m->bar->win.base, true);

        loft_widget_show_all(&m->bar->lt_main.base);

        updatebartags(m);
        updatebarlayout(m);
        updatebartitle(m);
        updatebarstatus(m);

        if (show_bar) {
            setstrut(m, STRUT_TOP, bh);
            loft_widget_show(&m->bar->win.base);
        }
    }
}

inline void updatebarlayout (Monitor* m) {
    loft_label_set_text(&m->bar->lb_layout, m->ltsymbol);
}

void updatebarstatus (Monitor* m) {
    loft_label_set_text(&m->bar->lb_status, stext);
}

void updatebartags (Monitor* m) {
    int i;
    int mask;
    TagLabel* t;

    Client* c = NULL;
    int cc;

    for (i = 0, cc = 0; i < LENGTH(tags); i++, cc = 0) {
        mask = 1 << i;
        t = m->lb_tags + i;

        t->current = m->curtag == i;
        t->selected = m->tagset[m->seltags] & mask;
        t->has_sel = m->sel != NULL && m->sel->tags & mask;

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
    loft_label_set_text(&m->bar->lb_title, m->sel != NULL ? m->sel->name : NULL);
}

void updateborders (Monitor *m) {
    Client *c;

    int oldw;
    int oldh;

    if (m->lts[m->curtag]->arrange == NULL) {
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

    if (m->lts[m->curtag]->arrange == &monocle || ntiled(m) == 1)
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
    for(m = mons; m; m = m->next)
        for(c = m->clients; c; c = c->next)
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

        /* only consider unique geometries as separate screens */
        if (!(unique = (XineramaScreenInfo *)malloc(sizeof(XineramaScreenInfo) * nn)))
            die("fatal: could not malloc() %u bytes\n", sizeof(XineramaScreenInfo) * nn);
        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));

        XFree(info);
        nn = j;

        if (n <= nn) {
            for(i = 0; i < (nn - n); i++) { /* new monitors available */
                for(m = mons; m && m->next; m = m->next);
                if (m)
                    m->next = createmon();
                else
                    mons = createmon();
            }
            for(i = 0, m = mons; i < nn && m; m = m->next, i++)
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
        else { /* less monitors available nn < n */
            for(i = nn; i < n; i++) {
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
                if (m == selmon)
                    selmon = mons;
                cleanupmon(m);
            }
        }

        free(unique);
    }
    else
#endif/* XINERAMA */
    /* default monitor setup */
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

void updatenumlockmask (void) {
    unsigned int i, j;
    XModifierKeymap *modmap;

    numlockmask = 0;
    modmap = XGetModifierMapping(dpy);

    for(i = 0; i < 8; i++)
        for(j = 0; j < modmap->max_keypermod; j++)
            if (modmap->modifiermap[i * modmap->max_keypermod + j]
               == XKeysymToKeycode(dpy, XK_Num_Lock))
                numlockmask = (1 << i);

    XFreeModifiermap(modmap);
}

void updatesizehints (Client *c) {
    long msize;
    XSizeHints size;

    if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
        /* size is uninitialized, ensure that size.flags aren't used */
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
    if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)))
        strcpy(stext, "ProperWM "VERSION);
    updatebarstatus(selmon);
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
        if (c == selmon->sel && wmh->flags & XUrgencyHint) {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(dpy, c->win, wmh);
        }
        else
            c->isurgent = (wmh->flags & XUrgencyHint) ? true : false;

        if (wmh->flags & InputHint)
            c->neverfocus = !wmh->input;
        else
            c->neverfocus = false;

        updatebartags(c->mon);

        XFree(wmh);
    }
}

void view (const Arg *arg) {
    if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
        return;

    selmon->seltags ^= 1;

    if (arg->ui & TAGMASK) {
        selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;

        selmon->prevtag = selmon->curtag;
        selmon->curtag = arg->ui;

        if (arg->ui != -1) {
            int i;
            for (i = 0; (arg->ui & 1 << i) == 0; i++);
            selmon->curtag = i;
        } else
            selmon->curtag = 0;
    }
    else {
        int oldcur = selmon->curtag;
        selmon->curtag = selmon->prevtag;
        selmon->prevtag = oldcur;
    }

    focus(selmon->focus[selmon->curtag]);
    updatebartags(selmon);

    arrange(selmon);
    updatebarlayout(selmon);
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

    for (m = mons; m; m = m->next)
        if (w == m->bar->win.base.xwin)
            return m;

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
    Client *c = selmon->sel;

    if (selmon->lts[selmon->curtag]->arrange == NULL || (selmon->sel && selmon->sel->isfloating))
        return;

    if (c == nexttiled(selmon->clients))
        if (c == NULL || (c = nexttiled(c->next)) == NULL)
            return;

    pop(c);
}

int main (int argc, char *argv[]) {
    loft_init();

    loftenv.font = (char*) font_name;
    loftenv.font_size = font_size;

    // init base color (used for bar)

    loft_rgba_set_from_str(&loftenv.colors.base, (char*) base_color);

    dpy = loftenv.display; 

    if (dpy == NULL)
        die("properwm: cannot open display");

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
