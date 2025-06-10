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
    void sendP2POffers(const s64& chatId);

    void sendApiKeySetup(const s64& chatId);
    void sendApiSecretSetup(const s64& chatId);
    void sendPaymentsSetup(const s64& chatId);

    void sendUnknownCommand(const s64& chatId);
private:
    void m_sendToUser(const s64& chatId, const std::string& message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard=nullptr);
    void m_sendToUser(const s64& chatId, const char *message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard=nullptr);

    void m_sendToAdmin(const std::string& message);
    void m_sendToAdmin(const char *message);

    TgBot::InlineKeyboardMarkup::Ptr m_paymentsKeyboard(const s64& chatId);
    TgBot::InlineKeyboardMarkup::Ptr m_offersKeyboard(std::array<BybitP2POffer, 10> offers);

    // TODO
    // void m_sendToAll(const std::string& message);

    s64 m_devId = -1;

    /* DATA */
    struct BybitBotUserData {
        std::string apiKey;
        std::string apiSecret;
        std::list<u64> paymentMethods; // indexes
    };
    std::unordered_map<s64, BybitBotUserData> m_data;

    void m_saveData();
    void m_loadData();

    /* STATE */
    enum BybitBotUserState {
        IDLE,
        API_KEY_INPUT,
        API_SECRET_INPUT,
        PAYMENTS_INPUT
    };
    std::unordered_map<s64, BybitBotUserState> m_state;

    void m_saveState();
    void m_loadState();

    const std::list<std::string> m_commands = {
        "/balance",
        "/offers",
        "/payments",
        "/key",
        "/secret",
        "/help",
        "/start",
    };

    const std::list<std::string> m_admin_commands = {
        "/save",
    };

    const bool m_isMessageACommand(const std::string& message) {
        for (auto& command : m_commands) {
            if (StringTools::startsWith(message, command))
                return true;
        }
        for (auto& command : m_admin_commands) {
            if (StringTools::startsWith(message, command))
                return true;
        }
        return false;
    }

    /* tgbot objects */
    TgBot::Bot m_bot;
    TgBot::ReplyKeyboardMarkup::Ptr m_keyboard;
};

#endif // BOT_H