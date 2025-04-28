//----------------------------------------------------------------------------------------------------
// GameCommon.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------------
class App;
class AudioSystem;
class BitmapFont;
class Game;
class Renderer;
class RandomNumberGenerator;

// one-time declaration
extern App*                   g_theApp;
extern AudioSystem*           g_theAudio;
extern BitmapFont*            g_theBitmapFont;
extern Game*                  g_theGame;
extern Renderer*              g_theRenderer;
extern RandomNumberGenerator* g_theRNG;

//----------------------------------------------------------------------------------------------------
template <typename T>
void SafeDeletePointer(T*& pointer)
{
    delete pointer;
    pointer = nullptr;
}

void PlaySoundClicked(char const* keyName);
