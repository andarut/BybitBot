#ifndef BOT_H
#define BOT_H

#include "Utils/Utils.hpp"
#include "Utils/Logger.hpp"
#include "json.hpp"

#include "Bybit.hpp"

#include <tgbot/tgbot.h>

class BybitBot {
public:
    BybitBot(const std::string &apiKey);
    void run();

    void setAdmin(const s64 devId) { m_devId = devId; }

    void sendGreetings(const s64& chatId);
    void sendHelp(const s64& chatId);
    void sendBalance(const s64& chatId);
    // void sendP2POffers(const s64& chatId);
    void sendTickers(const s64& chatId);

    void sendApiKeySetup(const s64& chatId);
    void sendApiSecretSetup(const s64& chatId);

    void sendTokensSetup(const s64& chatId);
    void sendCurrenciesSetup(const s64& chatId);

    // void sendPaymentsSetup(const s64& chatId);

    void sendUnknownCommand(const s64& chatId);
private:
    void m_sendToUser(const s64& chatId, const std::string& message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard=nullptr);
    void m_sendToUser(const s64& chatId, const char *message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard=nullptr);

    void m_sendToAdmin(const std::string& message);
    void m_sendToAdmin(const char *message);

    void m_deleteMessage(const s64& chatId, const s64& messageId);

    // TgBot::InlineKeyboardMarkup::Ptr m_paymentsKeyboard(const s64& chatId);
    // TgBot::InlineKeyboardMarkup::Ptr m_offersKeyboard(std::array<BybitP2POffer, 10> offers);

    TgBot::InlineKeyboardMarkup::Ptr m_menuKeyboard(const s64& chatId);

    TgBot::InlineKeyboardMarkup::Ptr m_tokensKeyboard(const s64& chatId);
    TgBot::InlineKeyboardMarkup::Ptr m_currenciesKeyboard(const s64& chatId);

    // TODO
    // void m_sendToAll(const std::string& message);

    s64 m_devId = -1;

    /* DATA */
    struct BybitBotUserData {
        std::string apiKey;
        std::string apiSecret;
        // std::list<u64> paymentMethods; // indexes
        std::list<std::string> tokens;
        std::list<std::string> currencies;
    };
    std::unordered_map<s64, BybitBotUserData> m_data;

    void m_saveData();
    void m_loadData();

    /* STATE */
    enum BybitBotUserState {
        IDLE,
        API_KEY_INPUT,
        API_SECRET_INPUT,
        // PAYMENTS_INPUT
        TOKENS_INPUT,
        CURRENCIES_INPUT
    };
    std::unordered_map<s64, BybitBotUserState> m_state;

    void m_saveState();
    void m_loadState();

    const std::list<std::string> m_menu = {
        "Balance",
        "Tickers",
        "API_KEY setup",
        "API_SECRET setup",
        "Tokens setup",
        "Currencies setup",
        "Help"
    };

    /* tgbot objects */
    TgBot::Bot m_bot;
};

#endif // BOT_H