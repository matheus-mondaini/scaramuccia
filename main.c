#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "curl/include/curl/curl.h"
#include "cJSON/cJSON.h"

#define MAX_JOKE_LENGTH 256

int getRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

struct MemoryBuffer {
    char* memory;
    size_t size;
};

size_t write_callback(void* ptr, size_t size, size_t nmemb, struct MemoryBuffer* data) {
    size_t total_size = size * nmemb;
    data->memory = realloc(data->memory, data->size + total_size + 1);

    if (data->memory == NULL) {
        printf("Failed to allocate memory for the API response.\n");
        return 0;
    }

    memcpy(&(data->memory[data->size]), ptr, total_size);
    data->size += total_size;
    data->memory[data->size] = '\0';

    return total_size;
}

int main() {
    srand(time(NULL));

    printf("Welcome to the Joke Generator!\n");

    struct MemoryBuffer api_response = {NULL, 0};

    int run = 1;

    while (run) {
        printf("\nEnter 'j' to get a joke, or 'q' to quit: ");
        char choice;
        scanf(" %c", &choice);

        if (choice == 'q' || choice == 'Q') {
            printf("Goodbye!\n");
            run = 0;
        } else if (choice == 'j' || choice == 'J') {
            CURL* curl = curl_easy_init();
            if (!curl) {
                printf("Failed to initialize curl.\n");
                return 1;
            }

            char url[] = "https://v2.jokeapi.dev/joke/Any";

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &api_response);

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                printf("Failed to fetch joke from the API.\n");
                curl_easy_cleanup(curl);
                continue;
            }

            cJSON* json = cJSON_Parse(api_response.memory);
            if (!json) {
                printf("Failed to parse JSON response.\n");
                curl_easy_cleanup(curl);
                continue;
            }

            cJSON* setup = cJSON_GetObjectItem(json, "setup");
            cJSON* delivery = cJSON_GetObjectItem(json, "delivery");
            cJSON* category = cJSON_GetObjectItem(json, "category");

            if (cJSON_IsString(setup) && cJSON_IsString(delivery) && cJSON_IsString(category)) {
                printf("\n%s %s\n", setup->valuestring, delivery->valuestring);
            } else {
                printf("Failed to extract joke from the API response.\n");
            }

            cJSON_Delete(json);
            free(api_response.memory);
            api_response.memory = NULL;
            api_response.size = 0;

            curl_easy_cleanup(curl);
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}