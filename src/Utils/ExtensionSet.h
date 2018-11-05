#ifndef EXTENSIONSET_H
#define EXTENSIONSET_H

#include "Utils/Pimpl.h"
#include "Utils/Set.h"

class ExtensionSet
{
	PIMPL(ExtensionSet)

	public:
		ExtensionSet();
		~ExtensionSet();
		ExtensionSet(const ExtensionSet& other);
		ExtensionSet& operator=(const ExtensionSet& other);

		void add_extension(const QString& ext, bool enabled=true);
		void remove_extension(const QString& ext);
		void clear();
		bool contains_extension(const QString& ext);
		ExtensionSet& operator<<(const QString& ext);

		void set_enabled(const QString& ext, bool b);
		void enable(const QString& ext);
		void disable(const QString& ext);

		bool has_enabled_extensions() const;
		bool has_disabled_extensions() const;

		bool is_enabled(const QString& ext) const;

		QStringList enabled_extensions() const;
		QStringList disabled_extensions() const;
		QStringList extensions() const;

};

#endif // EXTENSIONSET_H
