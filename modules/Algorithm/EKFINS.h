// yet another filter for GPS-INS
// earth frame: [north, east, up] (wirtten xxx_ned...)
// body frame: [forward, right, up]

#pragma once

#include <stdint.h>
#include <Protocol/common.h>
#include <utils/fifo.h>
#include <math/matrix.h>
#include <HAL/Interface/IGPS.h>


enum EKFINS_healthy
{
	EKFINS_healthy_none = 0,
	EKFINS_healthy_att = 1,
	EKFINS_healthy_alt = 2,
	EKFINS_healthy_3D_pos = 4,
};

class EKFINS
{
public:
	EKFINS();
	~EKFINS();

	int reset();		// mainly for after GPS glitch handling and mag calibration
	int update(const float gyro[3], const float acc_body[3], const float mag[3], devices::gps_data gps, float baro, float dt);
	void set_gps_latency(int new_latency){latency = new_latency;}
	int healthy();
	int warning();	

//protected:
	CircularQueue<matrix, 20> history_pos;
	int64_t last_history_push;
	int latency;
	double home_lat;
	double home_lon;
	float ticker;
	bool position_healthy;

	matrix P;
	matrix x; // {q[4], pos_ned[3], vel_ned[3], gyro_bias[3], acc_bias_body[3], vel_bias_ned[3]}, 19 states, no mag bias, since EKF won't track mag bias correctly.
	matrix Q;
	matrix R;

	float acc_ned[3];
	float gps_north;
	float gps_east;
};