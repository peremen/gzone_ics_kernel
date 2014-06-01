/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include "msm_actuator.h"
#include "msm_camera_eeprom.h"
#include <linux/debugfs.h>



#define MT9E013_TOTAL_STEPS_NEAR_TO_FAR_MAX		50




#define REG_VCM_NEW_CODE			0x30F2
#define REG_VCM_STEP_TIME			0x30F4
#define REG_VCM_CONTROL				0x30F0

DEFINE_MUTEX(mt9e013_act_mutex);
static int mt9e013_actuator_debug_init(void);
static struct msm_actuator_ctrl_t mt9e013_act_t;

static int32_t mt9e013_wrapper_i2c_write(struct msm_actuator_ctrl_t *a_ctrl,
	int16_t next_lens_position, void *hwparams)
{

	int32_t rc = 0;


	uint16_t mt9e013_vcm_step_time = *(uint16_t *)hwparams;
	CDBG("%s mt9e013_vcm_step_time:%d next_lens_position:%d\n",
		__func__, mt9e013_vcm_step_time, next_lens_position);



	rc = msm_camera_i2c_write(&a_ctrl->i2c_client,
				REG_VCM_STEP_TIME,
				mt9e013_vcm_step_time,
				MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0){
		pr_err("%s: X  I2C write failed \n", __func__);
		return rc;
	}
	rc = msm_camera_i2c_write(&a_ctrl->i2c_client,
				REG_VCM_NEW_CODE,
				next_lens_position,
				MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0){
		pr_err("%s: X  I2C write failed \n", __func__);
		return rc;
	}











	return next_lens_position;
}

static uint16_t g_near_vcm_step_time[] = {
	0x0023,
	0x0708,
	0x0320,
	0x017C,
	0x0096,
};
static uint16_t g_far_vcm_step_time[] = {
	0x0064,
	0x0708,
	0x017C,
	0x0096,
};
static uint16_t g_near_dir_scenario[] = {
	
	1,
	3,
	8,


	MT9E013_TOTAL_STEPS_NEAR_TO_FAR_MAX,




};

static uint16_t g_far_dir_scenario[] = {
	
	1,
	6,
	20,


	MT9E013_TOTAL_STEPS_NEAR_TO_FAR_MAX,




};

static struct region_params_t g_regions[] = {
	


	
	{
		.step_bound = {1, 0},
		.code_per_step = 30,
	},


	
	{
		.step_bound = {15, 1},
		.code_per_step = 3,
	},
	
	{
		.step_bound = {38, 15},
		.code_per_step = 3,
	},
	
	{
		.step_bound = {50, 38},
		.code_per_step = 3,
	}








};

static struct damping_params_t g_near_dir_damping_reg1[] = {
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_near_vcm_step_time[0],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_near_vcm_step_time[0],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_near_vcm_step_time[0],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_near_vcm_step_time[0],
	},

};

static struct damping_params_t g_near_dir_damping_reg2[] = {
	
	{
		.damping_step = 0xFF,
		.damping_delay = 6000,
		.hw_params = &g_near_vcm_step_time[1],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 6000,
		.hw_params = &g_near_vcm_step_time[2],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 6000,
		.hw_params = &g_near_vcm_step_time[3],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 3000,
		.hw_params = &g_near_vcm_step_time[4],
	},
};

static struct damping_params_t g_far_dir_damping_reg1[] = {
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_far_vcm_step_time[0],
	},

	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_far_vcm_step_time[0],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_far_vcm_step_time[0],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_far_vcm_step_time[0],
	},
};


static struct damping_params_t g_far_dir_damping_reg2[] = {
	
	{
		.damping_step = 0xFF,
		.damping_delay = 6000,
		.hw_params = &g_far_vcm_step_time[1],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 6000,
		.hw_params = &g_far_vcm_step_time[2],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 3000,
		.hw_params = &g_far_vcm_step_time[3],
	},
	
	{
		.damping_step = 0xFF,
		.damping_delay = 5000,
		.hw_params = &g_far_vcm_step_time[0],
	},
};

static struct damping_t g_near_dir_regions[] = {
	
	{
		.ringing_params = g_near_dir_damping_reg1,
	},
	
	{
		.ringing_params = g_near_dir_damping_reg2,
	},

	
	{
		.ringing_params = g_near_dir_damping_reg2,
	},
	
	{
		.ringing_params = g_near_dir_damping_reg2,
	},

};

static struct damping_t g_far_dir_regions[] = {
	
	{
		.ringing_params = g_far_dir_damping_reg1,
	},
	
	{
		.ringing_params = g_far_dir_damping_reg2,
	},

	
	{
		.ringing_params = g_far_dir_damping_reg2,
	},
	
	{
		.ringing_params = g_far_dir_damping_reg2,
	},

};

static int32_t mt9e013_set_params(struct msm_actuator_ctrl_t *a_ctrl)
{
	struct msm_camera_eeprom_client *calib_data =
		(struct msm_camera_eeprom_client *)a_ctrl->user_data;
	struct sensor_afcalib_data *afcalib_data =
		(struct sensor_afcalib_data *)(calib_data->data_tbl[0].data);


	g_regions[0].code_per_step = afcalib_data->af_pos_worst_inf << 8;
	g_regions[1].code_per_step = ((afcalib_data->af_pos_typ_inf-
		afcalib_data->af_pos_worst_inf) << 8) /
		(15 - 1);
	g_regions[2].code_per_step = ((afcalib_data->af_pos_typ_macro-
		afcalib_data->af_pos_typ_inf) << 8) /
		(38 - 15);
	g_regions[3].code_per_step = ((afcalib_data->af_pos_worst_macro-
		afcalib_data->af_pos_typ_macro) << 8) /
		(49 - 38);





















	return 0;
}

static int32_t mt9e013_power_down(void *data)
{
	mt9e013_act_t.func_tbl.actuator_set_default_focus(&mt9e013_act_t);
	msleep(100);
	msm_camera_i2c_write(&mt9e013_act_t.i2c_client,
				REG_VCM_CONTROL,
				0x0000,
				MSM_CAMERA_I2C_WORD_DATA);

	return 0;
}

static const struct i2c_device_id mt9e013_act_i2c_id[] = {
	{"mt9e013_act", (kernel_ulong_t)&mt9e013_act_t},
	{ }
};

static int mt9e013_act_config(
	void __user *argp)
{
	CDBG("%s called\n", __func__);
	return (int) msm_actuator_config(&mt9e013_act_t, argp);
}

static int mt9e013_i2c_add_driver_table(
	void)
{
	CDBG("%s called\n", __func__);
	return (int) msm_actuator_init_table(&mt9e013_act_t);
}

static struct i2c_driver mt9e013_act_i2c_driver = {
	.id_table = mt9e013_act_i2c_id,
	.probe  = msm_actuator_i2c_probe,
	.remove = __exit_p(mt9e013_act_i2c_remove),
	.driver = {
		.name = "mt9e013_act",
	},
};

static int __init mt9e013_i2c_add_driver(
	void)
{
	CDBG("%s called\n", __func__);
	return i2c_add_driver(mt9e013_act_t.i2c_driver);
}

static struct v4l2_subdev_core_ops mt9e013_act_subdev_core_ops;

static struct v4l2_subdev_ops mt9e013_act_subdev_ops = {
	.core = &mt9e013_act_subdev_core_ops,
};

static int32_t mt9e013_act_probe(
	void *board_info,
	void *sdev)
{
	struct msm_actuator_info *info;
	info = (struct msm_actuator_info *)board_info;
	CDBG("%s called\n", __func__);
	mt9e013_actuator_debug_init();

	mt9e013_act_t.user_data = info->eeprom_client;
	return (int) msm_actuator_create_subdevice(&mt9e013_act_t,
		info->board_info,
		(struct v4l2_subdev *)sdev);
}

static struct msm_actuator_ctrl_t mt9e013_act_t = {
	.i2c_driver = &mt9e013_act_i2c_driver,
	.i2c_addr = 0x6C,
	.act_v4l2_subdev_ops = &mt9e013_act_subdev_ops,
	.actuator_ext_ctrl = {
		.a_init_table = mt9e013_i2c_add_driver_table,
		.a_create_subdevice = mt9e013_act_probe,
		.a_config = mt9e013_act_config,
		.a_power_down = mt9e013_power_down,
	},

	.i2c_client = {
		.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
	},
	.set_info = {
		.total_steps = MT9E013_TOTAL_STEPS_NEAR_TO_FAR_MAX,
	},

	.curr_step_pos = 0,
	.curr_region_index = 0,
	.initial_code = 0,
	.actuator_mutex = &mt9e013_act_mutex,


	.get_info = {
		.focal_length_num = 397,
		.focal_length_den = 100,
		.f_number_num = 264,
		.f_number_den = 100,
		.f_pix_num = 14,
		.f_pix_den = 10,
		.total_f_dist_num = 21322,
		.total_f_dist_den = 100,
		.hor_view_angle_num = 619,
		.hor_view_angle_den = 10,
		.ver_view_angle_num = 482,
		.ver_view_angle_den = 10,
	},


	
	.ringing_scenario[MOVE_NEAR] = g_near_dir_scenario,
	.scenario_size[MOVE_NEAR] = ARRAY_SIZE(g_near_dir_scenario),
	.ringing_scenario[MOVE_FAR] = g_far_dir_scenario,
	.scenario_size[MOVE_FAR] = ARRAY_SIZE(g_far_dir_scenario),

	
	.region_params = g_regions,
	.region_size = ARRAY_SIZE(g_regions),

	
	.damping[MOVE_NEAR] = g_near_dir_regions,
	.damping[MOVE_FAR] = g_far_dir_regions,

	.func_tbl = {
		.actuator_set_params = mt9e013_set_params,
		.actuator_init_table = msm_actuator_init_table,
		.actuator_move_focus = msm_actuator_move_focus,
		.actuator_write_focus = msm_actuator_write_focus,
		.actuator_i2c_write = mt9e013_wrapper_i2c_write,
		.actuator_set_default_focus = msm_actuator_set_default_focus,
	},
};

static int mt9e013_actuator_set_hwparam(void *data, u64 val)
{
	g_near_vcm_step_time[3] = val;
	return 0;
}

static int mt9e013_actuator_get_hwparam(void *data, u64 *val)
{
	*val = g_near_vcm_step_time[3];
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mt9e013_hwdelay,
	mt9e013_actuator_get_hwparam,
	mt9e013_actuator_set_hwparam,
	"%llu\n");

static int mt9e013_actuator_set_hwparam_default(void *data, u64 val)
{
	g_near_vcm_step_time[0] = val;
	return 0;
}

static int mt9e013_actuator_get_hwparam_default(void *data, u64 *val)
{
	*val = g_near_vcm_step_time[0];
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mt9e013_defhwdelay,
	mt9e013_actuator_get_hwparam_default,
	mt9e013_actuator_set_hwparam_default,
	"%llu\n");

static int mt9e013_actuator_debug_init(void)
{
	struct dentry *debugfs_base =
		debugfs_create_dir("mt9e013_actuator", NULL);
	if (!debugfs_base)
		return -ENOMEM;

	if (!debugfs_create_file("mt9e013_hwdelay",
		S_IRUGO | S_IWUSR, debugfs_base, NULL, &mt9e013_hwdelay))
		return -ENOMEM;

	if (!debugfs_create_file("mt9e013_defhwdelay",
		S_IRUGO | S_IWUSR, debugfs_base, NULL, &mt9e013_defhwdelay))
		return -ENOMEM;

	return 0;
}

subsys_initcall(mt9e013_i2c_add_driver);
MODULE_DESCRIPTION("MT9E013 actuator");
MODULE_LICENSE("GPL v2");
