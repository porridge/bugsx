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




/***************************************************************************
 *	BUGS software
 *	Copyright 1990 Joshua R. Smith
 * 	file: breed.c	5/10/90	
 *
 *	This is a simple interactive genetic design system. You can grow
 *	creatures (graphs of Fourier Series, at this point) on the screen and 
 *	specify which are fit enough to reproduce by clicking. When you've 
 *	choosen the ones you want to reproduce, click breed. Breeder varies
 *	their genetic material and displays a new generation for you to 
 *	inspect. As of now, breeder uses only the three most basic variation 
 *	mechanisms: reproduction, crossover, and mutation. Reproduction
 *	means that only the fit will affect the next generation. Those which 
 *	are not fit do not reproduce and therefore do not affect the next
 *	generation. In crossover, pairs of parents from the breeding (ie fit)
 *	subpopulation are choosen. We then choose a random chromosome locus
 *	(a locus is an index into the array holding the genes, ie an index
 *	into the chromosome), snip both Chromosomes at that point, and cross
 *	the strands; that is, we glue the first piece from the first parent
 *	onto the second piece from the second parent and the first piece
 *	from the second parent onto the second piece from the first parent.
 *	Got it? Or wait... Did I get that backwards? No. Just Kidding.
 *	During crossover, mutation can also occur.  If an allele (chromosome 
 *	array element value) is to be mutated, we just add some noise-- a
 *	gaussian random variable with a specified STD.
 *
 *	Crossover and mutation do not happen every time. There are variables 
 *	specifying the probabilities of these events. Right now, these and 
 *	many other useful parameters are in the file Curves.h. To experiment
 *	with them, modify the values set in Curves.h and make to recompile. 
 *	Only breed.c should need to be recompiled.
 *	(At some point in the near future, useful parameters will appear in   
 *	the control panel so you'll be able to tinker with them without
 *	recompiling.  At a later date, all or some parameters may be specified 
 *	BY THE GENETIC MATERIAL ITSELF! Then things will really get
 *	interesting.  If some variation mechanism is counter-productive in a  
 *	particular application, the probability of that variation should
 *	diminish and perhaps vanish. Further down the road, hopefully the
 *	genes will code (some of) the variation	mecahisms.  Then new ones will 
 *	be able to develop as required by the problem, the changing search 
 *	space or the environment--whatever you want to call it.
 *
 *
 *	This file is where the action is. The main routine, which sets up the 
 *	interactive system and then lets it go, is in this file. Also, all the
 *	genetic operators are implemented here. Once again, parameters to  
 *	tweak are in Curves.h. The embryological stuff (routine to graph
 *	polynomials) is in the file Grow.c.
 *
 *	To run, type 'b' (the letter b).  This is a command script which 
 * 	executes 'breed' (compiled program) and passes it the time
 *	as a seed for the random number generator. Hint: if you want
 *	everything to go faster, shrink the population window. The smaller 
 *	it is, the faster everything goes.
 *
 *	Acknowledgements: the code for the user interface was largely
 *	appropriated from GED, written by Mike McDougall, Rob Allen, me, and 
 *	others at Williams College.  The Gaussian noise generator was written 
 *	by Donald House and translated by Rob Allen.  And of course the idea 
 *	for the program was inspired by Richard Dawkins' Blind Watchmaker.
 *	This program was originally written as a project in Donald House's AI
 *	course.  Thanks to Duane Bailey and Don House for supervision and 
 *	funding over the summer of 1990.
 *
 ***************************************************************************/


#include "bugsx.h"

extern Display		*mydisplay;
extern WinType          main_win,               /* main window */
                        menu[],  		/* menu items */
                        draw_win[];		/* windows we draw in */
extern unsigned long	fg, bg;			/* foreground, background */
extern Population	G_Population[];		/* Array of Organisms */
extern Population	G_Kids_Pop[];		/* Next generation */
extern double		G_fit_thresh,		/* fitness threshold */
			G_pCross,		/* probability of crossover */
			G_pMutation,		/* probability of mutation */
			G_mutation_std,		/* gauss fn STD used to mutate*/
			G_weight[],		/* weight for each term */
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
			extend_print,
			verbose;
extern long		seed;


/* ************************************************************************ */
/* ************************  initialize the system  *********************** */
/* ************************************************************************ */
void init ()
{

	G_size_pop	=	INIT_POP;
	G_fit_thresh	=	INIT_FIT_THRESH;
	G_switch_default=	INIT_SWITCH_DEF;
	G_show_genes	=	INIT_SHOW_GENES; 
	G_pCross	=	pCROSS;
	G_pMutation	=	pMUTATION;
	G_mutation_std	=	MUTATION_STD;
	G_generation	=	0;
	seed 		= 	time (NULL);
}





/* ************************************************************************ */
/* ****** set organism's genes to random values between -.5 and +.5 ******* */
/* *** 6/4/90, changed max and min gene values from -.5,.+5 to -1.0,+1.0 ** */
/* ************************************************************************ */
void randomize_org (org, name, size_chrom)
Organism	*org;
int		name, size_chrom;
{
	int	i;

	org->name = name;
	org->size_chrom	= size_chrom;

	for (i=0; i<size_chrom; i++) 
		{
		org->X_Chrom[i]	= (2.0*drand48())-1.0;
		org->Y_Chrom[i] = (2.0*drand48())-1.0;
		G_weight[i] 	= dpow (WEIGHT_BASE, i);
		G_sum_weights 	+= G_weight[i];
		}

	org->fitness	= 0.0;
	org->mom	= 0;
	org->dad	= 0;
	G_sum_weights	= 0.0;
}



/* ************************************************************************ */
/* ***********************  initialize the population ********************* */
/* ************************************************************************ */
void randomize_pop()
{
	int	i;
	
	for (i=0; i<G_size_pop; i++) 
		if (!selected[i])
			randomize_org (&G_Population[i], i, INIT_CHROM);
}




/* ************************************************************************ */
/* * get a uniformly distributed random # between low && high (inclusive) * */
/* ************************************************************************ */
int rnd (low, high)
int low, high;
{
	double	alpha;
	int	beta;

	alpha = drand48(); /* alpha is a uniform rand dist var on [0.0, 1.0) */
	beta = (int) (low + ((high-low+1)*alpha));
	return (beta+low);
}



/* ************************************************************************ */
/* *********** get a boolean value with specified probability ************* */
/* ************************************************************************ */
int flip (p)
double p;
{
	int result;

	if (p == 1.0) 
		result = 1;
	else 
		{
		if (drand48() <= p) 
			result = 1;
		else 
			result = 0;
		}
	return(result);
}



/* ************************************************************************ */
/* ********** copy genetic material from one organism to another ********** */
/* ************************************************************************ */
void copy_org (org1, org2)
Organism *org1, *org2;
{
	int i;

	org2->name = org1->name;
	org2->size_chrom = org1->size_chrom;
	for (i=0; i<org1->size_chrom; i++) 
		{
		org2->X_Chrom[i] = org1->X_Chrom[i];
		org2->Y_Chrom[i] = org1->Y_Chrom[i];
		}
	org2->mom = org1->mom;
	org2->dad = org1->dad;
}




/* ************************************************************************ */
/* ************************ copy entire population ************************ */
/* ************************************************************************ */
void copy_pop(pop1, pop2, size_pop)
Population 	*pop1, *pop2;
int		size_pop;
{
	int i;

	for (i=0; i<size_pop; i++) 
		copy_org (&pop1[i], &pop2[i]);
}



/* ************************************************************************ */
/* *************************** erase an organism ************************** */
/* ************************************************************************ */
void erase_org (org)
Organism *org;
{
	int i;

	for (i=0; i<org->size_chrom; i++) 
		{
		org->X_Chrom[i]	= 0.0;
		org->Y_Chrom[i]	= 0.0;
		org->size_chrom	= 0;
		org->fitness	= 0.0;
		}
}



/* ************************************************************************ */
/* ***************** pick out a single eligible individual **************** */
/* ************************************************************************ */
int select_org (size_pop)
int	size_pop;
{
	return (rnd(0,size_pop-1));
}




/* ************************************************************************ */
/* ******************************** mutate ******************************** */
/* ************************************************************************ */
double mutation(allele)
double allele;
{
	if (flip(G_pMutation)==1) 
		allele = noise(allele, G_mutation_std);
	return(allele);
}




/* ************************************************************************ */
/* ************************** crossover == sex **************************** */
/* ************************************************************************ */
/* * Note: if we removed possibility of combining chromosomes of different* */
/* ** lengths and it were still possible for chrom length to change then ** */ 
/* ************************ we'd have...  speciation! ********************* */
/* * Should try to have GENES specify how genetic material is  recombined.* */
/* ************************************************************************ */
/* ************************************************************************ */
/* ************************************************************************ */
/*	This is how Crossover works: choose a crossover point, i_cross.
 *	p1 = p1a.p1b	p2 = p2a.p2b.
 *
 *	Then the children c1 and c2, are:
 *	c1 = p1a.p2b	c2 = p2a.p1b
 * ************************************************************************ */
void crossover (parent1, parent2, child1, child2, first_born_name)
Organism	*parent1, *parent2, *child1, *child2;
int		first_born_name;
{
	int	i, i_cross, short_chrom, long_chrom;

	if (extend_print)
		{
		printf("Mom: \n");
		print_org(parent1);
		printf("Dad: \n");
		print_org(parent2);
		}
	child1->name = first_born_name;
	child2->name = first_born_name+1;

	/* *** use size of smaller chromosome as max crossover locus *** */
	if (parent1->size_chrom > parent2->size_chrom) 
		{
		long_chrom	= parent1->size_chrom;
		short_chrom	= parent2->size_chrom;
		}
	else 
		{
		long_chrom	= parent2->size_chrom;
		short_chrom	= parent1->size_chrom;
		}

	/* *** Now we vary X and Y Chroms completely seperately.  *** */
	/* *** They don't cross over at the same points or at the *** */
	/* *** same points or at the same times. Also, we don't   *** */
	/* *** use any inter chromosomal variation operators like *** */
	/* *** segregation and translocation.			  *** */

	/* *** X_CHROM  *** */
	/* *** Crossover occurs with p(G_pCross) *** */
	if (flip(G_pCross)==1) 	
		/* *** pick crossover locus *** */
		i_cross = rnd(0,short_chrom-1); 
	else 
		/* *** INCORRECT if diff Chrom sizes *** */
		i_cross = short_chrom; 

	/* *** Copy first part of Chroms directly to children *** */
	for (i=0; i<i_cross; i++) 
		{
		child1->X_Chrom[i] = mutation(parent1->X_Chrom[i]);
		child2->X_Chrom[i] = mutation(parent2->X_Chrom[i]);
		}

	/* *** parent2 to child 1 (a cross) *** */
	for (i=i_cross;	i<short_chrom; i++)  
		child1->X_Chrom[i] = mutation(parent2->X_Chrom[i]);

	/* *** parent1 to child 2 (a cross) *** */
	for (i=i_cross;	i<long_chrom; i++)  
		child2->X_Chrom[i] = mutation(parent1->X_Chrom[i]);
	

	/* *** Y_CHROM *** */
	/* *** Crossover occurs with p(G_pCross) *** */
	if (flip(G_pCross)==1) 			
		/* *** pick crossover locus *** */
		i_cross = rnd(0,short_chrom-1);	
	else 
		/* *** INCORRECT if diff Chrom sizes *** */
		i_cross = short_chrom;	
	
	/* *** Copy first part of Chroms directly to children *** */
	for (i=0; i<i_cross; i++) 
		{
		child1->Y_Chrom[i] = mutation(parent1->Y_Chrom[i]);
		child2->Y_Chrom[i] = mutation(parent2->Y_Chrom[i]);
		}

	/* *** parent2 to child 1 (a cross) *** */
	for (i=i_cross;	i<short_chrom; i++)  
		child1->Y_Chrom[i] = mutation(parent2->Y_Chrom[i]);

	/* *** parent1 to child 2 (a cross) *** */
	for (i=i_cross; i<long_chrom; i++)  
		child2->Y_Chrom[i] = mutation(parent1->Y_Chrom[i]);

	child1->size_chrom	= parent2->size_chrom;
	child2->size_chrom	= parent1->size_chrom;
	child1->fitness		= 0.0;
	child2->fitness		= 0.0;
	child1->mom		= parent1->name;
	child2->mom		= parent1->name;
	child1->dad		= parent2->name;
	child2->dad		= parent2->name;

	if (extend_print)
		{
		printf("Fred: \n");
		print_org(child1);
		printf("Jane: \n");
		print_org(child2);
		}
}



/* ************************************************************************ */
/* ************************** mix up the genes **************************** */
/* ************************************************************************ */
void breed() 
{
	int i, size_eligible_pop, Mom, Dad;

	/* *** Isolate the breeding subpopulation *** */
	size_eligible_pop = 0;
	for (i=0; i<G_size_pop; i++) 
		{
		/* *** Set fitness values  *** */
		G_Population[i].fitness = fitness(i);
		if (fitness(i)) 
			size_eligible_pop++;
		set_toggle_to_default(i);
		}
	if (size_eligible_pop<2) 
		size_eligible_pop = G_size_pop;
	else 
		{
		size_eligible_pop = 0;
		for (i=0; i<G_size_pop; i++) 
		   {
		   if (G_Population[i].fitness > G_fit_thresh) 
			{
			if (extend_print)
				{
				printf("Copying %d to %d \n",
					i, size_eligible_pop);
				printf("original %d: \n", i);
				print_org(&G_Population[i]);
				printf("\n");
				printf("original %d: \n", size_eligible_pop);
				print_org(&G_Population[size_eligible_pop]);
				printf("\n");
				}
			/* *** This is a tiny optimization *** */ 
			/* *** don't copy to same place    *** */
			if (i != size_eligible_pop) 
				copy_org(&G_Population[i], 
				     &G_Population[size_eligible_pop]);
			if (extend_print)
				{
				printf("%d after copy: \n", size_eligible_pop);
				print_org(&G_Population[size_eligible_pop]);
				printf("\n");
				}
			/* *** Increment once per FIT organism *** */
			size_eligible_pop++;  
			}
		   G_Population[i].fitness = 0.0;
	 	   }
		}
	/* *** Each couple has two kids. Nuclear families! *** */
	for (i=0; i<G_size_pop; i+=2) 
		{
		Dad = select_org (size_eligible_pop);
		do 
			{
			Mom = select_org(size_eligible_pop);
			}
		while (Mom == Dad);
		crossover(&G_Population[Mom], &G_Population[Dad],
			  &G_Kids_Pop[i], &G_Kids_Pop[i+1], i);
		}

	/* *** Now the kids grow up - make new generation the current one *** */
	copy_pop(G_Kids_Pop, G_Population, G_size_pop);

	/* *** Here we actually put the kids on the screen *** */
	grow_pop();
}




/* ************************************************************************ */
/* *** calculate fitness - in this case by checking for selected windows ** */
/* ************************************************************************ */
int fitness(i)
int i;
{
#ifdef DEBUG
	if (selected [i])
		printf  ("%d is fit ...\n", i);
#endif
	return (selected [i]);
}




/* ************************************************************************ */
/* ********** set the fitness indicator back to default (FALSE) *********** */
/* ************************************************************************ */
void set_toggle_to_default(i)
int i;
{	
	if (selected[i])
		{
		XFillRectangle (mydisplay, draw_win[i].win, draw_win[i].gc, 
			0, 0, draw_win[i].width, draw_win[i].height);
		XSetForeground (mydisplay, draw_win[i].gc, fg);
		selected[i]=FALSE;
		}
}




/* ************************************************************************ */
/* ******************** print the entire population *********************** */
/* ************************************************************************ */
void print_pop()
{
	int i;

	for (i=0; i<G_size_pop; i++)
		print_org (&G_Population[i]);
	printf("\n");
}



/* ************************************************************************ */
/* ************************** print an organism *************************** */
/* ************************************************************************ */
void print_org (org)
Organism* org;
{
	int i;

	printf("%d of %d and %d:\n",org->name, org->mom, org->dad);
	for (i=0; i<org->size_chrom; i++) 
		{
		printf("   (%1.3g, %1.3g)",
			org->X_Chrom[i], org->Y_Chrom[i]);
		if ((i+1)%4 == 0)
			printf ("\n");
		}
	printf("\t%g", org->fitness);
	printf("\n");
}

