/*
 * NAME:        eval_eqn.c
 *
 * PURPOSE:     To handle equations
 *
 * USAGE:       ee_clean_up_equation creates a EQUATION_INFO structure
 *                      from the equation string.
 *              ee_evaluate_equation evaluates the EQUATION_INFO structure.
 *              ee_show_err_mesg shows the error messages generated in the
 *              functions.
 *
 * RETURNS:
 *
 * DESCRIPTION:
 *
 * SYSTEM DEPENDENT FUNCTIONS:  None.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    
 *
 *  constants:   (preceded by colon)
 *
 * :e  = 2.71828...
 * :pi = 3.14159...
 *
 *  symbols:
 *
 *    ~  = negative sign
 *    ^  = exponentiation
 *    %  = modulus
 *    *  = multiplication
 *    /  = division
 *    +  = addition
 *    -  = subtraction
 *    !  = logical not (Also "not")
 *    &  = logical and (Also && to conform to C-type convention, and "and")
 *    |  = logical or (Also || to conform to C-type convention, and "or")
 *    x| = logical exclusive or (TRUE if one and only one condition is TRUE, also "xor")
 *    =  = equal to (Also == to conform to C-type convention)
 *    <  = less than
 *    >  = greater than
 *    != = not equal to ("<>" and "><" are converted to "!=")
 *    <= = less than or equal to ("=<" is converted to "<=")
 *    >= = greater than or equal to ("=>" is converted to ">=")
 *    () = parenthesis
 *    [] = surround variables
 *    "" = surround string constants
 *
 *  functions:
 *
 * acosh inverse hyperbolic cosine
 * asinh inverse hyperbolic sine
 * atanh inverse hyperbolic tangent
 * asech inverse hyperbolic secant
 * acsch inverse hyperbolic cosecant
 * acoth inverse hyperbolic cotangent
 * acos  inverse cosine
 * asin  inverse sine
 * atan  inverse tangent
 * asec  inverse secant
 * acsc  inverse cosecant
 * acot  inverse cotangent
 * cosh  hyperbolic cosine
 * sinh  hyperbolic sine
 * tanh  hyperbolic tangent
 * sech  hyperbolic secant
 * csch  hyperbolic cosecant
 * coth  hyperbolic cotangent
 * sqrt  square root
 * sign  sign of argument (1 if pos, -1 if neg, 0 if 0)
 * cos   cosine
 * sin   sine
 * tan   tangent
 * sec   secant
 * csc   cosecant
 * cot   cotangent
 * abs   absolute value
 * exp   e to the power
 * log   logarithm base 10
 * fac   factorial
 * deg   radians to degrees
 * rad   degrees to radians
 * rup   round to nearest larger integer
 * rdn   round to nearest smaller integer
 * rnd   round to nearest integer
 * sqr   square
 * ten   ten to the power
 * ln    logarithm base e
 *
 * KEYWORDS: equation
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <eval_eqn.h>

#define PI 3.14159265359
#define E  2.71828182846
#define EE_INIT_EQNL 100
#define EE_MAX_VAR_NAME_LENGTH 100
#define EE_SCRATCH_EQN_LEN 500
/* EE_SCRATCH_EQN_LEN MUST be bigger than 256 */
#define TRUE 1
#define FALSE 0

int ee_replace_op(char *eqn, char *operator_, char repwith, char char_allowed, char crepwith, EQUATION_INFO_PTR einfo, int *error);

#define ee_seek_and_replace(functnm, rwith, fnlen) while\
   ((position = memStrstr(eqn, functnm,NO_TAG)) != NULL)\
   if(!(ee_repl(rwith, fnlen, position, einfo, error))) return(0)
   
#define domain_error {*(error) = 7; return(0);}
/*    This is EE_ERR_DOMAIN     ^^^ */

int   ee_get_num_len(char *eqn);
int   ee_get_next_term_len(char * eqn);
int   ee_choose_new_var(EQUATION_INFO_PTR einfo, int x, int y, int *error);
int   ee_translate_expression(EQUATION_INFO_PTR einfo, char *eqn, int *error);
int   ee_get_num_out(char *eqn, int *error);
int   ee_repl(char rwith, int fnl, char *position, EQUATION_INFO_PTR einfo, int *error);

int   ee_check_for_char(int x, int y, EQUATION_INFO_PTR einfo, int *error);
char *ee_extract_next_term(char *eqn, char *scratch);
char *ee_get_prev_num(char *eqn, int *error);
void  ee_insert_char(char *eqn, int i, char c);
void  ee_replace(char *eqn, int num_chars, int replace_with);

/*
 * NAME:        ee_evaluate_equation
 *
 * PURPOSE:     To evaluate a EQUATION_INFO structure and return a value.
 *
 * USAGE: double ee_evaluate_equation(EQUATION_INFO_PTR einfo, int *error)
 *
 * RETURNS:     The evaluation of the EQUATION_INFO structure.
 *              A return of 0 could be the evaluation, but 0 is also
 *              returned in the event of an error.  If an error has
 *              happened, an error value will be returned in the error
 *              argument.  The only two errors that can be generated in
 *              this function are EE_ERR_DOMAIN, which indicates a domain
 *              error in one of the functions.  The other error is
 *              EE_ERR_MEM_CORRUPT, which indicates that something has
 *              gone wrong with the EQUATION_INFO structure.
 *
 * DESCRIPTION: Basically, this function is a giant switch() statement that
 *                      goes through the EQUATION_INFO structure.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    
 *
 * KEYWORDS: equation
 *
 */ 
#undef ROUTINE_NAME
#define ROUTINE_NAME "ee_evaluate_equation"
 
double ee_evaluate_equation(EQUATION_INFO_PTR einfo, int *error)
{
   double x, y;
   char *s1;
   char *s2;
   int pos = -1;

	assert(einfo);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}
	
	while(1){
		pos++;
		if(einfo->equation[pos] < 'a'){
			if(einfo->equation[pos] < 'A'){
				switch(einfo->equation[pos]){
					case '*': /* multiplication */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = x * y;
						continue;
					case '/': /* division */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						if((y = einfo->eqn_vars[einfo->equation[++pos]]) == 0)
								domain_error;
						einfo->eqn_vars[einfo->equation[++pos]] = x / y;
						continue;
					case '%': /* modulus */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						if((y = einfo->eqn_vars[einfo->equation[++pos]]) == 0)
								domain_error;
						einfo->eqn_vars[einfo->equation[++pos]] = fmod(x, y);
						continue;
					case '+': /* addition */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = x + y;
						continue;
					case '-': /* subtraction */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = x - y;
						continue;
					case '!': /* logical not */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						if(x) einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						else einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						continue;
					case '=': /* All done! */
						return(einfo->eqn_vars[einfo->result]);
					default:
						*(error) = EE_ERR_MEM_CORRUPT;
						return(0); /* memory corrupt */
				}
			}
	
			if(einfo->equation[pos] < 'G'){
				switch(einfo->equation[pos]){
					case 'A': /* exp */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = exp(x);
						continue;
					case 'B': /* log */
						if((x = einfo->eqn_vars[einfo->equation[++pos]]) <= 0)
								domain_error;
						einfo->eqn_vars[einfo->equation[++pos]] = log10(x);
						continue;
					case 'C': /* fac */
						if((x = einfo->eqn_vars[einfo->equation[++pos]]) <= 0)
								domain_error;
						y = 1;
						for(x = floor(x); x > 1; x--) y *= x;
						einfo->eqn_vars[einfo->equation[++pos]] = y;
						continue;
					case 'D': /* deg */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = 180 * x / PI;
						continue;
					case 'E': /* rad */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = PI * x / 180;
						continue;
					case 'F': /* rup */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = ceil(x);
				}
				continue;
			}
	
			if(einfo->equation[pos] < 'M'){
				switch(einfo->equation[pos]){
					case 'G': /* rdn */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = floor(x);
						continue;
					case 'H': /* rnd */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						if(x == 0) einfo->eqn_vars[einfo->equation[++pos]] = 0;
						else einfo->eqn_vars[einfo->equation[++pos]] = floor(x + 0.50);
						continue;
					case 'I': /* sqr */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = x * x;
						continue;
					case 'J': /* ten */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						einfo->eqn_vars[einfo->equation[++pos]] = pow(10, x);
						continue;
					case 'K': /* ln */
						if((x = einfo->eqn_vars[einfo->equation[++pos]]) <= 0)
								domain_error;
						einfo->eqn_vars[einfo->equation[++pos]] = log(x);
						continue;
					case 'L': /* equal to */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						if(x == y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
				}
			}
	
			if(einfo->equation[pos] < 'S'){
				switch(einfo->equation[pos]){
					case 'M': /* less than */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						if(x < y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'N': /* greater than */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						if(x > y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'O': /* and */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						if(x && y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'P': /* or */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						if(x || y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'Q': /* less than or equal to */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						if(x <= y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'R': /* greater than or equal to */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						y = einfo->eqn_vars[einfo->equation[++pos]];
						if(x >= y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
				}
			}
			
			if(einfo->equation[pos] < 'Y'){
				switch(einfo->equation[pos]){
					case 'S': /* less than or equal to, string */
						s1 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						s2 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						if((memStrcmp(s1, s2,NO_TAG)) <= 0)
							einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'T': /* greater than or equal to, string */
						s1 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						s2 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						if((memStrcmp(s1, s2,NO_TAG)) >= 0)
								einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'U': /* equal to, string */
						s1 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						s2 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						if((memStrcmp(s1, s2,NO_TAG)) == 0)
								einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'V': /* less than, string */
						s1 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						s2 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						if((memStrcmp(s1, s2,NO_TAG)) < 0)
								einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'W': /* greater than, string */
						s1 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						s2 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
						if((memStrcmp(s1, s2,NO_TAG)) > 0)
								einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
						else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
						continue;
					case 'X': /* sign */
						x = einfo->eqn_vars[einfo->equation[++pos]];
						if(x == 0) einfo->eqn_vars[einfo->equation[++pos]] = 0;
						else einfo->eqn_vars[einfo->equation[++pos]] = x / fabs(x);
						continue;
				}
			}

			switch(einfo->equation[pos]){
				case 'Y': /* not equal to */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					y = einfo->eqn_vars[einfo->equation[++pos]];
					if(x != y) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
					else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
					continue;
				case 'Z': /* not equal to, string */
					s1 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
					s2 = (char *)((long)einfo->eqn_vars[einfo->equation[++pos]]);
					if((memStrcmp(s1, s2,NO_TAG)) != 0)
							einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
					else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
					continue;
				case '^': /* exponentiation */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					y = einfo->eqn_vars[einfo->equation[++pos]];
					if((x < 0) && (y != ceil(y))) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = pow(x, y);
					continue;
				default:
					*(error) = EE_ERR_MEM_CORRUPT;
					return(0); /* memory corrupt */
			}
		} /* end if < 'a' */

		if(einfo->equation[pos] < 'g'){
			switch(einfo->equation[pos]){
				case 'a': /* acosh */
					if((x = einfo->eqn_vars[einfo->equation[++pos]]) < 1)
							domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] =
					log(x + sqrt(x * x - 1));
					continue;
				case 'b': /* asinh */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					einfo->eqn_vars[einfo->equation[++pos]] =
					log(x + sqrt(x * x + 1));
					continue;
				case 'c': /* atanh */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if((x >= 1) || (x <= -1)) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] =
					log((1 + x) / (1 - x)) / 2;
					continue;
				case 'd': /* asech */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if((x <= 0) || (x > 1)) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] =
					log((1 + sqrt(1 - x * x)) / x);
					continue;
				case 'e': /* acsch */
					if((x = einfo->eqn_vars[einfo->equation[++pos]]) == 0)
							domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] =
					log(((1 / x) + (sqrt(1 + x * x)) / fabs(x)));
					continue;
				case 'f': /* acoth */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if((x <= 1) && (x >= -1)) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] =
					log((x + 1) / (x - 1)) / 2;
			}
			continue;
		}

		if(einfo->equation[pos] < 'm'){
			switch(einfo->equation[pos]){
				case 'g': /* acos */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if((x < -1) || (x > 1)) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = acos(x);
					continue;
				case 'h': /* asin */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if((x < -1) || (x > 1)) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = asin(x);
					continue;
				case 'i': /* atan */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					einfo->eqn_vars[einfo->equation[++pos]] = atan(x);
					continue;
				case 'j': /* asec */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if((x < 1) && (x > -1)) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = acos(1 / x);
					continue;
				case 'k': /* acsc */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if((x < 1) && (x > -1)) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = asin(1 / x);
					continue;
				case 'l': /* acot */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					if(x == 0) domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = atan(1 / x);
			}
			continue;
		}

		if(einfo->equation[pos] < 't'){
			switch(einfo->equation[pos]){
				case 'm': /* cosh */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					einfo->eqn_vars[einfo->equation[++pos]] = cosh(x);
					continue;
				case 'n': /* sinh */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					einfo->eqn_vars[einfo->equation[++pos]] = sinh(x);
					continue;
				case 'o': /* tanh */
					x = einfo->eqn_vars[einfo->equation[++pos]];
					einfo->eqn_vars[einfo->equation[++pos]] = tanh(x);
					continue;
				case 'p': /* sech */
					if((x = cosh(einfo->eqn_vars[einfo->equation[++pos]])) == 0)
							domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = 1 / x;
					continue;
				case 'q': /* csch */
					if((x = sinh(einfo->eqn_vars[einfo->equation[++pos]])) == 0)
							domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = 1 / x;
					continue;
				case 'r': /* coth */
					if((x = tanh(einfo->eqn_vars[einfo->equation[++pos]])) == 0)
					domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = 1 / x;
					continue;
				case 's': /* sqrt */
					if((x = einfo->eqn_vars[einfo->equation[++pos]]) < 0)
					domain_error;
					einfo->eqn_vars[einfo->equation[++pos]] = sqrt(x);
			}
			continue;
		}

		switch(einfo->equation[pos]){
			case 't': /* cos */
				x = einfo->eqn_vars[einfo->equation[++pos]];
				einfo->eqn_vars[einfo->equation[++pos]] = cos(x);
				continue;
			case 'u': /* sin */
				x = einfo->eqn_vars[einfo->equation[++pos]];
				einfo->eqn_vars[einfo->equation[++pos]] = sin(x);
				continue;
			case 'v': /* tan */
				x = einfo->eqn_vars[einfo->equation[++pos]];
				einfo->eqn_vars[einfo->equation[++pos]] = tan(x);
				continue;
			case 'w': /* sec */
				if((x = cos(einfo->eqn_vars[einfo->equation[++pos]])) == 0)
						domain_error;
				einfo->eqn_vars[einfo->equation[++pos]] = 1 / x;
				continue;
			case 'x': /* csc */
				if((x = sin(einfo->eqn_vars[einfo->equation[++pos]])) == 0)
						domain_error;
				einfo->eqn_vars[einfo->equation[++pos]] = 1 / x;
				continue;
			case 'y': /* cot */
				if((x = tan(einfo->eqn_vars[einfo->equation[++pos]])) == 0)
						domain_error;
				einfo->eqn_vars[einfo->equation[++pos]] = 1 / x;
				continue;
			case 'z': /* abs */
				x = einfo->eqn_vars[einfo->equation[++pos]];
				einfo->eqn_vars[einfo->equation[++pos]] = fabs(x);
				continue;
			case '|': /* exclusive or */
				x = einfo->eqn_vars[einfo->equation[++pos]];
				y = einfo->eqn_vars[einfo->equation[++pos]];
				if((x || y) && (!x || !y)) einfo->eqn_vars[einfo->equation[++pos]] = TRUE;
				else einfo->eqn_vars[einfo->equation[++pos]] = FALSE;
				continue;
			default:
				*(error) = EE_ERR_MEM_CORRUPT;
				return(0); /* memory corrupt */
		}
	}
}

/*
 * NAME:        ee_show_err_mesg
 *
 * PURPOSE:     To generate a text error message from the error arguments
 *              returned by the ee_ functions.
 *
 * USAGE:       void ee_show_err_mesg(char *buffer, int error)
 *
 * RETURNS:     void
 *
 * DESCRIPTION: Copies the appropriate error message for error into buffer.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:	buffer must be allocated; an allocation length of 100 should
 *				be more than enough.
 *
 * KEYWORDS: equation
 *
 */
void ee_show_err_mesg(char *buffer, int error)
{
	switch(error){
		case EE_ERR_UNKNOWN:
			memStrcpy(buffer, "Unknown error",NO_TAG);
			break;
		case EE_ERR_ODD_NUM_PARENS:
			memStrcpy(buffer, "Odd number of parenthesis in equation",NO_TAG);
			break;
		case EE_ERR_ODD_NUM_BRACKETS:
			memStrcpy(buffer, "Odd number of brackets in equation",NO_TAG);
			break;
		case EE_ERR_MEM_LACK:
			memStrcpy(buffer, "Out of memory",NO_TAG);
			break;
		case EE_ERR_NO_VARS:
			memStrcpy(buffer, "No variables found in equation",NO_TAG);
			break;
		case EE_ERR_TOO_MANY_VARS:
			memStrcpy(buffer, "Too many variables/constants in equation",NO_TAG);
			break;
		case EE_ERR_DOMAIN:
			memStrcpy(buffer, "Error in function domain",NO_TAG);
			break;
		case EE_ERR_MEM_CORRUPT:
			memStrcpy(buffer, "Memory corrupt",NO_TAG);
			break;
		case EE_ERR_POUND_SIGN:
			memStrcpy(buffer, "Misplaced pound sign in equation",NO_TAG);
			break;
		case EE_ERR_DOLLAR_SIGN:
			memStrcpy(buffer, "Misplaced dollar sign in equation",NO_TAG);
			break;
		case EE_ERR_EQN_BAD:
			memStrcpy(buffer, "Equation bad- cause unknown",NO_TAG);
			break;
		case EE_ERR_ODD_NUM_QUOTES:
			memStrcpy(buffer, "Odd number of quotes in equation",NO_TAG);
			break;
		case EE_ERR_VAR_NAME_BAD:
			memStrcpy(buffer, "Bad variable name",NO_TAG);
			break;
		case EE_ERR_BAD_OP_ON_CHAR:
			memStrcpy(buffer, "Attempted operation on character type",NO_TAG);
			break;
		default:
			memStrcpy(buffer, "Exact error unknown",NO_TAG);
	}
}

/*
 * NAME:        ee_translate_expression
 *
 * PURPOSE:     To translate the "string" form of the equation to a
 *              EQUATION_INFO structure
 *
 * USAGE:
 * int ee_translate_expression(EQUATION_INFO_PTR einfo, char *eqn, int *error)
 *
 * RETURNS:     The position in the EQUATION_INFO->eqn_vars array where
 *              the final evaluation will be placed.
 *
 * DESCRIPTION:
 * This function does a translation on the "string" form of the equation
 * to the eqn_info structure, which is just a step-by-step set of instructions
 * on how to interpret the equation. It returns 0 on error, otherwise, it
 * returns the index of the variable in eqn_info.eqn_vars which the result
 * will be in.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by the ee_clean_up_equation function.
 *
 * KEYWORDS: equation
 *
 */
int ee_translate_expression(EQUATION_INFO_PTR einfo, char *eqn, int *error)
{
   char *position = NULL;
   char *position_two = NULL;
   char scratch_eqn[EE_SCRATCH_EQN_LEN];
   int x, y;

   /*** look for parens, and replace with value in them ***/

  assert(einfo);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}
   
   while ((position = memStrstr(eqn, "(",NO_TAG)) != NULL) {
	 ee_replace(position, ee_get_next_term_len(position),
	       ee_translate_expression(einfo,
		     ee_extract_next_term(position, scratch_eqn), error));
	 if(ee_get_num_out(position, error) <= 0) return(0);
   }

   /*** look for functions, and replace them with their evaluations ***/
   ee_seek_and_replace("acosh", 'a', 5);
   ee_seek_and_replace("asinh", 'b', 5);
   ee_seek_and_replace("atanh", 'c', 5);
   ee_seek_and_replace("asech", 'd', 5);
   ee_seek_and_replace("acsch", 'e', 5);
   ee_seek_and_replace("acoth", 'f', 5);
   ee_seek_and_replace("acos", 'g', 4);
   ee_seek_and_replace("asin", 'h', 4);
   ee_seek_and_replace("atan", 'i', 4);
   ee_seek_and_replace("asec", 'j', 4);
   ee_seek_and_replace("acsc", 'k', 4);
   ee_seek_and_replace("acot", 'l', 4);
   ee_seek_and_replace("cosh", 'm', 4);
   ee_seek_and_replace("sinh", 'n', 4);
   ee_seek_and_replace("tanh", 'o', 4);
   ee_seek_and_replace("sech", 'p', 4);
   ee_seek_and_replace("csch", 'q', 4);
   ee_seek_and_replace("coth", 'r', 4);
   ee_seek_and_replace("sqrt", 's', 4);
   ee_seek_and_replace("sign", 'X', 4);
   ee_seek_and_replace("cos", 't', 3);
   ee_seek_and_replace("sin", 'u', 3);
   ee_seek_and_replace("tan", 'v', 3);
   ee_seek_and_replace("sec", 'w', 3);
   ee_seek_and_replace("csc", 'x', 3);
   ee_seek_and_replace("cot", 'y', 3);
   ee_seek_and_replace("abs", 'z', 3);
   ee_seek_and_replace("exp", 'A', 3);
   ee_seek_and_replace("log", 'B', 3);
   ee_seek_and_replace("fac", 'C', 3);
   ee_seek_and_replace("deg", 'D', 3);
   ee_seek_and_replace("rad", 'E', 3);
   ee_seek_and_replace("rup", 'F', 3);
   ee_seek_and_replace("rdn", 'G', 3);
   ee_seek_and_replace("rnd", 'H', 3);
   ee_seek_and_replace("sqr", 'I', 3);
   ee_seek_and_replace("ten", 'J', 3);
   ee_seek_and_replace("ln", 'K', 2);
   ee_seek_and_replace("!!", '!', 2); /* This is logical not */

	/*** end of function evaluation ***/

	/*** now, do simple operations in order of importance ***/
	if(!ee_replace_op(eqn, "^", '^', 0, ' ', einfo, error)) return(0); /*** exponentiation ***/
    
	while ((position = memStrstr(eqn, "$",NO_TAG)) != NULL) { /* mult, div, mod */
		switch(position[1]) {
			case '*': /*** multiplication ***/
				einfo->equation[einfo->eqn_len++] = '*';
				if(!(position_two = ee_get_prev_num(position, error))) return(0);
				einfo->equation[einfo->eqn_len++] = x =
						ee_get_num_out(position_two, error);
				einfo->equation[einfo->eqn_len++] = y =
						ee_get_num_out(position + 2, error);
				if(ee_check_for_char(x, y, einfo, error)) return(0);
				if((einfo->equation[einfo->eqn_len] =
						ee_choose_new_var(einfo, x, y, error)) == 0)
						return(0); /* error in choose_new_var */
				ee_replace(position_two,
						position - position_two + ee_get_num_len(position + 2) + 2,
						(int)einfo->equation[einfo->eqn_len++]);
				break;
			case '/': /*** division ***/
				einfo->equation[einfo->eqn_len++] = '/';
				if(!(position_two = ee_get_prev_num(position, error))) return(0);
				einfo->equation[einfo->eqn_len++] = x =
						ee_get_num_out(position_two, error);
				einfo->equation[einfo->eqn_len++] = y =
						ee_get_num_out(position + 2, error);
				if(ee_check_for_char(x, y, einfo, error)) return(0);
				if((einfo->equation[einfo->eqn_len] =
						ee_choose_new_var(einfo, x, y, error)) == 0)
						return(0); /* error in choose_new_var */
				ee_replace(position_two,
						position - position_two + ee_get_num_len(position + 2) + 2,
						(int)einfo->equation[einfo->eqn_len++]);
				break;
			case '%': /*** modulus ***/
				einfo->equation[einfo->eqn_len++] = '%';
				if(!(position_two = ee_get_prev_num(position, error))) return(0);
				einfo->equation[einfo->eqn_len++] = x =
						ee_get_num_out(position_two, error);
				einfo->equation[einfo->eqn_len++] = y =
						ee_get_num_out(position + 2, error);
				if(ee_check_for_char(x, y, einfo, error)) return(0);
				if((einfo->equation[einfo->eqn_len] =
						ee_choose_new_var(einfo, x, y, error)) == 0)
						return(0); /* error in choose_new_var */
				ee_replace(position_two,
						position - position_two + ee_get_num_len(position + 2) + 2,
						(int)einfo->equation[einfo->eqn_len++]);
				break;
			default:
				*(error) = EE_ERR_DOLLAR_SIGN;
				return(0); /* misplaced '$' in equation */
		}
	}

	while ((position = memStrstr(eqn, "#",NO_TAG)) != NULL) { /* add, sub */
		switch(position[1]) {
			case '+': /*** addition ***/
				einfo->equation[einfo->eqn_len++] = '+';
				if(!(position_two = ee_get_prev_num(position, error))) return(0);
				einfo->equation[einfo->eqn_len++] = x =
						ee_get_num_out(position_two, error);
				einfo->equation[einfo->eqn_len++] = y =
						ee_get_num_out(position + 2, error);
				if(ee_check_for_char(x, y, einfo, error)) return(0);
				if((einfo->equation[einfo->eqn_len] =
						ee_choose_new_var(einfo, x, y, error)) == 0)
				return(0); /* error in choose_new_var */
				ee_replace(position_two,
						position - position_two + ee_get_num_len(position + 2) + 2,
						(int)einfo->equation[einfo->eqn_len++]);
				break;
			case '-': /*** subtraction ***/
				einfo->equation[einfo->eqn_len++] = '-';
				if(!(position_two = ee_get_prev_num(position, error))) return(0);
				einfo->equation[einfo->eqn_len++] = x =
						ee_get_num_out(position_two, error);
				einfo->equation[einfo->eqn_len++] = y =
						ee_get_num_out(position + 2, error);
				if(ee_check_for_char(x, y, einfo, error)) return(0);
				if((einfo->equation[einfo->eqn_len] =
						ee_choose_new_var(einfo, x, y, error)) == 0)
						return(0); /* error in choose_new_var */
				ee_replace(position_two,
						position - position_two + ee_get_num_len(position + 2) + 2,
						(int)einfo->equation[einfo->eqn_len++]);
				break;
			default:
				*(error) = EE_ERR_POUND_SIGN;
				return(0); /* misplaced '#' in equation */
		}
	}

	if(!ee_replace_op(eqn, "<=", 'Q', 1, 'S', einfo, error)) return(0); /* less than or equal to */
	if(!ee_replace_op(eqn, ">=", 'R', 1, 'T', einfo, error)) return(0); /* greater than or equal to */
	if(!ee_replace_op(eqn, "!=", 'Y', 1, 'Z', einfo, error)) return(0); /* not equal to */
	if(!ee_replace_op(eqn, "=", 'L', 1, 'U', einfo, error)) return(0); /* equal to */
    if(!ee_replace_op(eqn, "<", 'M', 1, 'V', einfo, error)) return(0); /* less than */
    if(!ee_replace_op(eqn, ">", 'N', 1, 'W', einfo, error)) return(0); /* greater than */
    if(!ee_replace_op(eqn, "&", 'O', 0, ' ', einfo, error)) return(0); /* and */
    if(!ee_replace_op(eqn, "x|", '|', 0, ' ', einfo, error)) return(0); /* exclusive or */
    if(!ee_replace_op(eqn, "|", 'P', 0, ' ', einfo, error)) return(0); /* or */

   position = memStrchr(eqn, ']',NO_TAG);
   if(!position){
      *(error) = EE_ERR_EQN_BAD;
      return(0); /* equation bad */
   }
   if(position[1] != '\0'){
      *(error) = EE_ERR_EQN_BAD;
      return(0); /* equation bad */
   }
   /*** The only thing left is now a single number; return it. ***/
   if(eqn[0] != '['){
      *(error) = EE_ERR_EQN_BAD;
      return(0); /* equation bad */
   }
   return (ee_get_num_out(eqn, error));
}

/*
 * NAME:        ee_replace_op
 *
 * PURPOSE:     To replace an operator with its evaluation and update the einfo struct.
 *
 * USAGE:       int ee_replace_op(char *eqn, char *operator, char repwith,
 *					char char_allowed, char crepwith, EQUATION_INFO_PTR einfo, int *error)
 *
 *
 * RETURNS:     1 if all is OK, 0 on error
 *
 * DESCRIPTION:	Searches for operator in eqn, replaces with repwith if variables being
 *				operated on are of numeric type;  If variables are of char type and
 *				char_allowed is true, the replacement is made with crepwith instead.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
int ee_replace_op(char *eqn, char *operator_, char repwith, char char_allowed, char crepwith, EQUATION_INFO_PTR einfo, int *error)
{
	char *position = NULL;
	char *position_two = NULL;
	char oplen = 1;
	int v, w, x, y, z;
	
	assert(einfo);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}

        oplen = strlen(operator_);

	while ((position = (char *)memStrstr(eqn, operator_,NO_TAG)) != NULL) { /* Check for operator */
		z = einfo->eqn_len;
		einfo->equation[einfo->eqn_len++] = repwith;
		if(!(position_two = ee_get_prev_num(position, error))) return(0);
		einfo->equation[einfo->eqn_len++] = x =
				ee_get_num_out(position_two, error);
		einfo->equation[einfo->eqn_len++] = y =
				ee_get_num_out(position + oplen, error);
		if((x < 0) || (y < 0)) return(0);
		if(char_allowed){ /* character data types allowed */
			v = w = EE_VAR_TYPE_NUMERIC;
			if(x < (int)einfo->result) v = einfo->variable_type[x];
			if(y < (int)einfo->result) w = einfo->variable_type[y];
			if(v == w){
				if(v == EE_VAR_TYPE_CHAR) einfo->equation[z] = crepwith;
			}
			else{
				*(error) = EE_ERR_BAD_OP_ON_CHAR;
				return(0);
			}
		}
		else{ /* Character data types not allowed */
			if(ee_check_for_char(x, y, einfo, error)) return(0);
		}
		if((einfo->equation[einfo->eqn_len] =
				ee_choose_new_var(einfo, x, y, error)) == 0) return(0);
		ee_replace(position_two,
				position - position_two + ee_get_num_len(position + oplen) + oplen,
				(int)einfo->equation[einfo->eqn_len++]);
	}
	return(1); /* all is OK */
}

/*
 * NAME:        ee_check_for_char
 *
 * PURPOSE:     To make sure that the variables in the equation which
 *              were passed were not character type variables
 *
 * USAGE:       int ee_check_for_char(int x, int y, 
 *								EQUATION_INFO_PTR einfo, int *error)
 *
 * RETURNS:     0 if all is OK, 1 on error
 *
 * DESCRIPTION:	Checks to make sure that variable x and y are not char
 *				type.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
int ee_check_for_char(int x, int y, EQUATION_INFO_PTR einfo, int *error)
{
	assert(einfo);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}

	if((x < 0) || (y < 0)) return(1);
	if(x < (int)einfo->result){ 
		if(einfo->variable_type[x] == EE_VAR_TYPE_CHAR){
			*(error) = EE_ERR_BAD_OP_ON_CHAR;
			return(1);
		}
		einfo->variable_type[x] = EE_VAR_TYPE_NUMERIC;
	}
	if(y < (int)einfo->result){ 
		if(einfo->variable_type[y] == EE_VAR_TYPE_CHAR){
			*(error) = EE_ERR_BAD_OP_ON_CHAR;
			return(1);
		}
		einfo->variable_type[y] = EE_VAR_TYPE_NUMERIC;
	}
	return(0);
}

/*
 * NAME:        ee_repl
 *
 * PURPOSE:     To replace the fnl bytes of position with an evaluation
 *              and update the EQUATION_INFO structure
 *
 * USAGE:       int ee_repl(char rwith, int fnl, char *position,
 *                              EQUATION_INFO_PTR einfo, int *error)
 *
 * RETURNS:     0 if all is OK, 1 on error
 *
 * DESCRIPTION:
 * This takes the function name at position, fnl characters long, and
 * inserts it into the eqn_info struct as a call to function represented
 * by rwith.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
int ee_repl(char rwith, int fnl, char *position, EQUATION_INFO_PTR einfo, int *error)
{
	int x;

	assert(einfo);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}

	einfo->equation[einfo->eqn_len++] = rwith;
	einfo->equation[einfo->eqn_len++] = x =
			ee_get_num_out(position + fnl, error);
	if(x < 0) return(0);
	if(x < (int)einfo->result){ /* could be a variable or string constant */
    	if(einfo->variable_type[x] == EE_VAR_TYPE_CHAR){
    		*(error) = EE_ERR_BAD_OP_ON_CHAR;
    		return(0);
    	}
    	einfo->variable_type[x] = EE_VAR_TYPE_NUMERIC;
	}
	if((einfo->equation[einfo->eqn_len] = ee_choose_new_var(einfo, x, 0, error))
			== 0) return(0); /* error in choose_new_var */
	ee_replace(position, fnl + ee_get_num_len(position + fnl),
			(int)einfo->equation[einfo->eqn_len++]);
	return(1);
}

/*
 * NAME:        ee_get_prev_num
 *
 * PURPOSE:     To return a pointer to the number just before eqn
 *
 * USAGE:       char *ee_get_prev_num(char *eqn, int *error)
 *
 * RETURNS:     A pointer to the previous number, else NULL on error
 *
 * DESCRIPTION:
 * This searches for the number in the expression occuring immediately before
 * the pointer passed to it, and returns a pointer to it.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *                      only by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
char *ee_get_prev_num(char *eqn, int *error)
{
   int i;

   if(*(eqn - 1) != ']'){
      *(error) = EE_ERR_EQN_BAD;
      return(NULL);
   }
   for (i = 0; i < 1; i--)
      if (eqn[i] == '[') return (eqn + i);

   return(NULL);
}

/*
 * NAME:        ee_replace
 *
 * PURPOSE:     To replace num_cars of eqn with replace_with
 *
 * USAGE:       void ee_replace(char *eqn, int num_chars, int replace_with)
 *
 * RETURNS:     void
 *
 * DESCRIPTION:
 * This replaces num_chars characters starting at the beginning of eqn with
 * a string replacement of replace_with
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
void ee_replace(char *eqn, int num_chars, int replace_with)
{
   char scratch[EE_SCRATCH_EQN_LEN];

   memStrcpy(scratch, eqn + num_chars,NO_TAG);
   eqn[0] = '[';

   sprintf(eqn + 1, "%d", replace_with);
   eqn[strlen(eqn) + 1] = '\0';
   eqn[strlen(eqn)] = ']';

   memStrcat(eqn, scratch,NO_TAG);
}

/*
 * NAME:        ee_get_next_term_len
 *
 * PURPOSE:     To return the length in character of the next term.
 *
 * USAGE:       int ee_get_next_term_len(char * eqn)
 *
 * RETURNS:     The length, in characters, of the next term.
 *
 * DESCRIPTION:
 * This returns the length of the term enclosed by the parenthesis.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
int ee_get_next_term_len(char * eqn)
{
	int paren_count = 1;
	int i;

	if (eqn[0] != '(') return strlen(eqn);

	eqn++;

	for (i = 0; i < (signed int)strlen(eqn); i++) {
		if (eqn[i] == '(') paren_count++;
		if (eqn[i] == ')') paren_count--;
		if (paren_count == 0) {
			return (i + 2);
		}
	}

        return(0);
}

/*
 * NAME:        ee_extract_next_term
 *
 * PURPOSE:     To copy the next term into scratch.
 *
 * USAGE:       char *ee_extract_next_term(char *eqn, char *scratch)
 *
 * RETURNS:     A pointer to scratch.
 *
 * DESCRIPTION:
 * This copies the next term inside the parenthesis from eqn to scratch
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
char *ee_extract_next_term(char *eqn, char *scratch)
{
	int paren_count = 1;
	int i;

	if (eqn[0] != '(') return (eqn);

	eqn++;

	for (i = 0; i < (signed int)strlen(eqn); i++) {
		if (eqn[i] == '(') paren_count++;
		if (eqn[i] == ')') paren_count--;
		if (paren_count == 0) {
			scratch[i] = '\0';
			return (scratch);
		}
		scratch[i] = eqn[i];
	}

        return(NULL);
}

/*
 * NAME:        ee_choose_new_var
 *
 * PURPOSE:
 * This checks various things in the einfo structure, as well as determining
 * the best position in the variable array to store the result in.
 *
 * USAGE:
 * int ee_choose_new_var(EQUATION_INFO_PTR einfo, int x, int y, int *error)
 *
 * RETURNS:     0 on error; non-zero if no errors.
 *
 * DESCRIPTION:
 * This checks various things in the einfo structure, as well as determining
 * the best position in the variable array to store the result in. returns
 * 0 on error
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should be called
 *              only by ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
int ee_choose_new_var(EQUATION_INFO_PTR einfo, int x, int y, int *error)
{
	char *tmp_ptr;
	
	assert(einfo);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}

	/* Check to see if equation needs to be bigger */
	if((einfo->eqn_len + 6) > einfo->ee_scratch_int){ /* enlarge equation */
		einfo->ee_scratch_int += 20;
		if(!(einfo->equation = (unsigned char *)memRealloc(einfo->equation,
				(size_t)einfo->ee_scratch_int, "ee_choose_new_var: einfo->equation"))){
			*(error) = EE_ERR_MEM_LACK;
			return(0); /* out of memory */
		}
	}
	tmp_ptr = (char *)einfo->variable_ptr[0];
	
	if(x >= (int)einfo->numconst + (int)einfo->num_vars){
		if(y >= (int)einfo->numconst + (int)einfo->num_vars){ 
			/* y is also already a work var; change its status so that it could be reused */
			tmp_ptr[y] = 0;
		}	
		return(x);
	}
	if(y >= (int)einfo->numconst + (int)einfo->num_vars) return(y);

	/* Check to see if we can recycle a variable */
	for(y = (int)einfo->numconst + (int)einfo->num_vars; y < (int)einfo->num_work; y++)
		if(tmp_ptr[y] == 0){
			tmp_ptr[y] = 1;
			return(y);
		}
		
	x = einfo->num_work++;
	tmp_ptr[x] = 1;
	
	if(einfo->num_work >= 240){
		*(error) = EE_ERR_TOO_MANY_VARS;
		return(0); /* exceeded max. # of variables */
	}
	return(x);
}

/*
 * NAME:        ee_clean_up_equation
 *
 * PURPOSE:     To clean up an equation and return a generated
 *              EQUATION_INFO structure.
 *
 * USAGE:       EQUATION_INFO_PTR ee_clean_up_equation(char *eqn, int *error)
 *
 * RETURNS:     NULL on error (with error being the actual error) or a
 *              pointer to the EQUATION_INFO structure it created.
 *
 * DESCRIPTION:
 * This removes all spaces from the equation, makes everything lowercase
 * and places [] around all the numbers. It then translates the equation
 * into the eqn_info structure, and returns a pointer to this structure.
 * The passed char *eqn must have some buffer space on the end, and the
 * function will destroy this copy.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function assumes that eqn has enough buffer room on
 *              the end, and eqn will be destroyed by this function.
 *
 * KEYWORDS: equation
 *
 */
EQUATION_INFO_PTR ee_clean_up_equation(char *eqn, int *error)
{
	int i, j, k;
	unsigned char num_vars = 0;
	unsigned char num_const = 0;
	unsigned char num_string = 0;
	char *position = NULL;
	char **var_list	= NULL;	/* The list of variable names */
	char **str_list	= NULL;	/* A list of string constants */
	EQUATION_INFO_PTR einfo = NULL;
	char var_name[EE_MAX_VAR_NAME_LENGTH];
	char scratch[EE_SCRATCH_EQN_LEN];
	char inside_variable_name = 0;
	char var_exists = 0;
	double x;
	void **allocated_list = NULL; /* This is where we store what has been allocated */
	int num_allocated = 0;

	*(error) = 0;

	/* get a tentative count of things */
	for(i = 0; i < (signed int)strlen(eqn); i++){
		if(eqn[i] == '[') num_vars++;
		if(eqn[i] == '\"') num_string++;
	}

	if(!num_vars){
		*(error) = EE_ERR_NO_VARS;
		return(NULL);
	}

	if(num_string % 2){
		*(error) = EE_ERR_ODD_NUM_QUOTES;
		return(NULL);
	}
	num_string /= 2;
	
	/* NEVER insert any allocations between this one and the allocation for var_list */
	if(!(allocated_list = (void **)memMalloc((size_t)(sizeof(void *) * (num_vars + num_string + 20)), "ee_clean_up_equation: allocated_list"))){
		*(error) = EE_ERR_MEM_LACK;
		return(NULL);
	}
	allocated_list[num_allocated++] = (void *)allocated_list;	

	if(!(var_list = (char **)memMalloc((size_t)(sizeof(char *) * num_vars), "ee_clean_up_equation: var_list"))){
		*(error) = EE_ERR_MEM_LACK;
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL);
	}
	allocated_list[num_allocated++] = (void *)var_list;
	/* IT IS _VERY_ IMPORTANT that var_list ALWAYS be the 2nd position in the
	 * allocated_list array.  It gets realloced and reset later on. */

	if(num_string){
		if(!(str_list = (char **)memMalloc((size_t)(sizeof(char *) * num_string), "ee_clean_up_equation: str_list"))){
			*(error) = EE_ERR_MEM_LACK;
			for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
			return(NULL);
		}
		allocated_list[num_allocated++] = (void *)str_list;
	}

	num_vars = 0;
	num_string = 0;
	/* Extract variable names and string constants */
	for(i = 0; i < (signed int)strlen(eqn); i++){
		if(eqn[i] == '['){ /* variable name */

			for(j = i; j < (signed int)strlen(eqn); j++){
				if(eqn[j] == ']') break;
			}
			if((j == (signed int)strlen(eqn)) || (j == (i + 1))){
				*(error) = EE_ERR_VAR_NAME_BAD;
				for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
				return(NULL);
			}
			if((j - i) >= EE_MAX_VAR_NAME_LENGTH){
				*(error) = EE_ERR_VAR_NAME_BAD;
				for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
				return(NULL);
			}

			k = 0;
			for(j = (i + 1); j < (signed int)strlen(eqn); j++) if(eqn[j] != ' ') break;
			for(; j < (signed int)strlen(eqn); j++){
				if(eqn[j] != ']')
					var_name[k++] = eqn[j];
				else
					break;
			}
			for(; k > 0; k--) if(var_name[k - 1] != ' ') break;
			var_name[k] = '\0';
			var_exists = 0;
			
			if(!strlen(var_name)){
				*(error) = EE_ERR_VAR_NAME_BAD;
				for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
				return(NULL);
			}

			for(k = 0; k < (int)num_vars; k++){
				if(!memStrcmp(var_name, var_list[k],NO_TAG)){ /* Variable already in */
					var_exists = 1;
					break;
				}
			}

			if(!var_exists){
				if(!(var_list[num_vars] =
							(char *)memMalloc((size_t)(strlen(var_name) + 2), "ee_clean_up_equation: var_list[num_vars]"))){
					*(error) = EE_ERR_MEM_LACK;
					for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
					return(NULL);
				}
				allocated_list[num_allocated++] = (void *)var_list[num_vars];
				k = num_vars;
				memStrcpy(var_list[num_vars++], var_name,NO_TAG);
			}

			ee_replace(eqn + i, j - i + 1, k);
		}

		if(eqn[i] == '\"'){ /* String constant */
			k = 0;
			var_name[0] = '\0';

			for(j = (i + 1); j < (signed int)strlen(eqn); j++){
				if(eqn[j] == '\"'){
					if(eqn[j + 1] == '\"'){
						var_name[k++] = '\"';
						j++;
						continue;
					}
					else{
						memStrcpy(scratch, eqn + j,NO_TAG);
						break; /* end of string */
					}
				}
				else{
					var_name[k++] = eqn[j];
				}

				if(k >= EE_MAX_VAR_NAME_LENGTH){
					*(error) = EE_ERR_VAR_NAME_BAD;
					for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
					return(NULL);
				}
			}
			var_name[k] = '\0';

			if(!strlen(var_name)){
				*(error) = EE_ERR_VAR_NAME_BAD;
				for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
				return(NULL);
			}
			var_exists = 0;

			for(j = 0; j < (int)num_string; j++){
				if(!memStrcmp(var_name, str_list[j],NO_TAG)){ /* constant exists */
					var_exists = 1;
					break;
				}
			}

			if(!var_exists){
				if(!(str_list[num_string] =
						(char *)memMalloc((size_t)(strlen(var_name) + 2), "ee_clean_up_equation: str_list[num_string]"))){
					*(error) = EE_ERR_MEM_LACK;
					for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
					return(NULL);
				}
				allocated_list[num_allocated++] = (void *)str_list[num_string];
				j = num_string;
				memStrcpy(str_list[num_string++], var_name,NO_TAG);
			}

			sprintf(eqn + i + 1, "%d", j);
			memStrcat(eqn, scratch,NO_TAG);
			for(i++; eqn[i] != '\"'; i++);
		}
	} /* end of variable and string constant extraction */

	for(j = 0, i = 0; i < (signed int)strlen(eqn); i++)
		if(eqn[i] == '(') j++; else if(eqn[i] == ')') j--;
	if(j != 0){
		*(error) = EE_ERR_ODD_NUM_PARENS;
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* Odd number of parens */
	}

	for(j = 0, i = 0; i < (signed int)strlen(eqn); i++)
		if(eqn[i] == '[') j++; else if(eqn[i] == ']') j--;
	if(j != 0){
		*(error) = EE_ERR_ODD_NUM_BRACKETS;
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* Odd number of brackets */
	}

	if(num_vars == 0){
		*(error) = EE_ERR_NO_VARS;
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL);
	}

	if(!(var_list = (char **)memRealloc(var_list, (size_t)(num_vars * sizeof(char *)), "ee_clean_up_equation: var_list"))){
		*(error) = EE_ERR_UNKNOWN;
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* ??? Weird goings on */
	}
	allocated_list[1] = (void *)var_list; /* var_list IS ALWAYS THE 2nd POS IN THIS ARRAY */

	/* strip out spaces, make everything lowercase */
	for (i = 0; i < (signed int)strlen(eqn); i++) {
		eqn[i] = tolower(eqn[i]);
		while(eqn[i] == ' ') {
			for (j = i; eqn[j] != '\0'; j++)
				eqn[j] = eqn[j + 1];
		}
	}
	i = 0;
	
	position = eqn; /* Replace "not" with "!" */
	while(position = memStrstr(position, "not",NO_TAG)){
		position[0] = '!';
		position[1] = ' ';
		position[2] = ' ';
		i++;
	}
	position = eqn; /* Replace "and" with "&" */
	while(position = memStrstr(position, "and",NO_TAG)){
		position[0] = '&';
		position[1] = ' ';
		position[2] = ' ';
		i++;
	}
	position = eqn; /* Replace "xor" with "x|" */
	while(position = memStrstr(position, "xor",NO_TAG)){
		position[0] = 'x';
		position[1] = '|';
		position[2] = ' ';
		i++;
	}
	position = eqn; /* Replace "or" with "|" */
	while(position = memStrstr(position, "or",NO_TAG)){
		position[0] = '|';
		position[1] = ' ';
		i++;
	}
	position = eqn; /* Replace "||" with "|" */
	while(position = memStrstr(position, "||",NO_TAG)){
		position[0] = '|';
		position[1] = ' ';
		i++;
	}
	position = eqn; /* Replace "&&" with "&" */
	while(position = memStrstr(position, "&&",NO_TAG)){
		position[0] = '&';
		position[1] = ' ';
		i++;
	}
	position = eqn; /* Replace "==" with "=" */
	while(position = memStrstr(position, "==",NO_TAG)){
		position[0] = '=';
		position[1] = ' ';
		i++;
	}
	
	if(i){ /* We introduced spaces in again */
		/* strip out spaces */
		for (i = 0; i < (signed int)strlen(eqn); i++) {
			while(eqn[i] == ' ') {
				for (j = i; eqn[j] != '\0'; j++)
					eqn[j] = eqn[j + 1];
			}
		}
	}
	
	position = eqn;
	while(position = memStrstr(position, "~[",NO_TAG)){
		position[0] = '(';
		ee_insert_char(++position, 0, '~');
		ee_insert_char(++position, 0, '1');
		ee_insert_char(++position, 0, '*');
		while(position[0] != ']') position++;
		ee_insert_char(++position, 0, ')');
	}
	position = eqn;
	while(position = memStrstr(position, "~(",NO_TAG)){
		ee_insert_char(++position, 0, '1');
		ee_insert_char(++position, 0, '*');
	}

	position = eqn; /* Replace "<>" with "!=" */
	while(position = memStrstr(position, "<>",NO_TAG)){
		position[0] = '!';
		position[1] = '=';
	}
	position = eqn; /* Replace "><" with "!=" */
	while(position = memStrstr(position, "><",NO_TAG)){
		position[0] = '!';
		position[1] = '=';
	}
	position = eqn; /* Replace "=<" with "<=" */
	while(position = memStrstr(position, "=<",NO_TAG)){
		position[0] = '<';
		position[1] = '=';
	}
	position = eqn; /* Replace "=>" with ">=" */
	while(position = memStrstr(position, "=>",NO_TAG)){
		position[0] = '>';
		position[1] = '=';
	}

	position = eqn;	/* Replace ! with !! */
	while(position = memStrstr(position, "!",NO_TAG)){
		if(position[1] == '='){
			position++;
			continue;
		}
		ee_insert_char(++position, 0, '!');
		position++;
	}

	num_const = num_string + 10;

	if(!(einfo = (EQUATION_INFO_PTR)memMalloc(sizeof(EQUATION_INFO), "ee_clean_up_equation: einfo"))){
		*(error) = EE_ERR_MEM_LACK;
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* out of memory */
	}
	allocated_list[num_allocated++] = (void *)einfo;

	einfo->check_address = einfo;
	if(!(einfo->equation = (unsigned char *)memMalloc((size_t)EE_INIT_EQNL, "ee_clean_up_equation: einfo->equation"))){
		*(error) = EE_ERR_MEM_LACK;
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* out of memory */
	}
	/* einfo->equation is not added to allocated_list for a reason */
    
	/* set to number of bytes in equation str */
	einfo->ee_scratch_int = EE_INIT_EQNL;
	einfo->eqn_len = 0;
	einfo->num_work = num_vars + num_const;
	if(!(einfo->eqn_vars =
			(double *)memMalloc((size_t)(einfo->num_work * sizeof(double)), "ee_clean_up_equation: einfo->eqn_vars"))){
		*(error) = EE_ERR_MEM_LACK;
		memFree(einfo->equation, "ee_clean_up_equation: einfo->equation");
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* out of memory */
	}
	/* This pointer is not stored in allocated_list for a reason */
	
	if(!(einfo->variable_type = 
			(unsigned char *)memMalloc((size_t)(num_vars + num_string), "ee_clean_up_equation: einfo->variable_type"))){
		*(error) = EE_ERR_MEM_LACK;
		memFree(einfo->eqn_vars, "ee_clean_up_equation: einfo->eqn_vars");
		memFree(einfo->equation, "ee_clean_up_equation: einfo->equation");
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL);
	}
	allocated_list[num_allocated++] = (void *)einfo->variable_type;
	
	if(!(einfo->variable_ptr = (void **)memMalloc((size_t)(num_vars * sizeof(void *)), "ee_clean_up_equation: einfo->variable_ptr"))){
		*(error) = EE_ERR_MEM_LACK;
		memFree(einfo->eqn_vars, "ee_clean_up_equation: einfo->eqn_vars");
		memFree(einfo->equation, "ee_clean_up_equation: einfo->equation");
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* out of memory */
	}
	allocated_list[num_allocated++] = (void *)einfo->variable_ptr;
	
	einfo->num_vars = num_vars;
	einfo->numconst = num_const;
	einfo->num_strc = num_string;
	einfo->variable = var_list;
	einfo->result = num_vars + num_string;  /* This is a temporary setting */

	num_const = num_string;

	for(i = 0; i < (int)einfo->num_vars; i++){
		if(einfo->variable[i][0] == '$'){ /* String type variable */
			for(j = 0; j < (signed int)strlen(einfo->variable[i]); j++)
				einfo->variable[i][j] = einfo->variable[i][j + 1];
			einfo->variable_type[i] = EE_VAR_TYPE_CHAR;
			einfo->eqn_vars[i] = (double)((long)((char *)NULL));
		}
		else{
			einfo->variable_type[i] = EE_VAR_TYPE_NUMERIC;
		}
	}
	for(; i < ((int)num_vars + (int)num_string); i++)
		einfo->variable_type[i] = EE_VAR_TYPE_UNKNOWN;

	/* Replace string constant references */
	while(position = memStrchr(eqn, '\"',NO_TAG)){
		for(i = 1; i < (signed int)strlen(position); i++) if(position[i] == '\"') break;
		j = atoi(position + 1);
		k = j;
		j = j + num_vars;
		einfo->eqn_vars[j] = (double)((long)str_list[k]);
		einfo->variable_type[j] = EE_VAR_TYPE_CHAR;
		ee_replace(position, ++i, j);
	}
	if(num_string) memFree(str_list, "ee_clean_up_equation: str_list");

	while ((position = memStrstr(eqn, ":pi",NO_TAG)) != NULL){ /* replace pi with 3.14 */
		memStrcpy(scratch, position + 3,NO_TAG);
		sprintf(position, "%e", PI);
		memStrcat(position, scratch,NO_TAG);
	}

	while ((position = memStrstr(eqn, ":e",NO_TAG)) != NULL){ /* replace e with 2.718 */
		memStrcpy(scratch, position + 2,NO_TAG);
		sprintf(position, "%e", E);
		memStrcat(position, scratch,NO_TAG);
	}

	/* Replace numeric constants */
	inside_variable_name = 0;
	for (i = 0; i < (signed int)strlen(eqn); i++) {
		if(eqn[i] == '[') inside_variable_name = 1;
		if(eqn[i] == ']') inside_variable_name = 0;
		if(((isdigit(eqn[i]) != 0) || (eqn[i] == '.') || (eqn[i] == '~')) &&
				!inside_variable_name) {
			if(eqn[i] == '~') eqn[i] = '-';
			j = i;
			ee_insert_char(eqn, j++, '[');
			if(eqn[j] == '-') j++;
			while ((isdigit(eqn[j]) != 0) || (eqn[j] == '.')) j++;
			ee_insert_char(eqn, j, ']');
			x = atof(eqn + i + 1);
			var_exists = 0;
			for(j = (int)num_vars + (int)num_string; j < (int)num_vars + (int)num_const; j++){
				if(einfo->eqn_vars[j] == x){ /* This constant already exists */
					var_exists = 1;
					break;
				}
			}
			if(!var_exists){
				j = num_vars + num_const;
				einfo->eqn_vars[j] = x;
				num_const++;
			}
			ee_replace(eqn + i, ee_get_num_len(eqn + i), j);
			while(eqn[i] != ']') i++;
			if(einfo->numconst <= num_const){
				/* reallocate the array */
				einfo->num_work += 10;
				einfo->numconst += 10;
				if(!(einfo->eqn_vars =
						(double *)memRealloc(einfo->eqn_vars,
						(size_t)(einfo->num_work * sizeof(double)), "ee_clean_up_equation: einfo->eqn_vars"))){
					*(error) = EE_ERR_MEM_LACK;
					memFree(einfo->equation, "ee_clean_up_equation: einfo->equation");
					for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
					return(NULL); /* out of memory */
				}
			}
		}
	}
	einfo->numconst = num_const;
    einfo->num_work = num_const + num_vars;

	position = eqn;
	while ((position = memStrstr(position, "*",NO_TAG)) != NULL){ /* replace * with $* */
		ee_insert_char(position, 0, '$'); position += 2;}
	position = eqn;
	while ((position = memStrstr(position, "/",NO_TAG)) != NULL){ /* replace / with $/ */
		ee_insert_char(position, 0, '$'); position += 2;}
	position = eqn;
	while ((position = memStrstr(position, "%",NO_TAG)) != NULL){ /* replace % with $% */
		ee_insert_char(position, 0, '$'); position += 2;}

	position = eqn;
	while ((position = memStrstr(position, "+",NO_TAG)) != NULL){ /* replace + with #+ */
		ee_insert_char(position, 0, '#'); position += 2;}
	position = eqn;
	while ((position = memStrstr(position, "-",NO_TAG)) != NULL){ /* replace - with #- */
		ee_insert_char(position, 0, '#'); position += 2;}
	/*
	while ((position = memStrstr(eqn, "~",NO_TAG)) != NULL) 
		position[0] = '-';
	*/

	/* einfo->variable_ptr[0] is used as a scratch buffer for ee_choose_new_var to keep
	 * track of the variables that have been previously used and can be recycled.  It
	 * is set here to 1's so that no possible error could occur in recycling a non-work
	 * variable. */
	memMemset((void *)scratch, 1, (size_t)256, "scratch,1,256");
	einfo->variable_ptr[0] = (void *)scratch;

	if(!(einfo->result = ee_translate_expression(einfo, eqn, error))){
		memFree(einfo->eqn_vars, "ee_clean_up_equation: einfo->eqn_vars");
		if(einfo->equation) memFree(einfo->equation, "ee_clean_up_equation: einfo->equation");
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL);
	}

	if(!(einfo->eqn_vars = (double *)memRealloc(einfo->eqn_vars,
			(size_t)(einfo->num_work * sizeof(double)), "ee_clean_up_equation: einfo->eqn_vars"))){
		*(error) = EE_ERR_MEM_LACK;
		memFree(einfo->equation, "ee_clean_up_equation: einfo->equation");
        for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* out of memory */
	}

	einfo->equation[einfo->eqn_len++] = '=';
	if(!(einfo->equation = (unsigned char *)memRealloc(einfo->equation,
			(size_t)einfo->eqn_len, "ee_clean_up_equation: einfo->equation"))){
		*(error) = EE_ERR_UNKNOWN;
		memFree(einfo->eqn_vars, "ee_clean_up_equation: einfo->eqn_vars");
		for(i = --num_allocated; i >= 0; i--) memFree(allocated_list[i], "ee_clean_up_equation: allocated_list[i]");
		return(NULL); /* ??? error unknown */
	}

/* The following memory block is referenced later -- cannot be free'd here
	memFree(var_list, "ee_clean_up_equation: var_list");
*/
	memFree(allocated_list, "ee_clean_up_equation: allocated_list");

	return(einfo); /* all is well */
}

/*
 * NAME:        ee_insert_char
 *
 * PURPOSE:     To insert c into eqn at position i
 *
 * USAGE:       void ee_insert_char (char *eqn, int i, char c)
 *
 * RETURNS:     void
 *
 * DESCRIPTION: inserts c into eqn at position i
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    
 *
 * KEYWORDS: equation
 *
 */
void ee_insert_char (char *eqn, int i, char c)
{
   int j;

   for (j = strlen(eqn) + 1; j > i - 1; j--)
      eqn[j + 1] = eqn[j];

   eqn[i] = c;
}

/*
 * NAME:        ee_get_num_out
 *
 * PURPOSE:     To return the number in eqn
 *
 * USAGE:       int ee_get_num_out(char *eqn, int *error)
 *
 * RETURNS:     the number, or -1 on error
 *
 * DESCRIPTION: Returns the number in eqn, which is enclosed in []
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should only be
 *              called by ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
int ee_get_num_out(char *eqn, int *error)
{
   if(eqn[0] != '['){
      *(error) = EE_ERR_EQN_BAD;
      return(-1);
   }
   return (atoi(eqn + 1));
}

/*
 * NAME:        ee_get_num_len
 *
 * PURPOSE:     To return the length of the number in eqn
 *
 * USAGE:       int ee_get_num_len(char *eqn)
 *
 * RETURNS:     The length of the number in eqn
 *
 * DESCRIPTION: returns the length of the number enclosed in [] in eqn
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function is not "user-callable", it should only be
 *              called by the ee_ functions.
 *
 * KEYWORDS: equation
 *
 */
int ee_get_num_len(char *eqn)
{
   int i;

   for (i = 0; i < (signed int)strlen(eqn); i++)
      if (eqn[i] == ']') return (i + 1);

   return(0);
}

