#include "RandomStringUtil.h"
#include <random>

std::string RandomStringUtil::GenerateRandomString(size_t length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t max_index = sizeof(charset) - 2;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, max_index);

    std::string randomString;
    randomString.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        randomString += charset[dist(gen)];
    }

    return randomString;
}
