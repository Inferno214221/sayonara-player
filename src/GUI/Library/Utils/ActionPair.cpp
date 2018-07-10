#include "ActionPair.h"

ActionPair::ActionPair() {}
ActionPair::ActionPair(const QString& name, Library::SortOrder so) :
	name(name),
	so(so)
{}

ActionPair::ActionPair(Lang::Term t1, Lang::Term t2, Library::SortOrder so)
{
	this->name = QString("%1 (%2)").arg(Lang::get(t1), Lang::get(t2));
	this->so = so;
}
