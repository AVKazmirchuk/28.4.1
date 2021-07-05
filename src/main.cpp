#include <iostream>


#include "../include/header.h"




#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>



bool cinNoFail()
{
    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid data!\n";
        return false;
    }
    else
    {
        if (std::cin.peek() != '\n')
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid data!\n";
            return false;
        }
    }

    return true;
}

void input(std::vector<std::pair<std::string, double>>& swimmers, const int numberOFTracks)
{
    for (int i{}; i < numberOFTracks; ++i)
    {
        std::string name;
        double speed;

        while (true)
        {
            std::cout << "Swimmer " << i + 1 << ", name: ";
            std::cin >> name;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Swimmer " << i + 1 << ", speed: ";
            std::cin >> speed;

            if (cinNoFail()) break;
        }

        swimmers.emplace_back( name, speed );
    }
}

void output(std::vector<std::pair<std::string, double>>& swimmers)
{
    for (const auto& swimmer : swimmers)
        std::cout << swimmer.first << ' ' << swimmer.second << '\n';

}

std::mutex mutexCout, mutexVector;

void swimming(std::pair<std::string, double>& swimmer,
              std::vector<std::pair<std::string, double>>& swimmers, int distance)
{
    double currentDistance{};

    Timer t;

    while (currentDistance != distance)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        currentDistance += swimmer.second;
        mutexCout.lock();
        std::cout << swimmer.first << " swam " << currentDistance << " meters\n";
        mutexCout.unlock();
    }

    mutexVector.lock();
    swimmers.emplace_back( swimmer.first, t.elapsed() );
    mutexVector.unlock();
}

int main()
{
    const int numberOfTracks{ 6 };
    const int distance{ 100 };

    std::vector<std::pair<std::string, double>> swimmersStart;
    swimmersStart.reserve(numberOfTracks);

    std::vector<std::pair<std::string, double>> swimmersFinish;
    swimmersFinish.reserve(numberOfTracks);

    input(swimmersStart, numberOfTracks);

    for (int i{}; i < numberOfTracks; ++i)
    {
        std::thread swimmer(swimming, std::ref(swimmersStart[i]), std::ref(swimmersFinish), distance);
        swimmer.detach();
    }

    while (swimmersFinish.size() != numberOfTracks)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::sort(swimmersFinish.begin(), swimmersFinish.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    output(swimmersFinish);
}