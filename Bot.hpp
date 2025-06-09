#ifndef BOT_H
#define BOT_H

#include "Utils/Utils.hpp"
#include "Utils/Logger.hpp"
#include "json.hpp"

#include "Bybit.hpp"

#include <tgbot/tgbot.h>

class BybitBot {
public:
    BybitBot(const std::string &apiKey) : m_bot(apiKey) {};
    void run();

    void setAdmin(const s64 devId) { m_devId = devId; }

    void sendGreetings(const s64& chatId);
    void sendHelp(const s64& chatId);
    void sendBalance(const s64& chatId);
    void sendP2POffers(const s64& chatId);

    void sendApiKeySetup(const s64& chatId);
    void sendApiSecretSetup(const s64& chatId);

    void sendUnknownCommand(const s64& chatId);
private:
    void m_sendToUser(const s64& chatId, const std::string& message);
    void m_sendToUser(const s64& chatId, const char *message);

    void m_sendToAdmin(const std::string& message);
    void m_sendToAdmin(const char *message);

    // TODO
    // void m_sendToAll(const std::string& message);

    s64 m_devId;
    TgBot::Bot m_bot;

    /* DATA */
    struct BybitBotUserData {
        std::string apiKey;
        std::string apiSecret;
    };
    std::unordered_map<s64, BybitBotUserData> m_data;

    void m_saveData();
    void m_loadData();

    /* STATE */
    enum BybitBotUserState {
        IDLE,
        API_KEY_INPUT,
        API_SECRET_INPUT
    };
    std::unordered_map<s64, BybitBotUserState> m_state;

    void m_saveState();
    void m_loadState();

    const std::list<std::string> m_commands = {
        /* User commands */
        "/start",
        "/help",
        "/balance",
        "/offers",
        "/key",
        "/secret",
        /* Admin commands */
        "/save",
    };

    const bool isMessageACommand(const std::string& message) {
        for (auto& command : m_commands) {
            if (StringTools::startsWith(message, command))
                return true;
        }
        return false;
    }
};

#endif // BOT_H