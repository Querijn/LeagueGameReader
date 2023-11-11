#include <Windows.h>

#include <league_gamereader.hpp>
#include <process.h>

int main()
{
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
		Sleep(1000);
	}

	LeagueGameReader::Destroy();
}
