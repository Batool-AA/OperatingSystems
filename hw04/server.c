#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h> // For server-client relation
#include <unistd.h>
#include <openssl/evp.h> // For SHA-256 checksum calculation

#define PORT 8088

typedef struct 
{
    int client_socket;
    char file_name[256];
    int segment_index;
    int segment_size;
} ThreadArgs;

// Compute SHA-256 checksum for a data buffer
void calculate_sha256(unsigned char *data, int size, unsigned char *checksum) {
    // Initialize the context for SHA-256
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        perror("Failed to create EVP_MD_CTX"); // Error handling if context creation fails
        exit(1);
    }

    const EVP_MD *md = EVP_sha256(); // Get the SHA-256 algorithm
    if (EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
        perror("EVP_DigestInit_ex failed"); // Error handling if initialization fails
        EVP_MD_CTX_free(mdctx); // Free context before exiting
        exit(1);
    }

    if (EVP_DigestUpdate(mdctx, data, size) != 1) {
        perror("EVP_DigestUpdate failed"); // Error handling if data feeding fails
        EVP_MD_CTX_free(mdctx); // Free context before exiting
        exit(1);
    }

    unsigned int md_len; // Variable to store the length of the resulting checksum
    if (EVP_DigestFinal_ex(mdctx, checksum, &md_len) != 1) {
        perror("EVP_DigestFinal_ex failed"); // Error handling if finalization fails
        EVP_MD_CTX_free(mdctx); // Free context before exiting
        exit(1);
    }

    EVP_MD_CTX_free(mdctx); // Free the context after checksum calculation
}

// Handle file transfer - done by threads
void *send_file_segment(void *args) 
{
    ThreadArgs *threadArgs = (ThreadArgs *)args;

    // Open file for reading
    FILE *file = fopen(threadArgs->file_name, "rb");
    if (!file) 
    {
        perror("File not found");
        close(threadArgs->client_socket);
        free(threadArgs);
        pthread_exit(NULL);
    }

    // Get to the segment of the file that the thread is responsible for
    fseek(file, threadArgs->segment_index * threadArgs->segment_size, SEEK_SET);
    char *buffer = malloc(threadArgs->segment_size);
    if (!buffer) 
    {
        perror("Failed to allocate memory for buffer");
        fclose(file);
        close(threadArgs->client_socket);
        free(threadArgs);
        pthread_exit(NULL);
    }

    // Read the segment of the file
    int bytes_read = fread(buffer, 1, threadArgs->segment_size, file);
    if (bytes_read < 0) 
    {
        perror("Failed to read file segment");
        free(buffer);
        fclose(file);
        close(threadArgs->client_socket);
        free(threadArgs);
        pthread_exit(NULL);
    }

    // Calculate checksum for the segment
    unsigned char checksum[EVP_MAX_MD_SIZE];
    calculate_sha256((unsigned char *)buffer, bytes_read, checksum);

    // Send segment index
    if (send(threadArgs->client_socket, &threadArgs->segment_index, sizeof(int), 0) == -1) 
    {
        perror("Failed to send segment index");
        free(buffer);
        fclose(file);
        close(threadArgs->client_socket);
        free(threadArgs);
        pthread_exit(NULL);
    }

    // Send segment size
    if (send(threadArgs->client_socket, &bytes_read, sizeof(int), 0) == -1) 
    {
        perror("Failed to send segment size");
        free(buffer);
        fclose(file);
        close(threadArgs->client_socket);
        free(threadArgs);
        pthread_exit(NULL);
    }

    // Send segment checksum
    if (send(threadArgs->client_socket, checksum, EVP_MD_size(EVP_sha256()), 0) == -1) 
    {
        perror("Failed to send checksum");
        free(buffer);
        fclose(file);
        close(threadArgs->client_socket);
        free(threadArgs);
        pthread_exit(NULL);
    }

    // Send segment data
    if (send(threadArgs->client_socket, buffer, bytes_read, 0) == -1) 
    {
        perror("Failed to send segment data");
        free(buffer);
        fclose(file);
        close(threadArgs->client_socket);
        free(threadArgs);
        pthread_exit(NULL);
    }

    // Free memory and close file
    free(buffer);
    fclose(file);
    free(threadArgs);
    pthread_exit(NULL);
}

int main() 
{
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) 
    {
        perror("Socket creation failed");
        return 1;
    }

    // Set socket option to allow frequent reuse of port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
    {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // Set up server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) 
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for incoming connections - upto 20 at a time
    if (listen(server_fd, 20) == -1) 
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }
    printf("Server listening on port %d\n\n", PORT);

    // Continously listen for client requests
    while (1) 
    {
        // Accept client request
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket == -1) 
        {
            perror("Accept failed");
            continue;
        }

        // Receive the file name length
        int file_name_len;
        if (recv(client_socket, &file_name_len, sizeof(int), 0) <= 0) 
        {
            perror("Failed to receive file name length");
            close(client_socket);
            continue;
        }

        // Receive the file name
        char file_name[256] = {};
        if (recv(client_socket, file_name, file_name_len, 0) <= 0) 
        {
            perror("Failed to receive file name");
            close(client_socket);
            continue;
        }

        // Receive number of segments
        int num_segments;
        if (recv(client_socket, &num_segments, sizeof(int), 0) <= 0) 
        {
            perror("Failed to receive number of segments");
            close(client_socket);
            continue;
        }
        printf("Request received: File = %s, Segments = %d\n\n", file_name, num_segments);

        // Read the file size
        FILE *file = fopen(file_name, "rb");
        if (!file) 
        {
            perror("File not found");
            close(client_socket);
            continue;
        }
        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        fclose(file);

        // Calculate segment size
        int segment_size = (file_size + num_segments - 1) / num_segments;

        // Create an array of threads
        pthread_t *threads = malloc(num_segments * sizeof(pthread_t));
        if (threads == NULL) 
        {
            perror("Failed to allocate memory for threads");
            close(client_socket);
            continue;
        }

        // Create threads for each segment
        for (int i = 0; i < num_segments; i++) 
        {
            ThreadArgs *args = malloc(sizeof(ThreadArgs));
            if (args == NULL) 
            {
                perror("Failed to allocate memory for thread arguments");
                free(threads);
                close(client_socket);
                continue;
            }

            // Assign arguments to the thread
            args->client_socket = client_socket;
            strcpy(args->file_name, file_name);
            args->segment_index = i;
            if (i == num_segments - 1)
            {
                args->segment_size = file_size - (i * segment_size);
            }
            else
            {
                args->segment_size = segment_size;
            }

            // Create the thread with the above arguments
            if (pthread_create(&threads[i], NULL, send_file_segment, (void*)args) != 0) 
            {
                perror("Failed to create thread");
                free(args);
                free(threads);
                close(client_socket);
                continue;
            }
        }

        // Wait for the threads
        for (int i = 0; i < num_segments; i++) {
            pthread_join(threads[i], NULL);
        }

        // Close client
        close(client_socket);
        free(threads);
    }

    close(server_fd);
    return 0;
}