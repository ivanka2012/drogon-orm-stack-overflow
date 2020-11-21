#include <drogon/drogon.h>
#include "HttpRequest.h"
#include "BlogPostStorage.h"

class BloggerIndex : public drogon::HttpSimpleController<BloggerIndex>{
    CBlogPostStorage sqlStorage;
public:
    virtual void asyncHandleHttpRequest(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    PATH_ADD("/",drogon::Get);
    PATH_LIST_END
};

void BloggerIndex::asyncHandleHttpRequest(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    CHttpRequest httpRequest{req, std::move(callback)};
    sqlStorage.RetrieveBlogPosts(httpRequest,
    [](const CHttpRequest& request, std::vector<CBlogPost> posts){
        const drogon::HttpRequestPtr req = request.GetRequest();
        std::function<void (const drogon::HttpResponsePtr &)> callback = request.GetCallback();

        drogon::HttpViewData data;
        data.insert("blogPosts", posts);
        auto resp = drogon::HttpResponse::newHttpViewResponse("index_blogposts.csp", data);
        callback(resp);
    },
    [](const CHttpRequest& request, const drogon::orm::DrogonDbException& ex){
        const drogon::HttpRequestPtr req = request.GetRequest();
        std::function<void (const drogon::HttpResponsePtr &)> callback = request.GetCallback();

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        std::string body = std::string{"Unable to get blogposts: "} + ex.base().what();
        resp->setBody(body);
        callback(resp);
    });
}


int main(){
    drogon::app().loadConfigFile("config.json");
    drogon::app().enableBrotli(true).enableGzip(false).run();
}