#include <ulfius.h>
#include <stdio.h>

#define PORT 8080

// GET "/index"
int callback_view(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const char *file_path = "./vendors/views/index.html";
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        ulfius_set_string_body_response(response, 500, "Error: Unable to load HTML file.");
        return U_ERROR;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *html_content = malloc(file_size + 1);
    if (html_content == NULL) {
        fclose(file);
        ulfius_set_string_body_response(response, 500, "Error: Memory allocation failed.");
        return U_ERROR;
    }

    fread(html_content, 1, file_size, file);
    html_content[file_size] = '\0';
    fclose(file);

    ulfius_set_string_body_response(response, 200, html_content);
    free(html_content);
    return U_OK;
}

int main(void) {
    struct _u_instance instance;

    // Initialize Ulfius
    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error initializing Ulfius framework\n");
        return 1;
    }
    
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/index", NULL, 0, &callback_view, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /view endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }

    // Run server
    printf("Starting server on port %d...\n", PORT);
    int ret = ulfius_start_framework(&instance);
    if (ret == U_OK) {
        printf("Server running on http://localhost:%d\n", PORT);
        printf("Press Enter to stop the server...\n");
        getchar();
    } else {
        fprintf(stderr, "Error starting server: %d\n", ret);
        fprintf(stderr, "Possible reasons: Port %d might be in use or requires root privileges\n", PORT);
    }

    // Stop server
    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}
