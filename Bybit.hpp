#ifndef BYBIT_H
#define BYBIT_H

#include "Utils/Utils.hpp"
#include "Utils/Logger.hpp"

#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "json.hpp"

f32 getBalance(const std::string& coin, const std::string& API_KEY, const std::string& API_SECRET);

const std::list<std::string> getCoinList(const std::string& API_KEY, const std::string& API_SECRET);

const std::list<std::string> currenciesList = {
    "USD",
};

struct ByBitTicker {
    std::string symbol;
    f32 lastPrice;
    f32 indexPrice;
    f32 markPrice;
    f32 bid1Price;
    f32 ask1Price;
};

const ByBitTicker getTicker(const std::string token, const std::string currency, const std::string& API_KEY, const std::string& API_SECRET);
const std::list<ByBitTicker> getTickers(const std::list<std::string>& tokens, const std::list<std::string>& currencies, const std::string& API_KEY, const std::string& API_SECRET);

/* P2P offers not supported freely in Bybit API */

struct BybitPaymentMethod {
    u64 index;
    std::string text;
};

const std::list<BybitPaymentMethod> supportedPaymentMethods = {
    {
        .index = 75,
        .text = "Tinkoff"
    },
    {
        .index = 377,
        .text = "Sberbank"
    },
    {
        .index = 64,
        .text = "Raiffeisenbank"
    },
    {
        .index = 378,
        .text = "Gazprombank"
    },
    {
        .index = 379,
        .text = "Alfa Bank"
    },
    {
        .index = 382,
        .text = "SBP"
    },
    {
        .index = 381,
        .text = "VTB Bank"
    },
    {
        .index = 595,
        .text = "OZON Bank"
    },
    {
        .index = 614,
        .text = "Sovkombank"
    },
    {
        .index = 90,
        .text = "Cash in Person"
    }
};

struct BybitP2POffer {
    f32 price;
    std::string nickName;
    u64 recentExecuteRate;
    u64 recentOrderNum;
    std::string remark;
    std::string link;
};

std::array<BybitP2POffer, 10> getP2POffers(const std::string token, const std::string currency, const u64 amount, const std::list<u64>& paymentMethods);

#endif // BYBIT_H