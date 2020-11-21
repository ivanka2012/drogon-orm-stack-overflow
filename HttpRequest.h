#pragma once

#include <drogon/drogon.h>

class CHttpRequest{
    drogon::HttpRequestPtr req;
    std::function<void (const drogon::HttpResponsePtr &)> callback;
public:
    CHttpRequest() = delete;
    CHttpRequest(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) : req{std::move(req)}, callback{std::move(callback)}{}
    CHttpRequest(const CHttpRequest& other) : req{std::move(other.req)}, callback{std::move(other.callback)}{}
    CHttpRequest(CHttpRequest&& other) : req{std::move(other.req)}, callback{std::move(other.callback)}{}
    drogon::HttpRequestPtr GetRequest() const;
    std::function<void (const drogon::HttpResponsePtr &)> GetCallback() const;
};
