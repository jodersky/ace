package com.github.jodersky.ace.jssc

import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import com.github.jodersky.ace.serial._
import com.github.jodersky.ace.serial.Serial
import jssc.SerialPortEvent
import jssc.SerialPort
import jssc.SerialPortEventListener
import java.io.IOException

object JSSCSerialProvider extends SerialProvider {

  def open(port: String, baudRate: Int) = new Serial {
    val serialPort = new SerialPort(port);
    serialPort.openPort()
    serialPort.setParams(
      baudRate,
      SerialPort.DATABITS_8,
      SerialPort.STOPBITS_1,
      SerialPort.PARITY_NONE)

    val listener = new SerialPortEventListener {
      override def serialEvent(event: SerialPortEvent) = {
        if (event.isRXCHAR()) {
          val bytes = serialPort.readBytes
          if (bytes != null) notifyHigher(bytes.map(_ & 0xff))
        }
      }
    }

    def send(data: Seq[Int]) = future {
      serialPort.writeBytes(data.toArray.map(_.toByte))
    } map { success =>
      if (success) data
      else throw new IOException("Could not write to serial port.")
    }

    def close() = serialPort.closePort()

    def begin() = {
      val mask = SerialPort.MASK_RXCHAR + SerialPort.MASK_CTS + SerialPort.MASK_DSR
      serialPort.setEventsMask(mask)
      serialPort.addEventListener(listener)
    }
  }

}