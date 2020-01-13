/*
 * OmniOperator.hpp
 *
 *  Created on: Jul 18, 2019
 *      Author: mfukunaga
 */

#ifndef OMNIOPERATOR_HPP_
#define OMNIOPERATOR_HPP_

#include <cstdint>
#include <Arduino.h>

class OmniOperator{

private:
	float top_motor_temp,left_motor_temp,right_motor_temp;

	const float TOP_MOTOR_RAD = 0;
	const float LEFT_MOTOR_RAD = 2*PI/3;
	const float RIGHT_MOTOR_RAD = -2*PI/3;
	uint32_t target_x,target_y,target_r;
	int max_count;
	int middle_count;
	int calc_max_count;

public:
	OmniOperator();
	void set_limit(int percent);
	/**
	 *param range : int. 1回の呼び出しあたりのモータの最大回転数を指定．
	 */
	void init(int range);
	// void move(float x, float y, float r);
	void get_targetValue();

	void calc_translation(float y, float x);
	void calc_rotation(float r);
};

#endif /* OMNIOPERATOR_HPP_ */
