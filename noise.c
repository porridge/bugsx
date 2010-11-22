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



/* ************************************************************************ 
 *
 *                   Gaussian Random Number Generator
 *
 *  Donald H. House     	July 1, 1982
 *
 * This function takes as parameters real valued mean and standard-deviation,
 * and an integer valued seed. It returns a real number which may be
 * interpreted as a sample of a normally distributed (Gaussian) random variable
 * with the specified mean and standard deviation. As a side effect the value
 * of the integer seed is modified.
 *
 * The computational technique used is to pass a uniformly distributed random
 * number through the inverse of the Normal Distribution function.
 *
 *  Translated into C by Robert Allen January 25, 1989
 * ************************************************************************ */

#include "bugsx.h"

#define	ITBLMAX 20
#define	didu	(20.0 / 0.5)
/* Where it says 20.0, put the value of ITBLMAX */


/* ************************************************************************ */
/* ****************************  noise generator ************************** */
/* ************************************************************************ */
double noise (mean, std)
double mean, std;
{
	double u, du;
	int index, sign;
	double temp;
	static double tbl[ITBLMAX+1]
	 	= {0.00000E+00, 6.27500E-02, 1.25641E-01, 1.89000E-01,
     		  2.53333E-01, 3.18684E-01, 3.85405E-01, 4.53889E-01,
     		  5.24412E-01, 5.97647E-01, 6.74375E-01, 7.55333E-01,
     		  8.41482E-01, 9.34615E-01, 1.03652E+00, 1.15048E+00,
    		  1.28167E+00, 1.43933E+00, 1.64500E+00, 1.96000E+00,
     		  3.87000E+00	};

	u = drand48();
	if (u >= 0.5) 
		{
		sign = 1;
		u = u - 0.5;
		}
	else
		sign = -1;

	index = (int)(didu * u);
#ifdef DEBUG
	/* *** leftover as a result of foretting #include <stdlib.h> *** */
	if (index >= ITBLMAX+1)
		{
		printf ("Index Sanity Check in noise() failed!!!\n");
		printf ("Index = (int)(didu * u)\n");
		printf ("didu = %f\n", didu);
		printf ("u = %f\n", u);
		printf ("index = %d\n", (int)(didu * u));
		printf ("Exiting ...\n");
		exit (1);
		}
#endif
	du = u - index / didu;
	temp = mean + sign * (tbl [index] + (tbl [index + 1] - tbl [index]) *
     		du * didu) * std;
	return (temp);
}


/* ************************************************************************ */
/* ************************  test the noise generator ********************* */
/* ************************************************************************ */
/*
void test_noise () 
{
	int i;
	double mean;

	for (mean=0.0; mean < 5.0; mean += 1.0) 
		{
		printf("mean: %g  ",mean);
		for (i=0; i<5; i++) 
			printf("%g, ", noise(mean,0.1));
		printf("\n");
		}
}
*/
