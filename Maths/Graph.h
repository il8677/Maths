#pragma once
#include "pch.h"
#include <queue>
#include <string>
#include <sstream>
#define XSIZE 4.f
class Graph : public sf::Drawable, public sf::Transformable {
	float * target;
	std::deque<float> data;
	float scale;
	sf::VertexArray graph;
	float average = 0;
	int count = 0;
	sf::Text text;
	float lastobs = 0;
	std::string ftos(float f) {
		std::stringstream ss;
		ss << f;
		return ss.str();
	}
public:
	void observe() {
		float change = *target - lastobs;
		lastobs = (*target);
		data.push_back(change);
		count++;
		average = *target / count;
		
		text.setString("AVG Delta time: " +ftos(average) + "\nDelta time Last Obs: " + ftos(change));
		if (data.size() > 10) {
			data.pop_front();
		}
		calculateVerticies();
	}
	Graph(float * t, sf::Font & f) {
		target = t;
		graph = sf::VertexArray(sf::LinesStrip, 10);
		text.setFillColor(sf::Color::Yellow);
		text.setFont(f);
		text.setScale(1, 1);
	}
	void calculateVerticies() {
		float lowest = 10000000;
		for (int i = 0; i < data.size(); i++) {
			
			if (data[i] < lowest) {
				lowest = data[i];
			}
		}
		for (int i = 0; i < data.size(); i++) {
			graph[i] = sf::Vertex(sf::Vector2f(XSIZE*i, -(data[i] - lowest)), sf::Color::Yellow);
		}
	}
	void draw(sf::RenderTarget &target, sf::RenderStates states) const {
		states.transform *= getTransform();
		target.draw(graph, states);
		target.draw(text);
	}
};