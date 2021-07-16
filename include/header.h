#pragma once
#include <string>
#include <chrono>



class Swimmer
{
private:
    std::string name;
    double speed;
public:
    Swimmer(std::string& inName, double inSpeed) : name(inName), speed(inSpeed) {}

    std::string getName() { return name; }

    double getSpeed() { return speed; }
};



class Swimming : public Swimmer
{
private:
    //Пройденная дистанция в момент времени
    double currentDistance{};

    //Пловец финишировал
    bool isFinish{};

    //Финишное время
    std::chrono::nanoseconds finishTime{};
public:
    Swimming(std::string& inName, double inSpeed) :
            Swimmer(inName, inSpeed) {}

    void setCurrentDistance(double inCurrentDistance) { currentDistance = inCurrentDistance; }

    double getCurrentDistance() { return currentDistance; }

    void setIsFinish(bool inIsFinish) { isFinish = inIsFinish; }

    bool getIsFinish() { return isFinish; }

    void setFinishTime(std::chrono::nanoseconds inFinishTime) { finishTime = inFinishTime; }

    std::chrono::nanoseconds getFinishTime() { return finishTime; }
};