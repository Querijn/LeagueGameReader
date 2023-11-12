#pragma once

namespace LeagueGameReader
{
	struct GameObjectManager;
	struct GameObject;
	struct Snapshot;

	enum class ConnectStatus
	{
		Connected,
		GameNotFound,
		CannotConnect,
		CannotFindBase,
		SettingsMissing,
	};

	ConnectStatus	ConnectToGame();
	void			SetSnapshotInterval(int milliseconds);
	const Snapshot& GetCurrentSnapshot();
	void			Destroy();
}