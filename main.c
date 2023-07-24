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

size_t write_callback(void* ptr, size_t size, size_t nmemb, char* data) {
    strncpy(data, (char*)ptr, MAX_JOKE_LENGTH - 1);
    data[MAX_JOKE_LENGTH - 1] = '\0';
    return size * nmemb;
}

int main() {
    srand(time(NULL));

    printf("Welcome to the Joke Generator!\n");

    char api_response[MAX_JOKE_LENGTH];

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
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, api_response);

            CURLcode res = curl_easy_perform(curl);

            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                printf("Failed to fetch joke from the API. Error: %s\n", curl_easy_strerror(res));
                continue;
            }

            printf("API Response:\n%s\n", api_response);

            cJSON* json = cJSON_Parse(api_response);
            if (!json) {
                printf("Failed to parse JSON response.\n");
                continue;
            }

            cJSON* setup = cJSON_GetObjectItemCaseSensitive(json, "setup");
            cJSON* delivery = cJSON_GetObjectItemCaseSensitive(json, "delivery");
            if (cJSON_IsString(setup) && cJSON_IsString(delivery)) {
                printf("\n%s %s\n", setup->valuestring, delivery->valuestring);
            } else {
                printf("Failed to extract joke from the API response.\n");
            }

            cJSON_Delete(json);
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}