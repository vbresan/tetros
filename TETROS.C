#include <stdio.h>
#include <string.h>


/****************************************************************************/
/* type declarations
*/

struct tetrominoe
 { char       * body;
   unsigned int width,
                height;
 };


/****************************************************************************/
/* function prototypes
*/

unsigned int bits_num(void);
int          is_valid_comb(void);

void         select_tetros(void);
int          ini_matrix(void);
void         place_tetros(void);

int          align(char * str, unsigned int line);
int          overlap(char * str, unsigned int line);
void         move_right(char * str);
void         move_down(char * str);
void         get_moves(char * str, int * right, int * down);
void         print_tetros(void);
int          not_known(char * str);


/****************************************************************************/
/* globals
*/

struct tetrominoe tetros[] =
 {
  /* body                       width   height  */
   { "CC00CC0000000000",        2,      2       },      /* C */

   { "AAAA000000000000",        4,      1       },      /* A */
   { "B000B000B000B000",        1,      4       },      /* B */

   { "E000EEE000000000",        3,      2       },      /* E */
   { "FFF0F00000000000",        3,      2       },      /* F */
   { "00G0GGG000000000",        3,      2       },      /* G */
   { "HHH000H000000000",        3,      2       },      /* H */
   { "II000I000I000000",        2,      3       },      /* I */
   { "0J000J00JJ000000",        2,      3       },      /* J */
   { "KK00K000K0000000",        2,      3       },      /* K */
   { "L000L000LL000000",        2,      3       },      /* L */

   { "Q000QQ00Q0000000",        2,      3       },      /* Q */
   { "0R00RR000R000000",        2,      3       },      /* R */
   { "0S00SSS000000000",        3,      2       },      /* S */
   { "TTT00T0000000000",        3,      2       },      /* T */

   { "0W00WW00W0000000",        2,      3       },      /* W */
   { "0XX0XX0000000000",        3,      2       },      /* X */
   { "Y000YY000Y000000",        2,      3       },      /* Y */
   { "ZZ000ZZ000000000",        3,      2       },      /* Z */

   { NULL,                      0,      0,      }
 };


FILE              * pF;
unsigned int        table_width,
                    table_height,
                    table_area,
                    tetro_num;          /* num of tetros to fill the table */
char                solution_matrix[20][77],
                    matrix[20][77];
unsigned long int   comb,               /* curr bin combination of tetros */
                    solutions_count;
struct tetrominoe * tetro_comb[20];     /* curr pointer combination ...   */
char              * selection[20];      /* selection of tetro strings forming
                                           solution */

/****************************************************************************/
/* function definitions
*/


/* counts number of nonzero bits in binary combination of tetros
*/

unsigned int bits_num(void)
 { unsigned int ret_val,
                i;

   ret_val = 0;
   for(i = 0; i < 19; ++i)
       if((((unsigned long int) 1) << i) & comb)
           ++ret_val;

   return ret_val;
 }


int is_valid_comb(void)
 {
   return ((comb & 0xF) && (comb & 0xF0) &&
           (comb & 0xFF00) && (comb & 0x30000));
 }


/* transformes binary combination of tetros to pointer array
*/

void select_tetros(void)
 { unsigned long int walk;
   unsigned int      i, j;

   walk = 0x40000;
   j = 0;
   i = 0;
   while(i < tetro_num)
     {
       while(!(comb & walk))
         {
           walk >>= 1;
           ++j;
         }

       tetro_comb[i++] = &tetros[j];

       walk >>= 1;
       ++j;
     }
 }


/* initializes char matrix with tetro strings
*/

int ini_matrix(void)
 { unsigned int i;

   i = 0;
   while(tetro_comb[i])
     { unsigned int j;

       if((table_width < tetro_comb[i]->width) ||
          (table_height < tetro_comb[i]->height))
           return 1;

       j = 0;
       while(j < 4)
         {
           memcpy(&matrix[i][j * table_width],
                  ((tetro_comb[i]->body) + (j * 4)), 4);
           memset(&matrix[i][(j * table_width) + 4], '0', (table_width - 4));

           ++j;
         }

       if(table_height > 4)
         {
           while(j < table_height)
             {
               memset(&matrix[i][(j * table_width)], '0', table_width);
               ++j;
             }
         }

       ++i;
     }

   return 0;
 }


void place_tetros(void)
 { unsigned int num_selected;

   num_selected = 0;
   while(selection[num_selected])
       ++num_selected;

   if(num_selected < tetro_num)
     { char       * local[20];
       unsigned int j, k,
                    num_free;
       int          selected;

       num_free = 0;
       for(j = 0; j < tetro_num; ++j)
         {
           selected = 0;
           for(k = 0; k < num_selected; ++k)
               if(&matrix[j][0] == selection[k])
                 {
                   selected = 1;
                   break;
                 }

           if(!selected)
               local[num_free++] = &matrix[j][0];
         }

       for(j = 0; j < num_free; ++j)
         { char tmp[77];

           strcpy(tmp, local[j]);
           if(num_selected)
               if(!align(local[j], num_selected))
                 {
                   strcpy(local[j], tmp);
                   break;
                 }

           selection[num_selected] = local[j];
           place_tetros();
           selection[num_selected] = 0;
           strcpy(local[j], tmp);
         }
     }
   else
       print_tetros();
 }


int align(char       * str,
          unsigned int line)
 { unsigned int temp_right,
                right,
                down;
   int          success;

   get_moves(str, &temp_right, &down);
   right = temp_right;
   success = 0;
   while((down || right) && !success)
     { char tmp[77];

       strcpy(tmp, str);
       while(right && !success)
         {
           if(overlap(str, line))
             {
               move_right(str);
               --right;

               if(!right && !overlap(str, line))
                   success = 1;
             }
           else
               success = 1;
         }

       if(down && !success)
         {
           strcpy(str, tmp);
           move_down(str);
           --down;
           right = temp_right;
         }
     }

   return success;
 }


int overlap(char       * str,
            unsigned int line)
 { unsigned int i,
                count;
   int          ret_val;

   ret_val = 1;
   count = 0;
   i = 0;
   while(i < table_area)
     {
       if(str[i] != '0')
         { unsigned int j;

           for(j = 0; j < line; ++j)
               if(selection[j][i] != '0')
                   ++count;
         }

       ++i;
     }

   if(!count)
       ret_val = 0;

   return ret_val;
 }


void move_right(char * str)
 { unsigned int i;
   char         tmp[77];

   for(i = 0; i < table_height; ++i)
     {
       strncpy(&tmp[1 + i * table_width], &str[i * table_width],
               (table_width - 1));
       tmp[i * table_width] = '0';
     }
   tmp[table_area] = 0;

   strcpy(str, tmp);
 }


void move_down(char * str)
 { unsigned int i;
   char         tmp[77];

   for(i = 0; i < (table_height - 1); ++i)
     {
       strncpy(&tmp[(1 + i) * table_width], &str[i * table_width],
               table_width);
     }
   strncpy(tmp, &str[i * table_width], table_width);
   tmp[table_area] = 0;

   strcpy(str, tmp);
 }


/* finds tetro in table and counts its movability
*/

void get_moves(char * str,
               int  * right,
               int  * down)
 { unsigned int i, j;
   char         letter;

   i = 0;
   while(str[i] == '0')
       ++i;

   letter = str[i];

   for(j = 0; j < 19; ++j)
     {
       i = 0;
       while(tetros[j].body[i] == '0')
           ++i;

       if(tetros[j].body[i] != letter)
           continue;

       *right = table_width - tetros[j].width;
       *down = table_height - tetros[j].height;
       break;
     }
 }


void print_tetros(void)
 { unsigned int i;
   char         tmp[77];

   for(i = 0; i < table_area; ++i)
      { unsigned int j;

         j = 0;
         while(selection[j][i] == '0')
             ++j;

         tmp[i] = selection[j][i];
      }
   tmp[i] = 0;

   if(not_known(tmp))
     {
       fprintf(pF, "%s\n", tmp);
       ++solutions_count;
     }
 }


int not_known(char * str)
 { unsigned int i;

   i = 0;
   while(solution_matrix[i][0])
     {
       if(!strcmp(solution_matrix[i], str))
           return 0;

       ++i;
     }

   strcpy(solution_matrix[i], str);
   return 1;
 }

/****************************************************************************/
/* main
*/

int main(void)
 {
   pF = fopen("input.txt", "rt");
   if(!pF)
     {
       printf("Error: can't open input file!\n");
       exit(2);
     }

   if(fscanf(pF, "%i,%i\n", &table_width, &table_height) != 2)
     {
       printf("Error: invalid input file!\n");
       fclose(pF);
       exit(3);
     }
   fclose(pF);

   table_area = table_width * table_height;
   if((table_area % 4) || (table_area < (5 * 4)))
     {
       printf("Error: invlaid input values!\n");
       exit(1);
     }

   pF = fopen("output.txt", "wt");
   if(!pF)
     {
       printf("Error: can't open output file!\n");
       exit(4);
     }

   tetro_num = table_area / 4;

   for(comb = 0x50111; comb <= 0x7FFFF; ++comb)
       if((bits_num() == tetro_num) && is_valid_comb())
         { unsigned int i;

           select_tetros();
           if(ini_matrix())
               continue;

           place_tetros();

           /* destroy matrix of known solutions
           */
           i = 0;
           while(solution_matrix[i][0])
             {
               solution_matrix[i][0] = 0;
               ++i;
             }
         }

   fprintf(pF, "%lu\n", solutions_count);

   fclose(pF);

   if(!solutions_count)
       printf("No solution!");

   return (0);
 }
