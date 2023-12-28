#include <stdio.h>
#include <curl/curl.h>

#define MAX_SIZE 800

// Callback function to write response data
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    printf("%.*s", (int)realsize, (char *)contents);
    return realsize;
}

// Callback function to write headers
size_t header_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    printf("Header: %.*s", (int)realsize, (char *)contents);
    return realsize;
}

int main(void) {
    char host[MAX_SIZE];
    printf("enter the host: ");
    scanf("%s", host);
    CURL *curl;
    CURLcode res;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Create a curl handle
    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, host);

        // Set the callback function to handle response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        // Set the callback function to handle headers
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);

        // Follow HTTP redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Add custom headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Fedora; Linux x86_64; rv:100.0) Gecko/20100101 Firefox/100.0");
        char customHeader[800]; // Adjust the size accordingly
        snprintf(customHeader, sizeof(customHeader), "Host: %s", host);
        headers = curl_slist_append(headers, customHeader);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request and capture the result
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    // Cleanup libcurl
    curl_global_cleanup();

    return 0;
}

