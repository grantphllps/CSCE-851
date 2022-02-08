/****************************************************************
 *
 * Author: Justin Bradley
 * Title: foo.c
 * Date: Tuesday, February  2, 2021
 * Description: implementation of foo.h (print_msg) 
 *
 ****************************************************************/
#include <stdio.h>
#include "foo.h"

int print_msg(char * msg)
{
		printf("%s", msg);
		return 0;
}
