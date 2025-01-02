#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("Please enter at least 3 file names: 2 files to be merged and 1 output file.\n");
        printf("Sample function call: ./merge a.txt b.txt output.txt\n");
        exit(1);
    }

    FILE* output_file = fopen(argv[argc - 1], "w"); 
    char line[1000];

    for (int i = 1; i <= argc - 2; i++)
    {
        printf("Merging file: %s\n", argv[i]);
        FILE* input_file = fopen(argv[i], "r"); 
        if (!input_file)
        {
            printf("Error opening input file %s\n", argv[i]);
            exit(1);
        }

        while (fgets(line, sizeof(line), input_file) != NULL)
        {
            fputs(line, output_file);
        }

        fclose(input_file); 
    }

    fclose(output_file); 
    printf("Files merged successfully into %s\n",argv[argc-1]);
    exit(0);

    return 0; 
}