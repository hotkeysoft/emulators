#pragma once
#include <Serializable.h>
#include "TimeFormatter.h"

namespace emul
{
	class ComputerBase;
}

namespace ui
{
	class SnapshotInfo
	{
	public:
		SnapshotInfo() = default;
		SnapshotInfo(const std::filesystem::path& snapshotDir);

		bool IsLoaded() const { return m_isLoaded; }
		void Clear();

		const std::string& GetDescription() const { return m_description; }
		const std::string& GetPC() const { return m_pc; }

		void SetDescription(const char* desc) { m_description = desc ? desc : ""; }

		void FromPC(emul::ComputerBase* pc);

		bool FromDisk();
		bool ToDisk() const;

		std::string ToString() const;

		TimeFormatter GetTimestamp() const;

	protected:
		bool m_isLoaded = false;
		std::filesystem::path m_snapshotDir;

		std::string m_pc;
		std::string m_model;
		std::string m_description;
	};
}
