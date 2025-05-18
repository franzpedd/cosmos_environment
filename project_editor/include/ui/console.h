#pragma once

#include <cosmos.h>

namespace Cosmos::Editor
{
	class Console : public Widget
	{
	public:

		/// @brief constructor
		Console();

		/// @brief destructor
		virtual ~Console() = default;

	public:

		/// @brief called on loop update
		virtual void OnUpdate() override;
	};
}