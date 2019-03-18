#include "stateMachine.h"



StateMachine::StateMachine()
{
}


StateMachine::~StateMachine()
{
}


void StateMachine::PopState()
{
	// TODO: Add your implementation code here.
	states.pop();
}


void StateMachine::AddState(GameState * gs, bool replace = false)
{
	if (replace) {

	}
}
