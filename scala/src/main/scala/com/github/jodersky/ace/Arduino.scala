package com.github.jodersky.ace

import scala.util.Try
import jssc.SerialPort
import jssc.SerialPortEventListener
import jssc.SerialPortEvent
import scala.util.Success
import com.github.jodersky.ace.protocol.SecureSerial
import scala.concurrent.Await
import scala.concurrent.duration._

object Arduino {

  private def open(port: String, rate: Int) =  {
    val serialPort = new SerialPort(port);
    serialPort.openPort();
    serialPort.setParams(rate,
      SerialPort.DATABITS_8,
      SerialPort.STOPBITS_1,
      SerialPort.PARITY_NONE); //Set params. Also you can set params by this string: serialPort.setParams(9600, 8, 1, 0);
    //  serialPort.writeBytes("This is a test string".getBytes()); //Write data to port
    //serialPort.closePort(); //Close serial port
    serialPort
  }
  
  def connect(port: String) = new SecureSerial(open(port, 115200))
  
}