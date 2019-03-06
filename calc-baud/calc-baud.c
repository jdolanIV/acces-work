#include <stdlib.h>
#include <stdio.h>




int main (int argc, char **argv)
{
    int target_baud;
    int scr;
    int actual_baud;
    int tolerance;

    if (2 == argc)
    {
        target_baud = strtol(argv[1], NULL, 0);
    }
    else
    {
        target_baud = 3E6;
    }
    
    for (int scr = 15 ; scr >= 5; scr--)
    {
        actual_baud = 921600 * 16 / scr;
        tolerance = actual_baud / 50;
        printf("actual_baud = %d, target_baud = %d tolerance = %d\n", 
                actual_baud, target_baud, tolerance);
        if (( target_baud < actual_baud + tolerance ) &&
            ( target_baud > actual_baud - tolerance ))
            {
                printf("Accept with scr = %d\n", scr);
                break;
            }
        else if (target_baud < actual_baud)
        {
            printf("Bail at scr = %d\n", scr);
            break;
        }

    }


}