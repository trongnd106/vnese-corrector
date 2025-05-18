#include <ulfius.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#define PORT 8080

// Function to initialize MySQL connection
MYSQL* init_mysql_connection() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return NULL;
    }

    if (mysql_real_connect(conn, "127.0.0.1", "root", "root", "corrector", 3306, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

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

// GET "/index"
int callback_view(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const char *file_path = "./views/index.html";
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

    // Initialize MySQL connection
    MYSQL *conn = init_mysql_connection();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to MySQL. Exiting...\n");
        return 1;
    }
    
    // Define endpoints
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/index", NULL, 0, &callback_view, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /view endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }

    // For handle static files
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/statics/*", NULL, 0, &callback_static, NULL) != U_OK) {
        fprintf(stderr, "Error adding static files endpoint\n");
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
