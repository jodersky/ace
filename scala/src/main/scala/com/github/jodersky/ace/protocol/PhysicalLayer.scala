package com.github.jodersky.ace.protocol

import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import jssc.SerialPort
import java.io.IOException
import jssc.SerialPortEvent
import jssc.SerialPortEventListener

class PhysicalLayer(serial: SerialPort) extends ReactiveLayer[Nothing, Array[Byte]] {

  def receive(nothing: Nothing) = throw new UnsupportedOperationException("A receive function cannot be called on the lowest layer.")

  private val listener = new SerialPortEventListener {
    override def serialEvent(event: SerialPortEvent) = {
      if (event.isRXCHAR()) {
        val bytes = serial.readBytes
        if (bytes != null) notifyHigher(bytes)
      }
    }
  }
  

  def write(data: Array[Byte]) = future {
    serial.writeBytes(data)
  } map { success =>
    if (success) data
    else throw new IOException("Could not write to serial port.")
  }
  
  def begin() = {
    val mask = SerialPort.MASK_RXCHAR + SerialPort.MASK_CTS + SerialPort.MASK_DSR
    serial.setEventsMask(mask)
    serial.addEventListener(listener)
  }

}