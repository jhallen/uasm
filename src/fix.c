#include <stdio.h>

int main()
{
                char buf[1024];
                while (gets(buf))
            {
                                int x;
                                for (x = 0; buf[x] == ' '; ++x)	;
                                if (buf[x] == '{' || buf[x] == '}')
                            {
                                                int y;
                                                for (y = 0; y < x - 1; ++y)
                                                                printf("    ");
                            }
                                else
                            {
                                                int y;
                                                for (y = 0; y != x; ++y)
                                                                printf("    ");
                            }
                                printf("%s\n", buf + x);
            }
                return 0;
}
