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

std::array<f32, 10> getP2POffers();

#endif // BYBIT_H