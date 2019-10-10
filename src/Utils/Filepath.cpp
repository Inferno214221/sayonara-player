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
	m->path = Util::File::clean_filename(path);
	return (*this);
}

Filepath& Filepath::operator=(const Filepath& path)
{
	m->path = path.path();
	return (*this);
}

bool Filepath::operator==(const QString& path) const
{
	return (m->path == Util::File::clean_filename(path));
}

bool Filepath::operator==(const Filepath& path) const
{
	return (m->path == path.path());
}

QString Filepath::path() const
{
	return m->path;
}

QString Filepath::filesystem_path() const
{
	if( is_resource() )
	{
		QString dir, filename;
		Util::File::split_filename(m->path, dir, filename);

		QString local_path = Util::temp_path(filename);
		Util::File::copy_file(m->path, Util::temp_path());

		return local_path;
	}

	return path();
}

bool Filepath::is_resource() const
{
	return (m->path.startsWith(":"));
}

bool Filepath::is_filesystem_path() const
{
	if(is_url() || is_resource()){
		return false;
	}

	return true;
}

bool Filepath::is_url() const
{
	return Util::File::is_www(m->path);
}
