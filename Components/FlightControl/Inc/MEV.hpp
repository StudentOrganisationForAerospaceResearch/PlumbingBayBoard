/**
 ******************************************************************************
 * File Name          : MEV.hpp
 * Description        : Servo State and Control Actions
 ******************************************************************************
*/

#ifndef SOAR_MEV_HPP_
#define SOAR_MEV_HPP_

class MEV {
    public:
        static void OpenMEV();
        static void CloseMEV();

        bool GetServoState();

    private:
        static bool servoState;
};

#endif
