#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "Trace.h"
#include <Windows.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "hittable_list.h"
#include <algorithm>
#include <random>

#define print(x) std::cout << x << std::endl;
float x_pos=0, y_pos=0;

class Mouse : public sf::Mouse {
private:
	static unsigned last_position_x, last_position_y;

public:
	static int get_move_x(const sf::RenderWindow& window);
	static int get_move_y(const sf::RenderWindow& window);

	static sf::Vector2i getPositionDelta(const sf::RenderWindow& window);
	static void setPosition(const sf::Vector2i& position, const sf::Window& relativeTo);
};

unsigned Mouse::last_position_x = 0;
unsigned Mouse::last_position_y = 0;

int Mouse::get_move_x(const sf::RenderWindow& window) {
	const int move_x = getPosition(window).x - last_position_x;
	last_position_x = getPosition(window).x;

	return move_x;
}

int Mouse::get_move_y(const sf::RenderWindow& window) {
	const int move_y = getPosition(window).y - last_position_y;
	last_position_y = getPosition(window).y;

	return move_y;
}

sf::Vector2i Mouse::getPositionDelta(const sf::RenderWindow& window)
{
	sf::Vector2i pos_real = getPosition(window);
	float delta_x = x_pos - pos_real.x;
	float delta_y = y_pos - pos_real.y;
	return sf::Vector2i(delta_x, delta_y);
}

// extend sf::Mouse setPosition function
void Mouse::setPosition(const sf::Vector2i& position, const sf::Window& relativeTo) {
	sf::Mouse::setPosition(position, relativeTo);

	Mouse::last_position_x = position.x;
	Mouse::last_position_y = position.y;
}

class SFML_CLASS
{
public:
	sf::RenderWindow* window;
	int width;
	int height;
	sf::VertexArray ver_arr;
	int fps_global;
	sf::Font* font;
	sf::Text* text;
	std::vector<sf::Vector2i>poll_index;

	void CalculateFrameRate()
	{
		static float framesPerSecond = 0.0f;
		static int fps;
		static float lastTime = 0.0f;
		float currentTime = GetTickCount64() * 0.001f;
		++framesPerSecond;
		//glPrint("Current Frames Per Second: %d\n\n", fps);
		fps_global = fps;
		if (currentTime - lastTime > 1.0f)
		{
			lastTime = currentTime;
			fps = (int)framesPerSecond;
			framesPerSecond = 0;
		}
	}


	void SFML_INIT(int width, int height)
	{
		this->width = width;
		this->height = height;

		window = new sf::RenderWindow(sf::VideoMode(width, height), "Ray");
		window->setVerticalSyncEnabled(true);
		window->setKeyRepeatEnabled(false);
		Mouse::setPosition(sf::Vector2i(width / 2, height / 2), *window);
		window->setMouseCursorVisible(false);
		x_pos = width / 2;
		y_pos = height / 2;

		ver_arr.setPrimitiveType(sf::Points);
		ver_arr.resize(width * height);

		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				poll_index.push_back(sf::Vector2i(x, y));

				sf::Color col(25, 25, 25, 255 );

				ver_arr[x + y * width].position = sf::Vector2f(x, y);
				ver_arr[x + y * width].color = col;
			}
		}

		auto rd = std::random_device{};
		auto rng = std::default_random_engine{ rd() };
		std::shuffle(poll_index.begin(), poll_index.end(), rng);

		//FPS
		font = new sf::Font();
		text = new sf::Text();

		font->loadFromFile("C:/WINDOWS/Fonts/arial.TTF");
		text->setFont(*font);
		text->setString("0");

		text->setCharacterSize(17);
		text->setFillColor(sf::Color::Red);
		text->setStyle(sf::Text::Bold | sf::Text::Underlined);
		text->setPosition(20, 20);
	}

	void SFML_LOOP(Camera& camera, hittable_list& world, std::shared_ptr<hittable_list> lights)
	{
		std::size_t const half_size = poll_index.size() / 2;
		std::vector<sf::Vector2i> split_lo(poll_index.begin(), poll_index.begin() + half_size);
		std::vector<sf::Vector2i> split_hi(poll_index.begin() + half_size, poll_index.end());

		std::atomic_bool state_close = true;

		sf::Thread thread([&](){
			while (true){
				if (state_close){
					render(split_lo, camera, world, lights, this->ver_arr, this->width, this->height);
				}
				else
				{
					break;
				}
			}
		});

		sf::Thread thread2([&]() {
		while (true) {
			if (state_close) {
				render(split_hi, camera, world, lights, this->ver_arr, this->width, this->height);
			}
			else
			{
				break;
			}
		}
		});

		thread.launch();
		thread2.launch();

		while (window->isOpen())
		{
			//sf::Time dt = deltaClock.getElapsedTime();
			CalculateFrameRate();
			text->setString(std::to_string(fps_global));
			sf::Event event;
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window->close();
					thread.terminate();
					thread2.terminate();
					goto end;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				{
					window->close();
					thread.terminate();
					thread2.terminate();
					goto end;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
				{
					camera.ProcessKeyboard(Camera_Movement::FORWARD, 0.1);
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
				{
					camera.ProcessKeyboard(Camera_Movement::BACKWARD, 0.1);
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
				{
					camera.ProcessKeyboard(Camera_Movement::LEFT, 0.1);
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
				{
					camera.ProcessKeyboard(Camera_Movement::RIGHT, 0.1);
				}
				if (event.type == sf::Event::MouseMoved)
				{
					auto pos = Mouse::getPositionDelta(*window);
					//camera.offsetOrientation(Mouse::get_move_x(*window), Mouse::get_move_x(*window));
					Mouse::setPosition(sf::Vector2i(width / 2, height / 2), *window);
					camera.offsetOrientation(pos.x, pos.y);	
				}
			}

			window->clear(sf::Color(0,0,0,255));
			//render(camera, world, lights, this->ver_arr, this->width,  this->height);
			window->draw(this->ver_arr);

			window->draw(*text);
			window->display();
		} 
		end:
		state_close = false;
	}

	~SFML_CLASS()
	{
		window->setMouseCursorVisible(true);
		delete this->window;
		delete this->font;
		delete this->text;
	}
};
