/*
I compiled the code using:
/home/issiac/Downloads/buildroot-2024.05//output/host/usr/bin/arm-linux-gcc --sysroot=/home/issiac/Downloads/buildroot-2024.05//output/staging -c requestor.c -o requestor.o
/home/issiac/Downloads/buildroot-2024.05//output/host/usr/bin/arm-linux-gcc --sysroot=/home/issiac/Downloads/buildroot-2024.05//output/staging  -o httpclient requestor.o  -lcurl -uClibc -lc
*/

// Issiac Baca - ECE 531 7/23/24

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>

#define HOST_IP "10.0.0.137"  // IP address to replace "localhost"
#define OK 0
#define INIT_ERR 1
#define REQ_ERR 2
#define ARG_ERR 3

// Function to print the usage/help message
void print_help() {
    printf("Usage: ./httpclient [OPTIONS] -u <URL> [STRING]\n");
    printf("\nOPTIONS:\n");
    printf("  -u, --url <URL>    URL to request (required)\n");
    printf("  -o, --post         Send a POST request (requires -u)\n");
    printf("  -g, --get          Send a GET request (requires -u)\n");
    printf("  -p, --put          Send a PUT request (requires -u)\n");
    printf("  -d, --delete       Send a DELETE request (requires -u)\n");
    printf("  -h, --help         Show this help message and exit\n");
}

// Callback function to handle data received from the server
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    fwrite(ptr, size, nmemb, stdout);
    return size * nmemb;
}

// Callback function to read data from a file for PUT requests
static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *stream) {
    size_t retcode = fread(ptr, size, nmemb, stream);
    if (retcode > 0) {
        fprintf(stderr, "*** We read %zu bytes from file\n", retcode);
    }
    return retcode;
}

int main(int argc, char *argv[]) {
    int opt;
    char *url = NULL;         // URL to request
    char *message = NULL;     // Message or file path for PUT requests
    FILE *hd_src = NULL;      // File pointer for PUT requests
    struct stat file_info;    // To get file information for PUT requests
    CURL *curl;               // CURL handle
    CURLcode res;             // CURL function return code
    int post = 0, get = 0, put = 0, del = 0; // Flags for request types

    // Define long options for command-line arguments
    static struct option long_options[] = {
        {"url", required_argument, 0, 'u'},
        {"post", no_argument, 0, 'o'},
        {"get", no_argument, 0, 'g'},
        {"put", no_argument, 0, 'p'},
        {"delete", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Parse command-line options
    while ((opt = getopt_long(argc, argv, "u:ogpdh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'u': url = optarg; break;
            case 'o': post = 1; break;
            case 'g': get = 1; break;
            case 'p': put = 1; break;
            case 'd': del = 1; break;
            case 'h': print_help(); return OK;
            default: print_help(); return ARG_ERR;
        }
    }

    // Check if URL is provided
    if (!url) {
        fprintf(stderr, "Error: URL is required.\n");
        print_help();
        return ARG_ERR;
    }

    // Get optional message or file path argument
    if (optind < argc) {
        message = argv[optind];
    }

    // Initialize CURL
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing CURL\n");
        return INIT_ERR;
    }

    // Modify URL if it contains "localhost"
    char modified_url[256];
    snprintf(modified_url, sizeof(modified_url), "%s", url);
    char *pos = strstr(modified_url, "localhost");
    if (pos) {
        snprintf(pos, sizeof(modified_url) - (pos - modified_url), "%s", HOST_IP);
    }

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, modified_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Set request-specific CURL options
    if (get) curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    if (post) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
    }
    if (put) {
        if (stat(message, &file_info) == 0) {
            hd_src = fopen(message, "rb");
            if (!hd_src) {
                perror("Error opening file");
                curl_easy_cleanup(curl);
                return REQ_ERR;
            }
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
        } else {
            perror("Error in stat()");
            fprintf(stderr, "Error: PUT request requires a file path.\n");
            curl_easy_cleanup(curl);
            return ARG_ERR;
        }
    }
    if (del) curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("\nHTTP Response Code: %ld\n", response_code);
    }

    // Cleanup
    curl_easy_cleanup(curl);
    if (hd_src) fclose(hd_src);

    return OK;
}

