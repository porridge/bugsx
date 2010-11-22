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




/* *************************************************************************
   This program displays a list of bar graphs for the file systems of the 
   machine you are on. The following switches are supported:

	+rv			reverse video (use to override xrdb entry) 
	+synchronous		syncronous mode (use to override xrdb entry)
	-?			help
	-background <arg>	backgound color 
	-batch			run program in batch mode
	-bg <arg>		same as -background 
	-bordercolor <arg>	border color
	-borderwidth <arg>	border width
	-cycle			# of iterations after which we re-initialize
	-display		display
	-extend_print		show extended reproduction info while running
	-fg <arg>		same as -forground
	-font <arg>		font
	-foreground <arg>	forground color (also file system bar color)
	-geometry <arg>		geometry (will override extreme window sizes to 
					apply reasonable settings)
	-help			help
	-iconic			iconic
	-interval <arg>		interval used per turn
	-mb			show menu border
	-name <arg>		run bugsx under this name
	-nobreed		do not breed when running in batch mode
	-number	<arg>		number of biomorphs to draw (must be a square #)
	-printpop		print the population when breeding
	-rv 			reverse video
	-seed <arg>		use this seed 
	-segments <arg>		use this many segments to draw an organism
	-showbreed		show brreding subpopulation when in batch mode
	-showgenes		show a graphical representation of the genes
	-synchronous		synchronous mode
	-v			verbose
	-xrm			make no entry in resrouce database
	help			help



   Version History:
	10-05-90: 1.00
			First version by Joshua R. Smith, running only 
			under Xview and Suntools.
	21-04-95: 1.01
			First version running under XWindows. 
	24-04-95: 1.02 
			Lots of improvements. Added resource handling,
			-printpop, -extend_print, -number, -batch, 
			-interval, -segments, -nobreed flags.
			Cleaned up some other code.
	26-04-95: 1.03 
			Added -seed, -showbreed flags, lots of parameter 
			sanity checking added.
	26-04-95: 1.04
			Made sure breeding subpop is selected when user 
			clicks on "Initialize" when running in batch mode.
			Fixed bug with handling of -nobreed.
	02-05-95: 1.05
			Added resize support, changed #ifdef __STDC__ to 
			#if __STDC__.
	02-06-95: 1.06
			Changed the #if __STDC__ back to #ifdef __STDC__
			since some compilers rely on the ambigous construct,
			changed HP check to #ifdef __hpux, updated resource 
			handling, updated README file. 
	29-03-96: 1.07
			Changed method for checking if -number is a square 
			number, added some (double) casts for sqrt(),
			got genes to print, added -showgenes, changed 
			address to Robert_Gasch@peoplesoft.com.
	20-08-97: 1.08
			Added code so that you can select organisms to 
			keep prior to re-initializing the gene pool, 
			added -cycle flag. 

  ************************************************************************* */


#include "bugsx.h"


Display			*mydisplay;
char*			myname;		/* Name we hunt resources with */
XrmDatabase		resourceDB;		/* X resources. */
WinType			main_win,		/* main window */
			menu[MENU_ITEMS],	/* menu items */
			draw_win[MAX_DRAW_WINS];/* windows we draw in */
XGCValues		gray_gc_val;		/* GC values for gray fill */
XEvent			myevent;		/* event */
unsigned long		fg, bg;			/* forgrund, background */
unsigned long		warn1col,		/* warn color level 1 */
			warn2col,		/* warn color level 2 */
			warn3col;		/* warn color level 3 */
XSizeHints		mainwin_hint,		/* main Window size hints */
			szhint;			/* detail Window size hints */ 
Pixmap			warn_pix;		/* Warning Pixmap */
Population		G_Population[MAX_POP];	/* Array of Organisms */
Population		G_Kids_Pop[MAX_POP];	/* Next generation */
double			G_fit_thresh,		/* fitness threshold */
			G_pCross,		/* probability of crossover */
			G_pMutation,		/* probability of mutation */
			G_mutation_std,		/* gauss fn STD used to mutate*/
			G_weight[MAX_CHROM_SIZE],/*weight for each term */
			G_sum_weights;		/* Sum of weights -- yscaling */
int			G_size_pop,		/* # organisms in population */
			G_size_breeding_pop,	/* # organisms reproducing */
			G_generation=NOGOOD,	/* # generations so far */
			G_switch_default,	/* switches start on or off */
			G_show_genes,		/* display gene window	*/
			G_current_width,	/* Current size of pixwin */
			G_current_height,
			G_org_height,		/* Size in pixels of organism */
			G_org_width,
			G_x_scale,		/* These are used for scaling */
			G_y_scale,		/*	curves.	*/
			G_x_trans,
			G_y_trans,
			cycle=0,
			verbose=FALSE,
			batch=FALSE, 
			batch_breed=TRUE,
			show_breed=FALSE,
			upd_interval=UPDINTERVAL,
			segments=ORG_SEGMENTS,
			selected[MAX_DRAW_WINS],
			do_big=FALSE,
			do_print_pop=FALSE,
			extend_print=FALSE,
			Draw_Wins=DRAW_WINS,
			Draw_Rows=DRAW_ROWS,
			Draw_Columns=DRAW_COLUMNS,
			menu_border=FALSE;	/* menu border */
long			seed,
			mstime=MSTIME;



static XrmOptionDescRec opTable[] = {
		/* KEEP THIS SORTED BY THE FIRST ARGUMENT! */
/* the char * should be XPointer but many systems don't support it */
{"+rv",		"*reverseVideo", XrmoptionNoArg,	(char *) "off"},
{"+synchronous","*synchronous",	XrmoptionNoArg,		(char *) "off"},
{"-?",		".help",	XrmoptionNoArg,		(char *) "on"},
{"-background",	"*background",	XrmoptionSepArg,	(char *) NULL},
{"-batch",	".batch",	XrmoptionNoArg,		(char *) "on"},
{"-bg",		"*background",	XrmoptionSepArg,	(char *) NULL},
{"-bordercolor","*borderColor",	XrmoptionSepArg,	(char *) NULL},
{"-borderwidth",".borderWidth",	XrmoptionSepArg,	(char *) NULL},
{"-cycle",	".cycle",	XrmoptionSepArg,	(char *) NULL},
{"-display",	".display",     XrmoptionSepArg,	(char *) NULL},
{"-extend_print",".extend_print",XrmoptionNoArg,	(char *) "on"},
{"-fg",		"*foreground",	XrmoptionSepArg,	(char *) NULL},
{"-font",	"*font",	XrmoptionSepArg,	(char *) NULL},
{"-foreground",	"*foreground",	XrmoptionSepArg,	(char *) NULL},
{"-geometry",	"*mainWin.geometry",XrmoptionSepArg,	(char *) NULL},
{"-help",	".help",	XrmoptionNoArg,		(char *) "on"},
{"-interval",	".interval",	XrmoptionSepArg,	(char *) NULL},
{"-iconic",	".iconic",	XrmoptionNoArg,		(char *) "on"},
{"-mb",		".menuborder",	XrmoptionNoArg,		(char *) "on"},
{"-name",	".name",	XrmoptionSepArg,	(char *) NULL},
{"-nobreed", 	".batchbreed",	XrmoptionNoArg,		(char *) "off"},
{"-number",	".number",	XrmoptionSepArg,	(char *) NULL},
{"-printpop",	".printpop",	XrmoptionNoArg, 	(char *) "on"},
{"-rv",		"*reverseVideo",XrmoptionNoArg,		(char *) "on"},
{"-seed",	".seed",     	XrmoptionSepArg,	(char *) NULL},
{"-segments",	".segments",	XrmoptionSepArg,	(char *) NULL},
{"-showbreed",	".showbreed",	XrmoptionNoArg, 	(char *) "on"},
{"-showgenes",	".showgenes",	XrmoptionNoArg, 	(char *) "on"},
{"-synchronous","*synchronous",	XrmoptionNoArg,		(char *) "on"},
{"-v",		".verbose",	XrmoptionNoArg,		(char *) "on"},
{"-xrm",	NULL,		XrmoptionResArg,	(char *) NULL},
{"help",	".help",	XrmoptionNoArg,		(char *) "on"},
};


void main (argc, argv)
int argc;
char **argv;
{
	int 		i=0;
	XrmDatabase 	commandlineDB=NULL;
	char* 		myDisplayName = NULL;
	XrmString 	str_type;
	XrmValue 	value;

	/* *** Try and work out the name the application will run under *** */
	/* *** This needed for finding the correct resources. *** */
	{
	char** ptr = &argv[1];

	for (; *ptr != NULL; ptr++)
	if (strcmp(*ptr, "-name") == 0 && *(ptr + 1) != NULL) 
		{
		myname = *(ptr + 1);
		break;
		}

	if (*ptr == NULL) 
		{
		char* ptr1 = rindex(argv[0], '/');
		if (ptr1)
			myname = ++ptr1;
		else
			myname = argv[0];
		}
	}

	XrmInitialize();
	XrmParseCommand(&commandlineDB, opTable, 
		sizeof(opTable) / sizeof(opTable[0]), myname, &argc, argv);
	if (XrmGetResource(commandlineDB, 
		catlist(myname, ".name", (char*) NULL), "Xfsm.Name", 
		&str_type, &value))
			myname = value.addr;
	if (XrmGetResource(commandlineDB, 
		catlist(myname, ".display", (char*) NULL), "Xfsm.Display", 
		&str_type, &value))
			myDisplayName = value.addr;

	/* *** connect to X Server and assign screen *** */
	if ((mydisplay = XOpenDisplay(myDisplayName)) == NULL) 
		{
		fprintf (stderr, "Error Opening Display ... Exiting\n");
		exit (1);
		}

	/* *** init the simulation constants *** */
	init();

	/* *** process arguments and the resource database *** */
	process_databases (argc, argv, commandlineDB);

	/* *** Turn on X server synchronization if the user wants it. *** */
	XSynchronize(mydisplay, getBoolResource(resourceDB,
		catlist(myname, ".synchronous", (char*) NULL),
		"Xfsm.Synchronous", False));

	init_all_windows();
	srand48 (seed);

	/* *** Check to see of the user wants the colors reversed. *** */
	if (getBoolResource(resourceDB, 
		catlist(myname, ".reverseVideo", (char*) NULL),
		"Xfsm.ReverseVideo", False)) 
		{
		long temp;
		temp = fg; 
		fg = bg ; 
		bg = temp;
		}

	/* *** create Main Program Window and set up the properties. *** */
	{
	XClassHint clshint;
	XWMHints wmhints;

	mainwin_hint.min_width = MIN_WIN_X;
	mainwin_hint.min_height = MIN_WIN_Y;
	mainwin_hint.width = main_win.width;
	mainwin_hint.height = main_win.height;
	mainwin_hint.x = main_win.x;
	mainwin_hint.y = main_win.y; 
	mainwin_hint.flags = main_win.flags | PMinSize | PSize;

	clshint.res_name  = myname;
	clshint.res_class = CLASS_NAME;

	wmhints.input = True; 
	wmhints.initial_state = (getBoolResource(resourceDB,
		catlist(myname, ".iconic", (char*) NULL), 
		"Xfsm.Iconic", False))
      		? IconicState : NormalState;
	wmhints.window_group = main_win.win;
	wmhints.flags = InputHint | StateHint | WindowGroupHint;

	create_window (DefaultRootWindow(mydisplay), &main_win, fg, bg);
	XSetStandardProperties (mydisplay, main_win.win, main_win.text,
		main_win.text, None, argv, argc, &mainwin_hint);
	XSetClassHint(mydisplay, main_win.win, &clshint);
	XSetWMHints(mydisplay, main_win.win, &wmhints);
	}

	if (do_big)
		Draw_Wins=1;
	/* *** create all the buttons using child windows *** */
 	for (i = 0; i < MENU_ITEMS; ++i) 
		create_window (main_win.win, &menu[i], fg, bg); 
	for (i = 0; i < Draw_Wins; i++) 
		create_window (main_win.win, &draw_win[i], fg, bg);

	/* *** Map window definitions onto screen *** */
	XMapRaised (mydisplay, main_win.win);
	XMapSubwindows (mydisplay, main_win.win);

	randomize_pop();

#ifdef DEBUG
	fprintf (stdout, "Setup is Done!\n");
#endif 

/* ************************************************************ */
/* ************** setup is done - main event loop ************* */
/* ************************************************************ */
	
	do_event_loop (argc, argv);

	/* *** we are done - destroy all the windows *** */

	destroy_menu (menu, MENU_ITEMS);
	destroy_menu (draw_win, Draw_Wins); 
	XFreeGC (mydisplay, main_win.gc);
	XDestroyWindow (mydisplay, main_win.win);
	XCloseDisplay (mydisplay);
	exit (0);
}



/* *** This is the main event loop. Here we handle events and deal *** */
/* ***                 with all the user inputs we get             *** */
void do_event_loop (argc, argv)
int argc;
char **argv;
{
	XEvent	tevent;
	int	iter=0, count, cont, this_item, i, done=FALSE; 
	char	text[10];
	long	target = time(NULL);

    while (!done)	
	{
	if (batch)
		{
		/* *** here we wait while there are no events *** */
        	/* *** and count out down our interval.       *** */
        	while (XPending (mydisplay) == 0)
               		{
			struct timeval  sleept;
			fd_set reads;

			FD_ZERO(&reads);
			FD_SET(ConnectionNumber(mydisplay), &reads);
			sleept.tv_sec = target - time(NULL);
			sleept.tv_usec = 0;
#ifdef __hpux
			select(ConnectionNumber(mydisplay) + 1, (int *) 
				&reads, NULL, NULL, &sleept);
#else
			select(ConnectionNumber(mydisplay) + 1, &reads, 
				NULL, NULL, &sleept);
#endif
			/* *** interval has passed *** */
			if ((time(NULL)) >= target)
				{
				grow_pop();
				target = target + upd_interval;
				if (!batch_breed)
					randomize_pop();
				else
					{
					breed();
					select_breeding_sub_pop();
					}

				if (++iter == cycle)
					{
					printf ("HERE\n");
					randomize_pop();
					iter=0;
					}
				}
			}
		}

	/* *** read the next event *** */
	XNextEvent (mydisplay, &myevent);

	switch (myevent.type) 
	{
	/* *** expose event -> redraw the window *** */
	 case Expose:
	/* ********************************************* *
	 *  since a window can generate multiple expose  *
	 *  events we can collapse these into one redraw *
	 * ********************************************* */
		count=0;
		do 
			{
			if (XPending (mydisplay) == 0)
				break;
			XPeekEvent (mydisplay, &tevent);
			if (tevent.xexpose.window ==  myevent.xexpose.window) 
				{
				XNextEvent (mydisplay, &myevent);
				count++;
				}
			} 
		while(tevent.xexpose.window==myevent.xexpose.window);

#ifdef DEBUG
		if (!count)
			fprintf(stdout, "got expose event in window: %d\n",
			myevent.xexpose.window);
		else
			fprintf(stdout, 
			"Compressed %d expose events into 1 in window: %d\n",
			count, myevent.xexpose.window);
#endif 

		/* ******************************************* *
		 * ** now deal with the actual expose event ** *
		 * ******************************************* */
		if (myevent.xexpose.window == main_win.win &&
		    myevent.xexpose.count == 0) 
			{
			redraw_main_win ();
			break;
			} 
		cont = expose_menu (menu, MENU_ITEMS);
		if (!cont)
			break;
		if (G_generation != NOGOOD)
			cont = expose_win (draw_win, Draw_Wins);

		break;

	/* *** change work window if main window changes *** */
	case ConfigureNotify:
#ifdef DEBUG
		fprintf(stdout, "got configure notify event in window: %d\n",
			myevent.xconfigure.window);
#endif
		/* *** we don't allow changes in the detail window *** */
		if (myevent.xconfigure.window != main_win.win)
			break;

		/* *** size stays the same -> nothing to do *** */
		if (myevent.xconfigure.width == main_win.width &&
		    myevent.xconfigure.height == main_win.height)
			break;

		XClearWindow (mydisplay, main_win.win);
		handle_resize (myevent.xconfigure.width, 
			       myevent.xconfigure.height);
		break;

	/* *** process keyboard mapping changes *** */
	case MappingNotify:
#ifdef DEBUG
	fprintf(stdout, "got mapping notify event in window: %d\n",
		myevent.xexpose.window);
#endif

		XRefreshKeyboardMapping ((XMappingEvent *) &myevent );
		break;

      /* *** drag in work window *** */
	case MotionNotify:
#ifdef DEBUG
	fprintf(stdout, "got motion notify event in window: %d\n",
		myevent.xbutton.window);
#endif
	break;

	/* *** mouse enters a window *** */
	case EnterNotify:
		this_item = highlight_menu (menu, MENU_ITEMS, TRUE);
		break;

	/* *** Mouse Leaves a window *** */
	case LeaveNotify:
		this_item = highlight_menu (menu, MENU_ITEMS, FALSE);
		break;

	/* *** process mouse-button presses *** */
	case ButtonPress:
#ifdef DEBUG
	fprintf(stdout, "button press (%d) in window: %d\n",
		myevent.xbutton.button, myevent.xbutton.window);
#endif

	if (myevent.xbutton.button == 1 || myevent.xbutton.button == 2) 
		{
		this_item = which_button_pressed (menu, MENU_ITEMS);
		if (this_item == INITIALIZE) 
			{
			randomize_pop();
			grow_pop();
			if (batch)
				select_breeding_sub_pop();
			break;
			} 
		else if (this_item == BREED)
			{
			if (batch || Draw_Wins == 1)
				break;
			breed ();
			break;
			}
		else if (this_item == QUIT) 
			{
			done = TRUE; 
			break;
			}
/*
		else if (this_item == PLUS)
			{
			while (!XPending (mydisplay))
				{
				msleep (mstime);
				if (mstime > 5000)
					mstime-=5000;
				if (segments < MAX_SEGMENTS)
					segments++;
				sprintf (menu[SHOW].text, "%8d", segments);
				XDrawImageString (mydisplay, 
					menu[SHOW].win,
					menu[SHOW].gc, 1, 
					LETTER_ASCENT (menu[SHOW]),
					menu[SHOW].text, 
					strlen (menu[SHOW].text));
				}
			mstime=MSTIME;
			break;
			}
		else if (this_item == MIN)
			{
			while (!XPending (mydisplay))
				{
				msleep (mstime);
				if (mstime > 5000)
					mstime-=5000;
				if (segments > MIN_SEGMENTS)
					segments--;
				sprintf (menu[SHOW].text, "%8d", segments);
				XDrawImageString (mydisplay, 
					menu[SHOW].win,
					menu[SHOW].gc, 1, 
					LETTER_ASCENT (menu[SHOW]),
					menu[SHOW].text, 
					strlen (menu[SHOW].text));
				}
			mstime=MSTIME;
			break;
			}
		else if (this_item == OK)
			{
			grow_pop();
			}
*/

		this_item = which_button_pressed (draw_win, Draw_Wins);
		if (this_item == NOGOOD || G_generation == NOGOOD)
			break;
		else
			highlight_org_window (this_item);
		}
	break;

	/* *** process mouse-button release *** */
	case ButtonRelease:
#ifdef DEBUG
		fprintf(stdout, "button release in window: %d\n", 
			myevent.xbutton.window);
#endif
	break;

	/* *** process Resize *** */
	case ResizeRequest:
#ifdef DEBUG
		fprintf (stdout, "got resize event \n");
#endif
	break;

	/* *** process keyboard input *** */
	case KeyPress:
#ifdef DEBUG
		fprintf (stdout, "got keypress event in window: %d\n", 
			myevent.xkey.window);
#endif
		i = XLookupString ((XKeyEvent *)&myevent, text, 10, NULL, 0 );

		if (text[0] == INIT_KEY)
			{
			randomize_pop();
			grow_pop();
			if (batch)
				select_breeding_sub_pop();
			break;
			}
		else 
		if (text[0] == BREED_KEY)
			{
			if (batch || Draw_Wins == 1)
				break;
			breed ();
			break;
			}
		else
		if (text[0] == QUIT_KEY) 
			{
			done = TRUE;
			break;
			}
		break;
	} /* switch (myevent.type) */
    } /* while (done == 0) */
}
