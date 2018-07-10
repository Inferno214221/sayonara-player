#ifndef ACTIONPAIR_H
#define ACTIONPAIR_H

#include <QString>
#include "Utils/Library/Sortorder.h"

struct ActionPair
{
	QString name;
	Library::SortOrder so;

	ActionPair();
	ActionPair(const QString& name, Library::SortOrder so);
};

#endif // ACTIONPAIR_H
