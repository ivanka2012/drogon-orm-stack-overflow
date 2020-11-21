#include "HttpRequest.h"

drogon::HttpRequestPtr CHttpRequest::GetRequest() const{
    return req;
}

std::function<void (const drogon::HttpResponsePtr &)> CHttpRequest::GetCallback() const{
    return callback;
}
