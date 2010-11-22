/* ************************************************************************* *
   bugsx - (C) Copyright 1990-1997 Joshua R. Smith (jrs@media.mit.edu)
	   http://physics.www.media.mit.edu/~jrs

	   (C) Copyright 1995-1997  Robert Gasch (Robert_Gasch@peoplesoft.com)
	   http://www.peoplesoft.com/peoplepages/g/robert_gasch/index.htm

   Permission to use, copy, modify and distribute this software for any 
   purpose and without fee is hereby granted, provided that this copyright
   notice appear in all copies as well as supporting documentation. All
   work developed as a consequence of the use of this program should duly
   acknowledge such use.

   No representations are made about the suitability of this software for
   any purpose. This software is provided "as is" without express or implied 
   warranty.

   See the GNU General Public Licence for more information.
 * ************************************************************************* */



#include "bugsx.h"


extern Display			*mydisplay;
extern char*			myname;
extern XrmDatabase		resourceDB;
extern WinType			main_win,
				menu[],
				draw_win[];
extern XGCValues		gray_gc_val;
extern XEvent			myevent;
extern unsigned long		fg, bg,
				warn1col,
				warn2col,
				warn3col;
extern XWindowAttributes  	attribs;
extern XSizeHints		mainwin_hint,
				szhint;
extern int			G_show_genes,
				verbose,
				segments,
				menu_border,
				do_big,
				Draw_Wins,
				Draw_Rows,
				Draw_Columns;



/* *** variables accessed only in util.c - make them static to make sure *** */
static char 	*Menu_Text[MENU_ITEMS]={
			" Initialize ",
			"   Breed    ",
			"    Quit    "};



/* ************************************************************************ */
/* ************************ set the menu's position *********************** */
/* ************************************************************************ */
void fix_menu_pos (width)
int width;
{
	int i;

	for (i=0; i<MENU_ITEMS; i++) 
		menu[i].x = ((width - SM_MENU_WIDTH(menu[i])) / 2);
}




/* *********************************************************************** */
/* *********************** redraw the main window ************************ */
/* *********************************************************************** */
void redraw_main_win ()
{
	/* REDRAY CHILD WINDOWS */
} 



/* ***************************************************************** */
/* msleep was taken from Tom Boutell's Broken Throne with permission */
/* ***************************************************************** */
void msleep (n)
long            n;
{
	struct timeval  sleept;

	sleept.tv_sec = 0;
	sleept.tv_usec = n;
	select(FD_SETSIZE, NULL, NULL, NULL, &sleept);
}



/* *********************************************************************** */
/* *********************** create a single window ************************ */
/* *********************************************************************** */
void create_window (parent_win, this_win, foreg, backg)
Window	parent_win; 
WinTypePtr this_win;
unsigned long foreg, backg;
{
	/* *** Create Actual Window *** */
	this_win->win = XCreateSimpleWindow (mydisplay, parent_win, this_win->x,
		this_win->y, this_win->width, this_win->height, 
		this_win->line_thick, foreg, backg);

	if (!this_win->win) 
		{
		fprintf (stderr,
			"Error creating Simple Window %s ... Exiting.\n",
			this_win->text);
		exit (1);
		}

#ifdef DEBUG
	fprintf (stdout, "Created Simple Window %s (%d) %d %d %d %d\n", 
		this_win->text, this_win->win, this_win->x, this_win->y,
		this_win->width, this_win->height, this_win->line_thick);
#endif

	/* *** set Graphics Content creation and initialize *** */
	this_win->gc = XCreateGC (mydisplay, this_win->win, 0, 0);

	if (! this_win->gc) 
		{
		fprintf (stderr, "Error creating GC for  %s ... Exiting.\n",
			this_win->text);
		exit (1);
		}

#ifdef DEBUG
	fprintf (stdout, "Created GC %d\n", this_win->gc);
#endif

	XSetBackground (mydisplay, this_win->gc, backg);
	XSetForeground (mydisplay, this_win->gc, foreg);

	if (this_win->font_info != 0)
		XSetFont(mydisplay, this_win->gc, this_win->font_info->fid);

	this_win->fg = foreg;
	this_win->bg = backg;

	/* *** specify which input we want this window to process *** */
	XSelectInput (mydisplay, this_win->win, this_win->event_mask);
}




/* *********************************************************************** */
/* highlight menu item (window) in response to a mouse entering or leaving */
/* *********************************************************************** */
int highlight_menu (menu, menu_num,highlight)
WinType menu[];
int	menu_num;
int	highlight;
{
	int i, state=NOGOOD;

	/* *** figure out which menu *** */
	for (i = 0; i < menu_num; i++) 
		if (myevent.xcrossing.window == menu[i].win) 
			{
			state = i;
			i = menu_num;
			}

	/* *** ignore breed menu if we only have 1 org *** */
	if (Draw_Wins == 1 && state == 1)
		return (state);

	/* *** check if menu is valid and change fg & bg *** */
	if (state >= 0 && state < menu_num) 
		{
		if (highlight) 
			{
#ifdef DEBUG
			fprintf(stdout, "Enter state in button %s\n", 
				menu[state].text);
#endif
		XSetBackground (mydisplay, menu[state].gc, fg);
		XSetForeground (mydisplay, menu[state].gc, bg);
			} 
		else 
			{
#ifdef DEBUG
			fprintf (stdout, "Exit state in button %s\n", 
				menu[state].text);
#endif
			XSetBackground (mydisplay, menu[state].gc, bg);
			XSetForeground (mydisplay, menu[state].gc, fg);
			}

		for (i = 0; i < menu_num; ++i)
			XDrawImageString (mydisplay, menu[i].win, menu[i].gc, 1,
			LETTER_ASCENT(menu[i]), menu[i].text, 
			strlen(menu[i].text)); 
		return (state);
	} 
	else
		return (NOGOOD);
}

/* *********************************************************************** */
/* ************** redraw the string in an exposed window ***************** */
/* *********************************************************************** */
int expose_menu (menu, menu_num)
WinType	menu[];
int	menu_num;
{
	int i;

	for (i = 0; i < menu_num; i++) 
		{
		if (myevent.xexpose.window == menu[i].win) 
			{
			XDrawImageString (mydisplay, menu[i].win, menu[i].gc, 1,
				LETTER_ASCENT(menu[i]), menu[i].text,
				strlen(menu[i].text));
			return (FALSE);
			}
		}

	return (TRUE);
}



/* *********************************************************************** */
/* ***************** redraw the org in an exposed window ***************** */
/* *********************************************************************** */
int expose_win (win, win_num)
WinType	win[];
int	win_num;
{
	int i;

	for (i = 0; i < win_num; i++) 
		{
		if (myevent.xexpose.window == win[i].win) 
			{
			grow (i);
			if (G_show_genes)
				display_genes(i);
			return (FALSE);
			}
		}

	return (TRUE);
}




/* *********************************************************************** */
/* *********************** destroy an entire menu  *********************** */
/* *********************************************************************** */
void destroy_menu (menu, menu_num)
WinType	menu[];
int	menu_num;
{
	int i;

	for (i=0; i<menu_num; i++)
		{
		XDestroyWindow (mydisplay, menu[i].win);
#ifdef DEBUG
		fprintf (stdout, "Destroyed menu: %s\n", menu[i].text);
#endif
		XFreeGC (mydisplay, menu[i].gc);
#ifdef DEBUG
		fprintf (stdout, "Destroyed GC: %d\n", menu[i].gc);
#endif
		}
}




/* *********************************************************************** */
/* *************** return the number of the button pressed *************** */
/* *********************************************************************** */
int which_button_pressed (menu, menu_num)
WinType	menu[];
int	menu_num;
{
	int i;

	for (i=0; i<menu_num; i++)
		if (myevent.xbutton.window == menu[i].win)
			return (i);
	return (NOGOOD);
}



/* ***************************************************************** */
/* ********** this basically inits the window definitions ********** */
/* ***************************************************************** */
void init_all_windows ()
{
	int i, x, y, flags, geom_used=FALSE;
	unsigned long small_event_mask, big_event_mask, fs_mask;
	unsigned int width, height;
	unsigned int border_width =
		getIntResource(resourceDB, catlist(myname, ".borderWidth", 
			(char*) NULL), "Xfsm.BorderWidth", DEF_BORDER_WIDTH);
	XFontStruct* def_font = XLoadQueryFont(mydisplay, DEFAULT_FONT);
	XrmString str_type;
	XrmValue value;
	small_event_mask = ButtonPressMask | ExposureMask| KeyPressMask | 
		EnterWindowMask | LeaveWindowMask;
	big_event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask | 
		EnterWindowMask | LeaveWindowMask;
	fs_mask = ButtonPressMask | ExposureMask;

	if (def_font == NULL) 
		{
		fprintf(stderr, "Could not load default font \"%s\".\n", 
			DEFAULT_FONT);
		exit(1);
		}

	strcpy (main_win.text, "bugsx");
	main_win.x = WIN_X;
	main_win.y = WIN_Y;
	main_win.font_info = getFontResource (resourceDB, 
		catlist(myname, ".mainWin.font", (char*) NULL),
		"Xfsm.MainWin.Font", def_font);
	main_win.width = WIN_X;
	main_win.height = (WIN_Y + MENU_SPACE(main_win));
	main_win.line_thick = border_width;
	main_win.event_mask = small_event_mask;
	main_win.flags = PPosition;

	if (XrmGetResource(resourceDB, catlist(myname, ".mainWin.geometry", 
		(char*) NULL), "Xfsm.MainWin.Geometry", &str_type, &value)) 
		{
		flags = XParseGeometry(value.addr, &x, &y, &width, &height);
		if (XValue & flags) 
			{
			if (XNegative & flags)
			   x = DisplayWidth(mydisplay, 
				DefaultScreen(mydisplay)) + x -
				main_win.width - 2 * main_win.line_thick;
			main_win.x = x;
			main_win.flags = USPosition;
			}

		if (YValue & flags) 
			{
			if (YNegative & flags)
			   y = DisplayHeight(mydisplay, 
				DefaultScreen(mydisplay)) + y -
				main_win.height - 2 * main_win.line_thick;
			main_win.y = y;
			main_win.flags = USPosition;
			}
		if (WidthValue & flags)
			{
			if (width < MIN_WIN_X)
				{
				width = MIN_WIN_X;
				if (verbose)
					fprintf (stderr, "Specified width too \
small ... adjusting to minimum size\n");
				}
			else
			if (width > 
			    DisplayWidth(mydisplay, DefaultScreen(mydisplay)))
				{
				width = DisplayWidth(mydisplay, 
					DefaultScreen(mydisplay))-1;
				if (verbose)
					fprintf (stderr, "Specified width too \
large ... adjusting to maximum size\n");
				}
			main_win.width = width;
			main_win.flags = USPosition;
			geom_used=TRUE;
			}
		if (HeightValue & flags)
			{
			if (height < MIN_WIN_Y)
				{
				height = MIN_WIN_Y;
				if (verbose)
					fprintf (stderr, "Specified height too \
small ... adjusting to minimum size\n");
				}
			else
			if (height > 
			    DisplayHeight(mydisplay, DefaultScreen(mydisplay)))
				{
				height = DisplayHeight(mydisplay,
					DefaultScreen(mydisplay))-1;
				if (verbose)
					fprintf (stderr, "Specified height too \
large ... adjusting to maximum size\n");
				}
			main_win.height = height;
			main_win.flags = USPosition;
			geom_used=TRUE;
			}
		}

	for (i = 0; i < MENU_ITEMS; i++) 
		{
		strcpy(menu[i].text, Menu_Text[i]);
		menu[i].font_info = main_win.font_info;
		menu[i].width = SM_MENU_WIDTH(menu[i]);
		menu[i].height = MENU_HEIGHT(menu[i]);
		menu[i].x = ((main_win.width - menu[i].width) / 2); 
		menu[i].y = MENU_Y + MENU_HEIGHT(menu[i]) * i;
		if (menu_border)
			menu[i].line_thick = 1;
		else
			menu[i].line_thick = 0;
		menu[i].event_mask = small_event_mask;
		}

	if (do_big)
		{
		Draw_Rows=1;
		Draw_Columns=1;
		}
	for (i = 0; i < Draw_Wins; i++)
		{
		strcpy(draw_win[i].text, "");
		draw_win[i].font_info = main_win.font_info;
		draw_win[i].width = DRAW_WIDTH;
		draw_win[i].height = DRAW_HEIGHT;
		draw_win[i].x = (i%Draw_Columns)*DRAW_WIDTH+MAIN_BORDER;
		draw_win[i].y = (i/Draw_Rows)*DRAW_HEIGHT+MENU_SPACE(main_win)+
				MAIN_BORDER;
		draw_win[i].line_thick = 1;
		draw_win[i].event_mask = small_event_mask;
		}
}



/* ***************************************************************** */
/* ********** Returns the value of the given bool resource ********** */
/* ***************************************************************** */
Bool getBoolResource(db, str_name, str_class, deflt)
XrmDatabase db;
char* str_name;
char* str_class;
Bool deflt;
{
	XrmString str_type;
	XrmValue value;

	if (XrmGetResource(db, str_name, str_class, &str_type, &value)) 
		{
		return ((!strcmp(value.addr, "true")) || 
			(!strcmp(value.addr, "True")) ||
			(!strcmp(value.addr, "TRUE")) ||
			(!strcmp(value.addr, "yes")) ||
			(!strcmp(value.addr, "Yes")) ||
			(!strcmp(value.addr, "YES")) ||
			(!strcmp(value.addr, "on")) || 
			(!strcmp(value.addr, "On")) || 
			(!strcmp(value.addr, "ON")) || 
			(!strcmp(value.addr, "1")));
		} 
	else
		return (deflt);
}



/* ***************************************************************** */
/* ********** Returns the value of the given int resource ********** */
/* ***************************************************************** */
int getIntResource(db, str_name, str_class, deflt)
XrmDatabase db;
char* str_name;
char* str_class;
int deflt;
{
	XrmString str_type;
	XrmValue value;

	if (XrmGetResource(db, str_name, str_class, &str_type, &value))
		return (atoi(value.addr));
	else
		return (deflt);
}



/* ***************************************************************** */
/* ********** Returns the value of the given Font resource ********* */
/* ***************************************************************** */
XFontStruct* getFontResource(db, str_name, str_class, deflt)
XrmDatabase db;
char* str_name;
char* str_class;
XFontStruct* deflt;
{
	XrmString str_type;
	XrmValue value;

	if (XrmGetResource(db, str_name, str_class, &str_type, &value)) 
		{
		XFontStruct* font = XLoadQueryFont(mydisplay, value.addr);
		if (font == NULL) 
			{
			fprintf(stderr, 
				"Could not load font \"%s\".\n", value.addr);
			return (deflt);
			} 
		else
			return (font);
  		} 
	else
		return (deflt);
}



/* ***************************************************************** */
/* ********** Returns the value of the given color resource ******** */
/* ***************************************************************** */
unsigned long getColorResource(db, str_name, str_class, deflt)
XrmDatabase db;
char* str_name;
char* str_class;
unsigned long deflt;
{
	XrmString str_type;
	XrmValue value;

	if (XrmGetResource(db, str_name, str_class, &str_type, &value)) 
		{
		XColor screen_def;
		if (XParseColor(mydisplay, DefaultColormapOfScreen
			(DefaultScreenOfDisplay(mydisplay)), value.addr, 
			&screen_def) == 0 
		    ||
		    XAllocColor(mydisplay, DefaultColormapOfScreen
			(DefaultScreenOfDisplay(mydisplay)), &screen_def) == 0)
			{
			fprintf(stderr, 
			   "Color specification \"%s\" invalid.\n", value.addr);
			return (deflt);
			} 
		else
 			return (screen_def.pixel);
		} 
	else
		return (deflt);
}



/* ***************************************************************** */
/* ********** Returns the concatenation of the NULL       ********** */
/* ********** terminated list of strings.		  ********** */
/* ***************************************************************** */
#ifdef __STDC__
#if NeedFunctionPrototypes
char* catlist(char *str1, ...)
#else
char* catlist(str1)
char* str1;
#endif /* NeedFunctionPrototypes */
{
	static char blocks[2][64];
	static int current = 0;
	va_list ap;
	char* c;

	strcpy(blocks[current], str1);
	va_start(ap, str1);
	while ((c = va_arg(ap, char*)) != NULL)
		strcat(blocks[current], c);
	va_end(ap);
	c = blocks[current];
	current = (current + 1) % 2;
	return c;
}
#else
char* catlist(va_alist)
va_dcl
{
	static char blocks[2][64];
	static int current = 0;
	va_list ap;
	char* c;

	va_start(ap);
	blocks[current][0] = '\0';
	while ((c = va_arg(ap, char*)) != NULL)
		strcat(blocks[current], c);
	va_end(ap);
	c = blocks[current];
	current = (current + 1) % 2;
	return c;
}
#endif /* __STDC__ */


#if defined (DYNIX) || defined (MACH)
/* *** for some reason DYNIX does not have strstr *** */
/* Copyright (C) 1991, 1992 Free Software Foundation, Inc. */
/* This file is part of the GNU C Library.                 */
/* Return the first ocurrence of NEEDLE in HAYSTACK.       */
#ifdef __STDC__
char *strstr (const char* haystack, const char* needle)
#else
char *strstr (haystack, needle)
char *haystack, *needle;
#endif
{
	char 	*needle_end = strchr(needle, '\0');
	char 	*haystack_end = strchr(haystack, '\0');
	size_t needle_len = needle_end - needle;
	size_t needle_last = needle_len - 1;
	char 	*begin;

	if (needle_len == 0)
 		return (char *) haystack_end;
	if ((size_t) (haystack_end - haystack) < needle_len)
		return NULL;

	for (begin = &haystack[needle_last]; begin < haystack_end; ++begin)
		{
 		char *n = &needle[needle_last];
		char *h = begin;

		do
		if (*h != *n)
			goto loop;		/* continue for loop */
		while (--n >= needle && --h >= haystack);

		return (char *) h;

		loop:;
    		}

	return NULL;
}
#endif /* DYNIX || MACH */
