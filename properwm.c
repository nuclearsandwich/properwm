#include <air/proto/st.h>

#include <errno.h>
#include <locale.h>

#include <pthread.h>
#include <signal.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __USE_BSD
#undef __USE_BSD
#endif

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
#define TEXTW(X)                (textnw(X, strlen(X)) + dc.font.height)

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };
enum { ColBorder, ColFG, ColBG, ColLast };
enum { NetSupported,
       NetWMName, NetWMState,
       NetWMFullscreen, NetActiveWindow,
       NetWMWindowType, NetWMWindowTypeDialog,
       NetClientList,
       NetLast };
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast };
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
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
    int x, y, w, h;
    unsigned long norm[ColLast];
    unsigned long sel[ColLast];
    Drawable drawable;
    GC gc;
    struct {
        int ascent;
        int descent;
        int height;
        XFontSet set;
        XFontStruct *xfont;
    } font;
} DC;

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

/* function declarations */
static void applyrules (Client *c);
static bool applysizehints (Client *c, int *x, int *y, int *w, int *h, bool interact);
static void arrange (Monitor *m);
static void arrangemon (Monitor *m);
static void attach (Client *c);
static void attachstack (Client *c);
static void buttonpress (XEvent *e);
static void checkotherwm (void);
static void cleanup (void);
static void cleanupmon (Monitor *mon);
static void clearurgent (Client *c);
static void clientmessage (XEvent *e);
static void configure (Client *c);
static void configurenotify (XEvent *e);
static void configurerequest (XEvent *e);
static Monitor *createmon (void);
static void destroynotify (XEvent *e);
static void detach (Client *c);
static void detachstack (Client *c);
static void die (const char *errstr, ...);
static Monitor *dirtomon (int dir);
static void drawbar (Monitor *m);
static void drawbars (void);
static void drawsquare (bool filled, bool empty, bool invert, unsigned long col[ColLast]);
static void drawtext (const char *text, unsigned long col[ColLast], bool invert);
static void enternotify (XEvent *e);
static void expose (XEvent *e);
static void focus (Client *c);
static void focusin (XEvent *e);
static void focusmon (const Arg *arg);
static void focusstack (const Arg *arg);
static unsigned long getcolor (const char *colstr);
static bool getrootptr (int *x, int *y);
static long getstate (Window w);
static bool gettextprop (Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons (Client *c, bool focused);
static void grabkeys (void);
static void incnmaster (const Arg *arg);
static void initfont (const char *fontstr);
static void keypress (XEvent *e);
static void killclient (const Arg *arg);
static void manage (Window w, XWindowAttributes *wa);
static void mappingnotify (XEvent *e);
static void maprequest (XEvent *e);
static void monocle (Monitor *m);
static void motionnotify (XEvent *e);
static void movemouse (const Arg *arg);
static Client *nexttiled (Client *c);
static int ntiled (Monitor *m);
static void pop (Client *);
static Client *prevtiled (Client *c);
static void propertynotify (XEvent *e);
static void pushdown (const Arg *arg);
static void pushup (const Arg *arg);
static void quit (const Arg *arg);
static Monitor *recttomon (int x, int y, int w, int h);
static void resetnmaster (const Arg *arg);
static void resize (Client *c, int x, int y, int w, int h, bool interact);
static void resizeclient (Client *c, int x, int y, int w, int h);
static void resizemouse (const Arg *arg);
static void restack (Monitor *m);
static void run (void);
static void scan (void);
static bool sendevent (Client *c, Atom proto);
static void sendmon (Client *c, Monitor *m);
static void setclientstate (Client *c, long state);
static void setfocus (Client *c);
static void setfullscreen (Client *c, bool fullscreen);
static void setlayout (const Arg *arg);
static void setmfact (const Arg *arg);
static bool setstrut (Monitor *m, int pos, int px, int mode);
static void setup (void);
static void showhide (Client *c);
static void sigchld (int unused);
static void spawn (const Arg *arg);
static void stack (Monitor *m );
static void tag (const Arg *arg);
static void tagmon (const Arg *arg);
static int textnw (const char *text, unsigned int len);
static void tile (Monitor *m);
static void togglebar (const Arg *arg);
static void togglebarpos (const Arg *arg);
static void togglefloating (const Arg *arg);
static void toggletag (const Arg *arg);
static void toggleview (const Arg *arg);
static void unfocus (Client *c, bool setfocus);
static void unmanage (Client *c, bool destroyed);
static void unmapnotify (XEvent *e);
static bool updategeom (void);
static void updatebars (void);
static void updateborders (Monitor *m);
static void updateclientlist (void);
static void updatenumlockmask (void);
static void updatesizehints (Client *c);
static void updatestatus (void);
static void updatestruts (Monitor *m);
static void updatewindowtype (Client *c);
static void updatetitle (Client *c);
static void updatewmhints (Client *c);
static void view (const Arg *arg);
static Client* wintoclient (Window w);
static Monitor* wintomon (Window w);
static int xerror (Display *dpy, XErrorEvent *ee);
static int xerrordummy (Display *dpy, XErrorEvent *ee);
static int xerrorstart (Display *dpy, XErrorEvent *ee);
static void zoom (const Arg *arg);

/* server targets */

static void* init_remote (void* arg);

static ST_TARGET(ClientName);
static ST_TARGET(CurrentLayout);
static ST_TARGET(CurrentMonitor);
static ST_TARGET(CurrentTag);
static ST_TARGET(SetStrut);
static ST_TARGET(ToggleBar);
static ST_TARGET(ToggleBarPosition);
static ST_TARGET(ToggleView);
static ST_TARGET(View);

/* variables */
static const char broken[] = "broken";
static char stext[256];
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh, blw = 0;      /* bar geometry */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
    [ButtonPress] = buttonpress,
    [ClientMessage] = clientmessage,
    [ConfigureRequest] = configurerequest,
    [ConfigureNotify] = configurenotify,
    [DestroyNotify] = destroynotify,
    [EnterNotify] = enternotify,
    [Expose] = expose,
    [FocusIn] = focusin,
    [KeyPress] = keypress,
    [MappingNotify] = mappingnotify,
    [MapRequest] = maprequest,
    [MotionNotify] = motionnotify,
    [PropertyNotify] = propertynotify,
    [UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast];
static bool running = true;
static Cursor cursor[CurLast];
static Display *dpy;
static DC dc;
static Monitor *mons = NULL, *selmon = NULL;
static Window root;

/* configuration, allows nested code to access above variables */
#include "config.h"

typedef enum StrutPosition {
    POS_TOP,
    POS_BOTTOM,
    POS_LEFT,
    POS_RIGHT
} StrutPosition;

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

    bool showbar;
    bool topbar;

    Client *clients;
    Client *sel;
    Client *stack;

    Window barwin;

    float mfacts[LENGTH(tags)];
    int nmasters[LENGTH(tags)];
    const Layout *lts[LENGTH(tags)];

    Monitor *next;
};

/* compile-time check to make sure all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

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

    if (resizehints || c->isfloating || c->mon->lts[c->mon->curtag]->arrange == NULL) {
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
    strncpy(m->ltsymbol, m->lts[m->curtag]->symbol, sizeof m->ltsymbol);

    if (smartborders)
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
    unsigned int i, x, click;
    Arg arg = {0};
    Monitor *m;
    XButtonPressedEvent *ev = &e->xbutton;

    click = ClkRootWin;

    /* focus monitor ifnecessary */

    m = wintomon(ev->window);
    if (m != NULL && m != selmon) {
        unfocus(selmon->sel, true);
        selmon = m;
        focus(NULL);
    }

    Client *c = wintoclient(ev->window);

    if (ev->window == selmon->barwin) {
        i = x = 0;

        do
            x += TEXTW(tags[i]);
        while (ev->x >= x && ++i < LENGTH(tags));

        if (i < LENGTH(tags)) {
            click = ClkTagBar;
            arg.ui = 1 << i;
        }
        else if (ev->x < x + blw)
            click = ClkLtSymbol;
        else if (ev->x > selmon->ww - TEXTW(stext))
            click = ClkStatusText;
        else
            click = ClkWinTitle;
    }
    else if (c != NULL) {
        focus(c);
        click = ClkClientWin;
    }

    for (i = 0; i < LENGTH(buttons); i++)
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
        && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
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
    if (dc.font.set)
        XFreeFontSet(dpy, dc.font.set);
    else
        XFreeFont(dpy, dc.font.xfont);
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    XFreePixmap(dpy, dc.drawable);
    XFreeGC(dpy, dc.gc);
    XFreeCursor(dpy, cursor[CurNormal]);
    XFreeCursor(dpy, cursor[CurResize]);
    XFreeCursor(dpy, cursor[CurMove]);
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
        for(m = mons; m && m->next != mon; m = m->next);
        m->next = mon->next;
    }
    XUnmapWindow(dpy, mon->barwin);
    XDestroyWindow(dpy, mon->barwin);
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
            if (dc.drawable != 0)
                XFreePixmap(dpy, dc.drawable);
            dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
            updatebars();
            for (m = mons; m; m = m->next)
                XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bh);
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

    m->prevtag = m->curtag = 0;
    m->tagset[0] = m->tagset[1] = 1;
    m->showbar = showbar;
    m->topbar = topbar;

    int i;

    for (i = 0; i < 4; i++) {
        if (m->showbar && ((i == POS_TOP && m->topbar) || (i == POS_BOTTOM && m->topbar == false)))
            m->struts[i] = bh;
        else
            m->struts[i] = 0;
    }

    updatestruts(m);

    for (i = 0; i < LENGTH(tags); i++) {
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

void drawbar (Monitor *m) {
    int x;
    unsigned int i, occ = 0, urg = 0;
    unsigned long *col;
    Client *c;

    for (c = m->clients; c; c = c->next) {
        occ |= c->tags;
        if (c->isurgent)
            urg |= c->tags;
    }

    dc.x = 0;

    for (i = 0; i < LENGTH(tags); i++) {
        dc.w = TEXTW(tags[i]);
        col = m->tagset[m->seltags] & 1 << i ? dc.sel : dc.norm;
        drawtext(tags[i], col, urg & 1 << i);
        drawsquare(m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
                   occ & 1 << i, urg & 1 << i, col);
        dc.x += dc.w;
    }

    dc.w = blw = TEXTW(m->ltsymbol);
    drawtext(m->ltsymbol, dc.norm, false);
    dc.x += dc.w;
    x = dc.x;

    if (m == selmon) { /* status is only drawn on selected monitor */
        dc.w = TEXTW(stext);
        dc.x = m->ww - dc.w;
        if (dc.x < x) {
            dc.x = x;
            dc.w = m->ww - x;
        }
        drawtext(stext, dc.norm, false);
    } else
        dc.x = m->ww;

    if ((dc.w = dc.x - x) > bh) {
        dc.x = x;
        if (m->sel) {
            col = m == selmon ? dc.sel : dc.norm;
            drawtext(m->sel->name, col, false);
            drawsquare(m->sel->isfixed, m->sel->isfloating, false, col);
        } else
            drawtext(NULL, dc.norm, false);
    }

    XCopyArea(dpy, dc.drawable, m->barwin, dc.gc, 0, 0, m->ww, bh, 0, 0);
    XSync(dpy, false);
}

void drawbars (void) {
    Monitor *m;

    for (m = mons; m; m = m->next)
        drawbar(m);
}

void drawsquare (bool filled, bool empty, bool invert, unsigned long col[ColLast]) {
    XSetForeground(dpy, dc.gc, col[invert ? ColBG : ColFG]);
    int x = (dc.font.ascent + dc.font.descent + 2) / 4;

    if (filled)
        XFillRectangle(dpy, dc.drawable, dc.gc, dc.x+1, dc.y+1, x+1, x+1);
    else if (empty)
        XDrawRectangle(dpy, dc.drawable, dc.gc, dc.x+1, dc.y+1, x, x);
}

void drawtext (const char *text, unsigned long col[ColLast], bool invert) {
    char buf[256];
    int i, x, y, h, len, olen;

    XSetForeground(dpy, dc.gc, col[invert ? ColFG : ColBG]);
    XFillRectangle(dpy, dc.drawable, dc.gc, dc.x, dc.y, dc.w, dc.h);

    if (text == NULL)
        return;

    olen = strlen(text);
    h = dc.font.ascent + dc.font.descent;
    y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
    x = dc.x + (h / 2);

    /* shorten text if necessary */
    for (len = MIN(olen, sizeof buf); len && textnw(text, len) > dc.w - h; len--);
    if (len == 0)
        return;

    memcpy(buf, text, len);

    if (len < olen)
        for (i = len; i && i > len - 3; buf[--i] = '.');

    XSetForeground(dpy, dc.gc, col[invert ? ColBG : ColFG]);

    if (dc.font.set)
        XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, x, y, buf, len);
    else
        XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
}

void enternotify (XEvent *e) {
    Client *c;
    Monitor *m;
    XCrossingEvent *ev = &e->xcrossing;

    if (ffm == false)
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

void expose (XEvent *e) {
    XExposeEvent *ev = &e->xexpose;
    Monitor *m = wintomon(ev->window);

    if (ev->count == 0 && m != NULL)
        drawbar(m);
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
        XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
        setfocus(c);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }

    selmon->sel = c;
    drawbars();
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

void initfont (const char *fontstr) {
    char *def, **missing;
    int n;

    dc.font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
    if (missing) {
        while(n--)
            fprintf(stderr, "properwm: missing fontset: %s\n", missing[n]);
        XFreeStringList(missing);
    }
    if (dc.font.set) {
        XFontStruct **xfonts;
        char **font_names;

        dc.font.ascent = dc.font.descent = 0;
        XExtentsOfFontSet(dc.font.set);
        n = XFontsOfFontSet(dc.font.set, &xfonts, &font_names);
        while(n--) {
            dc.font.ascent = MAX(dc.font.ascent, (*xfonts)->ascent);
            dc.font.descent = MAX(dc.font.descent,(*xfonts)->descent);
            xfonts++;
        }
    }
    else {
        if (!(dc.font.xfont = XLoadQueryFont(dpy, fontstr))
        && !(dc.font.xfont = XLoadQueryFont(dpy, "fixed")))
            die("error, cannot load font: '%s'\n", fontstr);
        dc.font.ascent = dc.font.xfont->ascent;
        dc.font.descent = dc.font.xfont->descent;
    }
    dc.font.height = dc.font.ascent + dc.font.descent;
}

#ifdef XINERAMA
static bool isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
    while(n--) {
        if (unique[n].x_org == info->x_org
        && unique[n].y_org == info->y_org
        && unique[n].width == info->width
        && unique[n].height == info->height)
            return false;
    }
    return true;
}
#endif/* XINERAMA */

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

    if (c->isfullscreen || (smartborders && c->mon->lts[c->mon->curtag]->arrange != NULL && c->isfloating == false
    && (c->mon->lts[c->mon->curtag]->arrange == &monocle || ntiled(c->mon) == 0)))
        c->bw = 0;
    else
        c->bw = borderpx;

    wc.border_width = c->bw;
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);

    XSetWindowBorder(dpy, w, dc.norm[ColBorder]);
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
    if (n > 0) /* override layout symbol */
        snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
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
            drawbars();
            break;
        }
        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
            updatetitle(c);
            if (c == c->mon->sel)
                drawbar(c->mon);
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

    drawbar(m);
    if (!m->sel)
        return;
    if (m->sel->isfloating || !m->lts[m->curtag]->arrange)
        XRaiseWindow(dpy, m->sel->win);
    if (m->lts[m->curtag]->arrange) {
        wc.stack_mode = Below;
        wc.sibling = m->barwin;
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
    XEvent ev;

    XSync(dpy, false);

    pthread_t sthr;
    pthread_create(&sthr, NULL, init_remote, NULL);

    while (running && XNextEvent(dpy, &ev) == 0) {
        XLockDisplay(dpy);
        if (handler[ev.type])
            handler[ev.type](&ev);
        XUnlockDisplay(dpy);
    }
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
    c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
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
        if (smartborders)
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

    strncpy(selmon->ltsymbol, selmon->lts[selmon->curtag]->symbol, sizeof selmon->ltsymbol);

    if (selmon->sel)
        arrange(selmon);
    else
        drawbar(selmon);
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

bool setstrut(Monitor *m, int pos, int px, int mode) {
    if (pos >= 4)
        return false;

    if (mode < -1 || mode > 1)
        return false;

    bool iter;
    int newpx;

    iter = (m == NULL);
    if (m == NULL)
        m = mons;

    if (mode == -1)
        newpx = m->struts[pos] - px;
    else if (mode == 0)
        newpx  = px;
    else if (mode == 1)
        newpx = m->struts[pos] + px;

    if (newpx < 0 || (pos < 2 && newpx > m->mh/2) || (pos < 4 && newpx > m->mw/2))
        return false;

    if (iter) {
        for (m = mons; m != NULL; m = m->next) {
            if (mode == -1)
                m->struts[pos] -= px;
            else if (mode == 0)
                m->struts[pos] = px;
            else if (mode == 1);
                m->struts[pos] += px;
        }
    } else {
        if (mode == 0)
            m->struts[pos] = px;
        else if (mode == -1)
            m->struts[pos] -= px;
        else if (mode == 1)
            m->struts[pos] += px;
    }

    updatestruts(m);
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
    initfont(font);

    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    bh = dc.h = dc.font.height + 2;

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

    /* init appearance */
    dc.norm[ColBorder] = getcolor(normbordercolor);
    dc.norm[ColBG] = getcolor(normbgcolor);
    dc.norm[ColFG] = getcolor(normfgcolor);

    dc.sel[ColBorder] = getcolor(selbordercolor);
    dc.sel[ColBG] = getcolor(selbgcolor);
    dc.sel[ColFG] = getcolor(selfgcolor);

    dc.drawable = XCreatePixmap(dpy, root, DisplayWidth(dpy, screen), bh, DefaultDepth(dpy, screen));
    dc.gc = XCreateGC(dpy, root, 0, NULL);

    XSetLineAttributes(dpy, dc.gc, 1, LineSolid, CapButt, JoinMiter);

    if (!dc.font.set)
        XSetFont(dpy, dc.gc, dc.font.xfont->fid);

    /* init bars */
    updatebars();
    updatestatus();

    /* EWMH support per view */
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
            PropModeReplace, (unsigned char *) netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);

    /* select for events */
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

    bw = (smartborders && n == 1 ? 0 : borderpx);

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
        arrange(selmon);
    }
}

void tagmon (const Arg *arg) {
    if (selmon->sel == NULL || mons->next == NULL)
        return;
    sendmon(selmon->sel, dirtomon(arg->i));
}

int textnw (const char *text, unsigned int len) {
    XRectangle r;

    if (dc.font.set) {
        XmbTextExtents(dc.font.set, text, len, NULL, &r);
        return r.width;
    }
    return XTextWidth(dc.font.xfont, text, len);
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

    bw = (smartborders && n == 1 ? 0 : borderpx);

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
    selmon->showbar = selmon->showbar == false;

    selmon->struts[(selmon->topbar ? POS_TOP : POS_BOTTOM)] = (selmon->showbar ? bh : 0);
    updatestruts(selmon);

    if (selmon->showbar)
        XMapRaised(dpy, selmon->barwin);
    else
        XUnmapWindow(dpy, selmon->barwin);

    arrange(selmon);
}

void togglebarpos (const Arg *arg) {
    if (selmon->showbar == false)
        return;

    selmon->struts[(selmon->topbar ? POS_TOP : POS_BOTTOM)] -= bh;

    selmon->topbar = selmon->topbar == false;

    selmon->struts[(selmon->topbar ? POS_TOP : POS_BOTTOM)] += bh;
    updatestruts(selmon);

    if (selmon->topbar)
        selmon->by = selmon->my;
    else
        selmon->by = selmon->my + selmon->mh - bh;

    XMoveResizeWindow(dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, bh);
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

        selmon->sel->bw = borderpx;

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
    unsigned int newtags, i;

    if (!selmon->sel)
        return;

    newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);

    if (newtags) {
        selmon->sel->tags = newtags;

        if (newtags == ~0) {
            selmon->prevtag = selmon->curtag;
            selmon->curtag = 0;
        }

        if (!(newtags & 1 << (selmon->curtag - 1))) {
            selmon->prevtag = selmon->curtag;
            for (i = 0; (newtags & 1 << i) == 0; i++);
            selmon->curtag = i + 1;
        }

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
    XSetWindowBorder(dpy, c->win, dc.norm[ColBorder]);

    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void unmanage (Client *c, bool destroyed) {
    Monitor *m = c->mon;
    XWindowChanges wc;

    /* The server grab construct avoids race conditions. */
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
    Client *c;
    XUnmapEvent *ev = &e->xunmap;

    if ((c = wintoclient(ev->window))) {
        if (ev->send_event)
            setclientstate(c, WithdrawnState);
        else
            unmanage(c, false);
    }
}

void updatebars (void) {
    Monitor *m;
    XSetWindowAttributes wa = {
        .override_redirect = true,
        .background_pixmap = ParentRelative,
        .event_mask = ButtonPressMask|ExposureMask
    };
    for (m = mons; m; m = m->next) {
        if (m->barwin)
            continue;

        m->barwin = XCreateWindow(dpy, root, m->wx, m->by, m->ww, bh, 0, DefaultDepth(dpy, screen),
                                  CopyFromParent, DefaultVisual(dpy, screen),
                                  CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);


        XDefineCursor(dpy, m->barwin, cursor[CurNormal]);
        XMapRaised(dpy, m->barwin);
    }
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

                c->bw = borderpx;

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
        bdr = borderpx;

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
        c->mina = (float)size.min_aspect.y / size.min_aspect.x;
        c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
    }
    else
        c->maxa = c->mina = 0.0;
    c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
                 && c->maxw == c->minw && c->maxh == c->minh);
}

void updatestruts (Monitor *m) {
    int i;
    int px;

    m->wy = m->my;
    m->wx = m->mx;
    m->wh = m->mh;
    m->ww = m->mw;

    for (i = 0; i < 4; i++) {
        px = m->struts[i];

        if (i == POS_TOP) {
            m->wy += px;
            m->wh -= px;
        }
        else if (i == POS_BOTTOM)
            m->wh -= px;
        else {
            if (i == POS_LEFT) {
                m->wx += px;
                m->ww -= px;
                XMoveWindow(dpy, m->barwin, m->wx, m->by);
            }
            else if (i == POS_RIGHT)
                m->ww -= px;

            XResizeWindow(dpy, m->barwin, m->ww, bh);
        }
    }
}

void updatetitle (Client *c) {
    if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
        gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
    if (c->name[0] == '\0') /* hack to mark broken clients */
        strcpy(c->name, broken);
}

void updatestatus (void) {
    if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)))
        strcpy(stext, "ProperWM-"VERSION);
    drawbar(selmon);
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
        } else
            c->isurgent = (wmh->flags & XUrgencyHint) ? true : false;
        if (wmh->flags & InputHint)
            c->neverfocus = !wmh->input;
        else
            c->neverfocus = false;
        XFree(wmh);
    }
}

void view (const Arg *arg) {
    if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
        return;

    selmon->seltags ^= 1; /* toggle sel tagset */

    if (arg->ui & TAGMASK) {
        selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
        selmon->curtag = arg->ui;
        if (arg->ui == ~0)
            selmon->curtag = 0;
        else {
            int i;
            for (i = 0; (arg->ui & 1 << i) == 0; i++);
            selmon->curtag = i;
        }
    } else {
        int oldcur = selmon->curtag;
        selmon->curtag = selmon->prevtag;
        selmon->prevtag = oldcur;
    }

    focus(NULL);
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

    for (m = mons; m; m = m->next)
        if (w == m->barwin)
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

void* init_remote (void* arg) {
    XInitThreads();

    st_node* stn = st_node_new(1100);
    stn->process_type = ST_QUEUED;

    st_map_set(stn->map, "GET", "/ClientName", ClientName);
    st_map_set(stn->map, "GET", "/CurrentLayout", CurrentLayout);
    st_map_set(stn->map, "GET", "CurrentMonitor", CurrentMonitor);
    st_map_set(stn->map, "GET", "/CurrentTag", CurrentTag);
    st_map_set(stn->map, "POST", "/SetStrut", SetStrut);
    st_map_set(stn->map, "POST", "/ToggleBar", ToggleBar);
    st_map_set(stn->map, "POST", "/ToggleBarPosition", ToggleBarPosition);
    st_map_set(stn->map, "POST", "/ToggleView", ToggleView);
    st_map_set(stn->map, "POST", "/View", View);

    air_node_start(&stn->base);

    return NULL;
}

ST_TARGET(ClientName) {
    if (selmon->sel == NULL)
        st_pkt_append(rsp, "</3");
    else
        st_pkt_append(rsp, selmon->sel->name);

    st_pkt_send(rsp, conn);
}

ST_TARGET(CurrentLayout) {
    st_pkt_append(rsp, (char*) selmon->lts[selmon->curtag]->symbol);
    st_pkt_send(rsp, conn);
}

ST_TARGET(CurrentMonitor) {
    char* mstr = malloc(16);
    sprintf(mstr, "%d", selmon->num);
    st_pkt_append(rsp, mstr);
    st_pkt_send(rsp, conn);
    free(mstr);
}

ST_TARGET(CurrentTag) {
    char* s = malloc(16);
    sprintf(s, "%d", selmon->curtag);

    st_pkt_append(rsp, s);
    st_pkt_send(rsp, conn);

    free(s);
}

ST_TARGET(SetStrut) {
    char* mstr;
    char* posstr;
    char* pxstr;

    int mode;
    int mon;
    int pos;
    int px;

    int mi;
    Monitor* m;
    bool succ;

    mstr = st_pkt_get_kwarg(req, "mon");
    posstr = st_pkt_get_kwarg(req, "pos");
    pxstr = st_pkt_get_kwarg(req, "px");

    if (mstr != NULL && posstr != NULL && pxstr != NULL) {
        if (pxstr[0] == '-') {
            mode = -1;
            pxstr++;
        } else if (pxstr[0] == '+') {
            mode = 1;
            pxstr++;
        } else
            mode = 0;

        mon = strtol(mstr, NULL, 10);
        pos = strtol(posstr, NULL, 10);
        px = strtol(pxstr, NULL, 10);

        mi = 0;
        m = mons;

        while (mi < mon) {
            if (m->next != NULL) {
                m = m->next;
                mi++;
            }
        }

        if (mi == mon && pos < 4) {
            XLockDisplay(dpy);
            succ = setstrut(m, pos, px, mode);
            XUnlockDisplay(dpy);

            if (succ)
                st_pkt_append(rsp, "<3");
            else
                st_pkt_append(rsp, "</3");
        } else
            st_pkt_append(rsp, "</3>");
    } else
        st_pkt_append(rsp, "</3>");

    st_pkt_send(rsp, conn);
}

ST_TARGET(ToggleBar) {
    XLockDisplay(dpy);
    togglebar(NULL);
    XUnlockDisplay(dpy);

    st_pkt_append(rsp, "<3");
    st_pkt_send(rsp, conn);
}

ST_TARGET(ToggleBarPosition) {
    XLockDisplay(dpy);
    togglebarpos(NULL);
    XUnlockDisplay(dpy);

    st_pkt_append(rsp, "<3");
    st_pkt_send(rsp, conn);
}

ST_TARGET(ToggleView) {
    char* tstr;
    int t;

    tstr = st_pkt_get_kwarg(req, "tag");

    if (tstr != NULL) {
        t = strtol(tstr, NULL, 10);

        if (t < LENGTH(tags)) {
            Arg arg = { .ui = 1 << t };

            XLockDisplay(dpy);
            toggleview(&arg);
            XUnlockDisplay(dpy);

            st_pkt_append(rsp, "<3");
        }
        else
            st_pkt_append(rsp, "</3");
    }
    else
        st_pkt_append(rsp, "</3");

    st_pkt_send(rsp, conn);
}

ST_TARGET(View) {
    char* tstr;
    int t;

    tstr = st_pkt_get_kwarg(req, "tag");

    if (tstr != NULL) {
        t = strtol(tstr, NULL, 10);

        if (t < LENGTH(tags)) {
            Arg arg = { .ui = 1 << t };

            XLockDisplay(dpy);
            view(&arg);
            XUnlockDisplay(dpy);

            st_pkt_append(rsp, "<3");
        }
        else
            st_pkt_append(rsp, "</3");
    }
    else
        st_pkt_append(rsp, "</3");

    st_pkt_send(rsp, conn);
}

int main (int argc, char *argv[]) {
    XInitThreads();
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("ProperWM "VERSION", 2012 Andrew Felske, 2006-2012 properwm authors -- see LICENSE for details\n");
    else if (argc != 1)
        die("usage: properwm [-v]\n");
    if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
        fputs("warning: no locale support\n", stderr);
    if (!(dpy = XOpenDisplay(NULL)))
        die("properwm: cannot open display\n");
    checkotherwm();
    setup();
    scan();
    run();
    cleanup();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
