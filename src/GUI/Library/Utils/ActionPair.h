#ifndef ACTIONPAIR_H
#define ACTIONPAIR_H

#include <QString>
#include "Utils/Library/Sortorder.h"
#include "Utils/Language.h"

struct ActionPair
{
	QString name;
	Library::SortOrder so;

	ActionPair();
	ActionPair(const QString& name, Library::SortOrder so);
	ActionPair(Lang::Term t1, Lang::Term t2, Library::SortOrder so);
};

#endif // ACTIONPAIR_H
