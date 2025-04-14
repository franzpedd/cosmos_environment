#pragma once

#include <cosmos.h>

namespace Cosmos::Editor
{
	class Demo : public Cosmos::Widget
	{
	public:

		// constructor
		Demo(Application* app);

		// destructor
		virtual ~Demo() = default;

	public:

		// user interface updating
		virtual void OnUpdate() override;

	private:

		Application* mApp = nullptr;
	};
}