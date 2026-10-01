#include "../include/crow_all.h"
#include "./helper_lib/helper_lib.h"
#include <expected>
extern sqlite3* database_connection;

crow::response packageIndexDetails(const crow::request& req)
{
    const std::string get_latest_repos_query = R"""(
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
        WHERE r.is_disabled = 0
          AND r.is_package = 1
        ORDER BY r.created_at DESC, r.id ASC
        LIMIT 10;
    )""";

    const std::string get_most_used_repos_query = R"""(
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
        WHERE r.is_disabled = 0
          AND r.is_package = 1
        ORDER BY r.stargazer_count DESC, r.id ASC
        LIMIT 10 OFFSET 0;
    )""";

    const std::string get_repo_gui_section = R"""(
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
        FROM index_sections s
        JOIN repos r ON r.id = s.repo_id
        WHERE s.section_name = 'gui'
          AND r.is_disabled = 0
          AND (r.is_package = 1 OR r.is_program = 1)
        LIMIT 10;
    )""";

    const std::string get_repo_games_section = R"""(
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
        FROM index_sections s
        JOIN repos r ON r.id = s.repo_id
        WHERE s.section_name = 'games'
          AND r.is_disabled = 0
          AND (r.is_package = 1 OR r.is_program = 1)
        LIMIT 10;
    )""";

    const std::string get_repo_web_section = R"""(
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
        FROM index_sections s
        JOIN repos r ON r.id = s.repo_id
        WHERE s.section_name = 'web'
          AND r.is_disabled = 0
          AND (r.is_package = 1 OR r.is_program = 1)
        LIMIT 10;
    )""";

    const std::string get_recently_updated_repos_query = R"""(
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
        WHERE r.is_disabled = 0
          AND r.is_package = 1
        ORDER BY r.pushed_at DESC, r.id ASC
        LIMIT 10;
    )""";

    auto latest_repositories = special_parsing(get_latest_repos_query);
    auto most_used_repos = special_parsing(get_most_used_repos_query);
    auto games_repos = special_parsing(get_repo_games_section);
    auto gui_repos = special_parsing(get_repo_gui_section);
    auto web_repos = special_parsing(get_repo_web_section);
    auto recently_updated_repos = special_parsing(get_recently_updated_repos_query);

    if (latest_repositories and most_used_repos and games_repos and gui_repos and web_repos and recently_updated_repos) {
        crow::json::wvalue normal_responce;

        normal_responce["latest"] = std::move(*latest_repositories);
        normal_responce["most_used"] = std::move(*most_used_repos);
        normal_responce["games"] = std::move(*games_repos);
        normal_responce["gui"] = std::move(*gui_repos);
        normal_responce["web"] = std::move(*web_repos);
        normal_responce["recently_updated"] = std::move(*recently_updated_repos);

        return crow::response(normal_responce);
    } else {
        crow::json::wvalue error_responce;
        error_responce["error"] = "some problem on server.";
        return crow::response(error_responce);
    }
}
