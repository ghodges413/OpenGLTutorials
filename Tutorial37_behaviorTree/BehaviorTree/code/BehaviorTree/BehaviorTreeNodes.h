//
//  BehaviorTreeNodes.h
//
#pragma once


// Types of nodes:
// Composite - This node has multiple children, processing them in whatever order it cares to (sequential, random, whatever)
//					A sequentially executed order is the most common composite node and it is called a "sequence" node.
//					A sequence is effectively an "AND" statement.  A selector is an "OR" statement.
// Decorator - a decorator has only one child, and it's purpose is to modify the child's result or repeat the child's execution.
//					The most common version is the "Inverter" which is effectively a "!" operator
// Leaf - This node has no children, it performs actual actions, such as "walk" "attack" "talk" "eat" "poop".
//					A leaf can also call into other behavior trees.  This allows for designers to chain together different trees.


// Composite - Sequence (and), Selector/Fallback (or), Random Sequence, Random Selector
// Decorator - Inverter, Succeeder, Failer, Repeater, Repeat Until Fail
// Leaf - aside from actions, it can also do conditional/control flow stuff...
//			such as PushStack(item, stackVar), PopStack(stack, itemVar), IsEmpty(stack)
//			SetVariable(varName, object), IsNull(object)
//			An example:  SetVariable( door, open )
//			Another example:  Find Doors Tree -> PushStack( door )... Open Doors Tree -> PopStack( door )... Repeat Until -> IsNull( doorStack )



// Character actions nodes:
// idle
// random walk (this could vary by alertness nodes)
// path to player
// attack player
// run from player
// regain health
// path back to navmesh (this assumes the thing got knocked off the navmesh.. it should die if it can't walk back)
// die


enum btState_t {
	BT_READY = 0,	// Not currently running, but can be run (we may not need this state)
	BT_SUCCESS,
	BT_FAILURE,
	BT_RUNNING,
};

struct btNode_t;

btState_t SpiderRoot( btNode_t * thisNode );
btState_t Idle( btNode_t * thisNode );
btState_t WalkToPlayer( btNode_t * thisNode );
btState_t Sequence( btNode_t * thisNode );
btState_t Selector( btNode_t * thisNode );
btState_t GetPlayerPos( btNode_t * thisNode );
btState_t WalkToPosition( btNode_t * thisNode );
btState_t EatFood( btNode_t * thisNode );
btState_t GetFoodPos( btNode_t * thisNode );
btState_t Eat( btNode_t * thisNode );



class BTNodeBase {
public:
	BTNodeBase() {}//{ m_children = NULL; m_numChildren = 0; }
	~BTNodeBase() {}

	virtual void Action() = 0;
	btState_t m_state;

	BTNodeBase ** m_children;
	int m_numChildren;
};

class BTNodeSpiderRoot : public BTNodeBase {
public:
	BTNodeSpiderRoot() : BTNodeBase() {}

	virtual void Action();
};