#include "Magasin_IF.hpp"

Magasin_IF::Magasin_IF()
{
    _interuptThread = std::thread(&Magasin_IF::trigger, this);
}

Magasin_IF::~Magasin_IF()
{
    _interuptThread.join();
}

bool Magasin_IF::decrementMag()
{
    if (_ammoAmount > 0)
    {
        _ammoAmount--;
        return true;
    }
    else
    {
        std::cout << "[Magasin][INFO] Magasin is empty" << std::endl;
        return false;
    }
}

void Magasin_IF::setMag(int difficulty)
{
    switch (difficulty)
    {
    case 1:
        _magSize = 13;
        break;
    case 2:
        _magSize = 9;
        break;
    case 3:
        _magSize = 5;
        break;
    default:
        std::cout << "[Magasin][WARN] Unexpected input for setMag. Input was: '" << difficulty << "'" << std::endl;
        return;
    }
    fillMag();
    std::cout << "[Magasin][INFO] Magsize set to " << _magSize << std::endl;
}

void Magasin_IF::trigger(void)
{
    while (true)
    {
        std::cout << "[Magasin][INFO] Starting interrupt thread" << std::endl;
        char *memblock = new char[1];
        std::ifstream triggerInterupt;
        triggerInterupt.open("/dev/targetbeam_magasin", std::ios::in);

        while (triggerInterupt.is_open())
        {
            memblock = new char[1];
            while (memblock[0] == NULL)
            {
                triggerInterupt.read(memblock, 1);
            }
            fillMag();
            if (_magSwitch != nullptr)
            {
                _magSwitch();
            }
            else
            {
                std::cout << "[Magasin][WARN] No callback for: MagSwitch" << std::endl;
            }
        }
        std::cout << "[Magasin][WARN] Could not open filestream. Restarting service in 5 seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void Magasin_IF::fillMag()
{
    _ammoAmount = _magSize;
}
