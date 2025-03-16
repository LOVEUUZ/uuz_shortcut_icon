#include "config.h"

json Coordinate::toJson() const {
  return json{{"x", x}, {"y", y}};
}

Coordinate Coordinate::fromJson(const json & j) {
  return Coordinate(j.at("x").get<int>(), j.at("y").get<int>());
}

// 将 Config 转为 JSON
json Config::toJson() const {
  return json{
    {"id", id},
    {"fileName", fileName},
    {"fileOrigenPath", fileOrigenPath},
    {"showName", showName},
    {"absolutePath", absolutePath},
    {"creationTime", creationTime},
    {"lastMoveTime", lastMoveTime},
    {"coordinate", coordinate.toJson()},
    // {"count", count}
  };
}

// 从 JSON 转为 Config
Config Config::fromJson(const json & j) {
  Config config;
  config.id       = j.at("id").get<int>();
  config.fileName = j.at("fileName").get<std::string>();

  //24.11.20 出现异常说明没有这个字段，则给一个默认值
  try {
    config.fileOrigenPath = j.at("fileOrigenPath").get<std::string>();
  }
  catch (...) {
    config.fileOrigenPath = "";
  }

  config.showName     = j.at("showName").get<std::string>();
  config.absolutePath = j.at("absolutePath").get<std::string>();
  config.creationTime = j.at("creationTime").get<std::string>();
  config.lastMoveTime = j.at("lastMoveTime").get<std::string>();
  config.coordinate   = Coordinate::fromJson(j.at("coordinate"));
  // config.count = j.at("count").get<int>();
  return config;
}
