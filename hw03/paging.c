#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -------------- Initialization ------------------ //

struct PTE 
{
    int frame_number;
    int valid;
    int size;
};

struct PageTable 
{
    struct PTE *pte;
    int num_pte;
};
 
struct PCB 
{
    int process_id;
    int process_size;
    char * process_file;
    struct PageTable page_table;
    int internal_frag;
};

#define EOF_FILE 0xFF

int* frames;
int num_frames;

struct PCB * processes;
int process_count = 0;

int physical_memory_size;
int logical_address_size;
int page_size;

int num_pages;

// ---------------- Frame Handling --------------------- //

int init_frames()
{
    frames = (int*)malloc(num_frames * sizeof(int));

    if (frames == NULL) 
    {
        printf("Error allocating memory for frames");
        return -1;
    }

    for (int i = 0; i < num_frames; i++) 
    {
        frames[i] = 0; 
    }
    return 0;
}

int allocate_frame()
{
    for (int i=0; i<num_frames; i++)
    {
        if (frames[i] == 0)
        {
            frames[i] = 1;
            return i;
        }
    }

    return -1;
}

// ----------------- Load Program -------------------- //

int load_program(char * filename, struct PCB *pcb) 
{
    // Opening the file
    FILE *file = fopen(filename, "rb");
    pcb->process_file = filename;
    printf("Loading Process from File: %s\n", pcb->process_file);
    if (file == NULL) 
    {
        printf("Error opening file");
        return -1;
    }

    // Process ID 
    if (fread(&pcb->process_id, 1, 1, file) != 1) {
        printf("Error reading process ID\n");
        fclose(file);
        return -1;
    }
    printf("Process_ID: %02X\n", pcb->process_id);

    // Code Segment Size
    unsigned char code_size_bytes[2];
    if (fread(code_size_bytes, sizeof(unsigned char), 2, file) != 2) {
        printf("Error reading code segment size\n");
        fclose(file);
        return -1;
    }
    unsigned short code_size = (code_size_bytes[0] << 8) | code_size_bytes[1];
    printf("Code Size: %d\n", code_size);

    // Code Segment
    unsigned char *code_segment = (unsigned char *)malloc(code_size);
    if (code_segment == NULL) 
    {
        printf("Error allocating memory for code segment");
        fclose(file);
        return -1;
    }
    if (fread(code_segment, 1, code_size, file) != code_size) 
    {
        printf("Error reading code segment\n");
        fclose(file);
        return -1;
    }
    printf("Code Segment: ");
    for (int i = 0; i < code_size; i++) 
    {
        printf("%02X ", code_segment[i]); 
    }
    printf("\n");

    // Data Segment Size
    unsigned char data_size_bytes[2];
    if (fread(data_size_bytes, sizeof(unsigned char), 2, file) != 2) {
        printf("Error reading data segment size");
        fclose(file);
        return -1;
    }
    unsigned short data_size = (data_size_bytes[0] << 8) | data_size_bytes[1];
    printf("Data Size: %d\n", data_size);

    // Data Segment
    unsigned char *data_segment = (unsigned char *)malloc(data_size);
    if (data_segment == NULL) 
    {
        printf("Error allocating memory for data segment");
        fclose(file);
        return -1;
    }
    if (fread(data_segment, 1, data_size, file) != data_size) 
    {
        printf("Error reading data segment\n");
        fclose(file);
        return -1;
    }
    printf("Data Segment: ");
    for (int i = 0; i < data_size; i++) 
    {
        printf("%02X ", data_segment[i]); 
    }
    printf("\n");

    // End Marker
    unsigned char end_marker;
    if (fread(&end_marker, 1, 1, file) != 1 || end_marker != EOF_FILE) 
    {
        printf("Error: End of process marker not found\n");
        fclose(file);
        return -1;
    }

    pcb->process_size = code_size + data_size; 
    printf("Process_Size for Process %02X: %d\n", pcb->process_id, pcb->process_size);

    pcb->page_table.num_pte = (pcb->process_size + page_size - 1) / page_size;
    printf("Number of Pages for Process %02X: %d\n", pcb->process_id, pcb->page_table.num_pte);

    if (pcb->page_table.num_pte > num_pages)
    {
        printf("Not enough pages for the process\n");
        return -1;
    }

    pcb->page_table.pte = (struct PTE *)malloc(pcb->page_table.num_pte * sizeof(struct PTE));
    if (pcb->page_table.pte == NULL) {
        printf("Error allocating memory for page table");
        return -1;
    }

    // ---------- Mapping Logical Pages to Physical Frames ------------------ //

    // printf("\n\nMapping Logical Pages to Physical Frames for Process %02X\n", pcb->process_id);
    int temp = pcb->process_size;
    for (int i=0; i < pcb->page_table.num_pte; i++)
    {
        int allocated_frame = allocate_frame();

        if (allocated_frame != -1)
        {
            pcb->page_table.pte[i].frame_number = allocated_frame;
            pcb->page_table.pte[i].valid = 1;
            pcb->page_table.pte[i].size = ((temp-page_size)<0) ? temp : page_size;
            temp = temp - pcb->page_table.pte[i].size;
        }
        else
        {
            printf("No free frames\n");
            pcb->page_table.pte[i].valid = 0;
            return -1;
        }
    }

    // ----------------- Calculating Internal Fragmentation ------------------- //

    pcb->internal_frag = page_size - (pcb->process_size % page_size);
    if (pcb->internal_frag == page_size)
    {
        pcb->internal_frag = 0;
    }
    // printf("\n");
    // printf("Internal Fragmentation for Process %02X is %d\n", pcb->process_id, pcb->internal_frag);

    fclose(file);
    free(code_segment);
    free(data_segment);

    return 0;
}

// --------------------------- Total Internal Fragmenttion ------------------ //

int internal_fragmentation()
{
    printf("\nInternal Fragmentation:\n");
    int total = 0;
    for (int i=0; i<process_count; i++)
    {
        total += processes[i].internal_frag;
        printf("Internal Fragmentation for Process %02X is %d\n", processes[i].process_id, processes[i].internal_frag);
    }
    printf("Total Internal Fragmentation = %d bytes\n", total);
    return total;
}

// -------------------- Memory Dump -------------------------- //

void memory_dump() 
{
    printf("Memory Dump:\n");
    for (int i = 0; i < process_count; i++) 
    {
        printf("Process %02X Page Table:\n", processes[i].process_id);
        for (int j = 0; j < processes[i].page_table.num_pte; j++) 
        {
            printf("Page %d -> %d Bytes -> Frame %d\n", j, processes[i].page_table.pte[j].size, processes[i].page_table.pte[j].frame_number);
        }
        printf("\n");
    }
}

// ---------------- Free Frames --------------------- //

void free_frames()
{
    printf("Total Number of Frames: %d\n", num_frames);
    printf("Free Frames: [");
    for (int i = 0; i < num_frames-1; i++) 
    {
        if (frames[i] == 0) 
        {
            printf("%d, ", i);
        }
    }
    
    if (frames[num_frames-1] == 0)
    {
        printf("%d]\n", num_frames-1);
    }
    else
    {
        printf("]\n");
    }
}

// --------------- main -------------------- //

int main(int argc, char * argv[]) 
{
    if (argc < 5)
    {
        printf("Usage: %s <physical memory size in B> <logical address size in bits> <page size> <process file 01> <process file 02> ...\n", argv[0]);
        return 1;
    }

    physical_memory_size = atoi(argv[1]);
    logical_address_size = atoi(argv[2]);
    page_size = atoi(argv[3]);

    int frame_size = page_size;

    num_frames = physical_memory_size / frame_size;
    init_frames();

    num_pages = (1 << logical_address_size) / page_size;

    processes = (struct PCB *)malloc((argc - 4) * sizeof(struct PCB));

    for (int i=4; i<argc; i++)
    {
        if (load_program(argv[i], &processes[process_count]) == 0) 
        {
            printf("Process loaded successfully\n\n");
            process_count += 1;
        } 
        else 
        {
            printf("Failed to load process.\n\n");
        }
    }

    memory_dump();
    free_frames();
    internal_fragmentation();

    // Freeing memory
    for (int i = 0; i < process_count; i++) 
    {
        free(processes[i].page_table.pte); 
    }

    free(processes); 
    free(frames);

    return 0;
}