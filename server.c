#include <ulfius.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include "dotenv.h"
#include "services/view.h"
#include "services/static.h"
#include <jansson.h>
#include "services/user.h"

#define PORT 8080

MYSQL* init_mysql_connection() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return NULL;
    }

    const char* host = getenv("DB_HOST");
    const char* user = getenv("DB_USER");
    const char* pass = getenv("DB_PASS");
    const char* dbname = getenv("DB_NAME");
    const char* port = getenv("DB_PORT");

    if (mysql_real_connect(conn, host, user, pass, dbname, 3306, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

int main(void) {
    struct _u_instance instance;

    load_dotenv(".env");

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

    // For handle static files
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/statics/*", NULL, 0, &callback_static, NULL) != U_OK) {
        fprintf(stderr, "Error adding static files endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    
    // Define endpoints
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/", NULL, 0, &callback_view_guest, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /view endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/index", NULL, 0, &callback_view, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /view endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/signup", NULL, 0, &callback_view_signup, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /signup endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/login", NULL, 0, &callback_view_login, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /login endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    if (ulfius_add_endpoint_by_val(&instance, "POST", "/users", NULL, 0, &callback_create_user, conn) != U_OK) {
        fprintf(stderr, "Error adding POST /users endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    if (ulfius_add_endpoint_by_val(&instance, "POST", "/login", NULL, 0, &callback_login_user, conn) != U_OK) {
        fprintf(stderr, "Error adding POST /login endpoint\n");
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
