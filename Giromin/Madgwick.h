//
//  Madgwick.h
//  
//
//=============================================================================================
//
// Madgwick's implementation of Mahony's AHRS algorithm.
// See: http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
//
// Date            Author            Notes
// 29/09/2011    SOH Madgwick    Initial release
// 02/10/2011    SOH Madgwick    Optimised for reduced CPU load
// 19/02/2012    SOH Madgwick    Magnetometer measurement is normalised
// 23/11/2017   Aster            Optimised time handling and melted in one library
// 02/12/2019   João Tragtenberg    Restricted to sending Quaternions

//

#ifndef Madgwick_h
#define Madgwick_h

#include <Giromin.h>
#include <math.h>


class Madgwick{
public:
    Madgwick();
    
    void MadgwickUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float deltat);
    void MadgwickUpdate(float gx, float gy, float gz, float ax, float ay, float az, float deltat);
    
    float deltatUpdate (){
        Now = micros();
        deltat = ((Now - lastUpdate) / 1000000.0f); // set integration time by time elapsed since last filter update
        lastUpdate = Now;
        return deltat;
    }
    float getQ0() {
        return q0;
    }
    float getQ1() {
        return q1;
    }
    float getQ2() {
        return q2;
    }
    float getQ3() {
        return q3;
    }
    
//    float* getQuaternions(){
//        q = {q0, q1, q2, q3};
//        return q;
//    }
private:
    float beta;                //Madgwick: 2 * proportional gain
    float twoKp;            //Mahony: 2 * proportional gain (Kp)
    float twoKi;            //Mahony: 2 * integral gain (Ki)
    float q0, q1, q2, q3;    // quaternion of sensor frame relative to auxiliary frame
    float integralFBx, integralFBy, integralFBz;  // integral error terms scaled by Ki
    bool anglesComputed;
    static float invSqrt(float x);
    void computeAngles();
    float roll, pitch, yaw;
    float Now,lastUpdate,deltat;
    float q[4];
};

#endif /* Madgwick_hpp */
