#include <stdio.h>
/* comment */
int main(void)
{
    int a = 0x1F, b = 017;
    float f = 3.14bgx; // error
    char c = 'A';
    char *s = "hi";
    if (a >= b && a != 0)   // trailing comment
        a++;
    return 0;
}