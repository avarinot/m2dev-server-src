#include "rules.h"

#include <optional>
#include <set>
#include <string>
#include <utility>

#include <toml.hpp>

namespace rules
{
	namespace
	{
		const std::set<std::string> kRuleFiles = { "bonuses.toml" };

		std::shared_ptr<const GameRules> g_current;

		// A TOML table read strictly: every key read is checked, and every key left unread is an error.
		class StrictTable
		{
		public:
			StrictTable(const toml::table& table, std::string path) : m_table(table), m_path(std::move(path)) {}

			StrictTable Table(const std::string& key)
			{
				const toml::table* table = Node(key).as_table();
				if (!table)
					Fail(key, "expected a table");
				return StrictTable(*table, Path(key));
			}

			template <size_t N>
			std::array<int, N> IntegerArray(const std::string& key, int min, int max)
			{
				const toml::array* array = Node(key).as_array();
				if (!array)
					Fail(key, "expected an array of " + std::to_string(N) + " integers");
				if (array->size() != N)
					Fail(key, "expected " + std::to_string(N) + " values, got " + std::to_string(array->size()));

				std::array<int, N> values{};
				for (size_t i = 0; i < N; ++i)
				{
					const std::optional<int64_t> value = (*array)[i].value_exact<int64_t>();
					if (!value)
						Fail(key, "value " + std::to_string(i + 1) + " is not an integer");
					if (*value < min || *value > max)
						Fail(key, "value " + std::to_string(i + 1) + " is " + std::to_string(*value) + ", outside "
							+ std::to_string(min) + ".." + std::to_string(max));
					values[i] = static_cast<int>(*value);
				}
				return values;
			}

			// Call once every expected key is read.
			void RejectUnknownKeys() const
			{
				for (const auto& [key, node] : m_table)
				{
					if (!m_read.contains(std::string(key.str())))
						throw RulesError(Path(std::string(key.str())) + ": unknown key");
				}
			}

		private:
			const toml::node& Node(const std::string& key)
			{
				const toml::node* node = m_table.get(key);
				if (!node)
					Fail(key, "missing");
				m_read.insert(key);
				return *node;
			}

			std::string Path(const std::string& key) const { return m_path + "." + key; }

			[[noreturn]] void Fail(const std::string& key, const std::string& problem) const
			{
				throw RulesError(Path(key) + ": " + problem);
			}

			const toml::table& m_table;
			std::string m_path;
			std::set<std::string> m_read;
		};

		toml::table ParseFile(const std::filesystem::path& file)
		{
			try
			{
				return toml::parse_file(file.string());
			}
			catch (const toml::parse_error& error)
			{
				const toml::source_position position = error.source().begin;
				throw RulesError(file.filename().string() + ":" + std::to_string(position.line) + ":"
					+ std::to_string(position.column) + ": " + std::string(error.description()));
			}
		}

		BonusRules LoadBonuses(const std::filesystem::path& file)
		{
			const toml::table document = ParseFile(file);
			StrictTable root(document, file.filename().string());

			BonusRules bonuses;
			StrictTable add = root.Table("add");
			bonuses.addSuccessPercent = add.IntegerArray<5>("success_percent", 0, 100);
			add.RejectUnknownKeys();

			root.RejectUnknownKeys();
			return bonuses;
		}
	}

	int BonusRules::AddSuccessPercent(int bonusesOnItem) const
	{
		if (bonusesOnItem < 0 || bonusesOnItem >= static_cast<int>(addSuccessPercent.size()))
			return 0;
		return addSuccessPercent[bonusesOnItem];
	}

	GameRules Load(const std::filesystem::path& directory)
	{
		std::error_code error;
		if (!std::filesystem::is_directory(directory, error))
			throw RulesError(directory.string() + ": rules directory not found");

		// A misnamed file would otherwise be ignored while its intended rules keep their old values.
		std::filesystem::directory_iterator files(directory, error);
		if (error)
			throw RulesError(directory.string() + ": " + error.message());
		for (const auto& entry : files)
		{
			const std::string name = entry.path().filename().string();
			if (entry.path().extension() == ".toml" && !kRuleFiles.contains(name))
				throw RulesError(name + ": unknown rules file");
		}
		for (const std::string& name : kRuleFiles)
		{
			if (!std::filesystem::is_regular_file(directory / name, error))
				throw RulesError(name + ": missing from " + directory.string());
		}

		GameRules gameRules;
		gameRules.bonuses = LoadBonuses(directory / "bonuses.toml");
		return gameRules;
	}

	void Install(GameRules gameRules)
	{
		g_current = std::make_shared<const GameRules>(std::move(gameRules));
	}

	std::shared_ptr<const GameRules> Current()
	{
		if (!g_current)
			throw RulesError("game rules used before they were loaded");
		return g_current;
	}
}
