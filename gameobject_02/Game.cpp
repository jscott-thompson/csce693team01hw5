
#include "Game.hpp"

#include <iostream>
#include "SDL2/SDL_image.h"
#include "sol/sol.hpp"

#include "sdl_utils.hpp"
#include "gameobjects/Chopper.hpp"
#include "gameobjects/Tank.hpp"
#include "gameobjects/Pacman.hpp"
#include <stdexcept>

SDL_Renderer* Game::renderer{};
SDL_Window* Game::window{};
sol::state Game::lua{};

Game::Game(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
   Uint32 flags{};
   if (fullscreen) {
      flags = SDL_WINDOW_FULLSCREEN;
   }

   if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
      std::cout << "Subsystems initialized..." << std::endl;
      window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
      if (window) {
         std::cout << "Window created..." << std::endl;
      }
      renderer = SDL_CreateRenderer(window, -1, 0);
      if (renderer) {
         SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
         std::cout << "Renderer created..." << std::endl;
      }
   is_running = true;
   } else {
      is_running = false;
   }

   // Initialize Lua via sol and load the config file
   try {
      lua.open_libraries(sol::lib::base, sol::lib::package);
      // throw sol::error("Forced error after opening Lua libs for testing");
   }
   catch (const sol::error& e) {
      std::cerr << e.what() << std::endl;
      throw std::runtime_error("Couldn't open lua libraries");
   }

   try {
      lua.script_file("config.lua");
   }
   catch (const sol::error& e) {
      std::cerr << e.what() << std::endl;
      throw std::runtime_error("Error loading lua config file");
   }
}

Game::~Game()
{
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   SDL_Quit();
}

void Game::load_level()
{
   //load the gameobj table from the config file
   sol::table t = lua["gameobjs"];
   
   //iterate over each player in the table
   for(const auto& key_value_pair : t) {
      //the key represents the player name
      sol::object key = key_value_pair.first;

      //use the key to get the table for that player
      sol::table player = lua["gameobjs"][key.as<std::string>()];

      //extract the needed attributes from the table
      std::string kind = static_cast<std::string>(player["kind"]);
      float xpos = static_cast<float>(player["xpos"]);
      float ypos = static_cast<float>(player["ypos"]); 
      float xvel = static_cast<float>(player["xvel"]);
      float yvel = static_cast<float>(player["yvel"]);
      
      //determine what type of game object to create and then add then to the game objects vector
      if(kind.compare("chopper") == 0) {
         auto chopper = std::make_unique<Chopper>(xpos, ypos, xvel, yvel);
         game_objs.emplace_back(std::move(chopper));
      } else if(kind.compare("tank") == 0) {
         auto tank = std::make_unique<Tank>(xpos, ypos, xvel, yvel);
         game_objs.emplace_back(std::move(tank));
      } else if(kind.compare("pacman") == 0) {
         auto pacman = std::make_unique<Pacman>(xpos, ypos, xvel, yvel);
         game_objs.emplace_back(std::move(pacman));
      } else {
         std::cerr << "Unknown type\n";
      }
   } 
}

void Game::handle_events()
{
   SDL_Event event;
   SDL_PollEvent(&event);
   switch (event.type) {
      case SDL_QUIT:
         is_running = false;
         break;
      default:
         break;
   }
}

void Game::update(const float dt)
{
   for (auto& i: game_objs) {
      i->update(dt);
   }
}

void Game::render()
{
   SDL_RenderClear(renderer);
   for (auto& i: game_objs) {
      i->render();
   }
   SDL_RenderPresent(renderer);
}

