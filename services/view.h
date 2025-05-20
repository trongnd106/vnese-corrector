#include <ulfius.h>
#include <stdio.h>

int load_view(const char *file_path, struct _u_response *response) {
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

// GET "/index"
int callback_view(const struct _u_request *request, struct _u_response *response, void *user_data) {
    return load_view("./views/index.html", response);
}

// GET "/signup"
int callback_view_signup(const struct _u_request *request, struct _u_response *response, void *user_data) {
    return load_view("./views/signup.html", response);
}

// GET "/login"
int callback_view_login(const struct _u_request *request, struct _u_response *response, void *user_data) {
    return load_view("./views/login.html", response);
}

// GET "/home"
int callback_view_guest(const struct _u_request *request, struct _u_response *response, void *user_data) {
    return load_view("./views/guest.html", response);
}