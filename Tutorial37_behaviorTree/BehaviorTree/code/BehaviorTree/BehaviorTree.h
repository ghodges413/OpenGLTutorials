//
//  BehaviorTree.h
//
#pragma once

struct btNode_t;

void BuildBehaviorTree();
void CleanUpTree();
void TickTree();

enum btState_t {
	BT_READY = 0,	// Not currently running, but can be run (we may not need this state)
	BT_SUCCESS,
	BT_FAILURE,
	BT_RUNNING,
};



typedef btState_t action_t( btNode_t * thisNode );	// Function type for the node

struct btNode_t {
	action_t * action;	// Function pointer for the node
	btState_t state;	// We probably don't need to store the state on the node (actually, we probably do)

	btNode_t * children;
	int numChildren;

	Vec3 data;	// This is the player position... the reality is OOP is probably the easiest way to write a behavior tree
};


// Types of nodes:
// Composite - This node has multiple children, processing them in whatever order it cares to (sequential, random, whatever)
//					A sequentially executed order is the most common composite node and it is called a "sequence" node.
//					A sequence is effectively an "AND" statement.  A selector is an "OR" statement.
// Decorator - a decorator has only one child, and it's purpose is to modify the child's result or repeat the child's execution.
//					The most common version is the "Inverter" which is effectively a "!" operator
// Leaf - This node has no children, it performs actual actions, such as "walk" "attack" "talk" "eat" "poop".
//					A leaf can also call into other behavior trees.  This allows for designers to chain together different trees.


// Composite - Sequence, Selector, Random Sequence, Random Selector
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

