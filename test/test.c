#include <unistd.h>

int main()
{
        int a ;
        for (int i = 0; i < 15; i++)
        {
                a = fork();
                if (!a)
                        while(1);
                else
                        continue;
        }

        while(1);
        return 0;
}
                
