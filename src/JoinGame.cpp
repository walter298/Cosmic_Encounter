#include "JoinGame.h"

#include "novalis/Button.h"
#include "novalis/Sound.h"
#include "novalis/Scene.h"

namespace ranges = std::ranges;

static sys::error_code connectToGame(const nv::Text& ipAddrInput, const nv::Text& portInput, tcp::socket& sock) {
    //parse the port value
    if (!ranges::all_of(portInput.value(), isdigit)) {
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

void joinGame(SDL_Renderer* renderer, tcp::socket& sock) {
    nv::Scene scene{ "Cosmic_Encounter/game_assets/scenes/join_game.nv_scene", renderer };

    auto findObj = [](auto& objs, std::string_view name) -> auto& {
        return *ranges::find_if(objs, [&](const auto& obj) {
            return obj.getName() == name;
        });
    };
    auto& ipAddrInputText = findObj(scene.text.at(0), "ip_address_input");
    auto& portInputText   = findObj(scene.text.at(0), "port_input");

    //connection button
    auto& joinButtonRect = findObj(scene.rects.at(0), "join_button_rect");
    nv::Button joinButton{ 
        joinButtonRect,
        [&] {
            if (!connectToGame(ipAddrInputText, portInputText, sock)) {
                scene.running = false;
            }
        },
        [&] { joinButtonRect.setRenderColor(34, 139, 34, 255); },
        [&] { joinButtonRect.setRenderColor(255, 255, 255, 255); }
    };
    scene.eventHandler.addMouseEvent(std::move(joinButton));

    //text inputs
    scene.eventHandler.addTextInput({ findObj(scene.rects.at(0), "ip_address_input_rect"), ipAddrInputText });
    scene.eventHandler.addTextInput({ findObj(scene.rects.at(0), "port_input_rect"), portInputText });

    nv::Music music{ nv::relativePath("Cosmic_Encounter/game_assets/music/mars.wav") };
    music.play(9000000);
    scene();
}
