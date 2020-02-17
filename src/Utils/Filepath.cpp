#include "Filepath.h"
#include "Utils.h"
#include "FileUtils.h"
#include <QString>

using Util::Filepath;

struct Filepath::Private
{
	QString path;

	Private(const QString& path) :
		path(path)
	{}
};

Filepath::Filepath(const QString& path)
{
	m = Pimpl::make<Private>(path);
}

Filepath::Filepath(const Filepath& other) :
	Filepath(other.path())
{}

Filepath::~Filepath() = default;

Filepath& Filepath::operator=(const QString& path)
{
	m->path = Util::File::cleanFilename(path);
	return (*this);
}

Filepath& Filepath::operator=(const Filepath& path)
{
	m->path = path.path();
	return (*this);
}

bool Filepath::operator==(const QString& path) const
{
	return (m->path == Util::File::cleanFilename(path));
}

bool Filepath::operator==(const Filepath& path) const
{
	return (m->path == path.path());
}

QString Filepath::path() const
{
	return m->path;
}

QString Filepath::fileystemPath() const
{
	if( isResource() )
	{
		QString dir, filename;
		Util::File::splitFilename(m->path, dir, filename);

		QString local_path = Util::tempPath(filename);

		QString new_name;
		Util::File::copyFile(m->path, Util::tempPath(), new_name);

		return local_path;
	}

	return path();
}

bool Filepath::isResource() const
{
	return (m->path.startsWith(":"));
}

bool Filepath::isFilesystemPath() const
{
	if(isUrl() || isResource()){
		return false;
	}

	return true;
}

bool Filepath::isUrl() const
{
	return Util::File::isWWW(m->path);
}
