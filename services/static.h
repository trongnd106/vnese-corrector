#include <ulfius.h>
#include <stdio.h>
#include <string.h>

// Callback function to serve static files
int callback_static(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const char *file_path = request->http_url + 1; // Remove leading slash
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        ulfius_set_string_body_response(response, 404, "File not found");
        return U_ERROR;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_content = malloc(file_size);
    if (file_content == NULL) {
        fclose(file);
        ulfius_set_string_body_response(response, 500, "Memory allocation failed");
        return U_ERROR;
    }

    fread(file_content, 1, file_size, file);
    fclose(file);

    // Set appropriate content type based on file extension
    const char *content_type = "application/octet-stream";
    if (strstr(file_path, ".png")) {
        content_type = "image/png";
    } else if (strstr(file_path, ".jpg") || strstr(file_path, ".jpeg")) {
        content_type = "image/jpeg";
    } else if (strstr(file_path, ".gif")) {
        content_type = "image/gif";
    } else if (strstr(file_path, ".css")) {
        content_type = "text/css";
    } else if (strstr(file_path, ".js")) {
        content_type = "application/javascript";
    }

    ulfius_set_binary_body_response(response, 200, file_content, file_size);
    ulfius_add_header_to_response(response, "Content-Type", content_type);
    free(file_content);
    return U_OK;
}