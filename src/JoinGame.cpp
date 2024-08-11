#include "JoinGame.h"

#include "novalis/Button.h"
#include "novalis/Sound.h"

namespace ranges = std::ranges;
namespace views  = std::views;

static sys::error_code connectToGame(const nv::Text& ipAddrInput, const nv::Text& portInput, Socket& sock) {
    //validate the port input
    if (!ranges::all_of(portInput.value(), isdigit)) {
        std::println("Error: {} contains non-digits", portInput.value());
        return sys::errc::make_error_code(sys::errc::bad_message);
    }
    if (portInput.value().size() > 4) {
        std::println("Error: {} has more than 4 digits", portInput.value());
        return sys::errc::make_error_code(sys::errc::bad_message);
    }

    sys::error_code ec;

    //parse the ip address
    auto ipAddr = ip::address_v4::from_string(ipAddrInput.value().data(), ec);
    if (ec) {
        return ec;
    }

    //connect
    tcp::endpoint endpoint{
        ip::address_v4::from_string(ipAddrInput.value().data(), ec),
        boost::lexical_cast<ip::port_type>(portInput.value()),
    };
    if (ec) {
        return ec;
    }
    sock.connect(endpoint, ec);
    return ec;
}

void joinGame(SDL_Renderer* renderer, Socket& sock, nv::TextureMap& texMap, nv::FontMap& fontMap) {
    nv::Scene scene{ "Cosmic_Encounter/game_assets/scenes/join_game.nv_scene", renderer, texMap, fontMap };

    constexpr int RECT_LAYER = 1;
    constexpr int TEXT_LAYER = 2;

    auto& ipAddrInputText = scene.find<nv::Text>(TEXT_LAYER, "ip_address_input");
    auto& portInputText   = scene.find<nv::Text>(TEXT_LAYER, "port_input");

    //connection button
    auto& joinButtonRect = scene.find<nv::Rect>(RECT_LAYER, "join_button_rect");
    nv::Button joinButton{ 
        joinButtonRect,
        [&] {
            auto ec = connectToGame(ipAddrInputText, portInputText, sock);
            if (!ec) {
                scene.running = false;
            } else {
                std::println("{}", ec.message());
            }
        },
        [&] { joinButtonRect.setRenderColor(34, 139, 34, 255); },
        [&] { joinButtonRect.setRenderColor(255, 255, 255, 255); }
    };
    scene.addEvent(std::move(joinButton));

    //text inputs
    scene.addTextInput({ scene.find<nv::Rect>(RECT_LAYER, "ip_address_input_rect"), ipAddrInputText });
    scene.addTextInput({ scene.find<nv::Rect>(RECT_LAYER, "port_input_rect"), portInputText });

    scene();
}
