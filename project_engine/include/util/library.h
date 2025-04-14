#pragma once

#include <unordered_map>
#include <string>

namespace Cosmos
{
	template<typename T>
	class Library
	{
	public:

		// constructor
		Library() = default;

		// destructor
		~Library()
		{
			mContent.clear();
		}

		// returns all objects ref
		inline std::unordered_map<std::string, T>& GetAllRefs() { return mContent; }

	public:

		// checks if a given content/element exists within the library
		inline bool Exists(std::string name)
		{
			return mContent.find(name) != mContent.end() ? true : false;
		}

		// inserts a new content/element into the library
		inline void Insert(std::string name, T object)
		{
			if (mContent.find(name) == mContent.end()) {
				mContent[name] = object;
			}
		}

		// deletes a content/element from the library
		inline void Remove(std::string name)
		{
			if (Exists(name)) {
				mContent.erase(name);
			}
		}

		// erases a given content/element from the library
		inline void Erase(std::string name)
		{
			if (mContent.find(name) != mContent.end()) {
				mContent.erase(name);
			}
		}

		// returns the object reference
		inline T& GetRef(std::string name)
		{
			if (!Exists(name)) { // cannot return a reference for a non-existent content
				std::abort();
			}

			return mContent[name];
		}

	private:

		std::unordered_map<std::string, T> mContent;
	};
}