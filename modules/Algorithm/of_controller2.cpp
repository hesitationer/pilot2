#include "of_controller2.h"
#include <Protocol/common.h>
#include <utils/log.h>

static float pid_factor[2][4] =
{
	{1.5f, 0.5f, 0.0f, 2.0f},	// roll position P, rate P, rate I, rate D, rate IMAX
	{1.5f, 0.5f, 0.0f, 2.0f},	// pitch position P, rate P, rate I, rate D, rate IMAX
};

static float pid_factor_outter[2] =
{
	1.0f,
	1.0f,
};

OpticalFlowController2::OpticalFlowController2()
{
	// from m/s^2 to radian:
	for(int i=0; i<3; i++)
	{
		pid_factor[0][i] /= G_in_ms2*1.5f;
		pid_factor[1][i] /= G_in_ms2*1.5f;
	}
}

OpticalFlowController2::~OpticalFlowController2()
{
}

int OpticalFlowController2::update_controller(float flow_roll, float flow_pitch, float stick_roll, float stick_pitch, float dt)			// unit: meter/s
{
	// save stick input
	float alpha5 = dt / (dt + 1.0f/(2 * PI * 5.0f));	
	m_user[0] = stick_roll;
	m_user[1] = stick_pitch;

	if ( (fabs(stick_roll) < 5 * PI / 180 ) && (fabs(stick_pitch) < 5 * PI / 180))
	{
		// position error is integration of flow reading
		m_position_error[0] += flow_roll * dt;
		m_position_error[1] += flow_pitch * dt;

		m_braking_time += dt;

		if (m_braking_time < 1.5f)
		{
			m_position_error[0] = 0;
			m_position_error[1] = 0;
		}

		// apply 5hz low pass filter to flow reading.
		if (isnan(m_flow_lpf[0]))
		{
			m_flow_lpf[0] = flow_roll;
			m_flow_lpf[1] = flow_pitch;
		}
		else
		{
			m_flow_lpf[0] = flow_roll * alpha5 + (1-alpha5) * m_flow_lpf[0];
			m_flow_lpf[1] = flow_pitch * alpha5 + (1-alpha5) * m_flow_lpf[1];

			flow_roll = m_flow_lpf[0];
			flow_pitch = m_flow_lpf[1];
		}

		// velocity error
		float new_P[2] = 
		{
			pid_factor_outter[0] * m_position_error[0] + flow_roll,
			pid_factor_outter[1] * m_position_error[1] + flow_pitch,
		};
		float devertive[2] = 
		{
			(new_P[0] - m_pid[0][0]) / dt,
			(new_P[1] - m_pid[1][0]) / dt,
		};
		m_pid[0][0] = new_P[0];
		m_pid[1][0] = new_P[1];

		// I
		m_pid[0][1] += m_pid[0][0] *dt;
		m_pid[1][1] += m_pid[1][0] *dt;
		m_pid[0][1] = limit(m_pid[0][1], -pid_factor[0][3], pid_factor[0][3]);
		m_pid[1][1] = limit(m_pid[1][1], -pid_factor[1][3], pid_factor[1][3]);

		// D, with 5hz LPF
		if (isnan(m_pid[0][2]))
		{
			m_pid[0][2] = devertive[0];
			m_pid[1][2] = devertive[1];
		}
		else
		{
			m_pid[0][2] = devertive[0] * alpha5 + m_pid[0][2] * (1-alpha5);
			m_pid[1][2] = devertive[1] * alpha5 + m_pid[1][2] * (1-alpha5);
		}

		// sum
		float new_roll =  m_pid[0][0] * pid_factor[0][0] + m_pid[0][1] * pid_factor[0][1] + m_pid[0][2] * pid_factor[0][2];
		float new_pitch = m_pid[1][0] * pid_factor[1][0] + m_pid[1][1] * pid_factor[1][1] + m_pid[1][2] * pid_factor[1][2];

		// limit rotation speed, 30 degree/s, then 2hz LPF
		float delta_roll = new_roll - m_result[0];
		float delta_pitch = new_pitch - m_result[1];
		//delta_roll = limit(delta_roll, -dt * 75 * PI / 180, dt * 75 * PI / 180);
		//delta_pitch = limit(delta_pitch, -dt * 75 * PI / 180, dt * 75 * PI / 180);
		m_result[0] += delta_roll;
		m_result[1] += delta_pitch;

		//float alpha = dt / (dt + 1.0f/(2 * PI * 5.0f));	
		//m_result[0] = (m_result[0] + delta_roll) * alpha + (1-alpha) * m_result[0];
		//m_result[1] = (m_result[1] + delta_pitch) * alpha + (1-alpha) * m_result[1];


		// limit maximum flow angle: 10 degree
		m_result[0] = limit(m_result[0], -20 * PI / 180, 20 * PI / 180);
		m_result[1] = limit(m_result[1], -20 * PI / 180, 20 * PI / 180);
	}

	else
	{
		reset();
	}
	
	log2(&m_pid[0][0], 7, 32);		// overrun to include m_position_error

	return 0;
}

int OpticalFlowController2::reset()
{
	m_braking_time = 0;

	// pass through
	m_result[0] = 0;
	m_result[1] = 0;
	m_position_error[0] = 0;
	m_position_error[1] = 0;

	// reset pos integral and pid integral
	m_pid[0][0] = 0;
	m_pid[1][0] = 0;
	m_pid[0][1] = 0;
	m_pid[1][1] = 0;
	m_pid[0][2] = NAN;
	m_pid[1][2] = NAN;
	m_flow_lpf[0] = NAN;
	m_flow_lpf[1] = NAN;

	return 0;
}

int OpticalFlowController2::get_result(float *result_roll, float *result_pitch)		// unit: radian
{
	*result_roll = m_result[0] + m_user[0];
	*result_pitch = m_result[1] + m_user[1];

	TRACE("\rof:%.2f,%.2f", *result_roll * 180 / PI, *result_pitch * 180 / PI);

	return 0;
}
