#include "Laser_IF.hpp"

int main(int argc, char **argv)
{
    int userInput = atoi(argv[1]);

    printf("PWM: %s\n", argv[1]);


    Laser_IF laserObj(userInput);

    laserObj.shootLaser();

    return 0;
}