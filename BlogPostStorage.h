#pragma once
#include <drogon/drogon.h>
#include "BlogPost.h"
#include "HttpRequest.h"
#include <functional>

class CBlogPostStorage{
    drogon::orm::DbClientPtr client;
protected:
    static std::string _Convert_Time_To_String(std::chrono::system_clock::time_point time);
    void _Create_Database_If_Havent();
public:
    void StoreBlogPost(
        const CHttpRequest& request, 
        CBlogPost&& post, 
        std::function<void(const CHttpRequest& request,const CBlogPost&&)> callback, 
        std::function<void(const CHttpRequest& request, const drogon::orm::DrogonDbException&, const CBlogPost&&)> errorCallback
    );
    void RetrieveBlogPosts(
        const CHttpRequest& request, 
        std::function<void(const CHttpRequest&, std::vector<CBlogPost>)> callback, 
        std::function<void(const CHttpRequest&, const drogon::orm::DrogonDbException&)> errorCallback
    );
    CBlogPostStorage(){
        _Create_Database_If_Havent();
    }
};