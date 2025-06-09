#include "Bot.hpp"

void BybitBot::run() {
    m_bot.getEvents().onCommand("start", [this](TgBot::Message::Ptr message) {
        sendGreetings(message->chat->id);
    });

    m_bot.getEvents().onCommand("help", [this](TgBot::Message::Ptr message) {
        sendHelp(message->chat->id);
    });

    m_bot.getEvents().onCommand("balance", [this](TgBot::Message::Ptr message) {
        sendBalance(message->chat->id);
    });

    m_bot.getEvents().onCommand("offers", [this](TgBot::Message::Ptr message) {
        sendP2POffers(message->chat->id);
    });

    m_bot.getEvents().onCommand("key", [this](TgBot::Message::Ptr message) {
        sendApiKeySetup(message->chat->id);
    });

    m_bot.getEvents().onCommand("secret", [this](TgBot::Message::Ptr message) {
        sendApiSecretSetup(message->chat->id);
    });

    m_bot.getEvents().onCommand("save", [this](TgBot::Message::Ptr message) {
        if (message->chat->id == m_devId) {
            m_saveState();
            m_saveData();
            m_sendToAdmin("Saved.\n");
        }
    });

    m_bot.getEvents().onAnyMessage([this](TgBot::Message::Ptr message) {
        if (m_state[message->chat->id] != BybitBotUserState::IDLE) {
            if (m_state[message->chat->id] == BybitBotUserState::API_KEY_INPUT) {
                m_data[message->chat->id].apiKey = message->text;
                m_state[message->chat->id] = BybitBotUserState::IDLE;
                m_sendToUser(message->chat->id, "API_KEY set\n");
            }
            if (m_state[message->chat->id] == BybitBotUserState::API_SECRET_INPUT) {
                m_data[message->chat->id].apiSecret = message->text;
                m_state[message->chat->id] = BybitBotUserState::IDLE;
                m_sendToUser(message->chat->id, "API_SECRET set\n");
            }
        }
        else if (!isMessageACommand(message->text)) {
            m_state[message->chat->id] = BybitBotUserState::IDLE;
            sendUnknownCommand(message->chat->id);
        }
    });

    m_loadState();
    m_loadData();

    try {
        TgBot::TgLongPoll longPoll(m_bot);
        m_sendToAdmin("Bot starting\n");
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        ERROR("error: %s\n", e.what());
    }
}

void BybitBot::sendGreetings(const s64& chatId) {
    const std::string greetingsMessage = "Hi! I am BybitBot. I can send you information about P2P market.\nUse /help command to get commands list\n";
    m_bot.getApi().sendMessage(chatId, greetingsMessage);
}

void BybitBot::sendHelp(const s64& chatId) {
    const std::string helpMessage = "/balance - your Bybit withdrawable amount\n/offers - P2P offers that suits your preferences\n";
    m_sendToUser(chatId, helpMessage);
}

void BybitBot::sendBalance(const s64& chatId) {
    f32 userBalance = getBalance(m_data[chatId].apiKey, m_data[chatId].apiSecret);
    const std::string balanceMessage = std::format("Your balance is {} USDT\n", std::to_string(userBalance));
    m_sendToUser(chatId, balanceMessage);
}

void BybitBot::sendP2POffers(const s64& chatId) {
    std::array<f32, 10> offers = getP2POffers();
    std::string offersMessage = std::format("Best 10 P2P offers are:\n");
    for (u64 i = 0; i < 10; i++) {
        offersMessage += std::format("{:2d}. {:.2f}\n", i+1, offers[i]);
    }
    m_sendToUser(chatId, offersMessage);
}

void BybitBot::sendApiKeySetup(const s64& chatId) {
    const std::string apiKeySetup = "Send your API_KEY from Bybit.\nMake it read-only and add permission for Assets.\n";
    m_state[chatId] = BybitBotUserState::API_KEY_INPUT;
    m_sendToUser(chatId, apiKeySetup);
}

void BybitBot::sendApiSecretSetup(const s64& chatId) {
    const std::string apiSecretSetup = "Send your API_SECRET from Bybit.\nIt will be avaliable with API_KEY.\n";
    m_state[chatId] = BybitBotUserState::API_SECRET_INPUT;
    m_sendToUser(chatId, apiSecretSetup);
}

void BybitBot::sendUnknownCommand(const s64& chatId) {
    const std::string& unkownCommandMessage = "Unknown command.\nUse /help to get list of all commands.\n";
    m_sendToUser(chatId, unkownCommandMessage);
}

void BybitBot::m_sendToUser(const s64& chatId, const std::string& message) {
    m_bot.getApi().sendMessage(chatId, message);
}

void BybitBot::m_sendToUser(const s64& chatId, const char *message) {
    m_bot.getApi().sendMessage(chatId, message);
}

void BybitBot::m_sendToAdmin(const std::string& message) {
    m_bot.getApi().sendMessage(m_devId, message);
}

void BybitBot::m_sendToAdmin(const char *message) {
    m_bot.getApi().sendMessage(m_devId, message);
}

void BybitBot::m_saveData() {
    nlohmann::json j;
    for (const auto& [id, userData] : m_data) {
        j[std::to_string(id)] = nlohmann::json{
            {"apiKey", userData.apiKey},
            {"apiSecret", userData.apiSecret}
        };
    }
    std::ofstream file("m_data.json");
    file << j.dump(4);
}

void BybitBot::m_loadData() {
    std::ifstream file("m_data.json");
    if (!file.is_open()) {
        WARN("m_data.json not found\n");
        return;
    }
    INFO("m_data.json is found\n");

    nlohmann::json j;
    file >> j;
    m_data.clear();
    for (auto& [key, value] : j.items()) {
        s64 id = std::stoll(key);
        m_data[id] = BybitBotUserData({
            value["apiKey"].get<std::string>(),
            value["apiSecret"].get<std::string>()
        });
    }
}

void BybitBot::m_saveState() {
    nlohmann::json j;
    for (const auto& [id, userState] : m_state) {
        j[std::to_string(id)] = static_cast<int>(userState);
    }
    std::ofstream file("m_state.json");
    file << j.dump(4);
}

void BybitBot::m_loadState() {
    std::ifstream file("m_state.json");
    if (!file.is_open()) {
         WARN("m_state.json not found\n");
        return;
    }

    INFO("m_state.json is found\n");

    nlohmann::json j;
    file >> j;
    m_state.clear();
    for (auto& [key, value] : j.items()) {
        s64 id = std::stoll(key);
        m_state[id] = static_cast<BybitBotUserState>(value.get<int>());
    }
}

int main() {

    /* Load config */
    std::ifstream config_file("config.json");
    if (!config_file.is_open()) {
        ERROR("config.json not found\n");
        return 1;
    }
    
    auto config = nlohmann::json::parse(config_file);
    const std::string& API_KEY = config["API_KEY"];
    const s64& DEV_ID = config["DEV_ID"].get<s64>(); 
    
    /* Start bot */
    BybitBot bot(API_KEY);
    bot.setAdmin(DEV_ID);
    bot.run();
    
    return 0;
}