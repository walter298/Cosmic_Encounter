#include "JoinGame.h"

#include <boost/lexical_cast.hpp>

#include "novalis/Button.h"
#include "novalis/Sound.h"

namespace ranges = std::ranges;
namespace views  = std::views;

static bool connectToGame(const nv::Text& ipAddrInput, const nv::Text& portInput, Socket& sock) {
    //validate the port input
    if (!ranges::all_of(portInput.value(), isdigit)) {
        std::println("Error: {} contains non-digits", portInput.value());
        return false;
    }
    if (portInput.value().size() > 4 || portInput.value().empty()) {
        std::println("Error: the entered port {} must have [1, 4] digits", portInput.value());
        return false;
    }

    sys::error_code ec;

    //parse the ip address
    auto ipAddr = ip::address_v4::from_string(ipAddrInput.value().data(), ec);
    if (ec) {
        std::println("{}", ec.message());
        return false;
    }

    //connect
    tcp::endpoint endpoint{
        ip::address_v4::from_string(ipAddrInput.value().data(), ec),
        boost::lexical_cast<ip::port_type>(portInput.value()),
    };
    if (ec) {
        std::println("{}", ec.message());
        return false;
    }
    auto connectionError = sock.connect(endpoint);
    if (connectionError) {
        std::println("{}", connectionError->ec.message());
        return false;
    } else {
        return false;
    }
}

void joinGame(SDL_Renderer* renderer, Socket& sock, nv::TextureMap& texMap, nv::FontMap& fontMap) {
    nv::Scene scene{ "Cosmic_Encounter/game_assets/scenes/join_game.nv_scene", renderer, texMap, fontMap };

    constexpr int RECT_LAYER = 1;
    constexpr int TEXT_LAYER = 2;

    auto& ipAddrInputText = scene.find<nv::Text>(TEXT_LAYER, "ip_address_input").get();
    auto& portInputText   = scene.find<nv::Text>(TEXT_LAYER, "port_input").get();

    //connection button
    auto& joinButtonText = scene.find<nv::Text>(TEXT_LAYER, "join_button_text").get();
    auto& joinButtonRect = scene.find<nv::Rect>(RECT_LAYER, "join_button_rect").get();
    nv::Button joinButton{ 
        joinButtonText,
        joinButtonRect,
        [&] {
            auto ec = connectToGame(ipAddrInputText, portInputText, sock);
            if (!ec) {
                scene.running = false;
            } 
        },
        [&] { joinButtonRect.setRenderColor(34, 139, 34, 255); },
        [&] { joinButtonRect.setRenderColor(255, 255, 255, 255); }
    };
    scene.addEvent(std::move(joinButton));

    //text inputs
    scene.addTextInput({ scene.find<nv::Rect>(RECT_LAYER, "ip_address_input_rect").get(), ipAddrInputText});
    scene.addTextInput({ scene.find<nv::Rect>(RECT_LAYER, "port_input_rect").get(), portInputText});

    scene();
}
