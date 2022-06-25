#pragma once

#include <vector>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Network.hpp>

namespace obf {


struct Player;

struct Entity;

void setupShip(Entity* ship);

struct movement {
	int forward: 1 = 0;
	int backward: 1 = 0;
	int turnright: 1 = 0;
	int turnleft: 1 = 0;
	int boost: 1 = 0;
	int hyperboost: 1 = 0;
	int primaryfire: 1 = 0;
	int secondaryfire: 1 = 0;
};

struct Point {
	double x;
	double y;
};

bool operator ==(movement& mov1, movement& mov2);

struct Entity {
	Entity();
	virtual ~Entity() noexcept;

	virtual void control(movement& cont);
	virtual void update();
	virtual void draw();

	virtual void collide(Entity* with, bool collideOther);

	std::vector<Entity*> near;
	std::vector<Entity*> resNear;

	virtual void syncCreation();

	virtual void loadCreatePacket(sf::Packet& packet) = 0;
	virtual void unloadCreatePacket(sf::Packet& packet) = 0;
	virtual void loadSyncPacket(sf::Packet& packet) = 0;
	virtual void unloadSyncPacket(sf::Packet& packet) = 0;

	virtual void simSetup();
	virtual void simReset();

	inline void setPosition(double x, double y) {
		this->x = x;
		this->y = y;
	}
	inline void setVelocity(double x, double y) {
		velX = x;
		velY = y;
	}
	inline void addVelocity(double dx, double dy) {
		velX += dx;
		velY -= dy;
	}

	inline void setColor(uint8_t r, uint8_t g, uint8_t b) {
		color[0] = r;
		color[1] = g;
		color[2] = b;
	}

	std::unique_ptr<sf::CircleShape> icon;

	std::unique_ptr<std::vector<Point>> trajectory;

	virtual uint8_t type() = 0;
	Player* player = nullptr;
	double x = 0.0, y = 0.0, velX = 0.0, velY = 0.0, radius = 0.0,
	mass = 0.0,
	lastCollideCheck = 0.0, lastCollideScan = 0.0,
	resX, resY, resVelX, resVelY, resMass, resRadius, resCollideScan;
	bool ghost = false;
	Entity* simRelBody = nullptr;
	unsigned char color[3]{255, 255, 255};
	uint32_t id;
};

struct Triangle: public Entity {
	Triangle();
	Triangle(bool ghost);

	void control(movement& cont) override;
	void draw() override;

	void loadCreatePacket(sf::Packet& packet) override;
	void unloadCreatePacket(sf::Packet& packet) override;
	void loadSyncPacket(sf::Packet& packet) override;
	void unloadSyncPacket(sf::Packet& packet) override;

	void simSetup() override;
	void simReset() override;

	uint8_t type() override;
	double accel = 0.015, rotateSpeed = 2.0, boostCooldown = 12.0, boostStrength = 1.5, reload = 8.0, shootPower = 2.0, hyperboostStrength = 0.12, hyperboostTime = 20.0 * 60.0, hyperboostTurnMult = 0.02, afterburnStrength = 0.3, minAfterburn = hyperboostTime + 8.0 * 60.0,
	lastBoosted = -boostCooldown, lastShot = -reload, hyperboostCharge = 0.0,
	resLastBoosted, resLastShot, resHyperboostCharge, resRotation;

	bool burning = false, resBurning;
	std::string name = "";

	std::unique_ptr<sf::CircleShape> shape, forwards;
	float rotation = 0.f;
};

struct Attractor: public Entity {
	Attractor(double radius);
	Attractor(double radius, double mass);
	Attractor(bool ghost);

	void update() override;
	void draw() override;

	void loadCreatePacket(sf::Packet& packet) override;
	void unloadCreatePacket(sf::Packet& packet) override;
	void loadSyncPacket(sf::Packet& packet) override;
	void unloadSyncPacket(sf::Packet& packet) override;

	uint8_t type() override;

	bool star = false, blackhole = false;

	std::unique_ptr<sf::CircleShape> shape, warning;
};

struct Projectile: public Entity {
	Projectile();

	void draw() override;

	void collide(Entity* with, bool collideOther) override;

	void loadCreatePacket(sf::Packet& packet) override;
	void unloadCreatePacket(sf::Packet& packet) override;
	void loadSyncPacket(sf::Packet& packet) override;
	void unloadSyncPacket(sf::Packet& packet) override;

	uint8_t type() override;

	std::unique_ptr<sf::CircleShape> shape;
};

struct Player {
	~Player();

	std::string name();

	Entity* entity = nullptr;

	sf::TcpSocket tcpSocket;
	std::vector<sf::Packet> tcpQueue;
	std::string username = "", ip = "";
	double lastAck = 0.0, lastPingSent = 0.0, lastSynced = 0.0, lastFullsynced = 0.0, ping = 0.0,
	viewW = 500.0, viewH = 500.0;
	movement controls;
	unsigned short port = 0;
};

}
