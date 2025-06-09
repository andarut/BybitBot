#include "Bybit.hpp"


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc &e) {
        return 0;
    }
}

static const std::string signRequest(const std::string& API_SECRET, const std::string& data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), API_SECRET.c_str(), API_SECRET.length(),
                  reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), NULL, NULL);

    std::ostringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}

static const std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    long long timestamp_ms = now_ms.time_since_epoch().count();
    return std::to_string(timestamp_ms);
}

f64 convert_string_numbers(nlohmann::json j) {
    try {
        std::string s = j.get<std::string>();
        if (s.find_first_not_of("0123456789+-.eE") == std::string::npos) {
            return std::stod(s);
        }
    } catch (...) {
        std::cerr << j.dump(4) << " is not convertible to f32" << std::endl;
    }
    return 0;
}

f32 getBalance(const std::string& API_KEY, const std::string& API_SECRET) {

    const std::string coin = "USDT";

    const std::string URL = "https://api.bybit.com/v5/asset/withdraw/withdrawable-amount?coin=" + coin;

    const std::string timestamp = getTimestamp();
    const std::string recvWindow = "50000";
    const std::string queryString = "coin=" + coin;

    const std::string API_SIGN = signRequest(API_SECRET, timestamp + API_KEY + recvWindow + queryString);

    CURL* curl = curl_easy_init();
    nlohmann::json response;
    if (curl) {
        std::string readBuffer;

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("X-BAPI-API-KEY: " + API_KEY).c_str());
        headers = curl_slist_append(headers, ("X-BAPI-TIMESTAMP: " + timestamp).c_str());
        headers = curl_slist_append(headers, ("X-BAPI-RECV-WINDOW: " + recvWindow).c_str());
        headers = curl_slist_append(headers, ("X-BAPI-SIGN: " + API_SIGN).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            try {
                response = nlohmann::json::parse(readBuffer);
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
                return 0;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    /* Response */
    
    if (response["retCode"] == 0) {
        return convert_string_numbers(response["result"]["withdrawableAmount"]["FUND"]["withdrawableAmount"]);
    }

    return 0;
}

std::array<f32, 10> getP2POffers() {

    const std::string URL = "https://api2.bybit.com/fiat/otc/item/online";

    CURL *curl;
    CURLcode res;

    /* Request params */
    const char* post_data = "{\"tokenId\":\"USDT\",\"currencyId\":\"RUB\",\"payment\":[\"75\"],\"side\":\"0\",\"size\":\"10\",\"page\":\"1\",\"amount\":\"20000\",\"vaMaker\":false,\"bulkMaker\":false,\"canTrade\":true,\"verificationFilter\":0,\"sortType\":\"TRADE_PRICE\",\"paymentPeriod\":[],\"itemRegion\":1}";

    /* Setup request headers */
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:139.0) Gecko/20100101 Firefox/139.0");
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Accept-Language: en");
    headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br, zstd");
    headers = curl_slist_append(headers, "Referer: https://www.bybit.com/");
    headers = curl_slist_append(headers, "platform: PC");
    headers = curl_slist_append(headers, "guid: e1b3e94c-e9a1-69c8-0de5-d1d2e19258b6");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=UTF-8");
    headers = curl_slist_append(headers, "lang: en");
    headers = curl_slist_append(headers, "riskToken: dmVyMQ^|MTRkNTJlNDQzOW0wemNpbnRoYWVoNm9lMGZjN2JkN2M2^|^|==");
    headers = curl_slist_append(headers, "traceparent: 00-1c5e229769a8be51a0c7804cf1e2c8e8-54145e244d80ebd6-01");
    headers = curl_slist_append(headers, "Origin: https://www.bybit.com");
    headers = curl_slist_append(headers, "Alt-Used: api2.bybit.com");
    headers = curl_slist_append(headers, "Connection: keep-alive");
    headers = curl_slist_append(headers, "Sec-Fetch-Dest: empty");
    headers = curl_slist_append(headers, "Sec-Fetch-Mode: cors");
    headers = curl_slist_append(headers, "Sec-Fetch-Site: same-site");
    headers = curl_slist_append(headers, "DNT: 1");
    headers = curl_slist_append(headers, "Sec-GPC: 1");
    headers = curl_slist_append(headers, "Priority: u=4");
    headers = curl_slist_append(headers, "TE: trailers");

    const char* cookie = 
            "_abck=C3F8E7671EF5831CEB33E3D650B5C695~0~YAAQFAxAF2CaQzyXAQAAOQ/qTw7ntPs9wzSUnaf9rEB11RsMBLTLrwIq7JObZ8JZijuXepn52+0bfsQqZcQUbx+PgGCmZOZfZ27eSmcbjYRMeFe2CA/D9d2dScDWWC3UDCsfxIelG+JvfkEiSAn7YXGDOy1FWCudHc0KySlTbb7ccg81nEdCaMKDQTJvq/hVx9sPJ0udxi5zyNFUdJ1taVY8POgC2Xt7CmPl/CH4UhZUAP7B+H9qGssorOeo4aAr5n9A1S8HtEcxW1KMp+9KOoGBIdf/Oi4cVrkU0OGcVaRezWxaAQb2Nuu1IivbMF53HvKlCSvRKXtzooDgsGBa1PtNu4xQaHvBoMBmIbsnYCGzVfPd5jYBZgKuWPE4gudzRxdUhyvVk89IRpeXTb68RJqICQQXhh+bFduLQ1jDQN/3OzsCH5dSCFAnDUCbeHb+o2z8GSLetDSTvFjx27ItuuBr5qWPWqq1fD78wiC1eQdW7GAkIlddYx63QVidwaFY6X5dZW4yZvISfU3r/OU2olvYDbXyKvGa0wAA29nEKUf6dLHRPx7ACNynMGWZ9WLI3pookZtyNNGwpCDRegdbXpupgh8AG26UWeaQFrHEf8VkukSEgicvWeDeOp+pqlGS9s+qXJi/Wr37dFRmRoodtOk5HlrM1QA+Pq89A6MzqrncDftsFGTCSwraY/9UVJoaAp7paQ/ETLG4ZlFcJ9yEy/XVA3fegNWnXs5j37EVdd/eoc8QHYqiS95IJfpuSWJaazt60srIjihWCZ0AdnBPQdtHk6SMdEuZ/eYrhdUd0c0tc+4mWwMLh1WPk8IaJxm86p/isFOj8waA3HJO/pEuULe8Xu9sQBOqcm+JjtwK2GkBuPAHdyoznzuoBhv5oRkEXT5jCW8b6IBpForyuYlKMxvPNDM6My1vmDL8MijwoMWjyzFEMiKz9uSs/ZYQ9CYTdzuz0frr7wP+VOiLtgVpLHlHhHNwpZzuSwiVBBTznA1y~-1~-1~-1; "
            "_by_l_g_d=e1b3e94c-e9a1-69c8-0de5-d1d2e19258b6; "
            "sensorsdata2015jssdkcross=%7B%22distinct_id%22%3A%22420450320%22%2C%22first_id%22%3A%22196d45670b386e-03ee29281a685e-43262f3c-2621440-196d45670b420c%22%2C%22props%22%3A%7B%22%24latest_traffic_source_type%22%3A%22%E7%9B%B4%E6%8E%A5%E6%B5%81%E9%87%8F%22%2C%22%24latest_search_keyword%22%3A%22%E6%9C%AA%E5%8F%96%E5%88%B0%E5%80%BC_%E7%9B%B4%E6%8E%A5%E6%89%93%E5%BC%80%22%2C%22%24latest_referrer%22%3A%22%22%2C%22_a_u_v%22%3A%220.0.6%22%2C%22%24latest_utm_source%22%3A%22account_menu%22%2C%22utm_source%22%3A%22account_menu%22%7D%2C%22identities%22%3A%22eyIkaWRlbnRpdHlfY29va2llX2lkIjoiMTk2ZDQ1NjcwYjM4NmUtMDNlZTI5MjgxYTY4NWUtNDMyNjJmM2MtMjYyMTQ0MC0xOTZkNDU2NzBiNDIwYyIsIiRpZGVudGl0eV9sb2dpbl9pZCI6IjQyMDQ1MDMyMCJ9%22%2C%22history_login_id%22%3A%7B%22name%22%3A%22%24identity_login_id%22%2C%22value%22%3A%22420450320%22%7D%7D; "
            "deviceId=893c0f35-88cd-abd2-58e4-45c33e0426d7; "
            "_gcl_au=1.1.1820345091.1747319168; "
            "sensorsdata2015session=%7B%7D; "
            "BYBIT_REG_REF_prod={\"lang\":\"en-US\",\"g\":\"e1b3e94c-e9a1-69c8-0de5-d1d2e19258b6\",\"referrer\":\"www.bybit.com/\",\"source\":\"bybit.com\",\"medium\":\"other\",\"url\":\"https://www.bybit.com/en/\",\"last_refresh_time\":\"Sat, 07 Jun 2025 22:36:17 GMT\",\"ext_json\":{\"dtpid\":null}}; "
            "secure-token=eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjo0MjA0NTAzMjAsImIiOjAsInAiOjMsInVhIjoiIiwiZ2VuX3RzIjoxNzQ5MzM1ODU2LCJleHAiOjE3NDk1OTUwNTYsIm5zIjoiIiwiZXh0Ijp7Im1jdCI6IjE3Mzc1NDQxNjAiLCJzaWQiOiJCWUJJVCJ9LCJkIjpmYWxzZSwic2lkIjoiQllCSVQifQ.IBViMRD3CGhLac7vQf1npYXPNy7ZWOSfWlhlEEpTjaRM0Ih7zjvKIdFB8jjiIN76oHlQlMXw6Rb2R5S-yB6hOA; "
            "bm_sz=6830D95841037BAA4FDDCDAA4876D653~YAAQB7b3SF7IhTGXAQAAWJb3TxwDChc2pz/UtI9GirLIw2ZcslE/QvkNGNEf2mPWbin/LLxDHCHCUygxW0K3euVSKT+NKtcfUgq7ooY7McbB9orDGDzK13bCLt0hoc/tHgtw/6McY79YUVWUNNEGBeuc9N9qGCX3lSMSH0ZR25yCXIFGJUc6ZuHefChjAmqmtDd8V6CM5cQRvg/O8PxE+rez8j+Z85EB6HT6hJSp/zLxrULHN1+fNIvN2lTenQsleIoY/wsb6ezAtR55sScPPV8QNzE82+7if5+MYi0yjWYKk0he7SlJ2f0Mb1AWKSVSMIJU9lqiwwVLb0o7hnQH3VAIQFng8dwBGHDnXutlo7tLVVldy43qEIoiK7KVyHFGPmtxMzJe4MV7DRELgzY5N47PvMfRu78js6WB1pHIMJumn5WG8hWzGUsEC+fMDG1cFUA7RvLHbarZTOrtdphegjShiZBv01TjSdqZhzlCIOHOW3H8bY0owH1Qr4WWCzYZTWt6FGo5azPDkUUn1Cu5ZPxljoUA36RVYs9ddVTeM7dWMEou~3223864~3750200; "
            "ak_bmsc=7996F4946D2896D20ACF112A0D422F2C~000000000000000000000000000000~YAAQBAxAF3Y8gDyXAQAAdh28TxwtI/EtkvaSpnnPkI8JC67SWexk1N19bXRdBmCtUCeZEUCx4rFqc5fTTWz9SrcMXvWQzl7HD4OtgqwT64GIGbTB/lgnlH6V64zgJizcd2k14pjC8Fs7pGqmFwHDTuWp/xHxz6REE+e45T2wGkAA6IapxS4I0zvmBgwTeaSt4Bxn6bNg6zXzr81sAg7YYf3mxRDUyKx5HacyyucXLI9mTaf2ywgJSvfBq2LnDk24+fYoFGrCh+lMPHKfCdB2K/VAf+fXz619kl5j3cbMLHPhSpZwtRnSaFQm5HPHyFLe806amyrXDqVlfXoFv+VjX7W+5M/8IGAljC2MaH75Qk83TTYT12N+Fu5WtayPlg9NHEd4GVtrspMmc6zIR8wP4BTSoO2kQL9btvhj94JENUB9gg27uuXdK1xRKiI=; "
            "bm_sv=275C0BD551E2E544CD5F5192FD958086~YAAQBAxAF94xgDyXAQAAEeu7Txz+CKG6850r0cefKRD3TiJGP3kckUyu6cKrzACU8DeLPCyAuDsA0kayx1CMQ8oc6dlb2V+VstqdQP0szl3HMrQW3ZW5UsTSwZo+4KrU4FRpAXa9AEnT6i8xl5JazSUPIj7ETJFKNtFULl0Y/KiCYpiSwqSPgve7Ov0unQH6YktmCLLH5k/IRhZbr6KreSqh/Cx/VuFW00268kTqE6YM/buZslHkUEbIe3vv068q~1; "
            "bm_mi=771E9F0BDC3F61C3412D8C71B01C1D88~YAAQFgxAF2Bp4DCXAQAAVMW7Txw/x2GvL3rm/KaMn7aweIvp/4XP9TLq0ZlfJt1ShcGUqwcGd74nRdPjSgOYCf257zPLG2PGp3xqc89bO3eiRqmsQQeK8/BfGsiwHbKLOdSnvbXG4EBLRaRb1CNG8Ely8tGODyGNtIGgm8r2P3X+UwNOmIyXLzl1OqrX3iT7qW3pjFLD2zM/nGUNm6j1BjhLKjyeTjloxn0+7oyidC9l2d2VtNIL5pdHBNHzsGjH0aWF8IyrbPVchTM9OPzhR0yRiMLbH2tKbfSyNYktx4AZjAIxPgvd1axoI2as5Q1RYqsRqlVYwV9EHP8B9eVrkpLfpA==~1";

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    /* Perform request */
    nlohmann::json response;
    if (curl) {
        std::string responseString;
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br, zstd");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            try {
                std::cout << "responseString = " << responseString << std::endl;
                response = nlohmann::json::parse(responseString);
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse response as JSON: " << e.what() << std::endl;
                return {};
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();

    /* Response */
    INFO("Response = %s\n", response.dump(4).c_str());

    if (response["ret_code"] == 0) {
        std::array<f32, 10> result;
        for (u64 i = 0; i < response["result"]["count"]; i++) {
            nlohmann::json item = response["result"]["items"][i];
            if (item["accountId"].dump(4) != "null") {
                result[i] = convert_string_numbers(item["price"]);
            }
        }
        return result;
    }

    return {};
}

void sellLink() {
    std::string URL = "https://www.bybit.com/en/fiat/trade/otc/profile/s4e7be301d77c43f09f0ccde81683ff9f/USDT/RUB/item";
}