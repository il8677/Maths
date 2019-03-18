#include 

class StateMachine;
class GameState
{
protected:
	static StateMachine * sm;
public:
	GameState();
	virtual ~GameState();
	virtual void update()=0;
};
