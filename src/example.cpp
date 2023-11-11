#include <Windows.h>
#include <string>
#include <vector>

#include <league_gamereader.hpp>
#include <process.h>

int main()
{
	LeagueGameReader::SetSnapshotInterval(1);
	while (true)
	{
		auto status = LeagueGameReader::ConnectToGame();
		if (status != LeagueGameReader::ConnectStatus::Connected)
		{
			Sleep(100);
			continue;
		}

		const LeagueGameReader::Snapshot& snapshot = LeagueGameReader::GetCurrentSnapshot();
		printf("Game time: %f\n", snapshot.time);
		Sleep(1);
	}

	LeagueGameReader::Destroy();
}
