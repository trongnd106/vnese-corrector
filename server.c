#include <ulfius.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include "dotenv.h"
#include <jansson.h>
#include "routes/api.h"

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

    // Initialize endpoints
    if (!init_api_endpoints(&instance, conn)) {
        fprintf(stderr, "Failed to initialize api endpoints. Exiting...\n");
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
