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

f32 getBalance(const std::string& coin, const std::string& API_KEY, const std::string& API_SECRET) {

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

const ByBitTicker getTicker(const std::string token, const std::string currency, const std::string& API_KEY, const std::string& API_SECRET) {

    ByBitTicker ticker;

    const std::string category = "inverse";

    const std::string URL = "https://api.bybit.com/v5/market/tickers?category=" + category + "&" + "symbol=" + \
        token + currency;

    const std::string timestamp = getTimestamp();
    const std::string recvWindow = "5000";
    const std::string queryString = "category=" + category + "&" + "symbol=" + token + currency;

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
                return ticker;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    /* Response */
    INFO("response = %s\n", response.dump(4).c_str());
    if (response["retCode"] == 0) {
        ticker.symbol = response["result"]["list"][0]["symbol"].get<std::string>();
        ticker.lastPrice  = convert_string_numbers(response["result"]["list"][0]["lastPrice"]);
        ticker.indexPrice = convert_string_numbers(response["result"]["list"][0]["indexPrice"]);
        ticker.markPrice  = convert_string_numbers(response["result"]["list"][0]["markPrice"]);
        ticker.bid1Price  = convert_string_numbers(response["result"]["list"][0]["bid1Price"]);
        ticker.ask1Price  = convert_string_numbers(response["result"]["list"][0]["ask1Price"]);
    }

    return ticker;
}

// sorted by token
const std::list<ByBitTicker> getTickers(const std::list<std::string>& tokens, const std::list<std::string>& currencies, const std::string& API_KEY, const std::string& API_SECRET) {
    std::list<ByBitTicker> tickers;

    for (auto& token : tokens) {
        for (auto& currency : currencies) {
            tickers.push_back(getTicker(token, currency, API_KEY, API_SECRET));
        }
    }

    return tickers;
}

const std::list<std::string> getCoinList(const std::string& API_KEY, const std::string& API_SECRET) {
    std::list<std::string> coins;

    // coins = {
    //     "BTC",
    //     "ETH",
    //     "XRP"
    // };

    // return coins;


    const std::string accountType = "eb_convert_inverse";

    const std::string URL = "https://api.bybit.com/v5/asset/exchange/query-coin-list?accountType=" + accountType;

    const std::string timestamp = getTimestamp();
    const std::string recvWindow = "5000";
    const std::string queryString = "accountType=" + accountType;

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
                return coins;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    /* Response */
    INFO("response = %s\n", response.dump(4).c_str());
    if (response["retCode"] == 0) {
        for (auto& coin : response["result"]["coins"]) {
            coins.push_back(coin["coin"].get<std::string>());
        }
    }

    return coins;
}

/* TODO: add test */
std::array<BybitP2POffer, 10> getP2POffers(const std::string token, const std::string currency, const u64 amount) {

    const std::string URL = "https://api2.bybit.com/fiat/otc/item/online";

    CURL *curl;
    CURLcode res;

    /* Request params */
    const char* post_data = "{\"userId\":420450320,\"tokenId\":\"USDT\",\"currencyId\":\"RUB\",\"payment\":[\"75\"],\"side\":\"0\",\"size\":\"10\",\"page\":\"1\",\"amount\":\"20000\",\"vaMaker\":false,\"bulkMaker\":false,\"canTrade\":true,\"verificationFilter\":0,\"sortType\":\"TRADE_PRICE\",\"paymentPeriod\":[],\"itemRegion\":1}";

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
            "_abck=C3F8E7671EF5831CEB33E3D650B5C695~0~YAAQhDxlX5hkE3qXAQAAa9c7lw6sXvrQN0hgmb2GsviLT9Wm+abU6hoHEHxt4sIqGSv6GkkIG4NSdE+j/x6allh+IyK2rmUTa6h4Li7vwzxDbrp2niT+4Ps/ayowG1Kxkn4FytBq2dBa9v//7Rb0ePv7HMp2b99I4JqYJMFri4Y2po5HHGRfF3FAfdjxlUTlwp3QDBp2eo2xZrDIvY0+djYARwfk0ypLIMhZisOY8EbDXDhJKLvIA0ycF7lPj5XR99XmAZ0U3HSYQS+yeHkKhlygnYl928mB580Ckj2SUtVRUDfk8hazbpFgf8YjjDJuVtIf0kaMfke4Elq/7rU7u47xdydHdlJVaEkPlDt8XkNmDnz9St8aBh3mIoR6Oqsq9LHVGmyiMZ9NjxpWRgDNlq5+gnDiSyTk90jSvqz1GzHpXt93Ue/wBvMpXHzgCUGfydGFUrdQvVurH6Cf7P7ahz2MllyG/QTJymkdWPFng1pywsFNDATYx/XiEZcaIV53JhM9mEusNwqryNE2BYD5y357WHFo/GN+xLLjnZS0ghJvgIIOVubJl4M0CVADR+ZvOzvHVeZcBRoWc2H3OxJ18FlnLoM4WyJugQ0CPiY1li1daU+QxclWjQPLN6zEe4NqAWTckdIMmwXOCZEmeIcDrZUxjiCSlkqWtX7b/Y6yF0vymSW2zO//NKPHdFtSVg6OXFrcPVSTv5MaHYyXXsO3JPXK4xsRzaWSOmh8ary6rvBfnA2V7nX6gfjDqxzPujlf5ubCWLVdwgwDIiFtIcBCSEUxi6cMAKJ1dJtybZiTJmwzO8qZ6IkZoVVrD2V4n6Ttt3Qmo8m+i5I1xc76WS2FSDPy/vjTJw1qgSQcAY0sLizhzfb4Cyjm1NoZhTv6o/GS+JPXo4feCtobFJPqRTCixZS4Tu5JmFV0k0y1EN7SiO5wHw2nuJ/6Lqz2CxpQ+gQsv4ggCVk9ntDEhFv9ggrE43BVYw0l9923NWNG4332M8LR4gdla/hb1OXopQ==~-1~-1~-1; "
            "_by_l_g_d=e1b3e94c-e9a1-69c8-0de5-d1d2e19258b6; "
            "sensorsdata2015jssdkcross=%7B%22distinct_id%22%3A%22420450320%22%2C%22first_id%22%3A%22196d45670b386e-03ee29281a685e-43262f3c-2621440-196d45670b420c%22%2C%22props%22%3A%7B%22%24latest_traffic_source_type%22%3A%22%E7%9B%B4%E6%8E%A5%E6%B5%81%E9%87%8F%22%2C%22%24latest_search_keyword%22%3A%22%E6%9C%AA%E5%8F%96%E5%88%B0%E5%80%BC_%E7%9B%B4%E6%8E%A5%E6%89%93%E5%BC%80%22%2C%22%24latest_referrer%22%3A%22%22%2C%22_a_u_v%22%3A%220.0.6%22%2C%22%24latest_utm_source%22%3A%22account_menu%22%2C%22utm_source%22%3A%22account_menu%22%7D%2C%22identities%22%3A%22eyIkaWRlbnRpdHlfY29va2llX2lkIjoiMTk2ZDQ1NjcwYjM4NmUtMDNlZTI5MjgxYTY4NWUtNDMyNjJmM2MtMjYyMTQ0MC0xOTZkNDU2NzBiNDIwYyIsIiRpZGVudGl0eV9sb2dpbl9pZCI6IjQyMDQ1MDMyMCJ9%22%2C%22history_login_id%22%3A%7B%22name%22%3A%22%24identity_login_id%22%2C%22value%22%3A%22420450320%22%7D%7D; "
            "deviceId=893c0f35-88cd-abd2-58e4-45c33e0426d7; "
            "_gcl_au=1.1.1820345091.1747319168; "
            "sensorsdata2015session=%7B%7D; "
            "BYBIT_REG_REF_prod={\"lang\":\"en-US\",\"g\":\"e1b3e94c-e9a1-69c8-0de5-d1d2e19258b6\",\"referrer\":\"www.bybit.com/\",\"source\":\"bybit.com\",\"medium\":\"other\",\"url\":\"https://www.bybit.com/en/\",\"last_refresh_time\":\"Sun, 22 Jun 2025 10:40:56 GMT\",\"ext_json\":{\"dtpid\":null}}; "
            "secure-token=eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjo0MjA0NTAzMjAsImIiOjAsInAiOjMsInVhIjoiIiwiZ2VuX3RzIjoxNzUwNTg4ODk2LCJleHAiOjE3NTA4NDgwOTYsIm5zIjoiIiwiZXh0Ijp7Im1jdCI6IjE3Mzc1NDQxNjAiLCJzaWQiOiJCWUJJVCJ9LCJkIjpmYWxzZSwic2lkIjoiQllCSVQifQ.Yo26EGgfmck-2f8NCu208-nszUWKQTPpcOJXaeO6VxcknYnLeOUwNhHW44NvKoCkoeen4fYqUzYv_u8Pn3tUWg; "
            "bm_sz=53D4D35A90DACE7FEA8D6ADB2EF836DE~YAAQhDxlX5lkE3qXAQAAa9c7lxz/tYBLsXjKY/mbkS4kC+5tNe7tL1xlpKfB2N+OiDaRLeqQf30BWc8Lq0CQ/nhkeqkaYg7+Lb851hiuqs0U43fAkyL1moJVaggXrWIc3dzkaRI7p5k5qAsbW0QlKS6NU6dPxbCX1mVGl2AAOVfkcwJ6zYJqAZCKwUQl9Pk2RPpjjTplp5cK0UftpjBYGNGBbA2W2EaVPmNfKBS43VgTxVn1kYQ2RTeTDLmvlD8h+dVer+DKzOkWvTghLvk+F6x87ZFbEQQ4+5I16nO1/9y80vj7lWdz7JuISxhWp7gduCVhLcwuR65Eed2g6CLzaZXVo/8FUCmPuPXluREWRz1VXB3PlAK/UHolJ0XjLVI5HVR0wKwN1c8S7SmC9m+dkYvdHdJSB+Mz+qF6PevhotawL096MFfX6zUytxTEnW1Bg+1hpjkK9x5/45F8SVuB3NetRFsjD17jb3I1Rv8ZCWAuHcEcSsNtk/wr90nn0sC3ZeHpsiAY8h0feV+MRFMUj+qpT8J9gXeyQg==~3621700~3556153; "
            "ak_bmsc=3DF59CAD50714D130D2F2906C4165724~000000000000000000000000000000~YAAQhDxlXyl1FHqXAQAA0IVOlxxGz3yrhGBCGJU0gzOJrCTWWkEM01UlVFGdKjNMZSqawgvkDym4c1wuzb5ZuSDokL9eOmDEkShJAMt3QXybIg7ZfcisoHhgoQHgZHEwiM2lDbpGs1+boIUMTHkpQeJ7ySi8MBHIoRBAaMOYaUVeaNxptD28rc6BhTCJhEJbWF87nBj87LK5VXyC7i+RofSR3+prMl0uBFTWBB1J9RqisoMqikAmWxy+6Bx6l38KpASWMitA7BCdpqJJD/6zDkeq4AdLu+pDze797qjZYD6YKEMILGF0rsK3t7LJwC8TOsF3ZE9qmvPGMXbPlAagkuvoyO9I/oA3aCJR7DdsUHi4T1IbGupWny8R8kD+gHRW; "
            "bm_sv=C85C674BC31984E1507D44F250E8F09C~YAAQhDxlX0RwF3qXAQAAMpF9lxynSgQnxKy0EK5JSRA7UKQJMJdV1rCRXYe/gzE3QNB2R6W9/n/iEtVWBAMbKikSjrWqs2ZIS3EfuvQYWUuqtU156evncbMr7XDAqzz1QpBy32+dSIzIqWZ3Dy0/zCp3n6BfMO2d4sSdErxDD10tJNG3HCIVq/4MBhtli963Ji19U0NQmDcrSFEqjD9+46xUZR8o/gGjbO3K0ol4gj3sjSts7ZClO4oP5HzuOEAE4w==~1; ";
            // "bm_mi=771E9F0BDC3F61C3412D8C71B01C1D88~YAAQFgxAF2Bp4DCXAQAAVMW7Txw/x2GvL3rm/KaMn7aweIvp/4XP9TLq0ZlfJt1ShcGUqwcGd74nRdPjSgOYCf257zPLG2PGp3xqc89bO3eiRqmsQQeK8/BfGsiwHbKLOdSnvbXG4EBLRaRb1CNG8Ely8tGODyGNtIGgm8r2P3X+UwNOmIyXLzl1OqrX3iT7qW3pjFLD2zM/nGUNm6j1BjhLKjyeTjloxn0+7oyidC9l2d2VtNIL5pdHBNHzsGjH0aWF8IyrbPVchTM9OPzhR0yRiMLbH2tKbfSyNYktx4AZjAIxPgvd1axoI2as5Q1RYqsRqlVYwV9EHP8B9eVrkpLfpA==~1";

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    /* Perform request */
    nlohmann::json response;
    if (curl) {
        std::string responseString;
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
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
                INFO("responseString = %s\n", responseString.c_str());
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
        std::array<BybitP2POffer, 10> result;
        for (u64 i = 0; i < response["result"]["count"]; i++) {
            nlohmann::json item = response["result"]["items"][i];
            if (item["accountId"].dump(4) != "null") {
                result[i].price = convert_string_numbers(item["price"]);
                result[i].nickName = item["nickName"].get<std::string>();
                result[i].recentExecuteRate = item["recentExecuteRate"].get<u64>();
                result[i].recentOrderNum = item["recentOrderNum"].get<u64>();
                result[i].remark = item["remark"].get<std::string>();
                result[i].link = std::format("https://www.bybit.com/en/fiat/trade/otc/profile/{}/USDT/RUB/item", item["userMaskId"].get<std::string>());
            }
        }
        return result;
    }

    return {};
}