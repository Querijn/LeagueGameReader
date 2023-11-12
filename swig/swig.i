%module LeagueGameReader

%include <std_string.i>
%include <std_vector.i>

%{
#include <league_gamereader.hpp>
%}

%include <league_gamereader.hpp>
%template(GameObjectVector) std::vector<LeagueGameReader::GameObject>;

