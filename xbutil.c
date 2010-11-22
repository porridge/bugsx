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
extern WinType			main_win,
				menu[],
				draw_win[];
extern char*			myname;
extern XrmDatabase		resourceDB;
extern unsigned long		fg, bg;
extern XSizeHints		szhint;
extern int 			G_size_pop,
				G_show_genes,
				verbose,
				do_print_pop,
				extend_print,
				Draw_Wins,
				Draw_Rows,
				Draw_Columns,
				segments,
				batch,
				batch_breed,
				cycle,
				show_breed,
				upd_interval,
				selected[],
				menu_border;
extern long			seed;



/* ***************************************************************** */
/* **************** select the breeding subpopulation ************** */
/* ***************************************************************** */
void select_breeding_sub_pop ()
{
	int	i, count=0;

	if (!batch)
		{
		printf ("Sanity check error! select_breeding_sub_pop() called \
without -batch ... Exiting\n");
		exit (1);
		}
	for (i=0; i<Draw_Wins; i++)
		if (rnd (0,1))
			{
			if (show_breed)
				highlight_org_window(i);
			count++;
			}

	while (count < 2)
		{
		i=rnd (0, Draw_Wins-1);
		if (!selected[i])
			{
			highlight_org_window(i);
			count++;
			}
		}
}



/* ***************************************************************** */
/* ****************** Highlight an organism window ***************** */
/* ***************************************************************** */
void highlight_org_window (i)
int 	i;
{
	if (selected[i])
		{
		XFillRectangle (mydisplay,
			draw_win[i].win, draw_win[i].gc, 
			0,0, draw_win[i].width, 
			draw_win[i].height);
		XSetForeground (mydisplay, draw_win[i].gc, fg);
		selected[i]=FALSE;
		}
	else
		{
		XFillRectangle (mydisplay,
			draw_win[i].win, draw_win[i].gc, 
			0,0, draw_win[i].width, 
			draw_win[i].height);
		XSetForeground (mydisplay, draw_win[i].gc, bg);
		selected[i]=TRUE;
		}
	grow (i);
	if (G_show_genes)
		display_genes(i);
}



/* ***************************************************************** */
/* ****************** Process the program arguments **************** */
/* ***************************************************************** */
void handle_resize (x, y)
int 	x, y;
{
	int 	i;
	main_win.width=x;
	main_win.height=y;
	for (i=0; i<MENU_ITEMS; i++)
		{
		menu[i].x=((main_win.width - menu[i].width) / 2);
		menu[i].y=MENU_Y + MENU_HEIGHT(menu[i]) * i;
		XResizeWindow (mydisplay, menu[i].win, menu[i].width,
			menu[i].height);
		XMoveWindow (mydisplay, menu[i].win, menu[i].x,
			menu[i].y);
		}
	for (i=0; i<Draw_Wins; i++)
		{
		draw_win[i].width = DRAW_WIDTH;
		draw_win[i].height = DRAW_HEIGHT;
		draw_win[i].x = (i%Draw_Columns)*DRAW_WIDTH+MAIN_BORDER;
                draw_win[i].y = (i/Draw_Rows)*DRAW_HEIGHT+MENU_SPACE(main_win)+
                                MAIN_BORDER;
		XResizeWindow (mydisplay, draw_win[i].win, draw_win[i].width,
			draw_win[i].height);
		XMoveWindow (mydisplay, draw_win[i].win, draw_win[i].x,
			draw_win[i].y);
		}
	grow_pop ();
}


/* ***************************************************************** */
/* ****************** Process the program arguments **************** */
/* ***************************************************************** */
void process_databases (argc, argv, commandlineDB)
int argc;
char **argv;
XrmDatabase commandlineDB;
{
	char 	name[255];
	char 	msg[255];
	int	tsi;
	double	tsd;

	strcpy (msg, "");
	/* *** Load the local app-defaults file. *** */
	if (getenv ("XAPPLRESDIR"))
		{
		strcpy(name, getenv ("XAPPLRESDIR"));
		strcat(name, "/");
		strcat(name, CLASS_NAME);
		resourceDB = XrmGetFileDatabase(name);
		if (resourceDB)
			sprintf (msg, 
				"Read Resource definitions in %s ...", name);
		}
		

	/* *** check if we really found something *** */
	/* *** if not, check in APP_DEFAULTS_DIR *** */
	if (!resourceDB) 
		{
		strcpy(name, APP_DEFAULTS_DIR);
		strcat(name, "/");
		strcat(name, CLASS_NAME);
		resourceDB = XrmGetFileDatabase(name);
		if (resourceDB)
			sprintf(msg, "Read Resource definitions in %s ...",
				name);
		}

	/* *** Load in any definitions supplied in the X server. *** */
	if (XResourceManagerString(mydisplay) != NULL)
		XrmMergeDatabases(XrmGetStringDatabase(XResourceManagerString
			(mydisplay)), &resourceDB);

	/* *** Merge in the command line arguments. *** */
	XrmMergeDatabases(commandlineDB, &resourceDB);

	/* *** See if the help flag was set. *** */
	if (getBoolResource(resourceDB, catlist(myname, ".help", (char*)NULL),
		"Bugsx.Help", False))
			do_help();

	/* *** See if the verbose flag was set. *** */
	if ((verbose = getBoolResource(resourceDB,
		catlist(myname, ".verbose", (char*) NULL), 
		"Bugsx.Verbose", False))) 
		{
		PRINT_COPYRIGHT;
		puts ("-v flag caught - verbose mode enabled...");
		if (resourceDB && strlen (msg))
			puts (msg);
		}

	/* *** assign foreground and backgound colors *** */
 	fg = getColorResource(resourceDB, catlist(myname, 
		".foreground", (char*) NULL), "Bugsx.Foreground",
		BlackPixel (mydisplay, DefaultScreen (mydisplay))); 
	bg = getColorResource(resourceDB, catlist(myname, 
		".background", (char*) NULL), "Bugsx.Background",
		WhitePixel (mydisplay, DefaultScreen (mydisplay)));
	if (bg != WhitePixel (mydisplay, DefaultScreen (mydisplay)))
		if (verbose) 
			printf ("-bg flag caught - background set\n");

	if ((menu_border = getBoolResource(resourceDB,
		catlist(myname, ".menuborder", (char*)NULL),
		"Bugsx.Menuborder", False)))
		if (verbose)
			puts("-mb flag caught - will draw menu border.");

	if ((do_print_pop = getBoolResource(resourceDB,
		catlist(myname, ".printpop", (char*)NULL),
		"Bugsx.Printpop", False)))
		if (verbose)
			puts("-printpop flag caught - will print population.");

	if ((extend_print = getBoolResource(resourceDB,
		catlist(myname, ".extend_print", (char*)NULL),
		"Bugsx.ExtendPrint", False)))
		if (verbose)
			puts("-extend_print flag caught - will print \
breeding output");

	if ((batch = getBoolResource(resourceDB,
		catlist(myname, ".batch", (char*)NULL),
		"Bugsx.Batch", False)))
		{
		randomize_pop();
		if (verbose)
			puts("-batch flag caught - will run program in batch \
mode");
		}

	if (!(batch_breed = getBoolResource(resourceDB,
		catlist(myname, ".batchbreed", (char*)NULL),
		"Bugsx.Batchbread", True)))
		{
		if (!batch)
			{
			puts ("-nobreed does not make sense without -batch ...\
 Exiting");
			exit (1);
			}
		if (verbose)
			puts("-nobreed flag caught - will not breed in batch \
mode");
		}

	if ((cycle = getIntResource(resourceDB,
		catlist(myname, ".cycle", (char*)NULL),
		"Bugsx.Cycle", 100)) != 100)
		{
		if (!batch)
			{
			puts ("-cycle does not make sense without -batch ...\
 Exiting");
			exit (1);
			}
		if (verbose)
			printf ("-cycle flag caught - will re-initialize population \
after %d turns ...\n", cycle);
		}


	if ((segments = getIntResource(resourceDB,
		catlist(myname, ".segments", (char*)NULL),
		"Bugsx.Segments", 150)) != ORG_SEGMENTS)
		{
		if (segments < 5)
			{
			printf ("Invalid -segments flag caught (%d) - must be \
between >= 5 Exiting ...\n", segments);
			exit (1);
			}
		if (verbose)
			printf ("-segments flag caught - will use %d \
segments to draw org\n", segments);
		}

	if ((upd_interval = getIntResource(resourceDB,
		catlist(myname, ".interval", (char*)NULL),
		"Bugsx.Interval", UPDINTERVAL)) != UPDINTERVAL)
		{
		if (!batch)
			{
			puts ("-interval does not make sense without -batch ...\
 Exiting");
			exit (1);
			}
		if (upd_interval < 0)
			{
			printf ("Invalid -interval flag caught (%d) - \
must be >= 1 ... Exiting\n", upd_interval);
			exit (1);
			}	
		if (verbose)
		    printf 
		   	 ("-interval flag caught - setting interval to %d\n",
			upd_interval);
		}

	if ((Draw_Wins = getIntResource(resourceDB,
		catlist(myname, ".number", (char*)NULL),
		"Bugsx.Number", 16)) != 16)
		{
		if (Draw_Wins < 1)
			{
			printf ("Invalid -number flag caught (%d) - must be \
>= 1\n", Draw_Wins);
			exit (1);
			}
		if (Draw_Wins == 1 && batch && batch_breed)
			{
			puts ("'-number 1' doesn't make sense if '-batch' is \
specified without -nobreed ... \nExiting"); 
			exit (1);
			}
		tsi=(int)sqrt(Draw_Wins);
		tsd=sqrt(Draw_Wins);
		if ((double)tsi == tsd)
		    {
		    if (Draw_Wins > 100)
			{
			printf ("Invalid -number flag caught (%d) - must be \
smaller than 100\n", Draw_Wins);
			exit (1);
			}
		    G_size_pop=Draw_Wins;
		    Draw_Columns=Draw_Rows=sqrt((double)Draw_Wins);
		    if (verbose)
			printf ("-number flag caught-will draw %d organisms\n",
				Draw_Wins);
		    }
		else
			{
			printf ("Invalid -num flag caught (%d) - must be a \
square number .... Exiting\n", Draw_Wins);
			exit (1);
			}
		}

	if ((show_breed = getBoolResource(resourceDB,
		catlist(myname, ".showbreed", (char*)NULL),
		"Bugsx.Showbreed", False)))
		{
		if (!batch)
			{
			puts ("-showbreed does not make sense without -batch \
... Exiting");
			exit (1);
			}
		if (!batch_breed)
			{
			puts ("-showbreed does not make sense with -nobreed \
... Exiting");
			exit (1);
			}
		if (Draw_Wins == 1)
			{
			puts ("-showbreed does not make sense with '-number 0' \
... Exiting");
			exit (1);
			}
		if (verbose)
			puts("-showbreed flag caught - will show breeding \
selection in batch mode");
		}

	if ((G_show_genes = getBoolResource(resourceDB,
		catlist(myname, ".showgenes", (char*)NULL),
		"Bugsx.Showgenes", False)))
		if (verbose)
		    printf 
		   	 ("-showgenes flag caught - setting showgenes=TRUE\n");

}


void do_help ()
{
	printf ("\n\
                      BBBB   U   U  GGGGG  SSSSS  X   X\n\
                      B   B  U   U  G      S       X X\n\
                      BBBB   U   U  G GGG  SSSSS    X\n\
                      B   B  U   U  G   G      S   X X\n\
                      BBBB    UUU   GGGGG  SSSSS  X   X\n\
\n\
bugsx runs under MIT's X11 window system. It was written under\n\
UNIX but should be easily portable.  It is a program which draws\n\
the biomorphs based on parametric plots of Fourier sine and cosine\n\
series and let's you play with them using the genetic algorithm.\n");

printf ("\n\
        +rv                     reverse video (use to override xrdb entry)\n\
        +synchronous            syncronous mode (use to override xrdb entry)\n\
        -?                      help\n\
        -background <arg>       backgound color\n\
        -batch                  run program in batch mode\n\
        -bg <arg>               same as -background\n\
        -bordercolor <arg>      border color\n\
        -borderwidth <arg>      border width\n\
	-cycle <arg>		re-initialize population after n batch turns\n\
        -display                display\n\
        -extend_print           show extended reproduction info while running\n\
        -fg <arg>               same as -forground\n\
        -font <arg>             font\n\
        -foreground <arg>       forground color (also file system bar color)\n\
        -geometry <arg>         geometry\n\
        -help                   help\n\
        -iconic                 iconic\n\
        -interval <arg>         interval used per turn\n\
        -mb                     show menu border\n\
        -name <arg>             run bugsx under this name\n\
        -nobreed                do not breed when running in batch mode\n\
        -number <arg>           number of biomorphs to draw (must be a square #)\n\
        -printpop               print the population when breeding\n\
        -rv                     reverse video\n\
        -seed <arg>             use this seed for random number generator\n\
        -segments <arg>         use this many segments to draw an organism\n\
        -showbreed              show breeding subpopulation when in batch mode\n\
	-showgenes		show a graphic representation for the genes\n\
        -synchronous            synchronous mode\n\
        -v                      verbose\n\
        -xrm                    make no entry in resrouce database\n\
        help                    help\n\n");


	PRINT_COPYRIGHT;
	exit (0);
}

