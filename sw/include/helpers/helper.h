#pragma once
#include <cstddef>
#include <curl/curl.h>
#include <string>

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

std::string http_get(const std::string& url);

int parse_minutes(const std::string& display_str);
