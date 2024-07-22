# cc -o hw requestor.c

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>

#define OK 0
#define INIT_ERR 1
#define REQ_ERR 2
#define ARG_ERR 3

void print_help() {
    printf("Usage: test [OPTIONS]\n");
    printf("OPTIONS:\n");
    printf("  -u, --url <URL>    URL to request (required)\n");
    printf("  -o, --post         Send a POST request\n");
    printf("  -g, --get          Send a GET request\n");
    printf("  -p, --put          Send a PUT request\n");
    printf("  -d, --delete       Send a DELETE request\n");
    printf("  -h, --help         Show this help message and exit\n");
}

static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  unsigned long nread;
 
  retcode = fread(ptr, size, nmemb, stream);
 
  if(retcode > 0) {
    nread = (unsigned long)retcode;
    fprintf(stderr, "*** We read %lu bytes from file\n", nread);
  }
 
  return retcode;
}

int main(int argc, char *argv[]) {
    char *url = NULL;
    char *message = NULL;
    CURL *curl;
    CURLcode res;
    FILE * hd_src;
    struct stat file_info;
    char *file;
    
    if(argc < 3)
    	return 1;
    file = argv[1];
    url = argv[2];
    stat(file, &file_info);
    hd_src = fopen(file, "rb");
 
    
    curl = curl_easy_init();
    
    int post = 0, get = 0, put = 0, del = 0;

    static struct option long_options[] = {
        {"url", required_argument, 0, 'u'},
        {"post", no_argument, 0, 'o'},
        {"get", no_argument, 0, 'g'},
        {"put", no_argument, 0, 'p'},
        {"delete", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "u:ogpdh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'u':
                url = optarg;
                break;
            case 'o':
                post = 1;
                break;
            case 'g':
                get = 1;
                break;
            case 'p':
                put = 1;
                break;
            case 'd':
                del = 1;
                break;
            case 'h':
                print_help();
                return OK;
            default:
                print_help();
                return ARG_ERR;
        }
    }

    if (!url) {
        fprintf(stderr, "Error: URL is required.\n");
        print_help();
        return ARG_ERR;
    }

    if (get) {
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_perform(curl);
    	curl_easy_cleanup(curl);
    }
    
    if (post) {
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "Here is the message I'm posting to the URL!");
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    	}
    	curl_easy_cleanup(curl);
    }
    
    if (put) {
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
 
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    	curl_easy_setopt(curl, CURLOPT_URL, url);
 
   	curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
 
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
 
	res = curl_easy_perform(curl);
	
	if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
 
	curl_easy_cleanup(curl);
    }
    
    if (del) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
    	        
    return OK;
}

