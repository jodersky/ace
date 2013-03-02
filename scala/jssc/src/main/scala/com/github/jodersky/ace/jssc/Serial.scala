package com.github.jodersky.ace.jssc

import com.github.jodersky.ace.PhysicalLayer
import jssc.SerialPortEvent
import jssc.SerialPort
import jssc.SerialPortEventListener
import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import java.io.IOException

class Serial(
    portName: String,
    baudRate: Int = 9600,
    dataBits: Int = SerialPort.DATABITS_8, 
    stopBits: Int = SerialPort.STOPBITS_1,
    parity: Int = SerialPort.PARITY_NONE) extends PhysicalLayer {
  
  private val port = new SerialPort(portName)

  private val listener = new SerialPortEventListener {
    override def serialEvent(event: SerialPortEvent) = {
      if (event.isRXCHAR()) {
        val bytes = port.readBytes
        if (bytes != null) notifyHigher(bytes.map(_ & 0xff))
      }
    }
  }

  def send(data: Seq[Int]) = future {
    port.writeBytes(data.toArray.map(_.toByte))
  } map { success =>
    if (success) data
    else throw new IOException("Could not write to serial port.")
  }
  
  def begin() {
    val mask = SerialPort.MASK_RXCHAR + SerialPort.MASK_CTS + SerialPort.MASK_DSR
    
    port.openPort()
    port.setParams(baudRate, dataBits, stopBits, parity)
    port.setEventsMask(mask)
    port.addEventListener(listener)
  }
  
  def close() = port.closePort()
  
}