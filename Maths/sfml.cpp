#include "pch.h"
#include <iostream>
#include <cmath>

using namespace std;

#define WIDTH 1000
#define HEIGHT 750

sf::Time questionTime;

enum state { MENU, SELECT, PLAY, DEATH };
state s = MENU;

sf::Font arial;

int difficulty = 1;

bool commit = false;

float roundtwo(float value)
{
	return floor(value * 100 + 0.5) / 100;
}
void roundtwo(float * value) {
	*value = floor(*value * 100 + 0.5) / 100;
}

void centerText(sf::Text& text) {
	sf::FloatRect textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f);
	text.setPosition(sf::Vector2f(WIDTH / 2.0f, text.getPosition().y));

}

class Particle {
public:
	float size=5;
	sf::Color colour;
	sf::Vector2f pos;
	float endsize;
	float lifetime = -1;
	float life;
	float vx;
	float vy;

	bool useAccelaration = true;
	
	sf::Vector2f target = sf::Vector2f(-1,-1);
	float a = 1000;
	void(*callback)() = nullptr;
	static void(*die)(Particle *);
	sf::Vector2f(*gettarget)(Particle *) = nullptr;

	float getsize() {
		return size;
	}

	void update(sf::Time dt) {
		if (target != sf::Vector2f(-1, -1) || gettarget != nullptr) {
			if (gettarget != nullptr) {
				target = gettarget(this);
			}
			float rise = abs(target.y - pos.y);
			float run = abs(target.x - pos.x);

			float ysign = target.y < pos.y ? -1 : 1;
			float xsign = target.x < pos.x ? -1 : 1;


			float theta = atan(rise / run);
			float ax = cos(theta) * a *xsign;
			float ay = sin(theta) * a * ysign;
			if (useAccelaration) {
				vy += ay * dt.asSeconds();
				vx += ax * dt.asSeconds();
			}
			else {
				vy = ay;
				vx = ax;
			}
		}else {
			vy += a * dt.asSeconds();
		}
#define SPEEDLIMIT 5
		if (vy > SPEEDLIMIT) {
			vy = SPEEDLIMIT;
		}	
		if (vx > SPEEDLIMIT) {
			vx = SPEEDLIMIT;
		}
		if (vy < -SPEEDLIMIT) {
			vy = -SPEEDLIMIT;
		}
		if (vx < -SPEEDLIMIT) {
			vx = -SPEEDLIMIT;
		}
		pos.x += vx;
		pos.y += vy;
		life += dt.asSeconds();
		if (life > lifetime && lifetime != -1) {
			die(this);
		}

		if (abs(pos.x - target.x) < 20 && abs(pos.y - target.y) < 20) {
			if (callback != nullptr) {
				callback();
			}
			die(this);
		}
	}
};

void(*Particle::die)(Particle *);

#define DEGTORAD(x) x*(3.1415/180)
#define RADTODEG(x) x*(180/3.1415)
static class ParticleSystem : public sf::Drawable {
private:
	vector<Particle> particlelist;
	sf::VertexBuffer buffer;
public:

	void die(Particle * p) {
		for (int i = 0; i < particlelist.size();i++) {
			if (&particlelist[i] == p) {
				particlelist.erase(particlelist.begin() + i);
				break;
			}
		}
		calculateVerticies();
	}

	ParticleSystem() {
		buffer = sf::VertexBuffer(sf::PrimitiveType::Quads, sf::VertexBuffer::Stream);
		buffer.create(2000);
	}
	
	void calculateVerticies() {
		if (particlelist.size() > 0) {
			buffer.create(0);
			buffer.create(2000);
			sf::VertexArray arr(sf::Quads, particlelist.size() * 4);
			arr.resize(particlelist.size() * 4);
			sf::Vertex* freespace = &arr[0];
			for (int i = 0; i < particlelist.size(); i++) {
				Particle * p = &particlelist[i];
				freespace[0] = sf::Vertex(p->pos, p->colour);
				freespace[1] = sf::Vertex(p->pos + sf::Vector2f(p->getsize(), 0),p->colour);
				freespace[2] = sf::Vertex(p->pos + sf::Vector2f(p->getsize(), p->getsize()), p->colour);
				freespace[3] = sf::Vertex(p->pos + sf::Vector2f(0, p->getsize()), p->colour);

				freespace = &arr[i * 4];
			}
			buffer.update(&arr[0],particlelist.size()*4,0);
		}
		else {
			buffer.create(0);
		}
	}

	void spawn(int n, Particle p, int anglestart, int angleend, sf::Vector2f pos,float svx = 0, float svy = 0) {
		for (int i = 0; i < n; i++) {
			Particle theParticle = p;
			theParticle.pos = pos;
			float angle = rand() % ((angleend - anglestart) + anglestart);
			float theta = angle - (floor(angle / 90) * 90);

			theParticle.vy = angle > 90 && angle < 270 ? svy * sin(DEGTORAD(theta))  * -1 : svy * sin(DEGTORAD(theta)) * 1;
			theParticle.vx = angle > 180 ? -1 * svx*cos(DEGTORAD(theta)) : svx * cos(DEGTORAD(theta));

			particlelist.push_back(theParticle);
		}
	}

	void update(sf::Time dt) {
		for (int i = 0; i < particlelist.size(); i++) {
			particlelist[i].update(dt);
		}
		calculateVerticies();
	}

	void draw(sf::RenderTarget & target, sf::RenderStates states)const {
		target.draw(buffer);
	}

}particles;

 void die(Particle * p) {
	 particles.die(p);
}

class Generator {
protected:
	float a; float b;
	void generateNumbers() {
#define DIFFICULTY (difficulty - (find((*generators).begin(), (*generators).end(),this)-(*generators).begin()))
#define NUMGEN (rand() % 5 * (DIFFICULTY - floor(DIFFICULTY/2))) + floor(DIFFICULTY / 2)
		
		a = NUMGEN;
		b = NUMGEN;
		roundtwo(a);
		roundtwo(b);
	}
public:
	static sf::Text * questionText;
	static vector<Generator*> * generators;

	virtual float getAnswer() = 0;
	virtual void getQuestion() = 0;
	virtual int getComplexity() {
		return a + b;
	}
};

sf::Text * Generator::questionText;
vector<Generator*> * Generator::generators;

Generator * selectedGenerator;

class Addition :  public Generator {
public:
	void getQuestion() override {
		generateNumbers();
		questionText->setString(to_string((int)a) + " + " + to_string((int)b));
	}
	float getAnswer() override {
		return a + b;
	}
};

class Subtraction : public Generator {
public:
	void getQuestion() override {
		generateNumbers();
		questionText->setString(to_string((int)a) + " - " + to_string((int)b));
	}
	float getAnswer() override {
		return a - b;
	}
};

class Multiplication : public Generator {
public:
	void getQuestion() override {
		generateNumbers();
		questionText->setString(to_string((int)a) + "*" + to_string((int)b));

	}

	float getAnswer() override {
		return a * b;
	}
};

class Button : public sf::Drawable{
	sf::Shape * shape;
	sf::Text t;
	void(*callback)();

	
public:
	bool checkMouseOver(sf::Vector2f p) {
		if (shape->getGlobalBounds().contains(p)) {
			shape->setFillColor(sf::Color::White);
			t.setFillColor(sf::Color::Black);
			return true;
		}
		else {
			shape->setFillColor(sf::Color::Black);
			t.setFillColor(sf::Color::White);
		}
		return false;
	}
	Button(sf::Shape * s, void(*cb)(),string name,sf::Vector2f position = sf::Vector2f(0,0)) {
		callback = cb;
		shape = s;

		shape->setPosition(position);

		t.setFont(arial);
		t.setString(name);
		t.setPosition(shape->getLocalBounds().width/4, 0);
		t.setFillColor(sf::Color::White);
		
		shape->setFillColor(sf::Color::Black);
		shape->setOutlineColor(sf::Color::White);
		shape->setOutlineThickness(5);
	}
	~Button(){
		delete shape;
	}


	bool checkClick(sf::Vector2f p) {
		if (checkMouseOver(p)) {
			callback();
			return true;
		}
		return false;
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= shape->getTransform();
		target.draw(*shape);
		target.draw(t, states);

	}
};
#define NEWGENERATOR(x) selectedGenerator=generators[rand()%x]
bool allow = true;
bool correct = false;
#define MAXHEIGHT 600

static class ScoreMachine  : public sf::Drawable, public sf::Transformable{
	int levelScore = 250;
	int score = 0;
	sf::VertexArray quad;
	int bank = 0;
	sf::Text difficultyTest;
public:
	float height = MAXHEIGHT;
	ScoreMachine() {
		quad = sf::VertexArray(sf::Quads, 4);
		difficultyTest.setString(to_string(score));
		difficultyTest.setFillColor(sf::Color::Green);
		difficultyTest.setFont(arial);
	}

	void calculateVerticies() {
		difficultyTest.setString(to_string(score));

		/*float percent = (float)score / levelScore;
		height = MAXHEIGHT - (percent * (MAXHEIGHT - 5));
		quad[0] = sf::Vertex(sf::Vector2f(0, MAXHEIGHT), sf::Color::Green);
		quad[1] = sf::Vertex(sf::Vector2f(25, MAXHEIGHT), sf::Color::Green);
		quad[2] = sf::Vertex(sf::Vector2f(25, height), sf::Color::Green);
		quad[3] = sf::Vertex(sf::Vector2f(0, height), sf::Color::Green);*/

	}

	void draw(sf::RenderTarget & target, sf::RenderStates state) const {
		state.transform *= getTransform();
		target.draw(difficultyTest, state);
		//target.draw(quad, state);
	}

	void addBank(int n) {
		bank += n;
		if (bank > levelScore) {
			commit = true;
		}
	}
	void subtractBank(int n) {
		bank -= n;
		if (bank < 0) {
			bank = 0;
			commit = false;
		}
	}

	void addScore(int n) {
		score += n;
		if (score > levelScore) {
			levelScore *= 1.5;
			score = 0;
			difficulty++;
		}
		calculateVerticies();
	}
	void subtractScore(int n) {
		score -= n;
		if (score < 0) {
			score = 0;
		}
		calculateVerticies();
	}
}sm;

int main() {
	sm.setPosition(WIDTH/2, 50);
	Particle::die = &die;

	vector<Generator*> generators = {new Addition(), new Subtraction(), new Multiplication()};
	Generator::generators = &generators;

	srand(time(NULL));
	sf::RenderWindow window(sf::VideoMode(WIDTH,HEIGHT),"Maths");
	window.setFramerateLimit(60);

	arial.loadFromFile("arial.ttf");

	sf::Text question;
	question.setFont(arial);
	question.setPosition(WIDTH / 2,HEIGHT/4);
	question.setCharacterSize(60);

	question.setString("Test");
	Generator::questionText = &question;
	
	sf::Text input;
	input.setPosition(WIDTH / 2 - 50, HEIGHT / 2);
	input.setFont(arial);
	input.setCharacterSize(60);

	sf::Text answerstring;
	answerstring.setPosition(WIDTH / 2 - 50, HEIGHT / 2 + 10);
	answerstring.setFont(arial);
	answerstring.setFillColor(sf::Color::Red);

	
	sf::String inputString;

	NEWGENERATOR(difficulty);
	selectedGenerator->getQuestion();

	centerText(question);

	Button playButton(new sf::RectangleShape(sf::Vector2f(100, 40)), []() {s = PLAY; }, "Play", sf::Vector2f(WIDTH / 2-50, HEIGHT / 2));
	sf::Clock lastQuestion;
	sf::Clock gameClock;
	lastQuestion.restart();

	while (window.isOpen()) {
		window.clear();
		sf::Event e;

		if (s == MENU) {
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
				playButton.checkClick((sf::Vector2f)sf::Mouse::getPosition(window));
			}
			while (window.pollEvent(e)) {
				if (e.type == sf::Event::Closed) {
					window.close();
				}

				if (e.type = sf::Event::MouseMoved) {
					playButton.checkMouseOver(sf::Vector2f(e.mouseMove.x, e.mouseMove.y));
				}
			}
			window.draw(playButton);
		}else if (s == PLAY) {
			sf::Time dt = gameClock.getElapsedTime();
			gameClock.restart();
			while (window.pollEvent(e)) {
				if (e.type == sf::Event::Closed) {
					window.close();

				}else if (e.type == sf::Event::TextEntered && allow) {
					if (e.text.unicode == 13) {
						float answer = stof(inputString.toAnsiString());
						if (answer == selectedGenerator->getAnswer()) {
							input.setFillColor(sf::Color::Green);
							allow = false;
							correct = true;
						} else {
							answerstring.setString(to_string(selectedGenerator->getAnswer()));
							input.setFillColor(sf::Color::Red);
							allow = false;
							correct = false;
						}
						question.setString("Press enter to continue");
						centerText(question);
						questionTime = lastQuestion.getElapsedTime();


					}else if (e.text.unicode == 8) {
						inputString.erase(inputString.getSize() - 1);
					}else if(e.text.unicode < 128){
						inputString += static_cast<char>(e.text.unicode);
					}
				input.setString(inputString);
				centerText(input);

				}else if (!allow) {
					Particle p;
					if (e.text.unicode == 13) {
						input.setFillColor(sf::Color::White);
						answerstring.setString("");

						input.setString("");
						inputString = "";
						allow = true;
						int totalpoints = correct ? selectedGenerator->getComplexity()* 10 : 50 * (questionTime.asSeconds() / 5);
						
						if (correct) {
							p.colour = sf::Color::Green;
							p.callback = []() {sm.addScore(1); sm.subtractBank(1); };
							sm.addBank(totalpoints);
						} else {
							p.colour = sf::Color::Red;
							sm.subtractBank(totalpoints);
							p.callback = []() {sm.subtractScore(1); };
						}
						if (difficulty > generators.size()) {
							NEWGENERATOR(generators.size());
						}
						else {
							NEWGENERATOR(difficulty);
						}
						selectedGenerator->getQuestion();
						centerText(question);

						lastQuestion.restart();

						p.gettarget = [](Particle * p) {if (!commit) { return sf::Vector2f(WIDTH / 4, HEIGHT / 2); } else { p->useAccelaration = false;  return sm.getPosition(); } };

						particles.spawn(totalpoints, p, 0, 360,sf::Vector2f(500,500),5,5);

					}
				}
			}

			particles.update(dt);

	
			window.draw(question);
			window.draw(input);
			window.draw(answerstring);
			window.draw(sm);
			window.draw(particles);

		}
		window.display();
		gameClock.restart();
	}

	return 0;
}