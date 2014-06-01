/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#include "mt9d115_sub_v4l2.h"































static int32_t mt9d115_sub_sensor_reg_polling(void)
{
	int32_t rc = 0;
	int cnt;
	uint16_t SEQ_STATE = 0xFFFF;

	
	rc = msm_camera_i2c_write(mt9d115_sub_s_ctrl.sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQRFM_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
    if(rc < 0){
        MT9D115_SUB_LOG_ERR("ERROR: msm_camera_i2c_write() rc = %d\n",rc);
        return rc;
    }

	rc = msm_camera_i2c_read(mt9d115_sub_s_ctrl.sensor_i2c_client, REG_MT9D115_MCUDAT, &SEQ_STATE, MSM_CAMERA_I2C_WORD_DATA );
    if(rc < 0){
        MT9D115_SUB_LOG_ERR("ERROR: msm_camera_i2c_read() rc = %d\n",rc);
        return rc;
    }

	MT9D115_SUB_LOG_DBG("SEQ_STATE = 0x%x\n", SEQ_STATE);
	cnt = 0;
	while(SEQ_STATE != MT9D115_SEQ_STATE_MASK2){
		mdelay(10);
		rc = msm_camera_i2c_write(mt9d115_sub_s_ctrl.sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQRFM_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
        if(rc < 0){
            MT9D115_SUB_LOG_ERR("ERROR: msm_camera_i2c_write() rc = %d\n",rc);
            return rc;
        }

		rc = msm_camera_i2c_read(mt9d115_sub_s_ctrl.sensor_i2c_client, REG_MT9D115_MCUDAT, &SEQ_STATE, MSM_CAMERA_I2C_WORD_DATA );
        if(rc < 0){
            MT9D115_SUB_LOG_ERR("ERROR: msm_camera_i2c_read() rc = %d\n",rc);
            return rc;
        }

		MT9D115_SUB_LOG_DBG("SEQ_STATE = 0x%x,cnt:%d\n", SEQ_STATE, cnt);
		cnt++;
		if(cnt ==100){
            MT9D115_SUB_LOG_ERR("ERROR: reg_polling timeout cnt = %d\n",cnt);
            return -EFAULT;
        }
	}

	return rc;
}


static void mt9d115_sub_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{

	;
}

static void mt9d115_sub_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{

	;
}

static int32_t mt9d115_sub_sensor_write_output_settings(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t res)
{






















	return 0;

}

static int32_t mt9d115_sub_sensor_write_res_settings(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t res)
{
	int32_t rc;
	
	MT9D115_SUB_LOG_DBG("mt9d115_sub_sensor_write_res_settings called res= %d\n",res);	

	rc = msm_sensor_write_conf_array(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->mode_settings, res);
	if (rc < 0){
		MT9D115_SUB_LOG_ERR("ERROR: msm_sensor_write_conf_array() rc = %d\n",rc);
		return rc;
	}

	rc = mt9d115_sub_sensor_write_output_settings(s_ctrl, res);
	return rc;
}

















































































































































static long mt9d115_sub_sensor_init_setting(struct msm_sensor_ctrl_t *s_ctrl)
{
	long rc;
	uint16_t sysctl = 0;
	MT9D115_SUB_LOG_DBG("mt9d115_sub_sensor_init_setting called\n");

	
	
	
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, REG_MT9D115_SYSCTL, &sysctl, MSM_CAMERA_I2C_WORD_DATA );
	if (rc < 0)
		return rc;
	sysctl = (sysctl & MT9D115_SYSCTL_DP_MASK);
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_SYSCTL, sysctl, MSM_CAMERA_I2C_WORD_DATA );
	if (rc < 0)
		return rc;

	
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, REG_MT9D115_SYSCTL, &sysctl, MSM_CAMERA_I2C_WORD_DATA );
	if (rc < 0)
		return rc;
	sysctl = (sysctl | MT9D115_SYSCTL_MP_MASK);
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_SYSCTL, sysctl, MSM_CAMERA_I2C_WORD_DATA );
	if (rc < 0)
		return rc;

	
	
	rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings, MT9D115_REG_INIT_00);
	if (rc < 0)
		return rc;

	
	msleep(1);

	
	rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings, MT9D115_REG_INIT_01);
	if (rc < 0)
		return rc;

	
	msleep(50);

	return 0;
}

static long mt9d115_sub_reg_setting(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t ret = 0;
	int32_t rc = 0;
	int cnt;
	uint16_t MON_PATCH_ID_0 = 0;
	uint16_t SEQ_STATE = 0;
	uint16_t PGA_ENABLE = 0;

	MT9D115_SUB_LOG_DBG("mt9d115_sub_reg_setting called\n");

	
	ret = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings, MT9D115_REG_INIT_02);
	if (ret < 0)
		return -EFAULT;

	
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_MONPAT_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
    if(rc < 0)
        return rc;

	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,  REG_MT9D115_MCUDAT, &MON_PATCH_ID_0, MSM_CAMERA_I2C_WORD_DATA );
    if(rc < 0)
        return rc;

	MT9D115_SUB_LOG_DBG("MON_PATCH_ID_0 = 0x%x\n", MON_PATCH_ID_0);
	cnt = 0;
	while(MON_PATCH_ID_0 == MT9D115_MONPAT_ID_MASK){
		MT9D115_SUB_LOG_DBG("wait 10 \n");
		mdelay(10);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_MONPAT_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
        if(rc < 0)
            return rc;

		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,  REG_MT9D115_MCUDAT, &MON_PATCH_ID_0, MSM_CAMERA_I2C_WORD_DATA );
        if(rc < 0)
            return rc;

		MT9D115_SUB_LOG_DBG("MON_PATCH_ID_0 = 0x%x,cnt:%d\n", MON_PATCH_ID_0, cnt);
		cnt++;
		if(cnt == 100)return -EFAULT;
	}

	
	ret = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings, MT9D115_REG_INIT_03);
	if (ret < 0)
		return -EFAULT;

	
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,  REG_MT9D115_PGA_ENABLE, &PGA_ENABLE, MSM_CAMERA_I2C_WORD_DATA );
	if (rc < 0)
		return rc;
	PGA_ENABLE = (PGA_ENABLE | MT9D115_PGA_ENABLE_MASK);
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_PGA_ENABLE, PGA_ENABLE, MSM_CAMERA_I2C_WORD_DATA );
	if (rc < 0)
		return rc;

	
	ret = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings, MT9D115_REG_INIT_04);
	if (ret < 0)
		return -EFAULT;

	
	ret = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings, MT9D115_REG_INIT_05);
	if (ret < 0)
		return -EFAULT;

	
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQSAT_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
    if(rc < 0)
        return rc;

	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,  REG_MT9D115_MCUDAT, &SEQ_STATE, MSM_CAMERA_I2C_WORD_DATA );
    if(rc < 0)
        return rc;

	MT9D115_SUB_LOG_DBG("SEQ_STATE = 0x%x\n", SEQ_STATE);
	cnt = 0;
	while(SEQ_STATE != MT9D115_SEQ_STATE_MASK){
		mdelay(10);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQSAT_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
        if(rc < 0)
            return rc;

		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,  REG_MT9D115_MCUDAT, &SEQ_STATE, MSM_CAMERA_I2C_WORD_DATA );
        if(rc < 0)
            return rc;

		MT9D115_SUB_LOG_DBG("SEQ_STATE = 0x%x,cnt:%d\n", SEQ_STATE, cnt);
		cnt++;
		if(cnt == 100)return -EFAULT;
	}

	
	ret = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->init_settings, MT9D115_REG_INIT_06);
	if (ret < 0)
		return -EFAULT;

	rc = mt9d115_sub_sensor_reg_polling();
    if(rc < 0)
        return rc;

	
	
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQRFM_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
	if(rc < 0)
		return rc;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUDAT, MT9D115_REFSEQ_DAT, MSM_CAMERA_I2C_WORD_DATA );
	if(rc < 0)
		return rc;

	rc = mt9d115_sub_sensor_reg_polling();
    if(rc < 0)
        return rc;

	return 0;
}

static int32_t mt9d115_sub_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
                                      int update_type, int res)
{
    int32_t rc = 0;

	MT9D115_SUB_LOG_DBG("mt9d115_sub_sensor_setting called\n");

    v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
        NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
        PIX_0, ISPIF_OFF_IMMEDIATELY));

    msleep(30);

    if (update_type == MSM_SENSOR_REG_INIT) {

		MT9D115_SUB_LOG_DBG("MSM_SENSOR_REG_INIT\n");

        s_ctrl->curr_csi_params = s_ctrl->csi_params[res];

        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
        NOTIFY_CSID_CFG,
            &s_ctrl->curr_csi_params->csid_params);
        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                    NOTIFY_CID_CHANGE, NULL);
        mb();
        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_CSIPHY_CFG,
            &s_ctrl->curr_csi_params->csiphy_params);
        mb();
        msleep(20);


		rc = mt9d115_sub_sensor_init_setting(s_ctrl);
		if(rc < 0){
			MT9D115_SUB_LOG_ERR("mt9d115_sub_sensor_init_setting err rc:%d\n", rc);
			return -EINVAL;
		}

		rc = mt9d115_sub_reg_setting(s_ctrl);
		if(rc < 0){
			MT9D115_SUB_LOG_ERR("mt9d115_sub_reg_setting err rc:%d\n", rc);
			return -EINVAL;
		}

    } else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {

		MT9D115_SUB_LOG_DBG("MSM_SENSOR_UPDATE_PERIODIC\n");
		MT9D115_SUB_LOG_DBG("res = %d\n",res);

		rc = mt9d115_sub_sensor_write_res_settings(s_ctrl, res);
        if(rc < 0)
            return rc;

        if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {

			MT9D115_SUB_LOG_DBG("curr_csi_params != csi_params[%d]\n",res);

            s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                NOTIFY_CSID_CFG,
                &s_ctrl->curr_csi_params->csid_params);
            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                        NOTIFY_CID_CHANGE, NULL);
            mb();
            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                NOTIFY_CSIPHY_CFG,
                &s_ctrl->curr_csi_params->csiphy_params);
            mb();
            msleep(20);
        }

        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
            output_settings[res].op_pixel_clk);
        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
            PIX_0, ISPIF_ON_FRAME_BOUNDARY));
        s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
        msleep(30);

    }

    return rc;
}

static int32_t mt9d115_sub_set_pict_size(struct msm_sensor_ctrl_t *s_ctrl, int pict_size)
{
	int32_t rc = 0;
	enum mt9d115_SIZE_setting size_setting = MT9D115_SIZE_MAX;
	int i;
	uint32_t record_sizes_width[]  = { 1424, 800, 400, 224 };
	uint32_t record_sizes_height[] = {  800, 600, 300, 180 };
	uint32_t width = 0;
	uint32_t height = 0;

	MT9D115_SUB_LOG_DBG("start pict_size = %d\n",pict_size);

	switch(pict_size){
	case 94: 
		MT9D115_SUB_LOG_DBG("SENSOR_CAMERA_QCIF_SIZE\n");
		size_setting = MT9D115_SIZE_05;	
		break;

	case 95: 
		MT9D115_SUB_LOG_DBG("SENSOR_CAMERA_QVGA_SIZE\n");
		size_setting = MT9D115_SIZE_04;	
		break;

	case 96: 

		MT9D115_SUB_LOG_DBG("SENSOR_CAMERA_VGA_SIZE\n");
		size_setting = MT9D115_SIZE_03;	
		break;













	case 97: 
		MT9D115_SUB_LOG_DBG("SENSOR_CAMERA_HD_SIZE\n");
		size_setting = MT9D115_SIZE_06;	
		break;

	case 98: 
		MT9D115_SUB_LOG_DBG("SENSOR_CAMERA_2MP_SIZE\n");
		size_setting = MT9D115_SIZE_01;	
		break;

	case 99: 
		MT9D115_SUB_LOG_DBG("SENSOR_CAMERA_WXGA_SIZE\n");
		size_setting = MT9D115_SIZE_07;	
		break;

	case 93:
		for (i = 0; i < 4; ++i) {
			if (width == record_sizes_width[i]
			   && height == record_sizes_height[i]) {
				switch (i)
				{
					case 0:	 
						size_setting = MT9D115_SIZE_08;	
						break;

					case 1:	 
						size_setting = MT9D115_SIZE_09;	
						break;

					case 2:	 
						size_setting = MT9D115_SIZE_10;	
						break;

					case 3:	 
						size_setting = MT9D115_SIZE_11;	
						break;

					default:
						MT9D115_SUB_LOG_ERR("set pict_size unsuported :%d\n", pict_size);
						return -EINVAL;
				}
			}
		}
		break;

	default: {
		MT9D115_SUB_LOG_ERR("set pict_size unsuported :%d\n", pict_size);
		return -EINVAL;
		}
	}

	if( size_setting != MT9D115_SIZE_MAX ) {




		if(rc < 0){
			MT9D115_SUB_LOG_ERR("msm_sensor_write_conf_array err pict_size:%d rc:%d\n", pict_size, rc);
			return -EINVAL;
		}

		
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQRFM_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return -EINVAL;
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUDAT, MT9D115_REFMOD_DAT, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return -EINVAL;

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;

		
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQRFM_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return -EINVAL;
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUDAT, MT9D115_REFSEQ_DAT, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return -EINVAL;

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;










































	}

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
	return rc;

}

static int32_t mt9d115_sub_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl, int antibanding)
{
	int32_t rc = 0;
	enum mt9d115_Flicker setting;

	MT9D115_SUB_LOG_DBG("start antibanding = %d\n",antibanding);

	switch (antibanding) {
	case CAMERA_ANTIBANDING_60HZ:
		MT9D115_SUB_LOG_DBG("CAMERA_ANTIBANDING_60HZ\n");
		setting = MT9D115_FL_00;
		break;

	case CAMERA_ANTIBANDING_50HZ:
		MT9D115_SUB_LOG_DBG("CAMERA_ANTIBANDING_50HZ\n");
		setting = MT9D115_FL_01;
		break;

	case CAMERA_ANTIBANDING_AUTO:
		MT9D115_SUB_LOG_DBG("CAMERA_ANTIBANDING_AUTO\n");
		setting = MT9D115_FL_02;
		break;

	case CAMERA_ANTIBANDING_OFF:
	default: {


		MT9D115_SUB_LOG_DBG("set param unsuported :0x%x\n", antibanding);
		setting = mt9d115_sub_flicker_mode;
		break;
		}
	}

	if(mt9d115_sub_flicker_mode != setting){
		rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
										mt9d115_Flicker_settings, setting);
		if (rc < 0)
			return rc;

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;

		
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQRFM_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return -EINVAL;
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUDAT, MT9D115_REFSEQ_DAT, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return -EINVAL;

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;

		mt9d115_sub_flicker_mode = setting;
	}

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
	return rc;
}

static int32_t mt9d115_sub_set_wb(struct msm_sensor_ctrl_t *s_ctrl, int wb_val)
{
	int32_t rc = 0;
	enum mt9d115_White_Balance setting;

	MT9D115_SUB_LOG_DBG("start wb_val = %d\n",wb_val);

    switch (wb_val)
    {
        case CAMERA_WB_AUTO:
			MT9D115_SUB_LOG_DBG("CAMERA_WB_AUTO\n");
			setting = MT9D115_WB_00;
            break;

        case CAMERA_WB_DAYLIGHT:
			MT9D115_SUB_LOG_DBG("CAMERA_WB_DAYLIGHT\n");
			setting = MT9D115_WB_04;
            break;

        case CAMERA_WB_CLOUDY_DAYLIGHT:
			MT9D115_SUB_LOG_DBG("CAMERA_WB_CLOUDY_DAYLIGHT\n");
			setting = MT9D115_WB_05;
            break;

        case CAMERA_WB_INCANDESCENT:
			MT9D115_SUB_LOG_DBG("CAMERA_WB_INCANDESCENT\n");
			setting = MT9D115_WB_01;
            break;

        case CAMERA_WB_FLUORESCENT:
        case CAMERA_WB_FLUORESCENT_H:
			MT9D115_SUB_LOG_DBG("CAMERA_WB_FLUORESCENT/CAMERA_WB_FLUORESCENT_H\n");
			setting = MT9D115_WB_03;
            break;

        case CAMERA_WB_FLUORESCENT_L:
			MT9D115_SUB_LOG_DBG("CAMERA_WB_FLUORESCENT_L\n");
			setting = MT9D115_WB_02;
            break;

        default:
			MT9D115_SUB_LOG_ERR("set param unsuported :0x%x\n", wb_val);
			setting = mt9d115_sub_wb_mode;
            break;
    }

	if(mt9d115_sub_wb_mode != setting){
		rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
										mt9d115_White_Balance_settings, setting);
        if(rc < 0)
            return rc;

		mt9d115_sub_wb_mode = setting;
	}

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}

static int32_t mt9d115_sub_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int effect)
{
	enum mt9d115_Special_Effects setting;
	int32_t rc = 0;

	MT9D115_SUB_LOG_DBG("start effect = %d\n",effect);

	switch (effect) {
	case CAMERA_EFFECT_OFF:
		MT9D115_SUB_LOG_DBG("CAMERA_EFFECT_OFF\n");
		setting = MT9D115_SE_00;
		break;

	case CAMERA_EFFECT_MONO:
		MT9D115_SUB_LOG_DBG("CAMERA_EFFECT_MONO\n");
		setting = MT9D115_SE_01;
		break;

	case CAMERA_EFFECT_SEPIA:
		MT9D115_SUB_LOG_DBG("CAMERA_EFFECT_SEPIA\n");
		setting = MT9D115_SE_02;
		break;







	default: {
		MT9D115_SUB_LOG_ERR("set param unsuported :0x%x\n", effect);
		setting = mt9d115_sub_effect;
		break;
		}
	}

	if(mt9d115_sub_effect != setting){
		rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
										mt9d115_Special_Effects_settings, setting);
		if (rc < 0){
			return rc;
		}

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;

		mt9d115_sub_effect = setting;
	}

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
	return rc;
}

static int32_t mt9d115_sub_set_exp_compensation(struct msm_sensor_ctrl_t *s_ctrl, int brightness)
{
	int32_t rc = 0;
	enum mt9d115_brightness setting;

	MT9D115_SUB_LOG_DBG("start brightness = %d\n",brightness);

	switch (brightness) {
	case CAMERA_BRIGHTNESS_LV0:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV0\n");
		setting = MT9D115_BN_00;	
		break;

	case CAMERA_BRIGHTNESS_LV1:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV1\n");
		setting = MT9D115_BN_01;	
		break;

	case CAMERA_BRIGHTNESS_LV2:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV2\n");
		setting = MT9D115_BN_02;	
		break;

	case CAMERA_BRIGHTNESS_LV3:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV3\n");
		setting = MT9D115_BN_03;	
		break;

	case CAMERA_BRIGHTNESS_LV4:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV4\n");
		setting = MT9D115_BN_04;	
		break;

	case CAMERA_BRIGHTNESS_LV5:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV5\n");
		setting = MT9D115_BN_05;	
		break;

	case CAMERA_BRIGHTNESS_LV6:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV6\n");
		setting = MT9D115_BN_06;	
		break;

	case CAMERA_BRIGHTNESS_LV7:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV7\n");
		setting = MT9D115_BN_07;	
		break;

	case CAMERA_BRIGHTNESS_LV8:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV8\n");
		setting = MT9D115_BN_08;	
		break;

	case CAMERA_BRIGHTNESS_LV9:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV9\n");
		setting = MT9D115_BN_09;	
		break;

	case CAMERA_BRIGHTNESS_LV10:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV10\n");
		setting = MT9D115_BN_10;	
		break;

	case CAMERA_BRIGHTNESS_LV11:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV11\n");
		setting = MT9D115_BN_11;	
		break;

	case CAMERA_BRIGHTNESS_LV12:
		MT9D115_SUB_LOG_DBG("CAMERA_BRIGHTNESS_LV12\n");
		setting = MT9D115_BN_12;	
		break;

	default:
		MT9D115_SUB_LOG_ERR("set param unsuported :0x%x\n", brightness);
		setting = mt9d115_sub_bright_mode;
		break;
	}

	if(mt9d115_sub_bright_mode != setting){
		rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
										mt9d115_brightness_settings, setting);
        if(rc < 0)
            return rc;

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;

		mt9d115_sub_bright_mode = setting;
	}

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
	return rc;
}

static int32_t mt9d115_sub_set_scene(struct msm_sensor_ctrl_t *s_ctrl, int scene)
{
	int32_t rc = 0;
	enum mt9d115_Scene_setting setting;

	MT9D115_SUB_LOG_DBG("start scene = %d\n",scene);

    switch (scene)
    {
        case CAMERA_SCENE_MODE_AUTO :
			MT9D115_SUB_LOG_DBG("Scene auto\n");
			setting = MT9D115_SN_00;
            break;

        case CAMERA_SCENE_MODE_PORTRAIT :
			MT9D115_SUB_LOG_DBG("Scene portrait\n");
			setting = MT9D115_SN_01;
            break;

        case CAMERA_SCENE_MODE_PORTRAIT_ILLUMI  :
			MT9D115_SUB_LOG_DBG("Scene portrait_and_illumination\n");
			setting = MT9D115_SN_02;
            break;

        case CAMERA_SCENE_MODE_OFF  :
			MT9D115_SUB_LOG_DBG("Scene off\n");
			setting = MT9D115_SN_03;
            break;

        default:
			MT9D115_SUB_LOG_ERR("mt9d115_sub_set_scene default scene=%d\n",scene);
			return 0;
    }








	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}












































































































































































































static int32_t mt9d115_sub_get_maker_note(struct msm_sensor_ctrl_t *s_ctrl,
											struct get_exif_maker_note_cfg *get_exif_maker_note)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t device_id =0;            
    uint16_t fd_freq =0;              
    uint16_t awb_temp =0;             
    uint16_t awb_gain_r =0;           
    uint16_t awb_gain_g =0;           
    uint16_t awb_gain_b =0;           
    uint16_t awb_saturation =0;       

    struct reg_access_param_t reg_access_params[]={
        {MT9D115_ACCESS_VARIABLES, REG_MT9D115_FD_FREQ,    &fd_freq       },
        {MT9D115_ACCESS_VARIABLES, REG_MT9D115_AWB_TEMP,   &awb_temp      },
        {MT9D115_ACCESS_VARIABLES, REG_MT9D115_AWB_GAIN_R, &awb_gain_r    },
        {MT9D115_ACCESS_VARIABLES, REG_MT9D115_AWB_GAIN_G, &awb_gain_g    },
        {MT9D115_ACCESS_VARIABLES, REG_MT9D115_AWB_GAIN_B, &awb_gain_b    },
        {MT9D115_ACCESS_VARIABLES, REG_MT9D115_SATURATION, &awb_saturation},
    };

	MT9D115_SUB_LOG_DBG("start\n");

    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
    {
        if(reg_access_params[cnt].reg_type == MT9D115_ACCESS_VARIABLES){
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                      REG_MT9D115_MCUADD, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     REG_MT9D115_MCUDAT, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { 
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     reg_access_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }
        
        *reg_access_params[cnt].reg_data = val;
    }

    if( fd_freq & (1<<7) ){
        
        fd_freq = 0x00;
    } else{
        
        fd_freq = (fd_freq>>5)&1 ? 0x02 : 0x03;
    }

    device_id = 0x2580;     

    get_exif_maker_note->device_id      = device_id;    
    get_exif_maker_note->fd_freq        = fd_freq;
    get_exif_maker_note->awb_temp       = awb_temp;
    get_exif_maker_note->awb_gain_r     = awb_gain_r;
    get_exif_maker_note->awb_gain_g     = awb_gain_g;
    get_exif_maker_note->awb_gain_b     = awb_gain_b;
    get_exif_maker_note->awb_saturation = awb_saturation;

    MT9D115_SUB_LOG_DBG("device_id      = 0x%04X",get_exif_maker_note->device_id     );
    MT9D115_SUB_LOG_DBG("fd_freq        = 0x%04X",get_exif_maker_note->fd_freq       );
    MT9D115_SUB_LOG_DBG("awb_temp       = 0x%04X",get_exif_maker_note->awb_temp      );
    MT9D115_SUB_LOG_DBG("awb_gain_r     = 0x%04X",get_exif_maker_note->awb_gain_r    );
    MT9D115_SUB_LOG_DBG("awb_gain_g     = 0x%04X",get_exif_maker_note->awb_gain_g    );
    MT9D115_SUB_LOG_DBG("awb_gain_b     = 0x%04X",get_exif_maker_note->awb_gain_b    );
    MT9D115_SUB_LOG_DBG("awb_saturation = 0x%04X",get_exif_maker_note->awb_saturation);

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}

static int32_t mt9d115_sub_get_exif_param(struct msm_sensor_ctrl_t *s_ctrl,
											struct get_exif_param_inf* get_exif_param)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t coarse_integration_time;      
    uint16_t line_length_DVE047;              
    uint16_t fine_integration_time;        
    uint16_t analog_gain_code_global;      
    uint16_t digital_gain_greenr;          

    struct reg_access_param_t reg_access_params[]={
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_COAINT_TIME,        &coarse_integration_time},
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_LINE_LENGTH_DVE905,    &line_length_DVE047        },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_FI_INT_TIME,        &fine_integration_time  },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_GLOBAL, &analog_gain_code_global},
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_D_GAIN_GREENR,      &digital_gain_greenr    },
    };

	MT9D115_SUB_LOG_DBG("start\n");

    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
    {
        if(reg_access_params[cnt].reg_type == MT9D115_ACCESS_VARIABLES){
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                      REG_MT9D115_MCUADD, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     REG_MT9D115_MCUDAT, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { 
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     reg_access_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }
        
        *reg_access_params[cnt].reg_data = val;
    }

    get_exif_param->coarse_integration_time = coarse_integration_time;
    get_exif_param->line_length_DVE047         = line_length_DVE047;
    get_exif_param->fine_integration_time   = fine_integration_time;
    get_exif_param->analog_gain_code_global = analog_gain_code_global;
    get_exif_param->digital_gain_greenr     = digital_gain_greenr;

    MT9D115_SUB_LOG_DBG("coarse_integration_time = 0x%04X",get_exif_param->coarse_integration_time);
    MT9D115_SUB_LOG_DBG("line_length_DVE047         = 0x%04X",get_exif_param->line_length_DVE047        );
    MT9D115_SUB_LOG_DBG("fine_integration_time   = 0x%04X",get_exif_param->fine_integration_time  );
    MT9D115_SUB_LOG_DBG("analog_gain_code_global = 0x%04X",get_exif_param->analog_gain_code_global);
    MT9D115_SUB_LOG_DBG("digital_gain_greenr     = 0x%04X",get_exif_param->digital_gain_greenr    );

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}

static int32_t mt9d115_sub_set_frame_rate_mode(struct msm_sensor_ctrl_t *s_ctrl, int frame_rate_mode)
{
	int32_t rc = 0;
	enum mt9d115_Frame_Rate_setting setting;
	enum mt9d115_Capture_Mode_setting setting2;

	MT9D115_SUB_LOG_DBG("start frame_rate_mode = %d\n",frame_rate_mode);

    switch (frame_rate_mode)
    {
		
		case CAMERA_FPS_MODE_QUICK_SCENE_OFF_SHAKE_AUTO:
		case CAMERA_FPS_MODE_QUICK_SCENE_OFF_SHAKE_OFF:
		case CAMERA_FPS_MODE_STD_SCENE_AUTO_SHAKE_AUTO:
		case CAMERA_FPS_MODE_STD_SCENE_OFF_SHAKE_AUTO:
		case CAMERA_FPS_MODE_HDR_SCENE_OFF_SHAKE_OFF:
		case CAMERA_FPS_MODE_MOVIE_SCENE_OFF:
			MT9D115_SUB_LOG_DBG("frame_rate_mode 15fixed, auto\n");
			setting = MT9D115_FR_02;
			setting2 = MT9D115_CM_00;
            break;

		
		case CAMERA_FPS_MODE_STD_SCENE_PORT_SHAKE_AUTO:
			MT9D115_SUB_LOG_DBG("frame_rate_mode 15fixed, char_illum\n");
			setting = MT9D115_FR_02;
			setting2 = MT9D115_CM_01;
            break;

        
		case CAMERA_FPS_MODE_STD_SCENE_AUTO_SHAKE_OFF:
		case CAMERA_FPS_MODE_STD_SCENE_OFF_SHAKE_OFF:
		case CAMERA_FPS_MODE_BEST_SCENE_AUTO_SHAKE_OFF:
		case CAMERA_FPS_MODE_BEST_SCENE_OFF_SHAKE_OFF:
			MT9D115_SUB_LOG_DBG("frame_rate_mode 7.5-15fps, auto\n");
			setting = MT9D115_FR_00;
			setting2 = MT9D115_CM_00;
            break;

        
		case CAMERA_FPS_MODE_STD_SCENE_PORT_SHAKE_OFF:
		case CAMERA_FPS_MODE_BEST_SCENE_PORT_SHAKE_OFF:
			MT9D115_SUB_LOG_DBG("frame_rate_mode 7.5-15fps, char_illum\n");
			setting = MT9D115_FR_00;
			setting2 = MT9D115_CM_01;
            break;

        
		case CAMERA_FPS_MODE_STD_SCENE_PORTILLUMI_SHAKE_OFF:
			MT9D115_SUB_LOG_DBG("frame_rate_mode 5-15fps, char_illum\n");
			setting = MT9D115_FR_01;
			setting2 = MT9D115_CM_01;
            break;

        
		case CAMERA_FPS_MODE_STD_SCENE_PORTILLUMI_SHAKE_AUTO:
			MT9D115_SUB_LOG_DBG("frame_rate_mode 10-15fps, char_illum\n");
			setting = MT9D115_FR_03;
			setting2 = MT9D115_CM_01;
            break;

        default:
			MT9D115_SUB_LOG_ERR("set param unsuported frame_rate_mode=%d\n",frame_rate_mode);
			setting = mt9d115_sub_frame_rate_mode;
			setting2 = mt9d115_sub_capture_mode;
            break;
    }

	if(mt9d115_sub_frame_rate_mode != setting){
		rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
										mt9d115_Frame_Rate_settings, setting);
		if(rc < 0)
			return -EINVAL;

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;

		
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUADD, MT9D115_SEQRFM_ADDRESS, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return rc;
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_MT9D115_MCUDAT, MT9D115_REFSEQ_DAT, MSM_CAMERA_I2C_WORD_DATA );
		if(rc < 0)
			return rc;

		rc = mt9d115_sub_sensor_reg_polling();
        if(rc < 0)
            return rc;

		mt9d115_sub_frame_rate_mode = setting;
	}

	if(mt9d115_sub_capture_mode != setting2){
		rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
										mt9d115_Capture_Mode_settings, setting2);
		if(rc < 0)
			return -EINVAL;

		rc = mt9d115_sub_sensor_reg_polling();
		if(rc < 0)
			return rc;

		mt9d115_sub_capture_mode = setting2;
	}

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}

static int32_t mt9d115_sub_get_hdr_brightness(struct msm_sensor_ctrl_t *s_ctrl,
											struct hdr_brightness_t *hdr_brightness)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t coarse_integration_time;      
    uint16_t line_length_DVE047;              
    uint16_t fine_integration_time;        
    uint16_t analogue_gain_code_greenr;    
    uint16_t analogue_gain_code_greenb;    
    uint16_t analogue_gain_code_red;       
    uint16_t analogue_gain_code_blue;      

    struct reg_access_param_t reg_access_params[]={
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_COAINT_TIME,        &coarse_integration_time  },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_LINE_LENGTH_DVE905,    &line_length_DVE047          },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_FI_INT_TIME,        &fine_integration_time    },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_GREENR, &analogue_gain_code_greenr},
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_GREENB, &analogue_gain_code_greenb},
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_RED,    &analogue_gain_code_red   },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_BLUE,   &analogue_gain_code_blue  },
    };

	MT9D115_SUB_LOG_DBG("start\n");

    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
    {
        if(reg_access_params[cnt].reg_type == MT9D115_ACCESS_VARIABLES){
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                      REG_MT9D115_MCUADD, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     REG_MT9D115_MCUDAT, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { 
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     reg_access_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }
        
        *reg_access_params[cnt].reg_data = val;
    }

    hdr_brightness->coarse_integration_time   = coarse_integration_time;
    hdr_brightness->line_length_DVE047           = line_length_DVE047;
    hdr_brightness->fine_integration_time     = fine_integration_time;
    hdr_brightness->analogue_gain_code_greenr = analogue_gain_code_greenr;
    hdr_brightness->analogue_gain_code_greenb = analogue_gain_code_greenb;
    hdr_brightness->analogue_gain_code_red    = analogue_gain_code_red;
    hdr_brightness->analogue_gain_code_blue   = analogue_gain_code_blue;

    MT9D115_SUB_LOG_DBG("coarse_integration_time   = 0x%04X",hdr_brightness->coarse_integration_time  );
    MT9D115_SUB_LOG_DBG("line_length_DVE047           = 0x%04X",hdr_brightness->line_length_DVE047          );
    MT9D115_SUB_LOG_DBG("fine_integration_time     = 0x%04X",hdr_brightness->fine_integration_time    );
    MT9D115_SUB_LOG_DBG("analogue_gain_code_greenr = 0x%04X",hdr_brightness->analogue_gain_code_greenr);
    MT9D115_SUB_LOG_DBG("analogue_gain_code_greenb = 0x%04X",hdr_brightness->analogue_gain_code_greenb);
    MT9D115_SUB_LOG_DBG("analogue_gain_code_red    = 0x%04X",hdr_brightness->analogue_gain_code_red   );
    MT9D115_SUB_LOG_DBG("analogue_gain_code_blue   = 0x%04X",hdr_brightness->analogue_gain_code_blue  );

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}

static int32_t mt9d115_sub_set_hdr_brightness(struct msm_sensor_ctrl_t *s_ctrl,
											struct hdr_brightness_t hdr_brightness)
{
    int32_t rc = 0;
    int cnt = 0;

    uint16_t grp_parm_hold_mask_corrupted_frm_on = 0x0101;                             
    uint16_t coarse_integration_time   = hdr_brightness.coarse_integration_time;      
    uint16_t analogue_gain_code_greenr = hdr_brightness.analogue_gain_code_greenr;    
    uint16_t analogue_gain_code_greenb = hdr_brightness.analogue_gain_code_greenb;    
    uint16_t analogue_gain_code_red    = hdr_brightness.analogue_gain_code_red;       
    uint16_t analogue_gain_code_blue   = hdr_brightness.analogue_gain_code_blue;      
    uint16_t grp_parm_hold_mask_corrupted_frm_off = 0x0001;                            

    struct reg_access_param_t reg_access_params[]={
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_PARM_HOLD_MASK_FRM, &grp_parm_hold_mask_corrupted_frm_on },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_COAINT_TIME,        &coarse_integration_time             },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_GREENR, &analogue_gain_code_greenr           },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_GREENB, &analogue_gain_code_greenb           },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_RED,    &analogue_gain_code_red              },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_A_GAIN_CODE_BLUE,   &analogue_gain_code_blue             },
        {MT9D115_ACCESS_REGISTERS, REG_MT9D115_PARM_HOLD_MASK_FRM, &grp_parm_hold_mask_corrupted_frm_off},
    };

	MT9D115_SUB_LOG_DBG("start set_pattern=%d\n",set_pattern);

	if( (hdr_brightness.set_pattern == CAMERA_HDR_BRIGHTNESS_M2)||
	    (hdr_brightness.set_pattern == CAMERA_HDR_BRIGHTNESS_P2)  )
	{
	    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
	    {
	        if(reg_access_params[cnt].reg_type == MT9D115_ACCESS_VARIABLES){
	        
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
	                                      REG_MT9D115_MCUADD, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
	            if (rc < 0) {
	                MT9D115_SUB_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
	                return rc;
	            }
	        
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
	                                     REG_MT9D115_MCUDAT, *(reg_access_params[cnt].reg_data), MSM_CAMERA_I2C_WORD_DATA);
	            if (rc < 0) {
	                MT9D115_SUB_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
	                return rc;
	            }
	        } else { 
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
	                                     reg_access_params[cnt].reg_addr, *(reg_access_params[cnt].reg_data), MSM_CAMERA_I2C_WORD_DATA);
	            if (rc < 0) {
	                MT9D115_SUB_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
	                return rc;
	            }
	        }
	    }
	}else{
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                 REG_MT9D115_COAINT_TIME, coarse_integration_time, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0) {
            MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
            return rc;
        }
	}

    MT9D115_SUB_LOG_DBG("set_pattern               = 0x%04X", hdr_brightness.set_pattern      );
    MT9D115_SUB_LOG_DBG("coarse_integration_time   = 0x%04X", *(reg_access_params[1].reg_data));
    MT9D115_SUB_LOG_DBG("analogue_gain_code_greenr = 0x%04X", *(reg_access_params[2].reg_data));
    MT9D115_SUB_LOG_DBG("analogue_gain_code_greenb = 0x%04X", *(reg_access_params[3].reg_data));
    MT9D115_SUB_LOG_DBG("analogue_gain_code_red    = 0x%04X", *(reg_access_params[4].reg_data));
    MT9D115_SUB_LOG_DBG("analogue_gain_code_blue   = 0x%04X", *(reg_access_params[5].reg_data));

	MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}

static int32_t mt9d115_sub_set_cap_mode_enable(struct msm_sensor_ctrl_t *s_ctrl, int cap_mode_enable)
{
    int32_t rc = 0;
	enum mt9d115_Mode_setting setting;

    MT9D115_SUB_LOG_DBG("START");
    MT9D115_SUB_LOG_DBG("cap_mode_enable = %d",cap_mode_enable);

    switch (cap_mode_enable)
    {
        case CAMERA_CAP_MODE_ENABLE_ON:
			MT9D115_SUB_LOG_DBG("CAMERA_CAP_MODE_ENABLE_ON\n");
			setting = MT9D115_MODE_CAP;
            break;

        case CAMERA_CAP_MODE_ENABLE_OFF:
			MT9D115_SUB_LOG_DBG("CAMERA_CAP_MODE_ENABLE_OFF\n");
			setting = MT9D115_MODE_PRE;
            break;

        default:
			MT9D115_SUB_LOG_DBG("NOT SUPPORTED\n");
			setting = mt9d115_sub_cap_mode_enable;
            break;
    }

	if(mt9d115_sub_cap_mode_enable != setting){
		rc = msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client,
										mt9d115_sub_confs, setting);
		if(rc < 0)
			return -EINVAL;

		mt9d115_sub_cap_mode_enable = setting;
	}

    MT9D115_SUB_LOG_DBG("END(%d)",rc);
    return rc;
}


static int32_t mt9d115_sub_get_device_id(struct msm_sensor_ctrl_t *s_ctrl, uint16_t *device_id)
{
    int32_t rc = 0;
    uint16_t chipid = 0xFFFF;

    MT9D115_SUB_LOG_DBG("start\n");

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid, 
                             MSM_CAMERA_I2C_WORD_DATA);
    if (rc < 0) {
        MT9D115_SUB_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
        return rc;
    }

    *device_id = chipid;

    MT9D115_SUB_LOG_DBG("chipid=%d",chipid);
    MT9D115_SUB_LOG_DBG("end rc = %d\n",rc);
    return rc;
}


static int32_t mt9d115_sub_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = -EINVAL;
	struct clk *clk = NULL;
	clk = clk_get(NULL, "cam2_clk");

    MT9D115_SUB_LOG_INF("START\n");

	mt9d115_sub_effect = MT9D115_SE_MAX;
	mt9d115_sub_flicker_mode = MT9D115_FL_02;
	mt9d115_sub_bright_mode = MT9D115_BN_06;
	mt9d115_sub_wb_mode = MT9D115_WB_00;
	mt9d115_sub_frame_rate_mode = MT9D115_FR_00;
	mt9d115_sub_cap_mode_enable = MT9D115_MODE_MAX;
	mt9d115_sub_capture_mode = MT9D115_CM_00;

	
	if(clk != NULL){
		clk_set_rate(clk, 25600000);
		MT9D115_SUB_LOG_DBG("clk_set_rate()\n");
	}else{
		
		MT9D115_SUB_LOG_ERR("cam2_clk is NULL!\n");
		mt9d115_sub_sensor_power_down(s_ctrl);
		return -EFAULT;
	}

	
	MT9D115_SUB_LOG_DBG("GPIO[43] CAM2_RST_N 1\n");
	rc = gpio_request(MT9D115_GPIO_CAM2_RST_N, "mt9d115_sub");
	if(!rc){
		gpio_direction_output(MT9D115_GPIO_CAM2_RST_N, 1);
	}else{
		
		mt9d115_sub_sensor_power_down(s_ctrl);
		MT9D115_SUB_LOG_ERR("sensor_pwd(%d) Error, rc = %d\n", MT9D115_GPIO_CAM2_RST_N, rc);
		return -EFAULT;
	}

	MT9D115_SUB_LOG_DBG("wait 1 \n");
	msleep(1);

	
	MT9D115_SUB_LOG_DBG("GPIO[6] CAM2_V_EN2 1\n");
	rc = gpio_request(MT9D115_GPIO_CAM2_V_EN2, "mt9d115_sub");
	if(!rc){
		gpio_direction_output(MT9D115_GPIO_CAM2_V_EN2, 1);
	}else{
		
		mt9d115_sub_sensor_power_down(s_ctrl);
		MT9D115_SUB_LOG_ERR("sensor_pwd(%d) Error, rc = %d\n", MT9D115_GPIO_CAM2_V_EN2, rc);
		return -EFAULT;
	}

	MT9D115_SUB_LOG_DBG("wait 2 \n");
	msleep(2);

	
	msleep(4);
    rc = clk_enable(clk);
	MT9D115_SUB_LOG_DBG("clk_enable():%d\n",rc);

	MT9D115_SUB_LOG_DBG("wait 1 \n");
	msleep(1);

	
	MT9D115_SUB_LOG_DBG("GPIO[43] CAM2_RST_N 0\n");
	gpio_direction_output(MT9D115_GPIO_CAM2_RST_N, 0);

	MT9D115_SUB_LOG_DBG("wait 1 \n");
	msleep(1);

	
	MT9D115_SUB_LOG_DBG("GPIO[43] CAM2_RST_N 1\n");
	gpio_direction_output(MT9D115_GPIO_CAM2_RST_N, 1);

	MT9D115_SUB_LOG_DBG("wait 1 \n");
	msleep(1);

	
	MT9D115_SUB_LOG_DBG("PM GPIO[42] CAM2_V_EN1 1\n");
	rc = pm8xxx_gpio_config(MT9D115_PMGPIO_CAM2_V_EN1, &mt9d115_cam2_v_en1_on);
	if (rc) {
		mt9d115_sub_sensor_power_down(s_ctrl);
		MT9D115_SUB_LOG_ERR("VDD_CAM2_V_EN1(%d) Error, rc = %d\n", MT9D115_PMGPIO_CAM2_V_EN1, rc);
		return -EFAULT;
	}

	MT9D115_SUB_LOG_DBG("wait 1 \n");
	msleep(1);

    MT9D115_SUB_LOG_INF("END(0)\n");
	return 0;
}

static int mt9d115_sub_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct clk *clk = NULL;

	clk = clk_get(NULL, "cam2_clk");

    MT9D115_SUB_LOG_INF("START\n");

	
	MT9D115_SUB_LOG_DBG("PM GPIO[42] CAM2_V_EN1 0\n");
    pm8xxx_gpio_config(MT9D115_PMGPIO_CAM2_V_EN1, &mt9d115_cam2_v_en1_off);

	MT9D115_SUB_LOG_DBG("wait 1 \n");
	msleep(1);

	
    clk_disable(clk);
	MT9D115_SUB_LOG_DBG("clk_disable\n");

	MT9D115_SUB_LOG_DBG("wait 2 \n");
	msleep(2);

	MT9D115_SUB_LOG_DBG("GPIO[6] CAM2_V_EN2 0\n");
	gpio_direction_output(MT9D115_GPIO_CAM2_V_EN2, 0);

	MT9D115_SUB_LOG_DBG("wait 1 \n");
	msleep(1);

	MT9D115_SUB_LOG_DBG("GPIO[43] CAM2_RST_N 0\n");
	gpio_direction_output(MT9D115_GPIO_CAM2_RST_N, 0);

	gpio_free(MT9D115_GPIO_CAM2_V_EN2);
	gpio_free(MT9D115_GPIO_CAM2_RST_N);

    MT9D115_SUB_LOG_INF("END(0)\n");
	return 0;
}

static int mt9d115_sub_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;

	MT9D115_SUB_LOG_DBG("mt9d115_sub_i2c_probe called\n");
	rc = msm_sensor_i2c_probe(client, id);
	return rc;
}



static int __init msm_sensor_init_module(void)
{
	MT9D115_SUB_LOG_DBG("msm_sensor_init_module called\n");
	return i2c_add_driver(&mt9d115_sub_i2c_driver);
}


static struct msm_sensor_fn_t mt9d115_sub_func_tbl = {
    .sensor_start_stream = mt9d115_sub_sensor_start_stream,
    .sensor_stop_stream = mt9d115_sub_sensor_stop_stream,
    .sensor_setting = mt9d115_sub_sensor_setting,
    .sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
    .sensor_mode_init = msm_sensor_mode_init,
    .sensor_get_output_info = msm_sensor_get_output_info,
    .sensor_config = msm_sensor_config,
    .sensor_power_up = mt9d115_sub_sensor_power_up,
    .sensor_power_down = mt9d115_sub_sensor_power_down,
    .sensor_set_parm_pm_obs = msm_sensor_set_parm_pm_obs,
	.sensor_set_wb = mt9d115_sub_set_wb,
	.sensor_set_effect = mt9d115_sub_set_effect,
	.sensor_set_antibanding = mt9d115_sub_set_antibanding,
	.sensor_set_exp_compensation = mt9d115_sub_set_exp_compensation,
	.sensor_set_scene = mt9d115_sub_set_scene,
	.sensor_set_pict_size = mt9d115_sub_set_pict_size,

	.sensor_get_maker_note = mt9d115_sub_get_maker_note,
	.sensor_get_exif_param = mt9d115_sub_get_exif_param,
	.sensor_set_frame_rate_mode = mt9d115_sub_set_frame_rate_mode,
    .sensor_get_hdr_brightness = mt9d115_sub_get_hdr_brightness,
    .sensor_set_hdr_brightness = mt9d115_sub_set_hdr_brightness,
    .sensor_set_cap_mode_enable = mt9d115_sub_set_cap_mode_enable,

	.sensor_get_device_id = mt9d115_sub_get_device_id,





};

static struct msm_sensor_reg_t mt9d115_sub_regs = {
    .default_data_type = MSM_CAMERA_I2C_WORD_DATA,
    .init_settings = &mt9d115_sub_init_confs[0],
    .init_size = ARRAY_SIZE(mt9d115_sub_init_confs),
    .mode_settings = &mt9d115_sub_confs[0],
    .output_settings = &mt9d115_sub_dimensions[0],
    .num_conf = ARRAY_SIZE(mt9d115_sub_confs),
};

static struct msm_sensor_ctrl_t mt9d115_sub_s_ctrl = {
    .msm_sensor_reg = &mt9d115_sub_regs,
    .sensor_i2c_client = &mt9d115_sub_sensor_i2c_client,
    .sensor_i2c_addr = 0x78, 
    .sensor_output_reg_addr = &mt9d115_sub_reg_addr,
    .sensor_id_info = &mt9d115_sub_id_info,
    .cam_mode = MSM_SENSOR_MODE_INVALID,
    .csi_params = &mt9d115_sub_csi_params_array[0],
    .msm_sensor_mutex = &mt9d115_sub_mut,
    .sensor_i2c_driver = &mt9d115_sub_i2c_driver,
    .sensor_v4l2_subdev_info = mt9d115_sub_subdev_info,
    .sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9d115_sub_subdev_info),
    .sensor_v4l2_subdev_ops = &mt9d115_sub_subdev_ops,
    .func_tbl = &mt9d115_sub_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 2MP YUV sensor driver");
MODULE_LICENSE("GPL v2");
