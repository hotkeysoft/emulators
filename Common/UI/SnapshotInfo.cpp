#include "stdafx.h"

#include <UI/SnapshotInfo.h>
#include "Computer.h"

namespace fs = std::filesystem;

namespace ui
{
	SnapshotInfo::SnapshotInfo(const std::filesystem::path& snapshotDir) :
		m_snapshotDir(snapshotDir)
	{
	}

	void SnapshotInfo::Clear()
	{
		m_description.clear();
		m_pc.clear();
		m_video.clear();
		m_isLoaded = false;
	}

	void SnapshotInfo::FromPC(emul::Computer* pc)
	{
		assert(pc);
		Clear();

		m_pc = pc->GetName();
		m_video = pc->GetVideo().GetDisplayName();
		m_isLoaded = true;
	}

	bool SnapshotInfo::FromDisk()
	{
		Clear();

		fs::path inFile = m_snapshotDir;
		inFile.append("snapshot.json");
		std::ifstream inStream(inFile);

		if (!inStream)
		{
			return false;
		}

		json j;
		try
		{
			inStream >> j;

			m_pc = j["pc"];
			m_video = j["video"];
			m_description = j["description"];
		}
		catch (std::exception e)
		{
			return false;
		}

		m_isLoaded = true;
		return true;
	}

	bool SnapshotInfo::ToDisk() const
	{
		json j;
		j["pc"] = m_pc;
		j["video"] = m_video;
		j["description"] = m_description;

		try
		{
			fs::path outFile = m_snapshotDir;
			outFile.append("snapshot.json");
			std::ofstream outStream(outFile.string());
			outStream << std::setw(4) << j;
		}
		catch (std::exception)
		{
			return false;
		}

		return true;
	}

	TimeFormatter SnapshotInfo::GetTimestamp() const
	{
		std::string name = m_snapshotDir.filename().string();

		time_t value = 0;
		if (sscanf(name.c_str(), "snap-%llu", &value) != 1)
		{
			value = 0;
		}
		return TimeFormatter(value);
	}

	std::string SnapshotInfo::ToString() const
	{
		std::ostringstream os;
		os << m_description;
		if (m_description.size())
		{
			os << ' ';
		}
		if (m_isLoaded && m_pc.size())
		{
			os << '[' << m_pc << "][" << m_video << "]";
		}
		else
		{
			os << "[Unknown]";
		}

		return os.str();
	}
}