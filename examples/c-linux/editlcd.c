#include <i2lcd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <panel.h>
#include <form.h>

#include "gcbitmap.h"

struct panelw;
typedef char (*t_EventHandler)(struct panelw *p, int c);
typedef void (*t_ActivateHandler)(struct panelw *p);

struct panelw {
    struct panelw *next; //Used to make linked list of panels
    struct panelw *prev; //To switch between windows and when windows are closed
    t_I2Lcd *lcd;
    WINDOW *win;
    PANEL *panel;
    t_EventHandler handler; //Called when window is active and receives event
    t_ActivateHandler ahandler; //Called when window activates
    char title[40];
    char help[512];
    char activ;
    int x;
    int y;
    int width;
    int height;
    unsigned int row; //window y cursor position
    unsigned int col; //window x cursor position
    char gc[8];     //Graphical character buffer
    char gcn;       //Selected GC number
};



struct editor {
    int brght;  //Brighness
    int contr;  //Contrast
    t_I2Lcd lcd;
    struct panelw *_actw; //Current active window
    struct panelw *potss; //Potentiometer settings
};

struct editor ed;

struct options_values
{
    int address;
    int bus;
    int columns;
    int rows;
    char headername[32];
};

struct options_values args;

void emove(t_I2Lcd *lcd, WINDOW *w, int col, int row);
void showPanel(struct panelw *panel, char *label);
void openEditor(struct options_values *options, struct editor *e);
void closeEditor(struct editor *e);
void editorCursor(struct editor *e);

void resizeEditor(struct editor *e);

char lcdWindowHandler(struct panelw *p, int c);
void lcdActivateHandler(struct panelw *p);

char gcWindowHandler(struct panelw *p, int c);
void gcActivateHandler(struct panelw *p);
void gcPrintGC(struct panelw *p);
void gcPrintHex(struct panelw *p, const char *ct);

char gchWindowHandler(struct panelw *p, int c);

struct panelw *openWindow(int x, int y,
                int width, int height,
                int color, const char *title,
		const char *help,
                t_EventHandler handler, t_ActivateHandler ahandler);
void refreshWindow(struct panelw *p, int x, int y);
void setWindowState(struct panelw *p, char state);
struct panelw *returnNextWindow(struct editor *e);
void moveWindow(struct panelw *p, int x, int y);
void closeWindow(struct panelw *p);
void joinWindows(struct panelw *current, struct panelw *next);
char savebin(struct panelw *p);
char savehex(struct panelw *p);

static struct option options[] =
{
    {"address", optional_argument, 0, 'a'},
    {"bus", optional_argument, 0, 'b'},
    {"columns", required_argument, 0, 'c'},
    {"rows", required_argument, 0, 'r'},
    {"header", optional_argument, 0, 'f'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0},
};

void help(char **argv, char *adstr)
{
    int i;
    uint8_t c, r;

    printf("%s\n%s\nAvailable options:\n"
           "\t--help display this help\n"
           "\t--address=<I2C chip address> default 0x20\n"
           "\t--bus=<bus number> default 0x02\n"
           "\t--columns=<number of columns>\n"
           "\t--rows=<number of rows>\n"
           "\t--header=<file> save character bitmap to given file\n\n"
           "Supported display configurations:\n", argv[0], adstr);
    for(i=0; i < LCD_TYPES_CNT;  i++)
    {
        getSize(lcdTypesArray[i], &c, &r);
        printf("  %dx%d\n", c, r);
    }
    exit(0);
}

int main(int argc, char **argv)
{
    int c, i;

    struct panelw *actw;
    MEVENT event;
    const char *cb;

    args.address = 0x20;
    args.bus = 0x02;
    args.columns = 16;
    args.rows = 2;

    do
    {
        i = 0;
        c = getopt_long(argc, argv, "abc::r::h", options, &i);
        switch(c)
        {
            case 'a':
                if (optarg)
                {
                    args.address = strtoul(optarg, NULL, 0);
                    if (args.address == 0x00)
                        help(argv, "Address of a chip must be a number!");
                }
                break;
            case 'b':
                if (optarg)
                {
                    args.bus = strtoul(optarg, NULL, 0);
                    if (args.bus == 0x00)
                        help(argv, "Bus number must be a number!");
                }
                break;
            case 'c':
                if (optarg)
                {
                    args.columns = strtoul(optarg, NULL, 0);
                    if (args.columns == 0x00)
                        help(argv, "The display is not in the list below:");
                }
                break;
            case 'r':
                if (optarg)
                {
                    args.rows = strtoul(optarg, NULL, 0);
                    if (args.rows == 0x00)
                        help(argv, "The display is not in the list below:");
                }
                break;
            case 'f':
                if (optarg)
                    strncpy(args.headername, optarg, 32);
                else
                    strncpy(args.headername, "./gcbitmap.h", 32);
                break;
            case 'h':
                help(argv, "");
            case -1:
                break;
            case '?':
                help(argv, "Unknown option");
                break;
            default:
                help(argv, "No options given");
                break;

        }
    }while(c != -1);

    initscr();
    refresh();
    keypad(stdscr, TRUE);
    nonl();
    cbreak();
    noecho();
    curs_set(1);

    if (has_colors())
    {
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_GREEN);
	init_pair(2, COLOR_WHITE, COLOR_BLACK);
	init_pair(3, COLOR_WHITE, COLOR_BLUE);
	init_pair(4, COLOR_GREEN, COLOR_BLACK);

	wattron(stdscr, COLOR_PAIR(4));
	wbkgd(stdscr, COLOR_PAIR(4));
    }

    openEditor(&args, &ed);
    actw = ed._actw;
    for(i=0; i < 8; i++)
        lcdSetGC(&ed.lcd, i, gcbitmap[i]);
    setWindowState(actw, 1);
    gcActivateHandler(actw->next);

    while(((c = wgetch(actw->win)) != KEY_F(10)))
    {
	//
	switch(c)
	{
	    case KEY_F(5):
		ed.brght -= ed.brght > 0 ? 1 : 0;
		lcdSetBacklight(&ed.lcd, ed.brght);
                mvwprintw(ed.potss->win, 1, 1, "Brightness:\t0x%02X", ed.brght);
                update_panels();
                doupdate();
		refresh();
	    continue;
	    case KEY_F(6):
		ed.brght += ed.brght < 0x3f ? 1 : 0;
		lcdSetBacklight(&ed.lcd, ed.brght);
		mvwprintw(ed.potss->win, 1, 1, "Brightness:\t0x%02X", ed.brght);
                update_panels();
                doupdate();
		refresh();
	    continue;
	    case KEY_F(4):
		ed.contr -= ed.contr > 0 ? 1 : 0;
		lcdSetContrast(&ed.lcd, ed.contr);
                mvwprintw(ed.potss->win, 2, 1, "Contrast:\t0x%02X", ed.contr);
                update_panels();
                doupdate();
		refresh();
	    continue;
	    case KEY_F(3):
		ed.contr += ed.contr < 0x3f ? 1 : 0;
		lcdSetContrast(&ed.lcd, ed.contr);
		mvwprintw(ed.potss->win, 2, 1, "Contrast:\t0x%02X", ed.contr);
                update_panels();
                doupdate();
		refresh();
	    continue;
            case 9:
                setWindowState(actw, 0);
                actw = returnNextWindow(&ed);
                setWindowState(actw, 1);
                continue;
	    case KEY_MOUSE:
		if(getmouse(&event) == OK)
		{
		    printf("%d %d\n", event.x, event.y);
		}
		continue;
            default:
                if(actw->handler)
                    actw->handler(actw, c);
                continue;
	}

    }

    closeEditor(&ed);

    clear();
    endwin();

    return 0;
}

void showPanel(struct panelw *panel, char *label)
{
    int startx, starty, x, y, h, w, len;

    getbegyx(panel->win, starty, startx);
    getmaxyx(panel->win, h, w);

    box(panel->win, 0, 0);

    snprintf(panel->title, 100, "%s", label);
    len = strlen(panel->title);
    x = ((w - len) / 2);
    y = 0;
    mvwprintw(panel->win, y, x, "%s", panel->title);
    show_panel(panel->panel);
    update_panels();
    refresh();
}

/**
 * Open main editor interface windows
 */
void openEditor(struct options_values *options, struct editor *e)
{
    char title[100];
    int i;
    struct panelw *p0, *p1, *p2;
    FIELD *f[2];

    openI2LCD2(&e->lcd, options->bus, options->address, options->columns, options->rows);
    lcdPower(&e->lcd, POWERON);
    lcdSetBacklight(&e->lcd, 0x3f);
    lcdSetContrast(&e->lcd, 0x17);
    lcdBlink(&e->lcd, 1);
    lcdCursor(&e->lcd, 1);
    lcdClear(&e->lcd);

    lcdPrintf(&e->lcd, "LCDEd v1.0\x01");

    e->brght = 0x3f;
    e->contr = 0x17;

    snprintf(title, 100, "LCD %dx%d:", e->lcd.cols, e->lcd.rows);

    p0 = openWindow(1, 1,
               e->lcd.cols, e->lcd.rows, COLOR_PAIR(1),
               title, "LCD Content editor\n"
                                 "\tCursor keys to move cursor around\n"
                                 "\t'PgUp'\tmoves cursor to the top row\n"
                                 "\t'PgDown'\tmoves cursor to the bottom row\n"
                                 "\t'Home'\tmoves cursor to the first column in row\n"
                                 "\t'End'\tmoves cursor to the last column in row\n\n"
                                 "\t'F1'\tto clear LCD screen\n"
                                 "\t'F3'/'F4'\tto decrease/increase contrast\n"
                                 "\t'F5'/'F6'\tto decrease/increase brightness\n",
	       lcdWindowHandler, lcdActivateHandler);
    lcdSetCursor(&e->lcd, p0->col, p0->row);
    p0->lcd = &e->lcd;

    p1 = openWindow((p0->x + (p0->width < 20 ? 20 : p0->width)) + 1, 1,
               5, 8, COLOR_PAIR(1),
               "C-1:", "Graphic character editor\n"
                       "\tCursor keys to move cursor around\n"
                       "\t'Space' to toggle pixel under the cursor\n"
                       "\t'PgUp'\tPrevious character\n"
                       "\t'PgDown'\tNext character\n"
                       "\t'F1'\tto wipe character\n"
                       "\t'F2'\tto reverse colors\n"
                       "\t'S'\tTo save GC to header file\n",
               gcWindowHandler, gcActivateHandler);
    p1->lcd = &e->lcd;
    p1->gcn = 0;

    p2 = openWindow((p1->x + p1->width), 1,
               2, 8, COLOR_PAIR(3),
               "Hex",  "Hex editor for graphic character\n"
                       "\tCursor keys to move cursor around\n"
                       "\tSet value 0-f for value under cursor\n"
                       "\t'PgUp'\tPrevious character\n"
                       "\t'PgDown'\tNext character\n"
                       "\t's'\tTo save GC to header file\n",
               gchWindowHandler, NULL);
    p2->lcd = &e->lcd;

    e->_actw = p0;

    joinWindows(p0, p1);
    joinWindows(p1, p2);
    joinWindows(p2, p0);

    e->potss = openWindow(p0->x, p0->y + p0->height + 1, 20, 3, COLOR_PAIR(3), "Potentiometers:", "", NULL, NULL);

    mvwprintw(e->potss->win, 1, 1, "Brightness:\t0x%02X", e->brght);
    mvwprintw(e->potss->win, 2, 1, "Contrast:\t0x%02X", e->contr);
    refresh();

}

/**
 * Close interface windows
 */
void closeEditor(struct editor *e)
{
    int i;
    struct panelw *p = e->_actw, *t;
    lcdPower(&e->lcd, POWEROFF);
    closeI2LCD(&e->lcd);

    do
    {
        closeWindow(p);
        t = p;
        p = p->next;
        free(t);
    } while((p != e->_actw) || !p);
    closeWindow(e->potss);
}

/**
 * Open window
 */
struct panelw *openWindow(int x, int y, int width, int height, int color, const char *title, const char *help, t_EventHandler handler, t_ActivateHandler ahandler)
{
    struct panelw *p;
    p = malloc(sizeof(struct panelw));
    if (p)
    {
        strncpy(p->title, title, 100);
        snprintf(p->help, 512, "%s\n\tTAB\tswitch to next window\n\tF10\tto quit", help);
        p->col = 0;
        p->row = 0;
        p->x = x;
        p->y = y;
        p->width = width + 2;
        p->height = height + 2;
        p->handler = handler;
        p->ahandler = ahandler;
        p->next = NULL;
        p->prev = NULL;
        p->win = newwin(p->height, p->width, p->y, p->x);
        wattron(p->win, color);
        wbkgd(p->win, color);
        keypad(p->win, TRUE);
        p->panel = new_panel(p->win);
        wattron(p->win, COLOR_PAIR(2));
        box(p->win, 0, 0);
        mvwprintw(p->win, 0, ((p->width - strlen(p->title)) / 2), p->title);
        wattroff(p->win, COLOR_PAIR(2));
        update_panels();
        doupdate();
        wmove(p->win, 1, 1);
        refresh();
    }
    return p;
}

/**
 * Refresh window content
 */
void refreshWindow(struct panelw *p, int x, int y)
{
    int atr = COLOR_PAIR(2);
    atr |= p->activ ? A_BOLD : 0;

    wattron(p->win, atr);
    box(p->win, 0, 0);
    mvwprintw(p->win, 0, ((p->width - strlen(p->title)) / 2), p->title);
    wattroff(p->win, atr);
    wmove(p->win, y + 1, x + 1);
    update_panels();
    doupdate();
    refresh();
}

/**
 * Set window state: active/inactive
 */
void setWindowState(struct panelw *p, char state)
{
    int atr = COLOR_PAIR(2);
    p->activ = state % 2;
    atr |= p->activ ? A_BOLD : 0;

    wattron(p->win, atr);
    box(p->win, 0, 0);
    mvwprintw(p->win, 0, ((p->width - strlen(p->title)) / 2), p->title);
    wattroff(p->win, atr);
    wmove(p->win, p->row + 1, p->col + 1);
    clear();
    mvprintw(14, 1, p->help);
    update_panels();
    doupdate();
    refresh();

    if(p->activ && p->ahandler)
        p->ahandler(p);
}

/**
 * Return next window in double linked list
 */
struct panelw *returnNextWindow(struct editor *ed)
{
    ed->_actw = ed->_actw->next;
    return ed->_actw;
}

/**
 * Move window around terminal
 */
void moveWindow(struct panelw *p, int x, int y)
{
    move_panel(p->panel, y, x);
    refresh();
}

/**
 * Close window and free allocated ram
 */
void closeWindow(struct panelw *p)
{
        wborder(p->win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	delwin(p->win);
        del_panel(p->panel);
        refresh();
}

/**
 * Join panels into double linked list
 */
void joinWindows(struct panelw *current, struct panelw *next)
{
    current->next = next;
    next->prev = current;
    set_panel_userptr(current->panel, next->panel);
}


/**
 * lcd content window activation handler
 */
void lcdActivateHandler(struct panelw *p)
{
    int i;
    const char *row;;

    for(i = 0; i < p->lcd->rows; i++)
    {
	row = lcdReadRow(p->lcd, i);
	mvwprintw(p->win, i + 1, 1, row);
    }
    wmove(p->win, p->row + 1, p->col + 1);
    lcdSetCursor(p->lcd, p->col, p->row);
}

/**
 * Graphic Character window activation handler
 */
void gcActivateHandler(struct panelw *p)
{
    const char *c;

    c = lcdGetGC(p->lcd, p->gcn);
    memcpy(p->gc, c, 8);

    snprintf(p->title, 100, "C: %d", p->gcn);

    gcPrintHex(p->next, p->gc);
    gcPrintGC(p);
    refreshWindow(p, p->col, p->row);
}

/**
 * Move LCD window cursor around and do the same with lcd cursor
 */
void lcdWindowCursor(t_I2Lcd *lcd, struct panelw *p, int col, int row)
{
    wmove(p->win, row + 1, col + 1);
    lcdSetCursor(lcd, col, row);
    p->row = row;
    p->col = col;
}

/**
 * Limit area of cursor movement in graphic character editor
 */
void gcWindowCursor(struct panelw *p, int col, int row)
{
    if (col > 4) col = 0;
    if (col < 0) col = 4;
    if (row > 7) row = 0;
    if (row < 0) row = 7;
    wmove(p->win, row + 1, col + 1);
    p->row = row;
    p->col = col;
}

/**
 * Keys handler of LCD window
 */
char lcdWindowHandler(struct panelw *p, int c)
{
     char s[2] = {0,0}, ret = 1;

	switch(c)
	{
	    case KEY_F(1):
                lcdWindowCursor(p->lcd, p, 0, 0);
		lcdClear(p->lcd);
                wclear(p->win);
                refreshWindow(p, p->col, p->row);
		return ret;
	    case KEY_DOWN:
		lcdWindowCursor(p->lcd, p, p->col, ((p->row + 1) % p->lcd->rows));
		return ret;
	    case KEY_UP:
                lcdWindowCursor(p->lcd, p, p->col, (p->row > 0 ? (p->row - 1) : p->lcd->rows - 1));
		return ret;
	    case KEY_RIGHT:
                lcdWindowCursor(p->lcd, p, ((p->col + 1) % p->lcd->cols), p->row);
		return ret;
	    case KEY_LEFT:
                lcdWindowCursor(p->lcd, p, (p->col > 0 ? (p->col - 1) : p->lcd->cols - 1), p->row);
		return ret;
	    case KEY_BACKSPACE:
		if (p->col == 0)
                {
		    if (p->row == 0)
			p->row = p->lcd->rows - 1;
		    else
			p->row = p->row > 0 ? p->row - 1 : 0;
		    p->col = p->lcd->cols - 1;
                }else
		    p->col = p->col > 0 ? (p->col - 1) : p->lcd->cols - 1;


		wmove(p->win, p->row + 1, p->col + 1);
		waddch(p->win, ' ');
		wmove(p->win, p->row + 1, p->col + 1);
		lcdWindowCursor(p->lcd, p, p->col, p->row);
		s[0] = ' ';
		lcdFastPrint(p->lcd, s);
		lcdSetCursor(p->lcd, p->col, p->row);
		refresh();
		return ret;
	    case KEY_HOME:
		p->col = 0;
		lcdWindowCursor(p->lcd, p, p->col, p->row);
		return ret;
	    case KEY_END:
		p->col = p->lcd->cols - 1;
		lcdWindowCursor(p->lcd, p, p->col, p->row);
		return ret;
	    case KEY_PPAGE:
		p->row = 0;
		lcdWindowCursor(p->lcd, p, p->col, p->row);
		return ret;
	    case KEY_NPAGE:
		p->row = p->lcd->rows - 1;
		lcdWindowCursor(p->lcd, p, p->col, p->row);
		return ret;
	    case KEY_ENTER:
		p->row = (p->row + 1) % p->lcd->rows;
		lcdWindowCursor(p->lcd, p, p->col, p->row);
		return ret;
	}

	ret = 0;
	s[0] = c;
	lcdFastPrint(p->lcd, s);
	waddch(p->win, s[0]);
	p->col++;

	if (p->col == p->lcd->cols)
	{
	    p->col = 0;
	    p->row = (p->row + 1) % p->lcd->rows;
	}
	lcdWindowCursor(p->lcd, p, p->col, p->row);
	return ret;
}

/**
 * Graphical character keys handler
 */
char gcWindowHandler(struct panelw *p, int c)
{
	int i;
	switch(c)
	{
	    case KEY_DOWN:
		gcWindowCursor(p, p->col, p->row + 1);
                break;
	    case KEY_UP:
                gcWindowCursor(p, p->col, p->row - 1);
		break;
	    case KEY_RIGHT:
                gcWindowCursor(p, p->col + 1, p->row);
		break;
	    case KEY_LEFT:
                gcWindowCursor(p, p->col - 1, p->row);
		break;
            case KEY_PPAGE:
                p->gcn = p->gcn == 0 ? p->gcn = 7 : (p->gcn - 1);
                gcActivateHandler(p);
                break;
            case KEY_NPAGE:
                p->gcn = (p->gcn + 1) % 8;
                gcActivateHandler(p);
                break;
            case 'S':
                savebin(p);
                break;
	    case KEY_F(1):
		memset(p->gc, 0, 8);
		gcPrintGC(p);
		lcdSetGC(p->lcd, p->gcn, p->gc);
		return 0;
	    case KEY_F(2):

		for(i=0; i < 8; i++)
		    p->gc[i] = (~p->gc[i]) & 0x1f;
		gcPrintGC(p);
		lcdSetGC(p->lcd, p->gcn, p->gc);
		return 0;
            case ' ':
                p->gc[p->row] ^= (1 << (4 - p->col));
                gcPrintGC(p);
                gcWindowCursor(p, p->col, p->row);
                lcdSetGC(p->lcd, p->gcn, p->gc);
                return 0;
        };
        return 0;
}

/**
 * Graphical character hex editor window keys handler
 */
char gchWindowHandler(struct panelw *p, int c)
{
    int i, byte;
    struct panelw *g = p->prev;
    switch(c)
    {
	case KEY_DOWN:
	    p->row = (p->row + 1) % (p->height - 2);
	break;
	case KEY_UP:
	    p->row = (p->row - 1) % (p->height - 2);
	break;
	case KEY_RIGHT:
	    p->col = (p->col + 1) % 2;
	break;
	case KEY_LEFT:
	    p->col = (p->col - 1) % 2;
	break;
        case '\r':
            p->col = 1;
            p->row = (p->row + 1) % (p->height - 2);
            wmove(p->win, p->row + 1, p->col + 1);
            break;
        break;
	case KEY_F(1):
	    memset(g->gc, 0, 8);
	    gcPrintGC(g);
	    lcdSetGC(p->lcd, g->gcn, g->gc);
	    return 0;
	case KEY_F(2):

	    for(i=0; i < 8; i++)
	        g->gc[i] = (~g->gc[i]) & 0x1f;
		gcPrintGC(g);
		lcdSetGC(p->lcd, g->gcn, g->gc);
	    return 0;
        case 'S':
            savehex(p);
            break;

	default:

	    if (c >='a' && c <= 'f')
		i = (15 - ('f' - c));
	    else if ((p->col && (c >= '0' && c <= '9')) || (!p->col && (c >= '0' && c <= '1')))
		i = (9 - ('9' - c));
	    else
	     break;

	    mvwaddch(p->win, p->row + 1, p->col + 1, c);
	    g->gc[p->row] = p->col ? ((g->gc[p->row] & 0xf0) | i) : (g->gc[p->row] & 0x0f) | (i << 4);
	    gcPrintGC(g);
            lcdSetGC(p->lcd, g->gcn, g->gc);
	break;
    }
    wmove(p->win, p->row + 1, p->col + 1);
    return 0;
}

/**
 * Print content of currently selected character as bitmap,
 * do refresh hex editor window
 */
void gcPrintGC(struct panelw *p)
{
    int i, j;
    wmove(p->win, 0, 1);

    for(i=0; i < 8; i++)
    {
	for(j=4; j>-1; j--)
	    mvwaddch(p->win, i + 1, j + 1, (p->gc[i] & (1 << (4 - j))) ? '*' : '.');
    }

    gcPrintHex(p->next, p->gc);
}

/**
 * Print content of currently selected character as hex values
 */
void gcPrintHex(struct panelw *p, const char *ct)
{
    int i;
    for(i=0; i < 8; i++)
	mvwprintw(p->win, i + 1, 1, "%02X", ct[i]);
    update_panels();
    doupdate();
    refresh();
}

/**
 * Save definition of all characters as c source in binary format
 */
char savebin(struct panelw *p)
{
    int i, j, b;
    FILE *f;
    const char *cb;
    if((f = fopen(args.headername, "w+")) > 0)
    {
        fprintf(f, "char gcbitmap[8][8] = {\n");
        for(i=0; i<8; i++)
        {
            fprintf(f, "\t\t{\n");
            cb = lcdGetGC(p->lcd, i);
            for (j = 0; j < 8; j++)
            {
                fprintf(f, "\t\t 0b");
                for(b = 7; b >=0 ; b--)
                    fprintf(f, "%d", ((cb[j] & 0x1f) & (1 << b)) ? 1 : 0);
                fprintf(f, ", \n", cb[j]);
            }
            fprintf(f, "\t\t},\n");
        }
        fprintf(f, "\t};");
        fclose(f);
    }
}

/**
 * Save definition of all characters as c source in hexadecimal format
 */
char savehex(struct panelw *p)
{
    int i, j, b;
    FILE *f;
    const char *cb;
    if((f = fopen(args.headername, "w+")) > 0)
    {
        fprintf(f, "char gcbitmap[8][8] = {\n");
        for(i=0; i<8; i++)
        {
            fprintf(f, "\t\t{");
            cb = lcdGetGC(p->lcd, i);
            for (j = 0; j < 8; j++)
            {
                fprintf(f, " 0x%02X,", cb[j] & 0x1f);
            }
            fprintf(f, " },\n");
        }
        fprintf(f, "\t};");
        fclose(f);
    }
}
