#include "keyboard.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

constexpr float PI = 3.14159;
constexpr float SPEED = 2.0;
constexpr int FOV = 60;

constexpr int MAP_SIZE = 20;
constexpr int TILE_SIZE = 64;
constexpr int MINIMAP_TILE_SIZE = 16;
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

constexpr int EYE_HEIGHT = 32;

float rads(float degs)
{
    return degs * PI / 180.f;
}

float wrap(float angle)
{
    if (angle < 0) {
        return angle + 360;
    }
    else if (angle > 360) {
        return angle - 360;
    }
    else {
        return angle;
    }
}

float distance(const sf::Vector2f& vecA, sf::Vector2f& vectB)
{
    float dx = (vecA.x - vectB.x);
    float dy = (vecA.y - vectB.y);
    return std::sqrt(dx * dx + dy * dy);
}

struct Map {
    // clang-format off
    const std::vector<int> MAP = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    // clang-format on

    Map()
    {
        assert(MAP.size() == (MAP_SIZE * MAP_SIZE));
    }

    int getTile(int x, int y) const
    {
        if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE)
            return 0;
        return MAP[y * MAP_SIZE + x];
    }
};

struct Player {
    float x = 153;
    float y = 221;
    float angle = 0;

    // How much the player moves in the X/ Y direction when moving forwards or back
    float dx = 0;
    float dy = 0;

    sf::RectangleShape rayCastSprite;

    Player()
    {
        rayCastSprite.setSize({10.0f, 10.0f});
        dx = std::cos(angle);
        dy = std::sin(angle);
    }

    void doInput(const Keyboard& keys)
    {
        float a = rads(angle);
        if (keys.isKeyDown(sf::Keyboard::W)) {
            x += dx * SPEED;
            y += dy * SPEED;
        }
        if (keys.isKeyDown(sf::Keyboard::A)) {
            angle -= SPEED;
            if (angle < 0) {
                angle += 360;
            }
            dx = std::cos(a);
            dy = std::sin(a);
        }
        if (keys.isKeyDown(sf::Keyboard::S)) {
            x -= dx * SPEED;
            y -= dy * SPEED;
        }
        if (keys.isKeyDown(sf::Keyboard::D)) {
            angle += SPEED;
            if (angle > 360) {
                angle -= 360;
            }
            dx = std::cos(a);
            dy = std::sin(a);
        }
    }

    void draw(sf::RenderTexture& window)
    {
        rayCastSprite.setPosition(x / 4 - 5, y / 4 - 5);
        window.draw(rayCastSprite);
    }
};

struct Drawbuffer {
    std::vector<sf::Uint8> pixels;
    std::vector<sf::Vertex> line;

    Drawbuffer()
        : pixels((WINDOW_WIDTH)*WINDOW_HEIGHT * 4)
    {
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
    }

    // For the minimap
    void drawLine(sf::RenderTarget& target, const sf::Vector2f& begin,
                  const sf::Vector2f& end, sf::Color colour)
    {
        line[0].position = begin;
        line[1].position = end;
        line[0].color = colour;
        line[1].color = colour;
        target.draw(line.data(), 2, sf::PrimitiveType::Lines);
    }

    void clear()
    {
        std::memset(pixels.data(), 0, pixels.size());
    }

    void set(int x, int y, sf::Uint8 red, sf::Uint8 green, sf::Uint8 blue)
    {
        if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
            return;
        }
        sf::Uint8* ptr = &pixels.at((y * WINDOW_WIDTH + x) * 4);
        ptr[0] = red;
        ptr[1] = green;
        ptr[2] = blue;
        ptr[3] = 255;
    }
};

int main()
{
    sf::RenderWindow window({WINDOW_WIDTH, WINDOW_HEIGHT}, "Raycast Test");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // For the minimap
    sf::RenderTexture minimapTexture;
    minimapTexture.create(MINIMAP_TILE_SIZE * MAP_SIZE, MINIMAP_TILE_SIZE * MAP_SIZE);
    sf::RectangleShape minimapSprite;
    minimapSprite.setSize(
        {(float)MINIMAP_TILE_SIZE * MAP_SIZE, (float)MINIMAP_TILE_SIZE * MAP_SIZE});

    Drawbuffer drawBuffer;
    Map map;
    Player player;

    sf::Texture texture;
    texture.create(WINDOW_WIDTH, WINDOW_HEIGHT);

    sf::RectangleShape rayCastSprite;
    rayCastSprite.setSize({WINDOW_WIDTH, WINDOW_HEIGHT});

    sf::RectangleShape minimapTile;
    minimapTile.setSize({MINIMAP_TILE_SIZE, MINIMAP_TILE_SIZE});
    minimapTile.setFillColor(sf::Color::White);
    minimapTile.setOutlineColor(sf::Color::Black);
    minimapTile.setOutlineThickness(1);

    Keyboard keyboard;
    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            keyboard.update(e);
            switch (e.type) {
                case sf::Event::Closed:
                    window.close();
                    break;

                default:
                    break;
            }
        }
        // Input
        player.doInput(keyboard);

        // Update

        // Clear
        window.clear();
        drawBuffer.clear();
        minimapTexture.clear(sf::Color::Transparent);

        // ================= Raycasting starts here ========================
        // Get the starting angle of the ray, that is half the FOV to the "left" of the
        // player's looking angle
        float rayAngle = wrap(player.angle - FOV / 2);
        for (int i = 0; i < WINDOW_WIDTH; i++) {
            // These need to be stored for later so they can be compared
            sf::Vector2f horizonatalIntersect;
            sf::Vector2f verticalIntersect;
            // =============== Horizontal line =========================
            // Y Intersection -> Divide the player's Y position by the size of the tiles,
            //  +64 if the ray is looking "down"
            // X Intersection -> Use tan and trig where:
            //  Opp = (Y Intersection - Player Y position)
            //  Theta = Ray's angle
            //  tan(Theta) = Opp / X Intersection so X Intersection = Opp / tan(Theta)
            {
                sf::Vector2f initialIntersect;
                initialIntersect.y = std::floor(player.y / TILE_SIZE) * TILE_SIZE +
                                     (rayAngle < 180 ? TILE_SIZE : -1);
                initialIntersect.x =
                    (initialIntersect.y - player.y) / std::tan(rads(rayAngle)) + player.x;

                // Find distances to the next intersection
                sf::Vector2f distance;
                distance.y = rayAngle < 180 ? TILE_SIZE : -TILE_SIZE;
                distance.x = TILE_SIZE / (rayAngle < 180 ? std::tan(rads(rayAngle))
                                                         : -std::tan(rads(rayAngle)));

                int gridX = std::floor(initialIntersect.x / TILE_SIZE);
                int gridY = std::floor(initialIntersect.y / TILE_SIZE);
                sf::Vector2f next = initialIntersect;
                while ((gridX >= 0 && gridX < MAP_SIZE) &&
                       map.getTile(gridX, gridY) == 0) {
                    next += distance;
                    gridX = std::floor(next.x / TILE_SIZE);
                    gridY = std::floor(next.y / TILE_SIZE);
                }
                horizonatalIntersect = next;
            }

            // =============== Vertical line =========================
            {
                bool left = rayAngle > 90 && rayAngle < 270;
                sf::Vector2f initialIntersect;
                initialIntersect.x = std::floor(player.x / TILE_SIZE) * TILE_SIZE +
                                     (left ? -1 : TILE_SIZE);
                initialIntersect.y =
                    (initialIntersect.x - player.x) * std::tan(rads(rayAngle)) + player.y;

                sf::Vector2f distance;
                distance.x = left ? -TILE_SIZE : TILE_SIZE;
                distance.y = TILE_SIZE * (left ? -std::tan(rads(rayAngle))
                                               : std::tan(rads(rayAngle)));

                sf::Vector2f next = initialIntersect;
                int gridX = std::floor(next.x / TILE_SIZE);
                int gridY = std::floor(next.y / TILE_SIZE);

                while ((gridX >= 0 && gridX < MAP_SIZE) &&
                       map.getTile(gridX, gridY) == 0) {
                    next += distance;
                    gridX = std::floor(next.x / TILE_SIZE);
                    gridY = std::floor(next.y / TILE_SIZE);
                }
                verticalIntersect = next;
            }

            // Find the shortest distance (And draw a ray on the minimap)
            float hDist = distance({player.x, player.y}, horizonatalIntersect);
            float vDist = distance({player.x, player.y}, verticalIntersect);
            float dist = 0;
            sf::Color colour;
            if (hDist < vDist) {
                dist = hDist;
                colour = {255, 153, 51};
            }
            else {
                dist = vDist;
                colour = {255, 204, 102};
            }

            // Fix the fisheye effect (not quite right...)
            dist = std::cos(rads(FOV / 2)) * dist;

            // Draw the walls
            float height =
                TILE_SIZE / dist * (WINDOW_WIDTH / 2 / std::tan(rads(FOV / 2)));
            int start = (WINDOW_HEIGHT / 2) - height / 2;
            // Draw the ceiling, then the wall, then the floor
            for (int y = 0; y < start; y++) {
                drawBuffer.set(i, y, 135, 206, 235);
            }
            for (int y = start; y < start + height; y++) {
                drawBuffer.set(i, y, colour.r / dist * 255, colour.g / dist * 255,
                               colour.b / dist * 255);
            }
            for (int y = start + height; y < WINDOW_HEIGHT; y++) {
                drawBuffer.set(i, y, 0, 153, 51);
            }

            // Find the next ray angle
            rayAngle = wrap(rayAngle + (float)FOV / (float)WINDOW_WIDTH);

            // Draw rays for the mini map
            if (hDist < vDist) {
                drawBuffer.drawLine(minimapTexture, {player.x / 4.0f, player.y / 4.0f},
                                    horizonatalIntersect / 4.0f, {0, 0, 255, 50});
            }
            else {
                drawBuffer.drawLine(minimapTexture, {player.x / 4.0f, player.y / 4.0f},
                                    verticalIntersect / 4.0f, {255, 0, 0, 50});
            }
        }

        // Actually render the walls
        rayCastSprite.setTexture(&texture);
        texture.update(drawBuffer.pixels.data());
        window.draw(rayCastSprite);

        // Render the minimap
        for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                switch (map.getTile(x, y)) {
                    case 1:
                        minimapTile.setFillColor({255, 255, 255, 200});
                        break;

                    default:
                        minimapTile.setFillColor({127, 127, 127, 200});
                        break;
                }
                minimapTile.setPosition(x * MINIMAP_TILE_SIZE, y * MINIMAP_TILE_SIZE);
                window.draw(minimapTile);
            }
        }
        player.draw(minimapTexture);
        drawBuffer.drawLine(
            minimapTexture, {player.x / 4, player.y / 4},
            {player.x / 4 + player.dx * 25, player.y / 4 + player.dy * 25},
            sf::Color::Yellow);

        minimapTexture.display();
        minimapSprite.setTexture(&minimapTexture.getTexture());
        window.draw(minimapSprite);

        window.display();
    }
}