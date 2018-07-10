#include "ActionPair.h"

ActionPair::ActionPair() {}
ActionPair::ActionPair(const QString& name, Library::SortOrder so) :
	name(name),
	so(so)
{}
