package com.github.jodersky.ace.rxtx

import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import java.io.IOException
import gnu.io.CommPort
import gnu.io.CommPortIdentifier
import gnu.io.SerialPort
import gnu.io.SerialPortEvent
import gnu.io.SerialPortEventListener
import scala.collection.mutable.ArrayBuffer
import com.github.jodersky.ace.PhysicalLayer

class Serial(
    portName: String,
    baudRate: Int = 9600,
    dataBits: Int = SerialPort.DATABITS_8, 
    stopBits: Int = SerialPort.STOPBITS_1,
    parity: Int = SerialPort.PARITY_NONE) extends PhysicalLayer {
  
  lazy val portIdentifier = {
    System.setProperty("gnu.io.rxtx.SerialPorts", portName);
    CommPortIdentifier.getPortIdentifier(portName)
  }

  private lazy val port: SerialPort = {
    if (portIdentifier.isCurrentlyOwned()) throw new IOException("Port " + portName + " is currently in use")
    val commPort = portIdentifier.open(this.getClass().getName(), 2000);
    val serialPort = commPort.asInstanceOf[SerialPort]
    serialPort.setSerialPortParams(baudRate, dataBits, stopBits, parity)
    serialPort
  }
  
  private lazy val listener = new SerialPortEventListener {
    override def serialEvent(event: SerialPortEvent) = {
      val in = port.getInputStream()
      while (in.available() > 0) {
        notifyHigher(Seq(in.read() & 0xff))
      }
    }
  }
  
  def send(data: Seq[Int]) = future {
    port.getOutputStream().write(data.map(_.toByte).toArray)
  } map { _ => data }

  def begin() = {
    port.addEventListener(listener)
    port.notifyOnDataAvailable(true)
  }
  
  def close() = {
    port.close()
  }
  
}