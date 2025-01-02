#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> // For server-client relation
#include <unistd.h>
#include <openssl/evp.h> // For SHA-256

#define PORT 8088
#define IP "127.0.0.1"

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

// Extract file extension from the filename
const char* get_file_extension(const char *filename) 
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) 
    {
        return "";
    }
    return dot + 1;
}

// Extract file name without the extension
void get_file_name_without_extension(const char *filename, char *basename) 
{
    const char *dot = strrchr(filename, '.');
    if (dot) 
    {
        size_t len = dot - filename;
        strncpy(basename, filename, len);
        basename[len] = '\0';
    } 
    else 
    {
        strcpy(basename, filename);
    }
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file_name> <num_segments>\n", argv[0]);
        return 1;
    }

    char *file_name = argv[1];
    int num_segments = atoi(argv[2]);

    // Create the socket (IPv4, Transfer Control Protocol, default)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
    {
        perror("Socket creation failed");
        return 1;
    }

    // Set up server address
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) 
    {
        perror("Invalid address / Address not supported");
        close(sock);
        return 1;
    }

    // Establish the connection
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Send file name and number of segments to server
    int file_name_len = strlen(file_name);
    if (send(sock, &file_name_len, sizeof(int), 0) < 0) 
    {
        perror("Failed to send file name length");
        close(sock);
        return 1;
    }

    if (send(sock, file_name, file_name_len, 0) < 0) 
    {
        perror("Failed to send file name");
        close(sock);
        return 1;
    }

    if (send(sock, &num_segments, sizeof(int), 0) < 0) 
    {
        perror("Failed to send number of segments");
        close(sock);
        return 1;
    }

    // Extract the file extension from the filename
    const char *extension = get_file_extension(file_name);
    if (strlen(extension) == 0) 
    {
        fprintf(stderr, "Error: File has no extension.\n");
        close(sock);
        return 1;
    }

    // Create the output filename by appending "_received" before the file extension
    char basename[256];
    get_file_name_without_extension(file_name, basename);

    char output_file_name[512];
    snprintf(output_file_name, sizeof(output_file_name), "%s_received.%s", basename, extension); 

    // Open the output file
    FILE *output_file = fopen(output_file_name, "wb");
    if (!output_file) 
    {
        perror("File creation failed");
        close(sock);
        return 1;
    }

    int file_transfer_complete = 1; // Flag to track the status of the file transfer

    // Receive and verify each segment
    for (int i = 0; i < num_segments; i++) 
    {
        int segment_index, segment_size;
        unsigned char received_checksum[EVP_MAX_MD_SIZE];

        // Receive segment index
        if (recv(sock, &segment_index, sizeof(int), 0) <= 0) 
        {
            perror("Failed to receive segment index");
            file_transfer_complete = 0;
            break;
        }

        // Receive segment size
        if (recv(sock, &segment_size, sizeof(int), 0) <= 0) 
        {
            perror("Failed to receive segment size");
            file_transfer_complete = 0;
            break;
        }

        // Recieve checksum
        if (recv(sock, received_checksum, EVP_MD_size(EVP_sha256()), 0) <= 0) 
        {
            perror("Failed to receive checksum");
            file_transfer_complete = 0;
            break;
        }

        // Dynamically allocate buffer size
        char *data = malloc(segment_size);
        if (data == NULL) 
        {
            perror("Memory allocation failed");
            file_transfer_complete = 0;
            break;
        }

        // Receive the actual segment data
        int total_received = 0;
        while (total_received < segment_size) 
        {
            int bytes_received = recv(sock, data + total_received, segment_size - total_received, 0);
            if (bytes_received <= 0) 
            {
                perror("Failed to receive segment data");
                file_transfer_complete = 0;
                break;
            }
            total_received += bytes_received;
        }

        if (total_received != segment_size) 
        {
            fprintf(stderr, "Incomplete data for segment %d\n", segment_index);
            file_transfer_complete = 0;
            break;
        }

        // Verify the checksum of the received segment
        unsigned char computed_checksum[EVP_MAX_MD_SIZE];
        calculate_sha256((unsigned char *)data, segment_size, computed_checksum);

        // Compare checksum values
        if (memcmp(received_checksum, computed_checksum, EVP_MD_size(EVP_sha256())) == 0) 
        {
            // Get to the segment location
            if (fseek(output_file, segment_index * segment_size, SEEK_SET) != 0) 
            {
                perror("fseek failed");
                file_transfer_complete = 0;
                break;
            }

            // Write in the output file
            if (fwrite(data, 1, segment_size, output_file) != segment_size) 
            {
                perror("fwrite failed");
                file_transfer_complete = 0;
                break;
            }
        }
        else 
        {
            fprintf(stderr, "Checksum mismatch for segment %d\n", segment_index);
            file_transfer_complete = 0;
            break;
        }

        free(data); // Free allocated buffer
    }

    // Clean up and close resources
    fclose(output_file);
    close(sock);

    // Final message
    if (file_transfer_complete) 
    {
        printf("File transfer completed\n");
    } 
    else 
    {
        printf("File transfer incomplete\n");
    }

    return 0;
}
