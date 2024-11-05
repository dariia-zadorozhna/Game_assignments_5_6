#include <SFML/Graphics.hpp>
#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>
using namespace std;

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;
const int FPS = 600;
const int GRID_SIZE = 50;

class Point {
public:
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

class Snake {
public:
    deque<Point> body;
    Point direction;

    Snake() {
        body.push_back(Point(WINDOW_WIDTH / (GRID_SIZE * 2), WINDOW_HEIGHT / (GRID_SIZE * 2)));
        direction = Point(1, 0);
    }

    void move() {
        Point newHead = body.front();
        newHead.x += direction.x;
        newHead.y += direction.y;
        body.push_front(newHead);
        body.pop_back();
    }

    void grow(int units) {
        Point tail = body.back();
        for (int i = 0; i < units; i++) {
            tail.x -= direction.x;
            tail.y -= direction.y;
            body.push_back(tail);
        }
    }

    bool checkCollision() const {
        const Point& head = body.front();
        if (head.x < 0 || head.x >= WINDOW_WIDTH / GRID_SIZE ||
            head.y < 0 || head.y >= WINDOW_HEIGHT / GRID_SIZE) {
            return true;
        }
        for (int i = 1; i < body.size(); i++) {
            if (body[i] == head) {
                return true;
            }
        }
        return false;
    }
};

class Fruit {
public:
    Point position;
    bool isGolden;
    sf::Texture* texture;

    Fruit() : isGolden(false), texture(nullptr) { srand(static_cast<unsigned>(time(0))); }

    void spawn(const deque<Point>& snakeBody, const vector<sf::Texture*>& fruitTextures, sf::Texture* coinTexture) {
        do {
            position.x = rand() % (WINDOW_WIDTH / GRID_SIZE);
            position.y = rand() % (WINDOW_HEIGHT / GRID_SIZE);
        } while (find(snakeBody.begin(), snakeBody.end(), position) != snakeBody.end());

        isGolden = (rand() % 10) < 1;
        texture = isGolden ? coinTexture : fruitTextures[rand() % fruitTextures.size()];
    }
};

class Board {
public:
    sf::RenderWindow window;
    sf::Texture snakeTexture;
    sf::Texture kiwiTexture;
    sf::Texture coinTexture;
    sf::Texture watermelonTexture;
    sf::Texture backgroundTexture;
    sf::Texture blueberryTexture;
    sf::Texture cherryTexture;
    vector<sf::Texture*> fruitTextures;

    Board() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SNAKE GAME") {
        window.setFramerateLimit(FPS);

        if (!snakeTexture.loadFromFile("blue_circle_2.png")) {
            cerr << "Error loading snake texture" << endl;
        }
        if (!backgroundTexture.loadFromFile("pink_background.jpg")) {
            cerr << "Error loading background texture" << endl;
        }
        if (!kiwiTexture.loadFromFile("kiwi.png")) {
            cerr << "Error loading kiwi texture" << endl;
        }
        if (!blueberryTexture.loadFromFile("blueberry.png")) {
            cerr << "Error loading blueberry texture" << endl;
        }
        if (!cherryTexture.loadFromFile("cherry.png")) {
            cerr << "Error loading cherry texture" << endl;
        }
        if (!watermelonTexture.loadFromFile("watermelon.png")) {
            cerr << "Error loading watermelon texture" << endl;
        }
        if (!coinTexture.loadFromFile("coin.png")) {
            cerr << "Error loading coin texture" << endl;
        }
        fruitTextures = { &kiwiTexture, &blueberryTexture, &watermelonTexture, &cherryTexture };
    }

    void draw(const Snake& snake, const Fruit& fruit) {
        sf::RectangleShape background(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        background.setTexture(&backgroundTexture);
        window.draw(background);

        sf::RectangleShape fruitSegment(sf::Vector2f(GRID_SIZE, GRID_SIZE));
        fruitSegment.setTexture(fruit.texture);
        fruitSegment.setPosition(fruit.position.x * GRID_SIZE, fruit.position.y * GRID_SIZE);
        window.draw(fruitSegment);

        sf::RectangleShape snakeSegment(sf::Vector2f(GRID_SIZE, GRID_SIZE));
        snakeSegment.setTexture(&snakeTexture);
        for (const auto& part : snake.body) {
            snakeSegment.setPosition(part.x * GRID_SIZE, part.y * GRID_SIZE);
            window.draw(snakeSegment);
        }
    }

    void render(const Snake& snake, const Fruit& fruit) {
        window.clear(sf::Color::Green);
        draw(snake, fruit);
        window.display();
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
    }
};

class Game {
private:
    Board board;
    Fruit currentFruit;
    Snake snake;
    int score;
    bool gameOver;
    bool justGrew;
    sf::Clock clock;
    sf::Time movementInterval;

    void handleInput() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && snake.direction.y == 0) {
            snake.direction = Point(0, -1);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && snake.direction.y == 0) {
            snake.direction = Point(0, 1);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && snake.direction.x == 0) {
            snake.direction = Point(-1, 0);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && snake.direction.x == 0) {
            snake.direction = Point(1, 0);
        }
    }

    void handleFruitEaten() {
        score++;
        snake.grow(currentFruit.isGolden ? 2 : 1);
        currentFruit.spawn(snake.body, board.fruitTextures, &board.coinTexture);
        justGrew = true;
    }

    void updateGameState() {
        if (clock.getElapsedTime() >= movementInterval) {
            clock.restart();
            snake.move();

            if (snake.body.front() == currentFruit.position) {
                handleFruitEaten();
            }
            if (!justGrew && snake.checkCollision()) {
                gameOver = true;
                cout << "Game over! Final score: " << score << endl;
                board.window.close();
            }
            justGrew = false;
        }
    }

public:
    Game() : score(0), gameOver(false), justGrew(false), movementInterval(sf::milliseconds(150)) {
        currentFruit.spawn(snake.body, board.fruitTextures, &board.coinTexture);
    }

    void run() {
        while (board.window.isOpen()) {
            board.handleEvents();
            handleInput();
            updateGameState();
            board.render(snake, currentFruit);
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
