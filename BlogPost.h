#pragma once
#include <string>
#include <chrono>
#include <type_traits>

class CBlogPost{
    std::string title;
    std::string body;

    std::chrono::system_clock::time_point posted;
protected:
    std::string _Markdown_To_HTML(std::string&& title);
public:
    CBlogPost() = default;
    virtual ~CBlogPost() = default;

    template<typename T>
    void SetTitle(T&& title){
        static_assert(std::is_constructible_v<std::string, T&&>);
        this->title = std::string{title};
    }
    const std::string& GetTitle() const{
        return title;
    }

    template<typename T>
    void SetBody(T&& body){
        static_assert(std::is_constructible_v<std::string, T&&>);
        this->body = std::string{body};
    }
    template<typename T>
    void SetBodyFromMarkdown(T&& body){
        static_assert(std::is_constructible_v<std::string, T&&>);
        SetBody(_Markdown_To_HTML(std::forward(body)));
    }
    const std::string& GetBody() const{
        return body;
    }

    template<typename T>
    void SetPosted(T&& posted){
        this->posted = std::chrono::system_clock::time_point{std::chrono::duration<long long int>(posted)};
    }
    const std::chrono::system_clock::time_point& GetPosted() const{
        return posted;
    }
};