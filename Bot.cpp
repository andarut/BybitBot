#include "Bot.hpp"

BybitBot::BybitBot(const std::string &apiKey) : m_bot(apiKey) {
    /* Setup keyboard */
    m_keyboard = TgBot::ReplyKeyboardMarkup::Ptr(new TgBot::ReplyKeyboardMarkup);
    for (auto& command : m_commands) {
        std::vector<TgBot::KeyboardButton::Ptr> row;
        TgBot::KeyboardButton::Ptr button(new TgBot::KeyboardButton);
        button->text = command;
        row.push_back(button);
        m_keyboard->keyboard.push_back(row);
    }
}

void BybitBot::run() {

    m_bot.getEvents().onCommand("start", [this](TgBot::Message::Ptr message) {
        INFO("id = %d\n", message->chat->id);
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

    m_bot.getEvents().onCommand("payments", [this](TgBot::Message::Ptr message) {
        sendPaymentsSetup(message->chat->id);
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
        else if (!m_isMessageACommand(message->text)) {
            m_state[message->chat->id] = BybitBotUserState::IDLE;
            sendUnknownCommand(message->chat->id);
        }
    });

    m_bot.getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query) {
        if (m_state[query->message->chat->id] == BybitBotUserState::PAYMENTS_INPUT) {
            INFO("QUERY DATA = %s\n", query->data.c_str());
            u64 data = stoi(query->data);
            if (data == -1) {
                // Done
                m_state[query->message->chat->id] = BybitBotUserState::IDLE;
                m_bot.getApi().editMessageText("Payment methods are set",
                    query->message->chat->id,
                    query->message->messageId,
                    query->inlineMessageId
                );
            } else {
                if (std::find(m_data[query->message->chat->id].paymentMethods.begin(), m_data[query->message->chat->id].paymentMethods.end(), data) != m_data[query->message->chat->id].paymentMethods.end())
                    m_data[query->message->chat->id].paymentMethods.remove(data);
                else
                    m_data[query->message->chat->id].paymentMethods.push_back(data);
                m_bot.getApi().editMessageReplyMarkup(
                    query->message->chat->id,
                    query->message->messageId,
                    query->inlineMessageId,
                    m_paymentsKeyboard(query->message->chat->id)
                );
            }
        }
    });

    m_loadState();
    m_loadData();

    try {
        TgBot::TgLongPoll longPoll(m_bot);
        m_bot.getApi().deleteWebhook();
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

TgBot::InlineKeyboardMarkup::Ptr BybitBot::m_paymentsKeyboard(const s64& chatId) {
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);

    std::vector<TgBot::InlineKeyboardButton::Ptr> row0;

    for (auto& paymentMethod : supportedPaymentMethods) {
        TgBot::InlineKeyboardButton::Ptr paymentMethodButton(new TgBot::InlineKeyboardButton);
        if (std::find(m_data[chatId].paymentMethods.begin(), m_data[chatId].paymentMethods.end(), paymentMethod.index) != m_data[chatId].paymentMethods.end())
            paymentMethodButton->text = std::format("✅ {}", paymentMethod.text);
        else
            paymentMethodButton->text = std::format("❌ {}", paymentMethod.text);
        paymentMethodButton->callbackData = std::to_string(paymentMethod.index);
        row0.push_back(paymentMethodButton);
    }

    std::vector<TgBot::InlineKeyboardButton::Ptr> done_row;
    TgBot::InlineKeyboardButton::Ptr doneButton(new TgBot::InlineKeyboardButton);
    doneButton->text = "Done!";
    doneButton->callbackData = std::to_string(-1);
    done_row.push_back(doneButton);

    keyboard->inlineKeyboard.push_back(row0);
    keyboard->inlineKeyboard.push_back(done_row);

    return keyboard;
}

void BybitBot::sendPaymentsSetup(const s64& chatId) {
    const std::string paymentsMessage = "Select your payments methods:\n";
    m_state[chatId] = BybitBotUserState::PAYMENTS_INPUT;
    m_sendToUser(chatId, paymentsMessage, m_paymentsKeyboard(chatId));
}

/* TODO: send including current balance */
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

void BybitBot::m_sendToUser(const s64& chatId, const std::string& message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard) {
    if (keyboard == nullptr)
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, m_keyboard);
    else
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, keyboard);
}

void BybitBot::m_sendToUser(const s64& chatId, const char *message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard) {
    if (keyboard == nullptr)
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, m_keyboard);
    else
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, keyboard);
}

void BybitBot::m_sendToAdmin(const std::string& message) {
    if (m_devId != -1)
        m_bot.getApi().sendMessage(m_devId, message, nullptr, nullptr, m_keyboard);
    else
        WARN("m_devId is -1");
}

void BybitBot::m_sendToAdmin(const char *message) {
    if (m_devId != -1)
        m_bot.getApi().sendMessage(m_devId, message, nullptr, nullptr, m_keyboard);
    else
        WARN("m_devId is -1");
}

void BybitBot::m_saveData() {
    nlohmann::json j;
    for (const auto& [id, userData] : m_data) {
        j[std::to_string(id)] = nlohmann::json{
            {"apiKey", userData.apiKey},
            {"apiSecret", userData.apiSecret},
            {"paymentMethods", userData.paymentMethods}
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
            value["apiSecret"].get<std::string>(),
            value["paymentMethods"].get<std::list<u64>>()
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