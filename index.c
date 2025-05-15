#include <ulfius.h>
#include <stdio.h>

#define PORT 8080

// GET "/hello"
int callback_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_t *json_body = json_object();
    json_object_set_new(json_body, "message", json_string("Hello from Ulfius!"));

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// POST /hello
int callback_post_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_error_t error;
    const json_t *json_req = ulfius_get_json_body_request(request, &error);

    json_t *json_body = json_object();

    if (json_req && json_is_object(json_req)) {
        const char *name = json_string_value(json_object_get(json_req, "name"));

        if (name) {
            char message[256];
            snprintf(message, sizeof(message), "POST: Hello, %s!", name);
            json_object_set_new(json_body, "message", json_string(message));
        } else {
            json_object_set_new(json_body, "error", json_string("Required field 'name'"));
        }
    } else {
        json_object_set_new(json_body, "error", json_string("Invalid JSON"));
    }

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// PUT /hello
int callback_put_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_t *json_body = json_object();
    json_object_set_new(json_body, "message", json_string("PUT: Update data successfully!"));

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// DELETE /hello
int callback_delete_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_t *json_body = json_object();
    json_object_set_new(json_body, "message", json_string("DELETE: Delete successfully!"));

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// GET "/view"
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

    // Define endpoint
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/hello", NULL, 0, &callback_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /hello endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    
    if (ulfius_add_endpoint_by_val(&instance, "POST", "/hello", NULL, 0, &callback_post_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding POST /hello endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    
    if (ulfius_add_endpoint_by_val(&instance, "PUT", "/hello", NULL, 0, &callback_put_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding PUT /hello endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    
    if (ulfius_add_endpoint_by_val(&instance, "DELETE", "/hello", NULL, 0, &callback_delete_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding DELETE /hello endpoint\n");
        ulfius_clean_instance(&instance);
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
