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



/**************************************************************************** 
 *
 *	Grow.c	
 *	Embryology routines for breed.c.  Grow takes an organism and grows 
 *	it on the screen; in other words it graphs a polynomial. 
 * 	This file also has a bunch of general graphic support routines.
 *
 ****************************************************************************/

#include "bugsx.h"

extern Display		*mydisplay;
extern WinType          main_win,               /* main window */
                        menu[], 	        /* menu items */
                        draw_win[];		/* windows we draw in */
extern unsigned long	fg, bg;			/* foreground, background */
extern Population	G_Population[];		/* Array of Organisms */
extern Population	G_Kids_Pop[];		/* Next generation */
extern double		G_fit_thresh,		/* fitness threshold */
			G_pCross,		/* probability of crossover */
			G_pMutation,		/* probability of mutation */
			G_mutation_std,		/* gauss fn STD used to mutate*/
			G_weight[],		/*weight for each term */
			G_sum_weights;		/* Sum of weights -- yscaling */
extern int		G_size_pop,		/* # organisms in population */
			G_size_breeding_pop,	/* # organisms reproducing */
			G_generation,		/* # generations so far */
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
			selected[],
			segments,
			Draw_Wins,
			Draw_Rows,
			Draw_Columns,
			do_print_pop,
			verbose;



/* ************************************************************************ */
/* ****************** take a double to an integer power ******************* */
/* ************************************************************************ */
double dpow (x, n)
double 	x;
int 	n;
{
	int i;
	double p;

	p = 1.0;
	for (i=1; i <= n; ++i)
		p = p*x;
	return(p);
}




/* ************************************************************************ */
/* ****************** change all scales upon resizing ********************* */
/* ************************************************************************ */
/*
void resize (TheCanvas, new_width, new_height)
{
	G_current_width	=	new_width;
	G_current_height=	new_height;

	G_org_width		=	G_current_width/ORG_X;
	G_org_height	=	G_current_height/ORG_Y;

	G_x_scale		=	G_org_width/10;
	G_y_scale		=	G_org_height/10;

	G_x_trans		=	G_org_width;
	G_y_trans		=	G_org_height;
}
*/



/* ************************************************************************ */
/* ************ grow the entire population on the screen ****************** */
/* ************************************************************************ */
void grow_pop()
{
	int i;

#ifdef DEBUG 
	fprintf (stdout, "Entering Polulation Growth ...\n");
#endif

	for (i=0; i < Draw_Wins; i++) 
		{
		if (selected[i])
			{
			XFillRectangle (mydisplay,
				draw_win[i].win, 
				draw_win[i].gc, 0,0,
				draw_win[i].width,
				draw_win[i].height);
			XSetForeground (mydisplay, 
				draw_win[i].gc, fg);
			selected[i]=FALSE;
			}
 		XClearWindow (mydisplay, draw_win[i].win); 
		grow(i);
		if (G_show_genes == TRUE)  
			display_genes(i);
		}
	if (do_print_pop) 
		print_pop();
}




/* ************************************************************************ */
/* *************** translate: macros to scale and center ****************** */
/* ************************************************************************ */
/* scale x and y to fit inside cannonical screen box*/
#define SCALEX(x,xs)			((int) (x*xs))
#define SCALEY(y,ys)			((int) (y*ys))

/* translate scaled drawing to center of canonical screen box*/
#define	CENTERX(x,xt)			(x+(xt/2))
#define	CENTERY(y,yt)			(y+(yt/2))

/* translate and scale x and y -- this uses global variables*/
#define TSx(x) (CENTERX(SCALEX(x,draw_win[org].width/10), draw_win[org].width))
#define TSy(y) (CENTERY(SCALEY(y,draw_win[org].height/10),draw_win[org].height))



/* ************************************************************************ */
/* ******************* grow an organism on the screen ********************* */
/* ************************************************************************ */
int grow (org)
int org;
{
	double	t, dt;
	int	X_scr, Y_scr, X_scr_old, Y_scr_old;

	/* *** shortcut to keep us from calculating with no pop *** */
	if (G_generation == NOGOOD)
		return (1);
	dt = (ORG_T_MAX - ORG_T_MIN)/segments;

	/* *** Base case: t = ORG_T_MIN *** */
	developF(org, ORG_T_MIN, &X_scr, &Y_scr);
	X_scr_old = X_scr;
	Y_scr_old = Y_scr;

	for (t = ORG_T_MIN + dt; t <= ORG_T_MAX; t+= dt) 
		{
		developF(org, t, &X_scr, &Y_scr);
		if ((X_scr_old	>= TSx(ORG_X_MIN)) && 
		    (X_scr_old	<= TSx(ORG_X_MAX)) &&
		    (Y_scr_old	>= TSy(ORG_Y_MIN)) && 
		    (Y_scr_old	<= TSy(ORG_Y_MAX)) &&
		    (X_scr	>= TSx(ORG_X_MIN)) && 
		    (X_scr	<= TSx(ORG_X_MAX)) &&
		    (Y_scr	>= TSy(ORG_Y_MIN)) && 
		    (Y_scr	<= TSy(ORG_Y_MAX))) 
			{
			XDrawLine (mydisplay, draw_win[org].win, 
				   draw_win[org].gc, X_scr_old, Y_scr_old, 
				   X_scr, Y_scr);
			}
		X_scr_old = X_scr;
		Y_scr_old = Y_scr;
		}
	return (0);
}




/* ************************************************************************ */
/* ************* grow next point--evaluates organism at time t ************ */
/* ************************************************************************ */
void develop (org, t, X_scr, Y_scr)
int	 org;
double	 t;
int	*X_scr;
int 	*Y_scr;
{
	double	X=0.0,Y=0.0;
	double	t_var;
	int	i;

	for (i=0; i < G_Population[org].size_chrom; i++) 
		{
		t_var = dpow(t, i);
		X += t_var * G_Population[org].X_Chrom[i];
		Y += t_var * G_Population[org].Y_Chrom[i];
		}
	*X_scr = TSx(X);
	*Y_scr = TSy(Y);
}





/* ************************************************************************ */
/* **** grow next point using Fourier - evaluates organism at time t ****** */
/* ************************************************************************ */
void developF (org, t, X_scr, Y_scr)
int	 org;
double	 t;
int	*X_scr;
int 	*Y_scr;
{
	double	X=0.0,Y=0.0;
	double	t_var;
	int	i;

	for (i=0; i<G_Population[org].size_chrom; i++) 
		{
		t_var = i*t;
		X += cos(t_var) * G_Population[org].X_Chrom[i];
		Y += sin(t_var) * G_Population[org].Y_Chrom[i];
		}
	*X_scr = TSx(X);
	*Y_scr = TSy(Y);
}




/* ************************************************************************ */
/* **** grow next point using Fourier - evaluates organism at time t ****** */
/* ************************************************************************ */
void developFG (org, t, X_scr, Y_scr)
int	 org;
double	 t;
int	*X_scr;
int 	*Y_scr;
{
	double	X=0.0,Y=0.0;
	double	freq;
	double	t_var;
	int	i;

	for (i=0; i<G_Population[org].size_chrom; i++) 
		{
		freq = 1<<i;
		t_var = freq*t;
		X += (1.0/freq) * cos(t_var) * G_Population[org].X_Chrom[i];
		Y += (1.0/freq) * sin(t_var) * G_Population[org].Y_Chrom[i];
		}
	*X_scr = TSx(X);
	*Y_scr = TSy(Y);
}




/* ************************************************************************ */
/* ********************* makes a bargraph of the genes ******************** */
/* ************************************************************************ */
void display_genes(org)
int org;
{
	int X_low, X_high, Y_low, Y_high;

	X_low = GENE_BORDER;
	X_high = DRAW_WIDTH/2-GENE_BORDER;
	Y_low = 10;
	Y_high = DRAW_HEIGHT/7;
	graph_chrom(G_Population[org].X_Chrom, G_Population[org].size_chrom,
		X_low, X_high, Y_low, Y_high, org);
	X_low = DRAW_WIDTH/2+GENE_BORDER;
	X_high = DRAW_WIDTH-GENE_BORDER;
	graph_chrom(G_Population[org].Y_Chrom, G_Population[org].size_chrom,
		X_low, X_high, Y_low, Y_high, org);
}



/* ************************************************************************ */
/* ************************* graph one chromosome ************************* */
/* ************************************************************************ */
void graph_chrom (chrom, size_chrom, Xl, Xh, Yl, Yh, org)
Gene*	chrom;
int	size_chrom, Xl, Xh, Yl, Yh, org;

{
	int i;
	int g, g_width, height, Yo=((Yl + Yh)/2);

	g_width = (Xh-Xl)/(size_chrom+1);
	XDrawLine (mydisplay, draw_win[org].win, draw_win[org].gc,
		   Xl, Yo, Xh, Yo);
	g = Xl;
	for (i=0; i < size_chrom; i++) 
		{
		height = Yl+(Yh-Yl)/2+chrom[i]*(Yh-Yl);
		XDrawLine (mydisplay, draw_win[org].win, draw_win[org].gc,
			g, height, g+g_width, height);
/* 
		printf ("%d, %d, %d, %d\n", g, height, g+g_width, height);
*/
		g += g_width;
		}
}
