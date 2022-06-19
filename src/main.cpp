#include "entities.hpp"
#include "globals.hpp"
#include "toml.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <cstring>
#include <fstream>
#include <iostream>

using namespace obf;

void connectToServer(){
	serverSocket = new sf::TcpSocket;
	while(true){
		printf("Specify server port\n");
		std::cin >> port;
		printf("Specify server address\n");
		std::cin >> serverAddress;
		if(serverSocket->connect(serverAddress, port) != sf::Socket::Done){
			printf("Could not connect to %s:%u.\n", serverAddress.c_str(), port);
		}else{
			printf("Connected to %s:%u.\n", serverAddress.c_str(), port);
			serverSocket->setBlocking(false);
			break;
		}
	}
}

int main(int argc, char** argv){
	for(int i = 1; i < argc; i++){
    	headless |= !std::strcmp(argv[i], "--headless");
	}
	if(parseTomlFile("config.txt") != 0){
		printf("No config file detected, creating config.txt.\n");
		std::ofstream out;
		out.open("config.txt");
	}
	std::fstream out;
	out.open("config.txt");
	if(headless){
		if(port == 0){
			printf("Specify the port you will host on.\n");
			std::cin >> port;
			out << "port = " << port << std::endl;
		}
	}else{
		if(name == ""){
			printf("Specify a username.\n");
			std::cin >> name;
			out << "name = " << name << std::endl;
		}
	}
	if(headless){
		connectListener = new sf::TcpListener;
		connectListener->setBlocking(false);
		if(connectListener->listen(port) != sf::Socket::Done){
			printf("Could not host server on port %u.\n", port);
			return 0;
		}
		printf("Hosted server on port %u.\n", port);
	}else{
		window = new sf::RenderWindow(sf::VideoMode(500, 500), "Test");
		connectToServer();
	}
	while(headless || window->isOpen()){
		if(headless){
			sf::Socket::Status status = connectListener->accept(sparePlayer->tcpSocket);
			if(status == sf::Socket::Done){
				sparePlayer->ip = sparePlayer->tcpSocket.getRemoteAddress().toString();
				sparePlayer->port = sparePlayer->tcpSocket.getRemotePort();
				printf("%s has connected.\n", sparePlayer->name().c_str());
				sparePlayer->lastAck = globalTime;
				playerGroup.push_back(sparePlayer);
				for(Entity* e : updateGroup){
					sf::Packet packet;
					packet << (uint16_t)1;
					e->loadCreatePacket(packet);
					sparePlayer->tcpSocket.send(packet);
				}
				sparePlayer = new Player;
			}else if(status != sf::Socket::NotReady){
				printf("An incoming connection has failed.\n");
			}
		}else{
			mousePos = sf::Mouse::getPosition(*window);
			sf::Event event;
			while(window->pollEvent(event)) {
				switch (event.type) {
				case sf::Event::Closed:
					window->close();
					break;
				case sf::Event::Resized:
					window->setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
				default:
					break;
				}
			}
			window->clear(sf::Color(25, 5, 40));
			for(auto* entity : updateGroup){
				entity->draw();
			}
			window->display();
			sf::Socket::Status status = sf::Socket::Done;
			while(status != sf::Socket::NotReady){
				sf::Packet packet;
				status = serverSocket->receive(packet);
				if(status == sf::Socket::Done){
					uint16_t type;
					packet >> type;
					serverSocket->setBlocking(true);
					switch(type){
						case 0:{
							sf::Packet ackPacket;
							ackPacket << (uint16_t)0;
							serverSocket->send(ackPacket);
						}
						break;
						case 1:{
							unsigned char entityType;
							packet >> entityType;
							switch(entityType){
								case 0:{
									Triangle e;
									e.unloadCreatePacket(packet);
								}
								break;
							}
						}
						break;
					}
					serverSocket->setBlocking(false);
				}else if(status == sf::Socket::Disconnected){
					printf("Connection to server closed.\n");
					connectToServer();
				}
			}
		}
		for(auto* entity : updateGroup){
			entity->update();
		}
		int to = playerGroup.size();
		for(int i = 0; i < to; i++){
			Player* player = playerGroup.at(i);
			if(headless){
				if(globalTime - player->lastAck > 1.0 && globalTime - player->lastPingSent > 1.0){
					if(globalTime - player->lastAck > maxAckTime){
						playerGroup.erase(playerGroup.begin() + i);
						i--;
						to--;
						printf("Player %s's connection has timed out.\n", player->name().c_str());
						player->tcpSocket.disconnect();
						delete player;
						continue;
					}
					sf::Packet pingPacket;
					pingPacket << (uint16_t)0;
					sf::Socket::Status status = player->tcpSocket.send(pingPacket);
					if(status != sf::Socket::Done){
						if(status == sf::Socket::Disconnected){
							playerGroup.erase(playerGroup.begin() + i);
							i--;
							to--;
							printf("Player %s has disconnected.\n", player->name().c_str());
							delete player;
							continue;
						}else if(status == sf::Socket::Error){
							printf("Error when trying to send ping packet to player %s.\n", player->name().c_str());
						}
					}
					player->lastPingSent = globalTime;
				}
				sf::Socket::Status status = sf::Socket::Done;
				while(status != sf::Socket::NotReady && status != sf::Socket::Disconnected){
					sf::Packet packet;
					player->tcpSocket.setBlocking(false);
					status = player->tcpSocket.receive(packet);
					player->tcpSocket.setBlocking(true);
					if(status == sf::Socket::Done) [[likely]]{
						player->lastAck = globalTime;
						unsigned char type;
						packet >> type;
						switch(type){
						}
					}else if(status == sf::Socket::Disconnected){
						playerGroup.erase(playerGroup.begin() + i);
						i--;
						to--;
						printf("Player %s has disconnected.\n", player->name().c_str());
						delete player;
						continue;
					}
				}
			}
		}
		delta = deltaClock.restart().asSeconds() * 60;
		sf::sleep(sf::seconds(std::max(1.0 / 60.0 - delta, 0.0)));
		globalTime = globalClock.getElapsedTime().asSeconds();
	}
	return 0;
}
