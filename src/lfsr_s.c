#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

/*
 * LFSR - Linear Feedback Shift Register.
 * lfsr_s finds length polynomials for linear feedback shift registers
 * Supposedly, there's an easier way to do this using finite field theory, 
 * but in the few cases I checked, they were flawed.
 *
 * I suspect that it's easy to find primitive polynomials, but
 * what's needed is irreducible polynomials, and that those are
 * hard to find.  Still, I hope to some day find a program that
 * can solve 2048 bits polys as fast as this one finds 16 bit.
 *
 * This program picks a poly, and then uses "that cordic nonsense" 
 * to prove it's maximal length.  Approximately 1/16 of the chosen
 * polys are maximal length.
 *
 * The idea behind chordic is to solve the equations of a function,
 * rather than the single bits.  Example;

 Suppose an 8 bit lfsr with poly 0x96 is in the state;
     a b c d e f g h
     0 0 1 0 1 1 0 1
 Since the bottom bit is a 1, atfer 1 shift, it becomes,
     a b c d e f g h
     0 0 0 1 0 1 1 0
 ^   1 0 0 1 0 1 1 0
 -------------------
     1 0 0 0 0 0 0 0

 (^ denotes exclusive or)

 Since the bottom bit is 0, after the next shift it becomes
     a b c d e f g h
     0 1 0 0 0 0 0 0

 and so on.

 We could write down the equations of each bit thus:
 0 shifts = a b c d e f g h
 1 shift  = h a b c^h d e^h f^h g
 2 shitfs = g h a b^g c^h d^g e^h^g f^h 

 For some equations, this blows up rapidily, but lfsr use only xor.
 a^a cancles a, so each position has, at most, N terms.  
 For 512 bits this is still 512*512 bits, but that's less than
 a megabyte, which is quite manageable for computers these days.


 Now for the tricky part.  We can skip the third shift and go straight
 to four, because we know what each bit transforms to after 2 shifts.
 the first position (a) becomes whatever is in the seventh
 position (g) and the forth position (d) becomes whatever is in the
 second position (b) xor'ed with the seventh posistion (g)

 Knowing the 4th shift, we can calulate the 8th, then the 16th and
 so on.  We can calculate _any_ number of shifts, by either shifting
 double (appling the current shift to itself) or by applying 1 shift.
 To do 18 shifts, write 18 in binary - %10010 for each '0' bit, do
 a doubling shift, for each '1' bit do a doubling shift and a single
 shift.


 Once we can perform an arbitary number of shifts, it's a realitively
 simple matter to perform 2**n-1 shifts and determine if it returns
 to it's starting condition.  If not, then the poly is certainly
 not maximal length.  Unfortunately if it does, that doesn't prove
 it's maximal length, since it's possible it reached that state earlier.
 However, if it did get there earlier, than 2**n-1 must be a multiple
 of the number of shifts needed.  If 2**n-1 is a prime, then it is
 enough.  If it's not, then we need to also check shifts that are
 factors of 2**n-1.  For each prime factor, we check M/f and if none
 of those return to the initial condition, then it is maximal length.

 For 8 bits, we need to know all the factors of 2**8-1.
 2**8-1 is 255, it's prime factors are 3, 5, and 17.
 After testing that 255 shifts brings us back to the beginning,
 we check that 85 shifts (255/3), 51 shitfs (255/5) and 
 15 shifts (255/17) do not bring us back.
 
 -------
 For speed, this version is limited to polys that fit in a uint64_t
 (was: long, which usually compiles to 32 bit integer).
 There is a version that will find polys up to 512 bits, and 
 'probable' polys for even larger bit sizes, but this isn't it.

 */

#define uchar unsigned char
#define ulong uint64_t

#define MAXIMUM_NUMBER_OF_BITS 64
#define TOPBIT  ((ulong)1<<(MAXIMUM_NUMBER_OF_BITS-1))

int number_of_bits;

ulong c_matrix[MAXIMUM_NUMBER_OF_BITS];
ulong m_matrix[MAXIMUM_NUMBER_OF_BITS];
ulong t_matrix[MAXIMUM_NUMBER_OF_BITS];

uchar taps[MAXIMUM_NUMBER_OF_BITS];

/* this was 37 for MAXIMUM_NUMBER_OF_BITS = 32
 * not knowing what i'm doing .. i increased this a bit!
 */
#define MAX_FACTORS 64
ulong bdfactors[MAX_FACTORS];

int number_of_factors = 0;
int verbose_mode = 10;
int limit_max_taps = -1;
int dot_filtered = 1;
int skipped_since_last_print = 0;

void note(int priority, char *fmt, ...) 
{
   va_list ap;
   char buff[512];

   if (skipped_since_last_print) {
      printf("\n");
      skipped_since_last_print = 0;
   }

   if (priority < verbose_mode) {
      va_start(ap, fmt);
      vsprintf(buff, fmt, ap);
      va_end(ap);

      printf("%s", buff);
   }
}

/*
 *
 * Find all the factors of 2^order-1 by brute force division.
 * This divides by all the odd numbers less than the square root,
 * (We can skip 2, since it can't possibly be even)
 * Not the fastest way, but one of the smallest.  :)
 * 
 */

void find_factors(int order) {
   double n, p, end, f;

   number_of_factors = 0;

   p = pow(2.0, (double) order) - 1.0;

   n = p;
   end = sqrt(n);

   for (f=3.0; f<=end; ) {
        if ( fmod(n,f) == 0.0) {
            note(3, "%.0f * ", f);
            if (number_of_factors >= MAX_FACTORS) {
               printf("Maximum number of factors exceeded.  Exiting\n");
               exit(0);
            }
            /* Don't add duplicate factors */
            if ((number_of_factors == 0) ||
                (p/f != bdfactors[number_of_factors-1]) ) {
               bdfactors[number_of_factors] = (ulong) p/f;
               number_of_factors++;
            }
            n /= f;
            end = sqrt(n);
        } else {
           f+=2.0;
        }
    }
    note (3, "%0.0f", n);
    bdfactors[number_of_factors] = p/n;
    number_of_factors++;
}

void dump_taps(void) {
   int i, j, n_taps;
   n_taps = 0;

   for (i=0; i<number_of_bits; i++) {
      if (taps[i])
         ++n_taps;
   }
   if (0 <= limit_max_taps && limit_max_taps < n_taps) {
	  if (dot_filtered) {
	     ++skipped_since_last_print;
         printf(".");
      }
      return;
   }

   note(1, "$");
   for (i= ((number_of_bits-1) & 0xfffc); i>=0 ; i-=4) {
      j = 0;
      if (taps[i+0]) j+=1;
      if (taps[i+1]) j+=2;
      if (taps[i+2]) j+=4;
      if (taps[i+3]) j+=8;
      note(1, "%x", j);
   }

   note(1, ": #=%2d: 0", n_taps);
   for (i=0; i<number_of_bits; i++) {
      if (taps[i]) {
         note(1, ",%d", i+1);
      }
   }
   note(1, "\n");
}
   
void dump_pair(void) {
   int i, j, n_taps;
   n_taps = 0;

   for (i=0; i<number_of_bits; i++) {
      if (taps[i])
         ++n_taps;
   }
   if (0 <= limit_max_taps && limit_max_taps < n_taps) {
	  if (dot_filtered) {
	     ++skipped_since_last_print;
         printf(".");
      }
      return;
   }

   note(1, "$");
   j = 1;
   for (i= 0; i < ((number_of_bits-1) & 3); i++) {
      j <<= 1;
      if (taps[i]) j|=1;
   }
   note(1, "%x", j);
   for (; i < number_of_bits-1; i+=4) {
      j = 0;
      if (taps[i+0]) j+=8;
      if (taps[i+1]) j+=4;
      if (taps[i+2]) j+=2;
      if (taps[i+3]) j+=1;
      note(1, "%x", j);
   }

   note(1, ": #=%2d: ", n_taps);
   for (i=number_of_bits-1; i>=0; i--) {
      if (taps[i]) {
         note(1, "%d,", number_of_bits-(i+1));
      }
   }
   note(1, "%d\n", number_of_bits);
}

void do_feedback_m(void) {
   int i, j;
   ulong l;

   for (i=0; i<number_of_bits; i++) {
      t_matrix[i] = c_matrix[i];
      c_matrix[i] = 0;
   }

   for (i=0; i<number_of_bits; i++) {
      for (l=TOPBIT,j=0; j<number_of_bits; j++) {
         if (m_matrix[i] & l) {
            c_matrix[i] ^= t_matrix[j];
         }
         l>>=1;
      }
   }
}

void copy_matrix(ulong dest[], ulong src[]) {
   int i;
   
   for (i=0; i<number_of_bits; i++) {
      dest[i] = src[i];
   }
}


void set_matrix_to_one_shift(ulong matrix[]) {
   ulong l;
   int i, k;

   /* Set the matrix to one shift */
   for (l=TOPBIT, i=0; i<number_of_bits-1; i++) {
      l>>=1;
      matrix[i] = l;
   }
   matrix[number_of_bits-1] = TOPBIT;

   /* Now set the taps */
   for (k=0; k<number_of_bits; k++) {
      if (taps[k]) {
         matrix[k] |= TOPBIT;
      }
   }
}

void do_n_shifts(ulong n) {
   ulong l;

   set_matrix_to_one_shift(c_matrix);
   copy_matrix(m_matrix, c_matrix);

   for(l=TOPBIT; l; l>>=1) {
      if (n & l) {
         break;
      }
   }
   l>>=1;

   for(; l; l>>=1) {
      copy_matrix(m_matrix, c_matrix);
      do_feedback_m();
      if (n & l) {
         set_matrix_to_one_shift(m_matrix);
         do_feedback_m();
      }
   }
}

int num_ones(void) {
   int i, c;

   c = 0;
   for(i=0; i<number_of_bits; i++) {
      c += taps[i];
   }
   return c;
}

int pair_type(void) {
   int i, mid;

   mid = number_of_bits/2;
   for (i = 1; i <= mid; i++) {
      if (taps[i-1] != taps[number_of_bits-i-1]) {
         if (taps[i-1]) {
            return -1;
         } else {
            return 1;
         }
      }
   }
   return 0;
}


int next_poly(void)
{
   int i, ones_count;

   do {
      i = 0;
      while (taps[i]) {
         taps[i] = 0;
         i++;
         if (i >= number_of_bits) {
            return 1;
         }
      }
      taps[i]=1;
   } while ( (num_ones() & 1) || (pair_type() == 1) );

   return 0;
}


void set_taps(int order) {

   memset(taps, 0, sizeof(taps));

   if (order> 0 && order<= MAXIMUM_NUMBER_OF_BITS) {
      taps[order-1] = 1;
   } else {
      fprintf(stderr, "Error, can't have a tap > %d or less than 0\n", MAXIMUM_NUMBER_OF_BITS);
      exit(0);
   }
}


int main(int argc, char *argv[]) {
   int i, k;
   ulong qt0;
   ulong count;
   ulong attempts;
   int sm;
   int start_time, end_time;
   ulong bad_pos;

   if (argc < 2 || ( 1 < argc && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-?")) )) {
      printf("Maximal length Linear Feedback Shift Register tool (%d bits max), Version 1.2 Feb 28, 2021\n", MAXIMUM_NUMBER_OF_BITS);
      printf("Usage: lfsr_s <Order> [<maxtaps> [<dot_filtered>]]\n");
      printf("  <Order> - the number of bits in the LFSR\n");
      printf("  <maxtaps> - don't output LFSR combinations with more than this number of taps\n");
      printf("              default: -1, to print all combinations\n");
      printf("  <dot_filtered> - print a '.' when a combination is not output (default: 1)\n");
      printf("                   set '0' to remove the dots\n");
      return 0;
   }

   start_time = (int) time(NULL);

   number_of_bits = atoi(argv[1]);
   if ((number_of_bits > MAXIMUM_NUMBER_OF_BITS) || (number_of_bits < 4)) {
      printf("Order must be between 4 and %d\n", MAXIMUM_NUMBER_OF_BITS);
      return 0;
   }

   if (2 < argc)
      limit_max_taps = atoi(argv[2]);

   set_taps(number_of_bits);

   if (2 < argc) {
      verbose_mode = atoi(argv[2]);
   }
   if (3 < argc) {
      dot_filtered = atoi(argv[3]);
   }

   note(1, "Order %d, using the 'in pairs' search method.\n", number_of_bits);
   note(1, "Starting with ");
   dump_taps();

   number_of_factors = 0;

   note(2, "Finding factors for 2^%d-1\n", number_of_bits);
   find_factors(number_of_bits);
   note(2, "\nFound %d unique factors\n", number_of_factors);

   count = 0;
   attempts = 0;
   bad_pos = 0;
   for (;;) {
      set_matrix_to_one_shift(c_matrix);
      qt0 = c_matrix[0];

      /* "quick" test to determine if the period is 2^number_of_bits-1 */
      attempts++;

      for (i = 1; i <= number_of_bits; i++) {
         copy_matrix(m_matrix, c_matrix);
         do_feedback_m();

         if (c_matrix[0] == qt0) {
            if (i != number_of_bits) {
               break;
            }
            /* passes first test, now for the possible factors */

            if (number_of_factors > 1) {
               for (k=0; k<number_of_factors; k++) {
                  do_n_shifts(bdfactors[k]);
                  if (c_matrix[0] == TOPBIT) {
                     bad_pos++;
                     goto fails;
                  }
               }
            }
            dump_taps();
            count++;
            if (pair_type() == -1) {
               dump_pair();
               count++;
            } else {
               printf("Hey! there really is a paired maximal length LFSR!\n");
            }
            break;
         }
      }
      fails:

      if (next_poly()) 
         break;

   }
   end_time = time(0l);
   note(0, "Elapsed seconds = %d\n", end_time - start_time);
   note(3, "attempts = %lu\n", attempts);
   note(3, "number which passed first, but failed subsequent checks %lu\n", bad_pos);
   note(0, "count = %lu\n", count);
   return 0;
}
