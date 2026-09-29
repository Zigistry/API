#pragma once
#include <iostream>
#include <mutex>
#include <sqlite3.h>
#include <string>
#include <expected>

#include "../../include/crow_all.h"

extern std::mutex db_mutex;
extern sqlite3* database_connection;

#define GET_ROW_UL(A, B) ((unsigned int)sqlite3_column_int64((A), (B)))
#define GET_ROW_BOOL(A, B) ((bool)sqlite3_column_int64((A), (B)))

std::string get_row_text(sqlite3_stmt* stmt, int col);
std::string adv_tokenizer(std::string s, char del, int index);
std::expected<crow::json::wvalue, std::string> special_parsing(std::string query);

bool create_indexes(sqlite3* db);
bool create_indexes(const std::string& db_path = "./zigistry.db");

const std::string search_packages_database_query = R"""(
            WITH filtered AS MATERIALIZED (
                SELECT r.id
                FROM repos r
                WHERE r.is_disabled = 0
                  AND r.is_package = 1
                  __INSERT_FTS_FILTER_HERE__
                  __INSERT_TOPIC_FILTER_HERE__
            ),
            we AS MATERIALIZED (
                SELECT
                    r.id,
                    r.minimum_zig_version,
                    r.dependents_count
                FROM repos r
                JOIN filtered f ON f.id = r.id
            )
            SELECT
                r.id,
                r.owner_avatar_id AS avatar_id,
                r.owner,
                r.platform_id AS platform,
                r.description,
                r.issues_count,
                r.default_branch_name,
                r.fork_count,
                r.stargazer_count,
                r.watchers_count,
                r.pushed_at,
                r.created_at,
                r.is_archived,
                r.is_disabled,
                r.is_fork,
                r.license,
                r.primary_language,
                r.minimum_zig_version,
                r.dependents_count,
                (SELECT COUNT(*) FROM filtered) AS total_results
            FROM filtered f
            JOIN repos r ON r.id = f.id
            LEFT JOIN we ON we.id = r.id
            __INSERT_SORT_HERE__
            LIMIT ? OFFSET ?
    )""";

const std::string search_programs_database_query = R"""(
            WITH filtered AS MATERIALIZED (
                SELECT r.id
                FROM repos r
                WHERE r.is_disabled = 0
                  AND r.is_program = 1
                  __INSERT_FTS_FILTER_HERE__
                  __INSERT_TOPIC_FILTER_HERE__
            ),
            we AS MATERIALIZED (
                SELECT
                    r.id,
                    r.minimum_zig_version,
                    r.dependents_count
                FROM repos r
                JOIN filtered f ON f.id = r.id
            )
            SELECT
                r.id,
                r.owner_avatar_id AS avatar_id,
                r.owner,
                r.platform_id AS platform,
                r.description,
                r.issues_count,
                r.default_branch_name,
                r.fork_count,
                r.stargazer_count,
                r.watchers_count,
                r.pushed_at,
                r.created_at,
                r.is_archived,
                r.is_disabled,
                r.is_fork,
                r.license,
                r.primary_language,
                r.minimum_zig_version,
                r.dependents_count,
                (SELECT COUNT(*) FROM filtered) AS total_results
            FROM filtered f
            JOIN repos r ON r.id = f.id
            LEFT JOIN we ON we.id = r.id
            __INSERT_SORT_HERE__
            LIMIT ? OFFSET ?
    )""";

const std::string infinite_scroll_packages_query = R"""(
        SELECT
            r.id,
            r.owner_avatar_id AS avatar_id,
            r.owner,
            r.platform_id AS platform,
            r.description,
            r.issues_count,
            r.default_branch_name,
            r.fork_count,
            r.stargazer_count,
            r.watchers_count,
            r.pushed_at,
            r.created_at,
            r.is_archived,
            r.is_disabled,
            r.is_fork,
            r.license,
            r.primary_language,
            r.minimum_zig_version,
            r.dependents_count
        FROM repos r
        WHERE
            r.is_disabled = 0
            AND r.is_package = 1
        ORDER BY r.stargazer_count DESC, r.id ASC
        LIMIT ? OFFSET ?;
)""";

const std::string infinite_scroll_programs_query = R"""(
        SELECT
            r.id,
            r.owner_avatar_id AS avatar_id,
            r.owner,
            r.platform_id AS platform,
            r.description,
            r.issues_count,
            r.default_branch_name,
            r.fork_count,
            r.stargazer_count,
            r.watchers_count,
            r.pushed_at,
            r.created_at,
            r.is_archived,
            r.is_disabled,
            r.is_fork,
            r.license,
            r.primary_language,
            r.minimum_zig_version,
            r.dependents_count
        FROM repos r
        WHERE
            r.is_disabled = 0
            AND r.is_program = 1
        ORDER BY r.stargazer_count DESC, r.id ASC
        LIMIT ? OFFSET ?;
)""";
