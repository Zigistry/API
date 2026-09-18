#include <sqlite3.h>
// doing #include <crow.h> works,
// but added this to maintain cross-platformity
#include "../include/crow_all.h"
#include "./all_routes.h"
#include "./helper_lib/helper_lib.h"
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>

sqlite3* database_connection;
std::mutex db_mutex;

void prepare_statements()
{
    sqlite3* new_db = nullptr;
    if (sqlite3_open_v2("./zigistry.db", &new_db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
    {
        if (new_db)
        {
            sqlite3_close(new_db);
        }
        return;
    }

    std::lock_guard<std::mutex> lock(db_mutex);
    if (database_connection)
    {
        sqlite3_close(database_connection);
    }
    database_connection = new_db;
}

int main()
{
    prepare_statements();

    if (!database_connection) {
        std::cerr << "Connection error: failed to open ./zigistry.db" << std::endl;
        return 1;
    }

    std::cout << "Connected..." << std::endl;

    std::thread([]() {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::hours(1));
            if (system("make download_database") == 0)
            {
                prepare_statements();
            }
        }
    }).detach();

    crow::App<crow::CORSHandler> app;
    app.loglevel(crow::LogLevel::Warning);

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods("GET"_method, "POST"_method, "OPTIONS"_method)
        .headers("Content-Type", "Authorization");

    CROW_ROUTE(app, "/search/packages/")
    ([](const crow::request& req) {
        return search(req, search_packages_database_query);
    });
    CROW_ROUTE(app, "/search/programs/")
    ([](const crow::request& req) {
        return search(req, search_programs_database_query);
    });
    CROW_ROUTE(app, "/packages/scroll/")
    ([](const crow::request& req) {
        return infinite_scroll(req, infinite_scroll_packages_query);
    });
    CROW_ROUTE(app, "/programs/scroll/")
    ([](const crow::request& req) {
        return infinite_scroll(req, infinite_scroll_programs_query);
    });
    CROW_ROUTE(app, "/users/")
    (get_user_route);

    CROW_ROUTE(app, "/programIndexDetails/")
    (programIndexDetails);

    CROW_ROUTE(app, "/packageIndexDetails/")
    (packageIndexDetails);

    CROW_ROUTE(app, "/packages/")
    ([](const crow::request& req) {
        return package_details(req);
    });

    CROW_ROUTE(app, "/programs/")
    ([](const crow::request& req) {
        return package_details(req);
    });
    app.port(7860).multithreaded().run();
}
