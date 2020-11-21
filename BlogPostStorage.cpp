//HACK! MinGW does not declare htonll, so define it here
#include <winsock2.h>

#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32)
# define ntohll(x) ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32)
#endif

#include "BlogPostStorage.h"
#include <iomanip>
#include "HttpRequest.h"

std::string CBlogPostStorage::_Convert_Time_To_String(std::chrono::system_clock::time_point time){
    time_t now_c = std::chrono::system_clock::to_time_t(time);
    tm now_tm = *std::localtime(&now_c);
    
    std::array<char, 16384> timeBuf;
    strftime(timeBuf.data(), timeBuf.size(), "%Y-%m-%d %H:%M:%S %Z", &now_tm);
    return std::string{timeBuf.data()};
}

void CBlogPostStorage::_Create_Database_If_Havent() {
    if(!client){
        client = drogon::orm::DbClient::newSqlite3Client("filename=blogposts.db", 8);
        try{
            //The reason why we want to do this in sync is because the table size could change in the meantime, so we want to own the table while we are at it
            auto result = client->execSqlSync("PRAGMA locking_mode = EXCLUSIVE");
            result = client->execSqlSync("PRAGMA busy_timeout = 30000");
        }catch(const drogon::orm::DrogonDbException &e){
            LOG_ERROR << "An error happened during setting the SQLite locking mode: " << e.base().what();
        }
    }
}

void CBlogPostStorage::StoreBlogPost(
        const CHttpRequest& request,
        CBlogPost&& post,
        std::function<void(const CHttpRequest& request, const CBlogPost&&)> callback, 
        std::function<void(const CHttpRequest& request, const drogon::orm::DrogonDbException&, const CBlogPost&&)> errorCallback
    ) {
    _Create_Database_If_Havent();
    
    auto transPtr = client->newTransaction();
    transPtr->execSqlAsync("CREATE TABLE IF NOT EXISTS \"blogposts\"(\"title\" TEXT, \"posted_time\" INTEGER, \"text\" BLOB)",
        [transPtr, request, post, errorCallback, callback](const drogon::orm::Result &r){
            transPtr->execSqlAsync("INSERT INTO \"blogposts\"(\"title\",\"posted_time\",\"text\") VALUES ($1,$2,$3);",
            [request, post, callback](const drogon::orm::Result &r){
                auto currentTime = std::chrono::system_clock::now();
                LOG_INFO << "Inserted new article \"" << post.GetTitle() << "\" (Article posted " << _Convert_Time_To_String(post.GetPosted()) << ") at " << _Convert_Time_To_String(currentTime) << "\n";
                if(callback){
                    callback(request, std::move(post));
                }
            },
            [request, post, errorCallback](const drogon::orm::DrogonDbException &e){
                LOG_ERROR << "Unable to insert article \"" << post.GetTitle() << "\": " << e.base().what() << "\n";
                if(errorCallback){
                    errorCallback(request, e, std::move(post));
                }
            },
            post.GetTitle(), std::chrono::system_clock::to_time_t(post.GetPosted()), post.GetBody());
        },
        [request, post,errorCallback](const drogon::orm::DrogonDbException &e){
            LOG_ERROR << "Unable to create table \"blogposts\": " << e.base().what() << "\n";
            if(errorCallback){
                errorCallback(request, e, std::move(post));
            }
        });
}

void CBlogPostStorage::RetrieveBlogPosts(
        const CHttpRequest& request,
        std::function<void(const CHttpRequest&, std::vector<CBlogPost>)> callback, 
        std::function<void(const CHttpRequest&, const drogon::orm::DrogonDbException&)> errorCallback
    ) {
    _Create_Database_If_Havent();

    auto connection = client;

    connection->execSqlAsync("SELECT * FROM \"blogposts\" ORDER BY posted_time DESC",
    [request, callback](const drogon::orm::Result &r){
        std::vector<CBlogPost> blogPosts;
        blogPosts.reserve(r.size());
        for(const auto& row : r){
            CBlogPost post;
            post.SetTitle(row["title"].as<std::string>());
            post.SetPosted(row["posted_time"].as<int64_t>());
            post.SetBody(row["text"].as<std::string>());
            blogPosts.push_back(post);
        }
        if(callback){
            callback(request, blogPosts);
        }
    },
    [connection, request, callback, errorCallback](const drogon::orm::DrogonDbException &e){
        //The database might not exist, check for that
        std::string previousReason{e.base().what()};

        try{
            //The reason why we want to do this in sync is because the table size could change in the meantime, so we want to own the table while we are at it
            auto result = connection->execSqlSync("SELECT name FROM sqlite_master WHERE type='table' AND name='blogposts'");
            assert(result.size() == 1 || result.size() == 0);
            if(result.size() == 1){
                //There's a table; it's an error
                LOG_ERROR << "Unable to select all from table \"blogposts\": " << previousReason;
                if(errorCallback){
                    errorCallback(request, e);
                }
            }else if(result.size() == 0){
                //Yeah, it just doesn't exist yet
                if(callback){
                    callback(request, std::vector<CBlogPost>{});
                }
            }

        }catch(const drogon::orm::DrogonDbException &e){
            LOG_ERROR << "Unable to select all from table \"blogposts\" (the error was: " << previousReason << ") as well as check if the table exists: " << e.base().what();
            if(errorCallback){
                errorCallback(request, e);
            }
        }
    });
}


