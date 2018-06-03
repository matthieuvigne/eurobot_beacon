/// \file BeaconDriver.h
/// \brief Driver for the IR receiver.
///
/// \details This file implements all the functions to work with the IR receiver to detect other robots.
///	\note	 All functions in this header should be prefixed with IR_.
/// \author Matthieu Vigne <matthieu.vigne@laposte.net>

#ifndef IR_DRIVER
	#define IR_DRIVER

	/// IR receiver structure
	typedef struct{
		int port; ///< I2C port file descriptor.
		int address; ///< Beacon I2C address.
		int frequency; ///< IR sensor frequency, this is initialized at IR_init and remains constant between calls to IR_getFrequency.
	}Beacon;


	/// \brief Initialize Beacon structure.
    ///
    /// \details This function tests the communication with the receiver, and, if successful, inits the structure.
    ///
    /// \param[out] ir The Beacon structure, to be used whenever communication with the IMU.
    /// \param[in] port File descriptor of the i2c port (as returned by the i2c_open function, see I2C-Wrapper.h).
    /// \param[in] address I2C address of the receiver.
    /// \returns   1 on success, 0 (false) otherwise.
	int IR_init(Beacon *ir, int port, int address);

	/// \brief Returns the raw sensor values from the IR receiver.
	///
    /// \param[in] ir The Beacon structure to use.
    /// \returns   In binary, s8s7s6s5s4s3s2s1s0, where si is the value of sensor i.
	int IR_getRawValue(Beacon ir);

	/// \brief Get the position and size of a detected robot.
	///
    /// \param[in] ir The Beacon structure to use.
    /// \param[in] obstacle Either 1 or 2, specifies which obstacle we want to see (the largest or second largest).
    /// \param[out] size The size of the obstacle (in number of sensors).
    /// \param[out] position The position of the obstacle, in deg.
    /// \return 1 if an obstacle is seen (size > 0), 0 (false) otherwise.
	int IR_getObstacle(Beacon ir, int obstacle, int *size, int *position);

	/// \brief Get the frequency of the sensors currently read.
	///
    /// \param[in] ir The Beacon structure to use.
	/// \return IR_FREQUENCY_38k if 38kHz sensor, IR_FREQUENCY_57k if 57kHz sensor
	unsigned char IR_getFrequency(Beacon *ir);


	#define IR_RECEIVER_ADDRESS	0x42	///< Default address of the receiver.

	#define IR_WHO_AM_I 0x00
	#define IR_FREQUENCY 0x01
	#define IR_RAW_DATA1 0x02
	#define IR_RAW_DATA2 0x03
	#define IR_ROBOT1_POS 0x04
	#define IR_ROBOT2_POS 0x05


	#define IR_WHO_AM_I_VALUE 0x0f
	#define IR_FREQUENCY_38k 0
	#define IR_FREQUENCY_57k 1
#endif
