#include <stack>

class GameState;

static class StateMachine
{
	std::stack<GameState*> states;
public:
	StateMachine();
	~StateMachine();
	GameState * top() { return states.top(); };
	void PopState();
	void AddState(GameState * gs, bool replace = false);
}sm;
