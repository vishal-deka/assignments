#include <stdio.h>

int main()
{
   // Write C code here
   int a[100];
   for (int i = 0; i < 100; i++)
   {
      a[i] = i;
   }
   printf("%d\n", a[99]);
   for (int j = 0; j < 3; j++)
   {
      for (int i = 1; i < 100; i++)
      {
         a[i] += a[i - 1];
      }
   }
   printf("%d\n", a[99]);
   for (int i = 0; i < 100; i++)
   {
      a[i] = i;
   }

   for (int i = 1; i < 100; i++)
   {
      for (int j = 0; j < 3; j++)
      {
         a[i] = a[i-1];
      }
      a[i]+=a[i];
   }

   printf("%d\n", a[99]);

   return 0;
}