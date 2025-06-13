#include "Bot.hpp"

BybitBot::BybitBot(const std::string &apiKey) : m_bot(apiKey) {
   
}

void BybitBot::run() {

    m_bot.getEvents().onCommand("start", [this](TgBot::Message::Ptr message) {
        m_sendToUser(message->chat->id, "Welcome\n", m_menuKeyboard(message->chat->id));
    });

    /* only admin commands */
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
    });

    m_bot.getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query) {
        // if (m_state[query->message->chat->id] == BybitBotUserState::PAYMENTS_INPUT) {
        //     u64 data = stoi(query->data);
        //     if (data == -1) {
        //         // Done
        //         m_state[query->message->chat->id] = BybitBotUserState::IDLE;
        //         m_bot.getApi().editMessageText("Payment methods are set",
        //             query->message->chat->id,
        //             query->message->messageId,
        //             query->inlineMessageId
        //         );
        //     } else {
        //         if (std::find(m_data[query->message->chat->id].paymentMethods.begin(), m_data[query->message->chat->id].paymentMethods.end(), data) != m_data[query->message->chat->id].paymentMethods.end())
        //             m_data[query->message->chat->id].paymentMethods.remove(data);
        //         else
        //             m_data[query->message->chat->id].paymentMethods.push_back(data);
        //         m_bot.getApi().editMessageReplyMarkup(
        //             query->message->chat->id,
        //             query->message->messageId,
        //             query->inlineMessageId,
        //             m_paymentsKeyboard(query->message->chat->id)
        //         );
        //     }
        // }

        if (m_state[query->message->chat->id] == BybitBotUserState::IDLE) {
            std::string data = query->data;
            if (data == "Balance") {
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                sendBalance(query->message->chat->id);   
            }
            if (data == "Tickers") {
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                sendTickers(query->message->chat->id);
            }
            if (data == "API_KEY setup") {
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                sendApiKeySetup(query->message->chat->id);
            }
            if (data == "API_SECRET setup") {
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                sendApiSecretSetup(query->message->chat->id);
            }
            if (data == "Tokens setup") {
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                sendTokensSetup(query->message->chat->id);
            }
            if (data == "Currencies setup") {
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                sendCurrenciesSetup(query->message->chat->id);
            }
            if (data == "Help") {
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                sendHelp(query->message->chat->id);
            }

            // Done
                // m_state[query->message->chat->id] = BybitBotUserState::IDLE;
                // m_bot.getApi().editMessageText("Tokens are set",
                //     query->message->chat->id,
                //     query->message->messageId,
                //     query->inlineMessageId
                // );
            
            // else {
            //     if (std::find(m_data[query->message->chat->id].tokens.begin(), m_data[query->message->chat->id].tokens.end(), data) != m_data[query->message->chat->id].tokens.end())
            //         m_data[query->message->chat->id].tokens.remove(data);
            //     else
            //         m_data[query->message->chat->id].tokens.push_back(data);
            //     m_bot.getApi().editMessageReplyMarkup(
            //         query->message->chat->id,
            //         query->message->messageId,
            //         query->inlineMessageId,
            //         m_tokensKeyboard(query->message->chat->id)
            //     );
            // }
        }

        if (std::find(m_menu.begin(), m_menu.end(), query->data) != m_menu.end()) return;

        if (m_state[query->message->chat->id] == BybitBotUserState::TOKENS_INPUT) { 
            INFO("query->data = %s\n", query->data.c_str());
            std::string data = query->data;
            if (data == "-1") {
                m_state[query->message->chat->id] = BybitBotUserState::IDLE;
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                m_sendToUser(query->message->chat->id, "Tokens are set", m_menuKeyboard(query->message->chat->id));
            } else {
                if (std::find(m_data[query->message->chat->id].tokens.begin(), m_data[query->message->chat->id].tokens.end(), data) != m_data[query->message->chat->id].tokens.end())
                    m_data[query->message->chat->id].tokens.remove(data);
                else
                    m_data[query->message->chat->id].tokens.push_back(data);
                m_bot.getApi().editMessageReplyMarkup(
                    query->message->chat->id,
                    query->message->messageId,
                    query->inlineMessageId,
                    m_tokensKeyboard(query->message->chat->id)
                );
            }
        }

        if (m_state[query->message->chat->id] == BybitBotUserState::CURRENCIES_INPUT) {
            std::string data = query->data;
            if (data == "-1") {
                m_state[query->message->chat->id] = BybitBotUserState::IDLE;
                m_deleteMessage(query->message->chat->id, query->message->messageId);
                m_sendToUser(query->message->chat->id, "Currencies are set", m_menuKeyboard(query->message->chat->id));
            } else {
                if (std::find(m_data[query->message->chat->id].currencies.begin(), m_data[query->message->chat->id].currencies.end(), data) != m_data[query->message->chat->id].currencies.end())
                    m_data[query->message->chat->id].currencies.remove(data);
                else
                    m_data[query->message->chat->id].currencies.push_back(data);
                m_bot.getApi().editMessageReplyMarkup(
                    query->message->chat->id,
                    query->message->messageId,
                    query->inlineMessageId,
                    m_currenciesKeyboard(query->message->chat->id)
                );
            }
        }
    });

    m_loadState();
    m_loadData();

    try {
        TgBot::TgLongPoll longPoll(m_bot);
        // m_bot.getApi().deleteWebhook();
        // m_sendToAdmin("Bot starting\n");
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
    f32 userBalance = getBalance("USDT", m_data[chatId].apiKey, m_data[chatId].apiSecret);
    const std::string balanceMessage = std::format("Your balance is {} USDT\n", std::to_string(userBalance));
    m_sendToUser(chatId, balanceMessage);
}

// TgBot::InlineKeyboardMarkup::Ptr BybitBot::m_paymentsKeyboard(const s64& chatId) {
//     TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);

//     std::vector<TgBot::InlineKeyboardButton::Ptr> row0;

//     for (auto& paymentMethod : supportedPaymentMethods) {
//         TgBot::InlineKeyboardButton::Ptr paymentMethodButton(new TgBot::InlineKeyboardButton);
//         if (std::find(m_data[chatId].paymentMethods.begin(), m_data[chatId].paymentMethods.end(), paymentMethod.index) != m_data[chatId].paymentMethods.end())
//             paymentMethodButton->text = std::format("✅ {}", paymentMethod.text);
//         else
//             paymentMethodButton->text = std::format("❌ {}", paymentMethod.text);
//         paymentMethodButton->callbackData = std::to_string(paymentMethod.index);
//         row0.push_back(paymentMethodButton);
//     }

//     std::vector<TgBot::InlineKeyboardButton::Ptr> done_row;
//     TgBot::InlineKeyboardButton::Ptr doneButton(new TgBot::InlineKeyboardButton);
//     doneButton->text = "Done!";
//     doneButton->callbackData = std::to_string(-1);
//     done_row.push_back(doneButton);

//     keyboard->inlineKeyboard.push_back(row0);
//     keyboard->inlineKeyboard.push_back(done_row);

//     return keyboard;
// }

// TgBot::InlineKeyboardMarkup::Ptr BybitBot::m_offersKeyboard(std::array<BybitP2POffer, 10> offers) {
//     TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);    

//     for (auto& offer : offers) {
//         std::vector<TgBot::InlineKeyboardButton::Ptr> row;
//         TgBot::InlineKeyboardButton::Ptr offerButton(new TgBot::InlineKeyboardButton);
//         offerButton->text = std::format("Price: {:.2f} | Rate: {:2d}% | Offers: {:4d}", offer.price, offer.recentExecuteRate, offer.recentOrderNum);
//         offerButton->url = offer.link;
//         offerButton->callbackData = offer.price;
//         row.push_back(offerButton);
//         keyboard->inlineKeyboard.push_back(row);
//     }

//     return keyboard;
// }

// void BybitBot::sendPaymentsSetup(const s64& chatId) {
//     const std::string paymentsMessage = "Select your payments methods:\n";
//     m_state[chatId] = BybitBotUserState::PAYMENTS_INPUT;
//     m_sendToUser(chatId, paymentsMessage, m_paymentsKeyboard(chatId));
// }

// void BybitBot::sendP2POffers(const s64& chatId) {
//     auto offers = getP2POffers("USDT", "RUB", 20000);
//     m_sendToUser(chatId, "Top 10 offers:\n", m_offersKeyboard(offers));
// }

TgBot::InlineKeyboardMarkup::Ptr BybitBot::m_menuKeyboard(const s64& chatId) {
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);

    for (auto& menu : m_menu) {
        std::vector<TgBot::InlineKeyboardButton::Ptr> row;
        TgBot::InlineKeyboardButton::Ptr menuButton(new TgBot::InlineKeyboardButton);
        menuButton->text = menu;
        menuButton->callbackData = menu;
        row.push_back(menuButton);
        keyboard->inlineKeyboard.push_back(row);
    }

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BybitBot::m_tokensKeyboard(const s64& chatId) {
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);

    auto coinList = getCoinList(m_data[chatId].apiKey, m_data[chatId].apiSecret);

    for (auto& token : coinList) {
        std::vector<TgBot::InlineKeyboardButton::Ptr> row;
        TgBot::InlineKeyboardButton::Ptr tokenButton(new TgBot::InlineKeyboardButton);
        if (std::find(m_data[chatId].tokens.begin(), m_data[chatId].tokens.end(), token) != m_data[chatId].tokens.end())
            tokenButton->text = std::format("✅ {}", token);
        else
            tokenButton->text = std::format("❌ {}", token);
        tokenButton->callbackData = token;
        row.push_back(tokenButton);
        keyboard->inlineKeyboard.push_back(row);
    }

    std::vector<TgBot::InlineKeyboardButton::Ptr> done_row;
    TgBot::InlineKeyboardButton::Ptr doneButton(new TgBot::InlineKeyboardButton);
    doneButton->text = "Done!";
    doneButton->callbackData = std::to_string(-1);
    done_row.push_back(doneButton);

    
    keyboard->inlineKeyboard.push_back(done_row);

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BybitBot::m_currenciesKeyboard(const s64& chatId) {
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);

    std::vector<TgBot::InlineKeyboardButton::Ptr> row0;

    for (auto& currency : currenciesList) {
        TgBot::InlineKeyboardButton::Ptr currencyButton(new TgBot::InlineKeyboardButton);
        if (std::find(m_data[chatId].currencies.begin(), m_data[chatId].currencies.end(), currency) != m_data[chatId].currencies.end())
            currencyButton->text = std::format("✅ {}", currency);
        else
            currencyButton->text = std::format("❌ {}", currency);
        currencyButton->callbackData = currency;
        row0.push_back(currencyButton);
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

void BybitBot::sendTickers(const s64& chatId) {
    auto& tokens     = m_data[chatId].tokens;
    auto& currencies = m_data[chatId].currencies;
    auto& apiKey     = m_data[chatId].apiKey;
    auto& apiSecret  = m_data[chatId].apiSecret;

    auto& tickers = getTickers(tokens, currencies, apiKey, apiSecret);

    std::string tickers_message = "Tickers\n\n";
    for (auto& ticker : tickers) {
        tickers_message += std::format("Symbol = {}\n",  ticker.symbol);
        tickers_message +=
            std::format("* Last price = {:.02f}\n", ticker.lastPrice) + \
            std::format("* Index price = {:.02f}\n", ticker.indexPrice) + \
            std::format("* Mark price = {:.02f}\n", ticker.markPrice) + \
            std::format("* Bid1 price = {:.02f}\n", ticker.bid1Price) + \
            std::format("* Ask1 price = {:.02f}\n", ticker.ask1Price) + "\n\n";
    }

    m_sendToUser(chatId, tickers_message);
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

void BybitBot::sendTokensSetup(const s64& chatId) {
    const std::string tokenSetupMessage = "Select your tokens\n";
    m_state[chatId] = BybitBotUserState::TOKENS_INPUT;
    m_sendToUser(chatId, tokenSetupMessage, m_tokensKeyboard(chatId));
}

void BybitBot::sendCurrenciesSetup(const s64& chatId) {
    const std::string currenciesSetupMessage = "Select your currencies\n";
    m_state[chatId] = BybitBotUserState::CURRENCIES_INPUT;
    m_sendToUser(chatId, currenciesSetupMessage, m_currenciesKeyboard(chatId));
}

void BybitBot::sendUnknownCommand(const s64& chatId) {
    const std::string& unkownCommandMessage = "Unknown command.\nUse /help to get list of all commands.\n";
    m_sendToUser(chatId, unkownCommandMessage, m_menuKeyboard(chatId));
}

void BybitBot::m_sendToUser(const s64& chatId, const std::string& message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard) {
    if (keyboard == nullptr)
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, m_menuKeyboard(chatId));
    else
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, keyboard);
}

void BybitBot::m_sendToUser(const s64& chatId, const char *message, const TgBot::InlineKeyboardMarkup::Ptr& keyboard) {
    if (keyboard == nullptr)
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, m_menuKeyboard(chatId));
    else
        m_bot.getApi().sendMessage(chatId, message, nullptr, nullptr, keyboard);
}

void BybitBot::m_sendToAdmin(const std::string& message) {
    if (m_devId != -1)
        m_bot.getApi().sendMessage(m_devId, message, nullptr, nullptr);
    else
        WARN("m_devId is -1");
}

void BybitBot::m_sendToAdmin(const char *message) {
    if (m_devId != -1)
        m_bot.getApi().sendMessage(m_devId, message, nullptr, nullptr);
    else
        WARN("m_devId is -1");
}

void BybitBot::m_deleteMessage(const s64& chatId, const s64& messageId) {
    m_bot.getApi().deleteMessage(chatId, messageId);
}

void BybitBot::m_saveData() {
    nlohmann::json j;
    for (const auto& [id, userData] : m_data) {
        j[std::to_string(id)] = nlohmann::json{
            {"apiKey", userData.apiKey},
            {"apiSecret", userData.apiSecret},
            {"tokens", userData.tokens},
            {"currencies", userData.currencies}
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
            value["tokens"].get<std::list<std::string>>(),
            value["currencies"].get<std::list<std::string>>()
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