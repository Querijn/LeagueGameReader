{
  "targets": [
    {
      "target_name": "LeagueGameReader",
      "include_dirs": ["inc"],
      "ccflags": [ "-std=c++17" ],
      "sources": [
          "src/game_reader.cpp",
          "src/library.cpp",
          "src/process.cpp",
          "src/settings.cpp",
          "swigJAVASCRIPT_wrap.cxx" // NOTE: This needs to actually point at the build folder's file. This is different on your own machine.
      ]
    }
  ]
}