
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#local pilot source file
LOCAL_SRC_FILES := \
	camera.cpp \
	../../../modules/main/pilot.cpp \
	../../../BSP/boards/rk3288UAV/init.cpp \
	../../../BSP/boards/rk3288UAV/ARCOUT.cpp \
	../../../HAL/rk32885.1/ASPI.cpp \
	../../../HAL/rk32885.1/AGpio.cpp \
	../../../HAL/rk32885.1/ASysTimer.cpp \
	../../../HAL/rk32885.1/AUART.cpp \
	../../../HAL/rk32885.1/AStorage.cpp \
	../../../HAL/rk32885.1/ATimer.cpp \
	../../../HAL/rk32885.1/ACriticalSection.cpp \
	../../../HAL/rk32885.1/AIMUFIFO.cpp \
	../../../HAL/rk32885.1/AI2C.cpp \
	../../../modules/utils/param.cpp \
	../../../modules/utils/space.cpp \
	../../../modules/utils/gauss_newton.cpp \
	../../../modules/utils/vector.cpp \
	../../../modules/utils/console.cpp \
	../../../modules/utils/ymodem.cpp \
	../../../modules/utils/SEGGER_RTT.c \
	../../../modules/utils/log_android.cpp \
	../../../modules/Algorithm/pos_controll.cpp \
	../../../modules/Algorithm/pos_controll_old.cpp \
	../../../modules/Algorithm/pos_estimator.cpp \
	../../../modules/Algorithm/pos_estimator2.cpp \
	../../../modules/Algorithm/attitude_controller.cpp \
	../../../modules/Algorithm/ahrs.cpp \
	../../../modules/Algorithm/motion_detector.cpp \
	../../../modules/Algorithm/ekf_estimator.cpp \
	../../../modules/Algorithm/altitude_controller.cpp \
	../../../modules/Algorithm/altitude_estimator.cpp \
	../../../modules/Algorithm/altitude_estimator2.cpp \
	../../../modules/Algorithm/of_controller2.cpp \
	../../../modules/Algorithm/battery_estimator.cpp \
	../../../modules/Algorithm/mag_calibration.cpp \
	../../../modules/Algorithm/ekf_lib/src/init_ekf_matrix.c \
	../../../modules/Algorithm/ekf_lib/src/INS_SetState.c \
	../../../modules/Algorithm/ekf_lib/src/LinearFG.c \
	../../../modules/Algorithm/ekf_lib/src/RungeKutta.c \
	../../../modules/Algorithm/ekf_lib/src/INS_CovariancePrediction.c \
	../../../modules/Algorithm/ekf_lib/src/INS_Correction.c \
	../../../modules/Algorithm/ekf_lib/src/quaternion_to_euler.c \
	../../../modules/Algorithm/ekf_lib/src/ned2body.c \
	../../../modules/Algorithm/ekf_lib/src/body2ned.c \
	../../../modules/Algorithm/ekf_lib/src/f.c \
	../../../modules/Algorithm/ekf_lib/src/normlise_quaternion.c \
	../../../modules/Algorithm/ekf_lib/src/LinearizeH.c \
	../../../modules/Algorithm/ekf_lib/src/inv.c \
	../../../modules/Algorithm/ekf_lib/src/h.c \
	../../../modules/Algorithm/ekf_lib/src/rt_nonfinite.c \
	../../../modules/main/mode_basic.cpp \
	../../../modules/main/mode_althold.cpp \
	../../../modules/main/mode_of_loiter.cpp \
	../../../modules/main/mode_poshold.cpp \
	../../../modules/main/mode_RTL.cpp \
	../../../modules/math/matrix.cpp \
	../../../modules/math/LowPassFilter2p.cpp \
	../../../HAL/Resources.cpp \
	../../../HAL/sensors/MPU6000.cpp \
	../../../HAL/sensors/MS5611_SPI.cpp \
	../../../HAL/sensors/HMC5983SPI.cpp \
	../../../HAL/sensors/UartUbloxNMEAGPS.cpp \
	../../../HAL/sensors/EBusIn.cpp  \

	
LOCAL_C_INCLUDES :=  \
	system/acantha \
	system/acantha/modules \
	system/acantha/modules/Algorithm/ekf_lib/inc \


LOCAL_SHARED_LIBRARIES := \
	libutils \
	libbinder \
	libui \
	libgui \
	libcamera_metadata \
	libcameraservice \
	libcamera_client \


LOCAL_CFLAGS := -Wno-format -Wno-unused -Wno-unused-parameter -Wfatal-errors -Wno-non-virtual-dtor
LOCAL_LDLIBS := -ldl -lm
#LOCAL_RTTI_FLAG := -frtti
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= acantha
LOCAL_STATIC_LIBRARIES := \
		libc++


include $(BUILD_EXECUTABLE)
