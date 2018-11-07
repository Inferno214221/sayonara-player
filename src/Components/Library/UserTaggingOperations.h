#ifndef USERTAGGINGOPERATIONS_H
#define USERTAGGINGOPERATIONS_H

#include <QObject>
#include "Utils/Pimpl.h"

class UserTaggingOperations :
		public QObject
{
	Q_OBJECT
	PIMPL(UserTaggingOperations)

public:
	UserTaggingOperations(const QObject* parent=nullptr);
	~UserTaggingOperations();
};

#endif // USERTAGGINGOPERATIONS_H
