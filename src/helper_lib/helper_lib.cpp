#include <sstream>
#include <sqlite3.h>
#include <expected>
#include "../../include/crow_all.h"
#include "./helper_lib.h"
extern sqlite3* database_connection;

std::string get_row_text(sqlite3_stmt* stmt, int col)
{
    if (sqlite3_column_type(stmt, col) == SQLITE_NULL)
        return "";
    const unsigned char* text = sqlite3_column_text(stmt, col);
    if (!text)
        return "";
    int len = sqlite3_column_bytes(stmt, col);
    if (len <= 0)
        return "";
    return std::string((const char*)text, len);
}

std::string adv_tokenizer(std::string s, char del, int index)
{
    std::stringstream ss(s);
    std::string word;
    int count = -1;
    while (!ss.eof() and count++ != index) {
        getline(ss, word, del);
    }
    return word;
}

bool create_indexes(sqlite3* db)
{
    if (!db)
        return false;

    const char* index_sql = R"""(
-- packages
CREATE INDEX IF NOT EXISTS idx_repos_pkg_stars ON repos (
    is_disabled, is_package, stargazer_count DESC, id ASC
);
CREATE INDEX IF NOT EXISTS idx_repos_pkg_dependents ON repos (
    is_disabled, is_package, dependents_count DESC, id ASC
);
CREATE INDEX IF NOT EXISTS idx_repos_pkg_pushed ON repos (
    is_disabled, is_package, pushed_at DESC, id ASC
);
CREATE INDEX IF NOT EXISTS idx_repos_pkg_created ON repos (
    is_disabled, is_package, created_at DESC, id ASC
);
-- programs
CREATE INDEX IF NOT EXISTS idx_repos_prog_stars ON repos (
    is_disabled, is_program, stargazer_count DESC, id ASC
);
CREATE INDEX IF NOT EXISTS idx_repos_prog_dependents ON repos (
    is_disabled, is_program, dependents_count DESC, id ASC
);
CREATE INDEX IF NOT EXISTS idx_repos_prog_pushed ON repos (
    is_disabled, is_program, pushed_at DESC, id ASC
);
CREATE INDEX IF NOT EXISTS idx_repos_prog_created ON repos (
    is_disabled, is_program, created_at DESC, id ASC
);
-- users
CREATE INDEX IF NOT EXISTS idx_repos_owner_stars ON repos (
    owner, is_disabled, stargazer_count DESC
);

CREATE INDEX IF NOT EXISTS idx_releases_repo_publish ON releases (repo_id, published_at DESC);

CREATE INDEX IF NOT EXISTS idx_topics_topic ON repo_topics (topic, repo_id);

CREATE INDEX IF NOT EXISTS idx_users_platform_id ON users (platform_id);

CREATE INDEX IF NOT EXISTS idx_pipeline_status ON repo_pipeline_queue (status, queued_at);
)""";

    char* error_message = nullptr;
    int returned_code = sqlite3_exec(db, index_sql, nullptr, nullptr, &error_message);
    if (returned_code != SQLITE_OK) {
        std::cerr << "Failed to create indexes: " << (error_message ? error_message : "unknown error") << std::endl;
        sqlite3_free(error_message);
        return false;
    }
    return true;
}

bool create_indexes(const std::string& db_path)
{
    sqlite3* db = nullptr;
    if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
        if (db) {
            sqlite3_close(db);
        }
        return false;
    }
    bool success = create_indexes(db);
    sqlite3_close(db);
    return success;
}

std::expected<crow::json::wvalue, std::string> special_parsing(std::string query)
{
    std::lock_guard<std::mutex> lock(db_mutex);

    sqlite3_stmt* query_stmt = nullptr;
    int rc = sqlite3_prepare_v2(database_connection, query.c_str(), -1, &query_stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::unexpected("Unable to parse the sql.");
    }

    crow::json::wvalue normal_responce;
    crow::json::wvalue::list items;

    while (true) {
        int step_rc = sqlite3_step(query_stmt);
        if (step_rc == SQLITE_DONE) {
            break;
        } else if (step_rc != SQLITE_ROW) {
            sqlite3_finalize(query_stmt);
            return std::unexpected("some problem with row and sql");
        }

        crow::json::wvalue item;
        std::string id = get_row_text(query_stmt, 0);
        item["id"] = id;

        std::string provider = get_row_text(query_stmt, 3);

        item["id"] = get_row_text(query_stmt, 0);
        item["avatar_url"] = get_row_text(query_stmt, 1);
        item["owner_name"] = get_row_text(query_stmt, 2);
        item["owner"] = get_row_text(query_stmt, 2);

        item["repo_name"] = adv_tokenizer(id, '/', 2);
        item["provider"] = (provider == "github" || provider == "gh") ? "gh" : "cb";

        item["description"] = get_row_text(query_stmt, 4);
        item["platform"] = provider;
        item["issues_count"] = GET_ROW_UL(query_stmt, 5);
        item["default_branch_name"] = get_row_text(query_stmt, 6);
        item["fork_count"] = GET_ROW_UL(query_stmt, 7);
        item["stargazer_count"] = GET_ROW_UL(query_stmt, 8);
        item["watchers_count"] = GET_ROW_UL(query_stmt, 9);
        item["pushed_at"] = get_row_text(query_stmt, 10);
        item["created_at"] = get_row_text(query_stmt, 11);
        item["is_archived"] = GET_ROW_UL(query_stmt, 12);
        item["is_disabled"] = GET_ROW_UL(query_stmt, 13);
        item["is_fork"] = GET_ROW_UL(query_stmt, 14);
        item["license"] = get_row_text(query_stmt, 15);
        item["primary_language"] = get_row_text(query_stmt, 16);
        item["minimum_zig_version"] = get_row_text(query_stmt, 17) == "" ? "0.0.0" : get_row_text(query_stmt, 17);

        item["dependents_count"] = GET_ROW_UL(query_stmt, 18);

        items.push_back(item);
    }

    sqlite3_finalize(query_stmt);
    crow::json::wvalue res;
    res = std::move(items);
    return res;
}
