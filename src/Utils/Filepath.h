#ifndef FILEPATH_H
#define FILEPATH_H

#include "Utils/Pimpl.h"

namespace Util
{
	class Filepath
	{
		PIMPL(Filepath)

	public:
		Filepath(const QString& path);
		Filepath(const Filepath& other);		
		~Filepath();

		Filepath& operator=(const QString& path);
		Filepath& operator=(const Filepath& path);

		bool operator==(const QString& path) const;
		bool operator==(const Filepath& path) const;

		QString path() const;
		QString filesystem_path() const;

		bool is_resource() const;
		bool is_filesystem_path() const;
		bool is_url() const;
	};
}


#endif // FILEPATH_H
