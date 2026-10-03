#pragma once

// Game rules as data (ADR-0009 in the mt2 repository): tunable numbers read from TOML files at boot instead of
// constants compiled into the server. Loading is strict: a missing file or key, an unknown key (a typo) or a value
// out of its bounds stops the server with a message naming the file and the key, rather than running with a
// silent default.

#include <array>
#include <filesystem>
#include <memory>
#include <stdexcept>

namespace rules
{
	class RulesError : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	// bonuses.toml
	struct BonusRules
	{
		// Chance in percent that adding a bonus succeeds, indexed by the number of bonuses the item already has.
		std::array<int, 5> addSuccessPercent{};

		// 0 for an item that already has every bonus an add-bonus scroll can give.
		int AddSuccessPercent(int bonusesOnItem) const;
	};

	struct GameRules
	{
		BonusRules bonuses;
	};

	// Reads every rules file of the directory; throws RulesError.
	GameRules Load(const std::filesystem::path& directory);

	// The rules the server runs with. Installed at boot; kept as an immutable snapshot so that a later reload
	// command can replace it without changing the readers.
	void Install(GameRules gameRules);
	std::shared_ptr<const GameRules> Current();
}
