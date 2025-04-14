#pragma once

#include <cosmos.h>

namespace Cosmos::Editor
{
	class Dockspace : public Cosmos::Widget
	{
	public:

		// constructor
		Dockspace(Application* application);

		// destructor
		virtual ~Dockspace() = default;

	public:

		// user interface updating
		virtual void OnUpdate() override;

	private:

		Application* mApp = nullptr;
	};
}