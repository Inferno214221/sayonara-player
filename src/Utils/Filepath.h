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
			QString fileystemPath() const;

			bool isResource() const;
			bool isFilesystemPath() const;
			bool isUrl() const;
	};
}


#endif // FILEPATH_H
