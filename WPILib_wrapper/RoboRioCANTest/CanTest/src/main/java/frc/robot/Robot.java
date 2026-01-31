// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

package frc.robot;

import java.nio.ByteBuffer;

import edu.wpi.first.hal.CANData;
import edu.wpi.first.wpilibj.CAN;
import edu.wpi.first.wpilibj.PowerDistribution;
import edu.wpi.first.wpilibj.TimedRobot;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * This is a sample program showing how to retrieve information from the Power Distribution Panel
 * via CAN. The information will be displayed under variables through the SmartDashboard.
 */
public class Robot extends TimedRobot {
  private final PowerDistribution m_pdp = new PowerDistribution();

  int device_id = 5; // Unique value for each device of the same type
  /*int manu = 8; // Team use
  int device_type = 10; // misc
  private final CAN m_cangio = new CAN(device_id, manu, device_type);
  */
  //This uses the "team" manufacturer and device types by default. The device ID is 6 bits (0-63).
  private final CAN m_canduit = new CAN(device_id);
  private static int count = 0;
  private final CANData m_data = new CANData();

  public Robot() {
    // Put the PDP itself to the dashboard
    SmartDashboard.putData("PDP", m_pdp);

    System.out.println("Hello World!");
  }

  // Sends an RTR request and returns the data
  public byte[] requestData() {
    int apiClass = 1; // This is device dependant - we will need to define our own set of message classes and IDs
    int apiIndex = 2;
    int apiId = apiClass << 6 | apiIndex;

    // Write an RTR packet requesting 1 byte of data
    m_canduit.writeRTRFrame(1, apiId);

    // Read the latest packet for the API ID
    boolean success = m_canduit.readPacketLatest(apiId, m_data);
    
    if (success) {
        return m_data.data;
    }
    return null; // Return null if read failed
  }


  @Override
  public void robotPeriodic() {
    // This function gets called approx 50 times per second
    count++;
    if ((count % 50) == 0)
    {
      System.out.println("Hello World!!!!");
    }

    /* 
    byte data[] = {0}; // max 8 bytes
    int apiClass = 0; // This is device dependant - we will need to define our own set of message classes and IDs
    int apiIndex = 0;
    int apiId = apiClass << 6 | apiIndex;
    m_cangio.writePacket(data, apiId);
    //m_cangio.writePacketRepeatingNoThrow(data, apiId, 100);

    */

    requestData();

    
    /* 
    // Get the current going through channel 7, in Amperes.
    // The PDP returns the current in increments of 0.125A.
    // At low currents the current readings tend to be less accurate.
    double current7 = m_pdp.getCurrent(7);
    SmartDashboard.putNumber("Current Channel 7", current7);

    // Get the voltage going into the PDP, in Volts.
    // The PDP returns the voltage in increments of 0.05 Volts.
    double voltage = m_pdp.getVoltage();
    SmartDashboard.putNumber("Voltage", voltage);

    // Retrieves the temperature of the PDP, in degrees Celsius.
    double temperatureCelsius = m_pdp.getTemperature();
    SmartDashboard.putNumber("Temperature", temperatureCelsius);

    // Get the total current of all channels.
    double totalCurrent = m_pdp.getTotalCurrent();
    SmartDashboard.putNumber("Total Current", totalCurrent);

    // Get the total power of all channels.
    // Power is the bus voltage multiplied by the current with the units Watts.
    double totalPower = m_pdp.getTotalPower();
    SmartDashboard.putNumber("Total Power", totalPower);

    // Get the total energy of all channels.
    // Energy is the power summed over time with units Joules.
    double totalEnergy = m_pdp.getTotalEnergy();
    SmartDashboard.putNumber("Total Energy", totalEnergy);
    */
  }
}
