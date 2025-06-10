#ifndef BYBIT_H
#define BYBIT_H

#include "Utils/Utils.hpp"
#include "Utils/Logger.hpp"

#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "json.hpp"

struct BybitPaymentMethod {
    u64 index;
    std::string text;
};

const std::list<BybitPaymentMethod> supportedPaymentMethods = {
    {
        .index = 75,
        .text = "Tinkoff"
    }
};

f32 getBalance(const std::string& API_KEY, const std::string& API_SECRET);

struct BybitP2POffer {
    f32 price;
    std::string nickName;
    u64 recentExecuteRate;
    u64 recentOrderNum;
    std::string remark;
    std::string link;
};

std::array<BybitP2POffer, 10> getP2POffers();

#endif // BYBIT_H