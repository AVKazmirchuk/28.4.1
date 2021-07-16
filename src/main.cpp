#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>

#include "../include/header.h"



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



void input(std::vector<Swimming>& swimming, const int numberOFTracks)
{
    std::string name;
    double speed;

    for (int i{}; i < numberOFTracks; ++i)
    {
        while (true)
        {
            std::cout << "Swimmer " << i + 1 << ", name: ";
            std::cin >> name;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Swimmer " << i + 1 << ", speed (m/s): ";
            std::cin >> speed;

            if (cinNoFail()) break;
        }

        swimming.emplace_back(name, speed);
    }
}



void output(std::vector<Swimming>& swimming)
{
    std::cout << "\nFinal results:\n";

    for (auto& swimmer : swimming)

        std::cout << swimmer.getName() << " -> " << std::chrono::duration_cast<std::chrono::duration<double,
                std::ratio<1>>>(swimmer.getFinishTime()).count() << 's' << '\n';
}

//Заплыв

void trackSwimming(Swimming& swimmer, const int distance, std::mutex& mutexSwimming, std::mutex& mutexCout)
{
    //Время старта пловца
    auto timePointStart = std::chrono::steady_clock::now();

    //Скорость пловца, м/нс
    double speed = swimmer.getSpeed() / 1'000'000'000;

    //Текущее время заплыва
    std::chrono::nanoseconds currentSwimTime{};

    //Текущая пройденная дистанция
    double currentDistance{};

    //Пловец на дистанции

    while (currentDistance < distance)
    {
        currentSwimTime = std::chrono::steady_clock::now() - timePointStart;

        currentDistance = speed * currentSwimTime.count();

        //Текущая пройденная дистанция за секунду для вывода отдельным потоком

        std::lock_guard<std::mutex> lg(mutexSwimming);

        swimmer.setCurrentDistance(currentDistance);
    }

    //Финиш пловца
    {
        std::lock_guard<std::mutex> lg(mutexSwimming);

        swimmer.setIsFinish(true);
        swimmer.setFinishTime(currentSwimTime);
    }

    //Вывод информации о финише пловца

    std::lock_guard<std::mutex> lg(mutexCout);

    std::cout << '\n' << swimmer.getName() << " came to the finish line!\n\n";
}

//Вывод пройденной дистанции пловцами за секунду

void outputCurrentDistance(std::vector<Swimming>& swimming,
                           const std::chrono::seconds& timeInterval, std::atomic<bool>& isFinish, std::mutex& mutexSwimming, std::mutex& mutexCout)
{
    while (!isFinish.load())
    {
        std::this_thread::sleep_for(timeInterval);

        std::lock_guard<std::mutex> lg(mutexSwimming);

        std::lock_guard<std::mutex> lgCout(mutexCout);

        for (auto& swimmer : swimming)
        {
            if (!swimmer.getIsFinish())
                std::cout << swimmer.getName() << " swam " << swimmer.getCurrentDistance() << " meters\n";
        }
    }
}



int main()
{
    const int numberOfTracks{ 6 };
    const int distance{ 100 };

    //Интервал времени для вывода пройденной дистанции
    const std::chrono::seconds timeInterval{ 1 };

    //Участники заплыва
    std::vector<Swimming> swimming;
    swimming.reserve(numberOfTracks);

    input(swimming, numberOfTracks);

    std::vector<std::thread> swimmingThreads;
    swimmingThreads.reserve(numberOfTracks);

    std::mutex mutexSwimming;
    std::mutex mutexCout;

    //Судья даёт команду "старт"
    for (auto& swimmer : swimming)
        swimmingThreads.emplace_back(trackSwimming, std::ref(swimmer), distance, std::ref(mutexSwimming), std::ref(mutexCout));

    //Флаг завершения работы счётчика, после прохождения дистанции всеми пловцами
    std::atomic<bool> isFinish(false);

    //Судья запускает счётчик пройденной дистанции пловцами за секунду
    std::thread count(outputCurrentDistance, std::ref(swimming), std::ref(timeInterval),
                      std::ref(isFinish), std::ref(mutexSwimming), std::ref(mutexCout));

    //Судья ожидает окончание заплыва
    for (auto& thread : swimmingThreads)
        thread.join();

    isFinish.store(true);

    count.join();

    //Судья подводит итоги и публикует результаты
    std::sort(swimming.begin(), swimming.end(),
              [](auto& a, auto& b) { return a.getFinishTime() < b.getFinishTime(); });

    output(swimming);
}