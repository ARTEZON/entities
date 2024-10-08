// entities2 © 2024 by norbcodes is licensed under CC BY-NC 4.0

/**
 * \file gameplay_loop.cpp
 * \author norbcodes
 * \brief The gameplay itself.
 * \copyright entities2 © 2024 by norbcodes is licensed under CC BY-NC 4.0
 */

#include <cstdint>
#include <iostream>
#include <fmt/core.h>

#include "colors.hpp"
#include "entity_constants.hpp"
#include "entity.hpp"
#include "status_constants.hpp"
#include "status.hpp"
#include "energy_constants.hpp"
#include "energy.hpp"
#include "rng.hpp"
#include "utils.hpp"
#include "discord_rpc.hpp"
#include "sleep.hpp"
#include "keyboard.hpp"
#include "ai.hpp"
#include "pick_move.hpp"
#include "gen_moves.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

static void GameOver(uint32_t& picker_flag, bool& is_running)
{
    fmt::print("{1}---<<< {3}{2}Player{0} {1}dead. {4}{2}Enemy{0}{1} wins!!! >>>---{0}\n\n", RESET, WHITE, BOLD, BLUE, RED);
    fmt::print("{3}[{0}{2}{1}1{0}{3}]{0} {4}Exit{0}\n", RESET, BOLD, GOLD, DARK_GRAY, RED);
    fmt::print("{3}[{0}{2}{1}2{0}{3}]{0} {4}Rematch!{0}\n", RESET, BOLD, GOLD, DARK_GRAY, HOT_PINK);
    EndDiv();

    uint32_t choice = WaitForNumkey();

    if (choice == 1)
    {
        picker_flag = false;
        is_running = false;
        return;
    }
    else if (choice == 2)
    {
        is_running = false;
        return;
    }
    else
    {
        picker_flag = false;
        is_running = false;
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

static void Victory(uint32_t& picker_flag, bool& is_running)
{
    fmt::print("{1}---<<< {4}{2}Enemy{0} {1}dead. {3}{2}Player{0}{1} wins!!! >>>---{0}\n\n", RESET, WHITE, BOLD, BLUE, RED);
    fmt::print("{3}[{0}{2}{1}1{0}{3}]{0} {4}Exit{0}\n", RESET, BOLD, GOLD, DARK_GRAY, RED);
    fmt::print("{3}[{0}{2}{1}2{0}{3}]{0} {4}Rematch!{0}\n", RESET, BOLD, GOLD, DARK_GRAY, HOT_PINK);
    EndDiv();

    uint32_t choice = WaitForNumkey();

    if (choice == 1)
    {
        picker_flag = false;
        is_running = false;
        return;
    }
    else if (choice == 2)
    {
        is_running = false;
        return;
    }
    else
    {
        picker_flag = false;
        is_running = false;
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

static void PlayerRound (
    uint32_t& picker_flag, 
    bool& is_running, 
    bool& enemy_turn, 
    std::string& what_happened, 
    Entity* Enemy, 
    Entity* Player,
    uint8_t difficulty_scale,
    uint32_t* moves,
    uint32_t* move_types,
    double* energy_costs
)
{
    ClearScreen();
    Div();

    if (Player->GetHealth() <= 0)
    {
        GameOver(picker_flag, is_running);
        return;
    }

    fmt::print("{3}---<<< {1}{2}Player's{0} {3}turn! >>>---{0}\n", RESET, BLUE, BOLD, DARK_GRAY);
    // Print history
    fmt::print("{2}What happened last round:{0}\n{1}{0}\n\n", RESET, what_happened, WHITE);
    what_happened = "";

    Player->UpdateStatuses(what_happened, false);

    // Print stats
    fmt::print("{1}{2}[PLAYER]{0}\t", RESET, BLUE, BOLD);
    PrintEntityStats(*Player);
    fmt::print("{1}{2}[ENEMY] {0}\t", RESET, RED, BOLD);
    PrintEntityStats(*Enemy);
    fmt::print("\n");
    // Print energy bars
    PrintEnergyBar(*Player);
    PrintEnergyBar(*Enemy);
    fmt::print("\n");

    GenerateMoves(moves, move_types, energy_costs);

    uint32_t picked_move;
    while (true)
    {
        fmt::print("{1}Choose your move. {2}{3}[0,1,2,3,4] (0 to exit)                                           {0}\n", RESET, WHITE, BOLD, GRAY);
        EndDiv();
        // Player
        picked_move = WaitForNumkey();

        if (picked_move == 0)
        {
            fmt::print("{1}Do you really wanna end the battle? {2}{3}[y,n]                                                             {0}\n", RESET, RED, GRAY, BOLD);
            EndDiv();
            if (BinaryChoice())
            {
                is_running = false;
                picker_flag = false;
                break;
            }
            else
            {
                continue;
            }
        }

        picked_move--;

        // Check if Player has enough energy
        if (energy_costs[picked_move] > Player->GetEnergy())
        {
            fmt::print("{1}Not enough energy!                                                            {0}\n", RESET, RED);
            continue;
        }

        break;
    }

    // if pick is not 0, 1, 2, 3 or 9 = skip round
    if (picked_move != 1 && picked_move != 2 && picked_move != 3 && picked_move != 4 && picked_move != 0)
    {
        what_happened += fmt::format("{1}{2}Player{0} {3}skipped the round.{0}", RESET, BLUE, BOLD, WHITE);
        return;
    }

    PickMove(Player, Enemy, picked_move, moves, move_types, energy_costs, enemy_turn, what_happened);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

static void EnemyRound (
    uint32_t& picker_flag, 
    bool& is_running, 
    bool& enemy_turn, 
    std::string& what_happened, 
    Entity* Enemy, 
    Entity* Player,
    uint8_t difficulty_scale,
    uint32_t* moves,
    uint32_t* move_types,
    double* energy_costs
)
{
    ClearScreen();
    Div();

    if (Enemy->GetHealth() <= 0)
    {
        Victory(picker_flag, is_running);
        return;
    }

    fmt::print("{3}---<<< {1}{2}Enemy's{0} {3}turn! >>>---{0}\n", RESET, RED, BOLD, DARK_GRAY);
    // Print history
    fmt::print("{2}What happened last round:{0}\n{1}{0}\n\n", RESET, what_happened, WHITE);
    what_happened = "";

    Enemy->UpdateStatuses(what_happened, true);

    // Print stats
    fmt::print("{1}{2}[PLAYER]{0}\t", RESET, BLUE, BOLD);
    PrintEntityStats(*Player);
    fmt::print("{1}{2}[ENEMY] {0}\t", RESET, RED, BOLD);
    PrintEntityStats(*Enemy);
    fmt::print("\n");
    // Print energy bars
    PrintEnergyBar(*Player);
    PrintEnergyBar(*Enemy);
    fmt::print("\n");

    GenerateMoves(moves, move_types, energy_costs);

    uint32_t picked_move = AiChoose(moves, move_types, energy_costs, *Player, *Enemy, difficulty_scale);

    // Print random num
    for (int i = 0; i != 20000; i++)
    {
        fmt::print("{1}{2}The AI is thinking... {3}{0}\n", RESET, GOLD, ITALIC, rng(1, 4));
        EndDivNoNewl();
    }

    fmt::print("{1}{2}The AI is thinking... {3}{0}\n", RESET, GOLD, ITALIC, picked_move + 1);
    SleepSeconds(2);

    // if pick is not 0, 1, 2, 3 or 9 = skip round
    if (picked_move > 3 && picked_move != 9)
    {
        what_happened += fmt::format("{2}{3}Enemy {1}skipped the round.{0}", RESET, WHITE, RED, BOLD);
        return;
    }

    PickMove(Enemy, Player, picked_move, moves, move_types, energy_costs, enemy_turn, what_happened);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief The game loop.
 * \param[in] mode Difficulty.
 * \param[out] picker_flag External flag to break a loop inside DifficultyPicker()
 */
void Game(uint32_t mode, uint32_t& picker_flag)
{
    // Wowie
    uint8_t difficulty_scale;
    if (mode == 1)
    {
        difficulty_scale = 0;
    }
    else if (mode == 2)
    {
        difficulty_scale = 2;
    }
    else if (mode == 3)
    {
        difficulty_scale = 4;
    }
    else if (mode == 4)
    {
        difficulty_scale = rng(0, 4);
    }

    // Create player and enemy
    // Heap alloc for funnidifficulty_scale
    Entity* Player = new Entity(
        (mode == 4) ? rng(10, 200) : PLAYER_START_HP - difficulty_scale * HEALTH_F, 
        (mode == 4) ? rng(10, 200) : PLAYER_START_AR - difficulty_scale * ARMOR_F
    );
    Entity* Enemy = new Entity(
        (mode == 4) ? rng(10, 200) : ENEMY_START_HP + difficulty_scale * HEALTH_F, 
        (mode == 4) ? rng(10, 200) : ENEMY_START_AR + difficulty_scale * ARMOR_F
    );
    // Yes, it literally rigs the game against you

    // Gameplay loop
    bool is_running = true;
    bool enemy_turn = false;  // true = Enemy, false = Player
    std::string what_happened = fmt::format("{1}{2}The fights begins.{0}", RESET, RED, ITALIC);

    while (is_running)
    {
        GameplayRPC(enemy_turn);
        // Generate 4 options to choose from.
        // There are 4 types: Attack, Heal, Regen armor, Status
        uint32_t* moves = new uint32_t[4]{100, 100, 100, 100};
        uint32_t* move_types = new uint32_t[4]{100, 100, 100, 100};
        double* energy_costs = new double[4]{100.0, 100.0, 100.0, 100.0};

        if (enemy_turn)
        {
            EnemyRound(picker_flag, is_running, enemy_turn, what_happened, Enemy, Player, difficulty_scale, moves, move_types, energy_costs);
        }
        else
        {
            PlayerRound(picker_flag, is_running, enemy_turn, what_happened, Enemy, Player, difficulty_scale, moves, move_types, energy_costs);
        }

        // Increase energy
        Player->GiveEnergy(ENERGY_INC);
        Enemy->GiveEnergy(ENERGY_INC);

        enemy_turn = !enemy_turn;
        delete[] moves;
        delete[] move_types;
        delete[] energy_costs;
    }

    // Make sure to annihilate
    delete Player;
    delete Enemy;
}

// entities2 © 2024 by norbcodes is licensed under CC BY-NC 4.0