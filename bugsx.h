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



/* ****** include files ****** */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <math.h>
#ifndef DYNIX
# include <stdlib.h>
#endif
#ifdef __STDC__
# include <stdarg.h>
#else
# include <varargs.h>
#endif
#include <stdio.h>


/* ******************************************************************** */
/* ******************** general program constants ********************* */
/* ******************************************************************** */
#define TRUE		1
#define FALSE		0
#define	STRLENGTH	255		/* length of a string */
#define NOGOOD		-1		/* to distinguish nogood from false */
#define MAXFS           50		/* maximum number of file systems */
#define QUIT_KEY	'q'		/* quit key */
#define INIT_KEY	'i'		/* initialize key */
#define BREED_KEY	'b'		/* breed key */
#define CLASS_NAME	"Bugsx"		/* How do we look for resources */
#define DEFAULT_FONT	"6x13"		/* default font */
#define UPDINTERVAL	5
#define MSTIME		100000L		/* sleep time for msleep () */

#ifndef APP_DEFAULTS_DIR
#define APP_DEFAULTS_DIR        "/usr/X11R6/lib/X11/app-defaults/"
#endif

					/* well, I did write the damn thing */
#define PRINT_COPYRIGHT		printf ("\
bugsx v1.08 - (C) Copyright 1990-1997 Joshua R. Smith (jrs@media.mit.edu)\n\
		  http://physics.www.media.mit.edu/~jrs \n\
              (C) Copyright 1995-1997 Robert Gasch (Robert_Gasch@peoplesoft.com)\n\
		  http://www.peoplesoft.com/peoplepages/g/robert_gasch/index.htm")


/* ****** define window constants - these are positions and sizes ****** */
#define DEF_BORDER_WIDTH 	4 /* Default border width. */
#define MAIN_BORDER		20
#define WIN_X 			700
#define WIN_Y 			700
#define MIN_WIN_X		200
#define MIN_WIN_Y		200
#define MENU_ITEMS		3
#define MENU_Y			10
#define OFF_X			10
#define OFF_Y			10
#define DRAW_ROWS		4
#define DRAW_COLUMNS		4
#define DRAW_WINS		(DRAW_ROWS*DRAW_COLUMNS)
#define MAX_DRAW_WINS		(10*10)
#define MENU_HEIGHT(w) 		(LETTER_HEIGHT(w))
#define MIN_MENU_HEIGHT		4
#define LETTER_HEIGHT(w)  	((w).font_info->ascent + (w).font_info->descent)
#define LETTER_ASCENT(w)  	((w).font_info->ascent)
#define LETTER_SPACE(w)	  	(LETTER_HEIGHT(w) + 5 )
#define FREE_LETTER_SP(w) 	(LETTER_SPACE(w)-LETTER_HEIGHT(w))
#define NFS_TEXT_Y 		(fs_win[i].y-(LETTER_SPACE(fs_win[i])-\
				(LETTER_HEIGHT(fs_win[i])+1)))
#define SM_MENU_WIDTH(w) 	(XTextWidth((w).font_info, (w).text, \
				strlen((w).text)))
#define MENU_SPACE(w)		(MENU_HEIGHT(w)*(MENU_ITEMS+2))
#define INTERVAL(w)		((MENU_HEIGHT(w)+FREE_LETTER_SP(w))*2) 
#define	MIN_INTERVAL(w)		(MIN_MENU_HEIGHT+FREE_LETTER_SP(w)+\
				MENU_HEIGHT(w))
#define DRAW_WIDTH      ((main_win.width-(MAIN_BORDER*2))/Draw_Columns)
#define DRAW_HEIGHT     ((main_win.height-(MAIN_BORDER*2)-MENU_SPACE(menu[0]))/\
			Draw_Rows)

#define MAX_CHROM_SIZE	30	/* *** chromosome can have up to 30 genes *** */
#define MAX_POP		100	/* *** Population can be up to 100 *** */
#define ORG_X		4	/* *** # orgs horizontally *** */ 
#define ORG_Y		4	/* *** # orgs vertically *** */
#define	INIT_POP	(ORG_X*ORG_Y)	/* *** Initial population *** */


/* *** defines, typedefs, and externs needed by genetic algorithm *** */
/* *** This should contain almost all the parameters which are    *** */
/* *** interesting to play with. Do a 'make' after changing them, *** */
#define	WEIGHT_BASE	1.4	/* *** How much of curve we see *** */
#define ORG_T_MIN	0.0	/* *** Length of organism *** */
#define	ORG_T_MAX	(8*3.14159265358979323846)
#define	ORG_X_MIN	((double)(DRAW_WIDTH/2*-1))/* Coordinate range for organism window		*/
#define	ORG_X_MAX	(double)(DRAW_WIDTH/2)
#define	ORG_Y_MIN	((double)(DRAW_HEIGHT/2*-1))
#define	ORG_Y_MAX	(double)(DRAW_HEIGHT/2)
#define GENE_BORDER	10
#define ORG_SEGMENTS	(150.0)	/* *** # of segments used drawing org *** */
#define	CHROM_WIND	0.1	/* *** Fraction of window to draw genes *** */
#define CHROM_OFFSET	10	/* *** How much to offset bargraphs *** */ 
#define INIT_CHROM	8	/* *** Initial chromosome size *** */ 
#define	pCROSS		1.0	/* *** Probability of Crossover *** */
#define pMUTATION	0.067	/* *** Probability of Mutation *** */
#define MUTATION_STD	0.01	/* *** Controls size of mutations *** */
#define	INIT_SWITCH_DEF	0	/* *** Whether switches start on or off *** */
#define	INIT_SHOW_GENES	1	/* *** Whether gene window is on *** */
#define	INIT_PRINT_OUT	0	/* *** Whether print out is enabled *** */
#define MAX_SEGMENTS	2500
#define MIN_SEGMENTS	5
/* Not interesting to play with if breeder is running interactively */
#define INIT_FIT_THRESH	0.5	/* *** Default fitness threshold *** */


/* ****** define constants used to identify menus and windows(buttons) ****** */
#define INITIALIZE	0
#define BREED		1
#define	QUIT		2
#define PLUS		3
#define MIN		4
#define OK		5
#define SHOW		6
/* ******************** define the warning bitmap size ********************** */
#define warn_width 4
#define warn_height 10



/* ******************************************************************** */
/* ********************* program data structures ********************** */
/* ******************************************************************** */

typedef char MyString[STRLENGTH];

/* ****** Structures related to the genetic scheme of things ****** */
typedef	double		Gene;
typedef	struct org {
	int	name;		/* *** Keep track of names, just for fun *** */
	int	size_chrom;	/* *** Actual # genes in chromosome *** */
	Gene	X_Chrom[MAX_CHROM_SIZE];/* *** Genetic material itself *** */
	Gene	Y_Chrom[MAX_CHROM_SIZE];
	double	fitness;	/* *** Fitness of organism *** */
	int	mom;		/* *** Keep track of parents just for fun *** */
	int	dad;
	} Organism;

typedef	Organism	Population;

/* ****** XWindow struct - this simplifies function calls ****** */
typedef struct {
	Window 		win;		/* window ID */
	GC		gc;		/* window graphics content */
	XFontStruct*	font_info;	/* Font the window is using. */
	char		text[40];	/* title (for menus) */
	int		x, y, 		/* position */
			width, height, 	/* size */
			line_thick;	/* line thickness of window border */
	unsigned long	fg, bg;		/* foreground and background */
	long		event_mask, 	/* which events will be registered */ 
			flags;		/* window flags */
		} WinType, *WinTypePtr;


/* ******************************************************************** */
/* ********************** function declarations *********************** */
/* ******************************************************************** */

void	do_event_loop(
#if NeedFunctionPrototypes
		      int argc,
		      char** argv
#endif
);

void	handle_NFS_change(
#if NeedFunctionPrototypes
			  int ppnfs,
			  int OFS
#endif
);

void	fix_menu_pos(
#if NeedFunctionPrototypes
		     int width
#endif
);

void	redraw_main_win(
#if NeedFunctionPrototypes
			void
#endif
);

void 	create_window (
#if NeedFunctionPrototypes
		       Window parent_win,
		       WinTypePtr this_win,
		       unsigned long foreg,
		       unsigned long backg
#endif
);

int	highlight_menu(
#if NeedFunctionPrototypes
		       WinType menu[],
		       int menu_num,
		       int hightlight
#endif
);

int	expose_menu(
#if NeedFunctionPrototypes
		   WinType menu[],
		   int menu_num
#endif
);

int	expose_win(
#if NeedFunctionPrototypes
		   WinType win[],
		   int win_num
#endif
);

void	destroy_menu(
#if NeedFunctionPrototypes
		     WinType menu[],
		     int menu_num
#endif
);

int	which_button_pressed(
#if NeedFunctionPrototypes
			     WinType menu[],
			     int menu_num
#endif
);

void	select_breeding_sub_pop(
#if NeedFunctionPrototypes
				void
#endif
);

void 	highlight_org_window(
#if NeedFunctionPrototypes
			   int i
#endif
);

void 	handle_resize(
#if NeedFunctionPrototypes
			int x,
			int y
#endif
);

void	process_databases(
#if NeedFunctionPrototypes
			  int argc,
			  char** argv,
			  XrmDatabase commandlineDB
#endif
);

void	do_help(
#if NeedFunctionPrototypes
			void
#endif
);

void 	init_all_windows(
#if NeedFunctionPrototypes
			 void
#endif
);

Bool	getBoolResource(
#if NeedFunctionPrototypes
			XrmDatabase db,
			char* str_name,
			char* str_class,
			Bool deflt
#endif
);

int	getIntResource(
#if NeedFunctionPrototypes
		       XrmDatabase db,
		       char* str_name,
		       char* str_class,
		       int deflt
#endif
);

unsigned long getColorResource(
#if NeedFunctionPrototypes
			       XrmDatabase db,
			       char* str_name,
			       char* str_class,
			       unsigned long deflt
#endif
);

XFontStruct* getFontResource(
#if NeedFunctionPrototypes
			     XrmDatabase db,
			     char* str_name,
			     char* str_class,
			     XFontStruct* deflt
#endif
);

char*	catlist(
#if NeedFunctionPrototypes
		char*,
		...
#endif
);

void	randomize_org(
#if NeedFunctionPrototypes
		      Organism	*org,
		      int	name,
		      int	size_chrom
#endif
);

void	randomize_pop(
#if NeedFunctionPrototypes
		      void
#endif
);

int	rnd(
#if NeedFunctionPrototypes
	    int low,
	    int high
#endif
);

int	flip(
#if NeedFunctionPrototypes
	     double p
#endif
);

void 	copy_org(
#if NeedFunctionPrototypes
		 Organism *org1,
		 Organism *org2
#endif
);

void 	copy_pop(
#if NeedFunctionPrototypes
		 Population *pop1,
		 Population *pop2,
		 int size_pop
#endif
);

void erase_org(
#if NeedFunctionPrototypes
		Organism *org
#endif
);

int select_org(
#if NeedFunctionPrototypes
		int size_pop
#endif
);

double mutation(
#if NeedFunctionPrototypes
	      double allele
#endif
);

void crossover(
#if NeedFunctionPrototypes
		Organism *parent1,
		Organism *parent2,
		Organism *child1,
		Organism *child2,
		int first_born_name
#endif
);

void breed(
#if NeedFunctionPrototypes
		void
#endif
);

int fitness(
#if NeedFunctionPrototypes
	    int i
#endif
);

void set_toggle_to_default(
#if NeedFunctionPrototypes
		int i
#endif
);

void print_pop(
#if NeedFunctionPrototypes
		void
#endif
);

void print_org(
#if NeedFunctionPrototypes
		Organism *org
#endif
);

double dpow(
#if NeedFunctionPrototypes
		double x,
		int n
#endif
);

void resize(
#if NeedFunctionPrototypes
	void
#endif
);

void grow_pop(
#if NeedFunctionPrototypes
		void
#endif
);

int grow(
#if NeedFunctionPrototypes
		int org
#endif
);

void develop(
#if NeedFunctionPrototypes
		int org,
		double t,
		int *X_scr,
		int *Y_scr
#endif
);

void developF(
#if NeedFunctionPrototypes
		int org,
		double t,
		int *X_scr,
		int *Y_scr
#endif
);

void developFG(
#if NeedFunctionPrototypes
		int org,
		double t,
		int *X_scr,
		int *Y_scr
#endif
);

void display_genes(
#if NeedFunctionPrototypes
		int org
#endif
);

void graph_chrom(
#if NeedFunctionPrototypes
		Gene	*chrom,
		int	size_chrom,
		int	Xl,
		int	Xh,
		int	Yl,
		int	Yh,
		int	org
#endif
);

double noise(
#if NeedFunctionPrototypes
		double mean,
		double std
#endif
);

void init(
#if NeedFunctionPrototypes
		void
#endif
);

void msleep(
#if NeedFunctionPrototypes
		long n
#endif
);

