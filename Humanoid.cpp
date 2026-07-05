#include "Humanoid.hpp"
#include "DebugPath.hpp"

Texture Humanoid::skin_tone_1("skin_tone_1.png", std::string(DEBUG_PATH));
Texture Humanoid::skin_tone_2("skin_tone_2.png", std::string(DEBUG_PATH));
Texture Humanoid::skin_tone_3("skin_tone_3.png", std::string(DEBUG_PATH));
Texture Humanoid::skin_tone_4("skin_tone_4.png", std::string(DEBUG_PATH));
Texture Humanoid::skin_tone_5("skin_tone_5.png", std::string(DEBUG_PATH));
Texture Humanoid::skin_tone_6("skin_tone_6.png", std::string(DEBUG_PATH));

std::unordered_set<Texture*> Humanoid::skin_tones{ &Humanoid::skin_tone_1,&Humanoid::skin_tone_2 ,&Humanoid::skin_tone_3 ,&Humanoid::skin_tone_4 ,&Humanoid::skin_tone_5 ,&Humanoid::skin_tone_6 };