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
	float top_motor,left_motor,right_motor;

	const float TOP_MOTOR_RAD = 0;
	const float LEFT_MOTOR_RAD = 2*PI/3;
	const float RIGHT_MOTOR_RAD = -2*PI/3;
	uint32_t target_x,target_y,target_r;
	int max_count;
	int middle_count;
	int calc_max_count;

	void calc_translation(float y, float x);
	void calc_rotation(float r);

public:
	OmniOperator();
	void set_limit(int percent);
	/**
	 *param range : int. 1回の呼び出しあたりのモータの最大回転数を指定．
	 */
	void init(float range);
	void calc_movement_value(float x, float y, float r);
	float get_top_motor_value(){
		return top_motor;
	}
	float get_left_motor_value(){
		return left_motor;

	}
	float get_right_motor_value(){
		return right_motor;
	}
};

#endif /* OMNIOPERATOR_HPP_ */
