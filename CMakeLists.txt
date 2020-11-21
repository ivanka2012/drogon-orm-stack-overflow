cmake_minimum_required(VERSION 3.10)
project(BloggerBugfixing)

find_package(Drogon REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(BloggerBugfixing 
	Index.cpp
	BlogPostStorage.cpp
	HttpRequest.cpp
	index_blogposts.cc
)
target_link_libraries(BloggerBugfixing PRIVATE
	ws2_32
	Drogon::Drogon
)