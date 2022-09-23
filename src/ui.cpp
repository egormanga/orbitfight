#include "camera.hpp"
#include "globals.hpp"
#include "math.hpp"
#include "ui.hpp"

#include <SFML/Graphics.hpp>

using namespace obf;

namespace obf {

int wrapText(std::string& string, sf::Text& text, float maxWidth) {
    int newlines = 0;
    text.setString(string);
    for (size_t i = 0; i < string.size(); i++) {
        if (text.findCharacterPos(i).x - text.findCharacterPos(0).x > maxWidth) {
            if (i == 0) [[unlikely]] {
                throw std::runtime_error("Couldn't wrap text: width too small.");
            }
            string.insert(i - 1, 1, '\n');
            text.setString(string);
            newlines++;
        }
    }
    return newlines;
}

UIElement::UIElement() {
    body.setOutlineThickness(2.f);
    body.setOutlineColor(sf::Color(32, 32, 32, 192));
    body.setFillColor(sf::Color(64, 64, 64, 64));
    body.setPosition(0.f, 0.f);
    body.setSize(sf::Vector2f(0.f, 0.f));
}

void UIElement::update() {
    window->draw(body);
}

void UIElement::resized() {
    float lerpf = std::max(std::min(1.f, (g_camera.w - widestAdjustAt) / narrowestAdjustAt), 0.f);
    width = g_camera.w * (lerpf * mulWidthMin + (1.f - lerpf) * mulWidthMax);
    lerpf = std::max(std::min(1.f, (g_camera.h - tallestAdjustAt) / shortestAdjustAt), 0.f);
    height = g_camera.h * (lerpf * mulHeightMin + (1.f - lerpf) * mulHeightMax);
}

bool UIElement::isMousedOver() {
    return mousePos.x > x && mousePos.y > y && mousePos.x < x + width && mousePos.y < y + height;
}

void UIElement::pressed(sf::Mouse::Button button) {
    return;
}

MiscInfoUI::MiscInfoUI() {
    text.setFont(*font);
    text.setCharacterSize(textCharacterSize);
    text.setFillColor(sf::Color::White);
    text.setPosition(padding, padding);
}

void MiscInfoUI::update() {
    std::string info = "";
    info.append("FPS: ").append(std::to_string(framerate))
    .append("\nPing: ").append(std::to_string((int)(lastPing * 1000.0))).append("ms");
    if (lastTrajectoryRef) {
        info.append("\nDistance: ").append(std::to_string((int)(dst(ownX - lastTrajectoryRef->x, ownY - lastTrajectoryRef->y))));
        if (ownEntity) [[likely]] {
            info.append("\nVelocity: ").append(std::to_string((int)(dst(ownEntity->velX - lastTrajectoryRef->velX, ownEntity->velY - lastTrajectoryRef->velY) * 60.0)));
        }
    }
    wrapText(info, text, width - padding * 2.f);
    sf::FloatRect bounds = text.getLocalBounds();
    float actualWidth = bounds.width + padding * 2.f;
    height = bounds.height + padding * 2.f;
    body.setSize(sf::Vector2f(actualWidth, height));
    UIElement::update();
    window->draw(text);
}

ChatUI::ChatUI() {
    text.setFont(*font);
    text.setCharacterSize(textCharacterSize);
    text.setFillColor(sf::Color::White);
    mulWidthMax = 0.5f;
    mulWidthMin = 0.25f;
    mulHeightMax = 0.5f;
    mulHeightMin = 0.24f;
}

void ChatUI::update() {
    std::string chatString = "";
    int displayMessageCount = std::min((int)(height / (textCharacterSize + 3)) - 1, storedMessageCount);
    messageCursorPos = std::max(0, std::min(messageCursorPos, storedMessageCount - displayMessageCount));
    chatString.append("> " + chatBuffer);
    for (int i = messageCursorPos; i < messageCursorPos + displayMessageCount; i++) {
        chatString.insert(0, storedMessages[i] + "\n");
    }
    int overshoot = wrapText(chatString, text, width - padding * 2.f);
    for (int i = 0; i < overshoot; i++) {
        for (size_t c = 0; c < chatString.size(); c++) {
            if (chatString[c] == '\n' && c + 1 < chatString.size()) {
                chatString = chatString.substr(c + 1, chatString.size());
                break;
            }
        }
    }
    if (chatting && std::sin(globalTime * TAU * 1.5) > 0.0) {
        chatString.append("|");
    }
    text.setString(chatString);
    displayMessageCount++;
    text.setPosition(padding, g_camera.h - (textCharacterSize + 3) * displayMessageCount - padding);
    UIElement::update();
    window->draw(text);
}

void ChatUI::resized() {
    float lerpf = std::max(std::min(1.f, (g_camera.w - widestAdjustAt) / narrowestAdjustAt), 0.f);
    width = std::min((float)(messageLimit * (textCharacterSize + 2)), g_camera.w * (lerpf * mulWidthMin + (1.f - lerpf) * mulWidthMax));
    lerpf = std::max(std::min(1.f, (g_camera.h - tallestAdjustAt) / shortestAdjustAt), 0.f);
    height = std::min((float)(storedMessageCount + 1) * (textCharacterSize + 3), g_camera.h * (lerpf * mulHeightMin + (1.f - lerpf) * mulHeightMax));
    body.setPosition(0.f, g_camera.h - height);
    body.setSize(sf::Vector2f(width, height));
}

}
